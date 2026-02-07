#include "LibraryPage.h"
#include "ui_LibraryPage.h"

#include "ui/widgets/CardPopupDialog.h"
#include "ui/widgets/CardWidget.h"
#include "ui/widgets/CardTableModel.h"
#include "ui/widgets/FolderListModel.h"
#include "ui/widgets/NotificationCenter.h"
#include "ui/widgets/ToastWidget.h"

#include <QHeaderView>
#include <QList>
#include <QMenu>
#include <QModelIndex>
#include <QPoint>
#include <QTableView>

namespace rewise::ui::pages {

LibraryPage::LibraryPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LibraryPage)
{
    ui->setupUi(this);

    // Splitter defaults
    ui->splitMain->setStretchFactor(0, 0);
    ui->splitMain->setStretchFactor(1, 1);
    ui->splitMain->setSizes(QList<int>{280, 700});

    // Panels as "cards" (styled via QSS dynamic property selectors)
    ui->foldersPanel->setProperty("card", true);
    ui->cardsPanel->setProperty("card", true);

    // Models
    m_foldersModel = new rewise::ui::widgets::FolderListModel(this);
    m_foldersModel->setIncludeAllItem(true);
    ui->lvFolders->setModel(m_foldersModel);

    m_cardsModel = new rewise::ui::widgets::CardTableModel(this);
    ui->tvCards->setModel(m_cardsModel);

    // Cards table presentation
    ui->tvCards->horizontalHeader()->setStretchLastSection(true);
    ui->tvCards->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tvCards->verticalHeader()->setVisible(false);
    ui->tvCards->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tvCards->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tvCards->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Hide noisy columns in library list
    ui->tvCards->setColumnHidden(rewise::ui::widgets::CardTableModel::AnswerCol, true);
    ui->tvCards->setColumnHidden(rewise::ui::widgets::CardTableModel::FolderCol, true);
    ui->tvCards->setColumnHidden(rewise::ui::widgets::CardTableModel::UpdatedCol, true);

    // Folder editor
    ui->folderEditor->setVisible(false);

    // Wiring: search
    connect(ui->leSearch, &QLineEdit::textChanged, this, [this](const QString& t) {
        m_cardsModel->setSearchQuery(t);
    });

    // Wiring: folder selection -> filter cards
    connect(ui->lvFolders->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex& current, const QModelIndex&) {
        const int row = current.row();
        const auto id = m_foldersModel->idAtRow(row);
        m_cardsModel->setFolderFilter(id);
    });

    // Default selection (All)
    if (m_foldersModel->rowCount() > 0) {
        ui->lvFolders->setCurrentIndex(m_foldersModel->index(0, 0));
    }

    // Folder actions
    connect(ui->btnNewFolder, &QToolButton::clicked, this, &LibraryPage::openFolderEditorCreate);

    connect(ui->btnFolderMore, &QToolButton::clicked, this, [this] {
        const QModelIndex idx = ui->lvFolders->currentIndex();
        if (!idx.isValid()) return;

        const int row = idx.row();
        if (m_foldersModel->isAllRow(row)) return;

        const auto folderId = m_foldersModel->idAtRow(row);
        const auto* f = m_db.folderById(folderId);
        if (!f) return;

        QMenu menu(this);
        QAction* actRename = menu.addAction(QStringLiteral("Переименовать"));
        QAction* actDelete = menu.addAction(QStringLiteral("Удалить"));

        const QPoint anchor = ui->btnFolderMore->mapToGlobal(QPoint(ui->btnFolderMore->width() / 2,
                                                                   ui->btnFolderMore->height()));
        QAction* chosen = menu.exec(anchor);
        if (!chosen) return;

        if (chosen == actRename) {
            openFolderEditorRename(*f);
        } else if (chosen == actDelete) {
            if (!m_notify) {
                emit folderDeleteRequested(folderId);
                return;
            }

            auto* toast = m_notify->showConfirm(
                QStringLiteral("Удалить папку «%1»? Карточки из неё будут перенесены в Default.").arg(f->name),
                QStringLiteral("Удалить"),
                QStringLiteral("Отмена"));

            connect(toast, &rewise::ui::widgets::ToastWidget::accepted, this, [this, folderId] {
                emit folderDeleteRequested(folderId);
            });
        }
    });

    connect(ui->btnFolderCancel, &QPushButton::clicked, this, &LibraryPage::closeFolderEditor);
    connect(ui->btnFolderCommit, &QPushButton::clicked, this, &LibraryPage::commitFolderEditor);
    connect(ui->leFolderName, &QLineEdit::returnPressed, this, &LibraryPage::commitFolderEditor);

    // Folder context menu (rename/delete)
    connect(ui->lvFolders, &QListView::customContextMenuRequested, this, [this](const QPoint& pos) {
        const QModelIndex idx = ui->lvFolders->indexAt(pos);
        if (!idx.isValid()) return;

        const int row = idx.row();
        if (m_foldersModel->isAllRow(row)) return;

        const auto folderId = m_foldersModel->idAtRow(row);
        const auto* f = m_db.folderById(folderId);
        if (!f) return;

        QMenu menu(this);
        QAction* actRename = menu.addAction(QStringLiteral("Переименовать"));
        QAction* actDelete = menu.addAction(QStringLiteral("Удалить"));

        QAction* chosen = menu.exec(ui->lvFolders->viewport()->mapToGlobal(pos));
        if (!chosen) return;

        if (chosen == actRename) {
            openFolderEditorRename(*f);
        } else if (chosen == actDelete) {
            if (!m_notify) {
                emit folderDeleteRequested(folderId);
                return;
            }

            auto* toast = m_notify->showConfirm(
                QStringLiteral("Удалить папку «%1»? Карточки из неё будут перенесены в Default.").arg(f->name),
                QStringLiteral("Удалить"),
                QStringLiteral("Отмена"));

            connect(toast, &rewise::ui::widgets::ToastWidget::accepted, this, [this, folderId] {
                emit folderDeleteRequested(folderId);
            });
        }
    });

    // Card actions
    connect(ui->btnNewCard, &QToolButton::clicked, this, &LibraryPage::openNewCardPopup);

    connect(ui->btnReview, &QPushButton::clicked, this, [this] {
        emit startReviewRequested(currentFolderId());
    });

    // Card click -> popup
    connect(ui->tvCards, &QTableView::clicked, this, [this](const QModelIndex& idx) {
        if (!idx.isValid()) return;

        const int row = idx.row();
        const auto* c = m_cardsModel->cardAtRow(row);
        if (!c) return;

        const QRect vr = ui->tvCards->visualRect(idx);
        const QPoint globalAnchor = ui->tvCards->viewport()->mapToGlobal(vr.center());

        openCardPopup(*c, globalAnchor);
    });

    // Card context menu: delete (quick)
    connect(ui->tvCards, &QTableView::customContextMenuRequested, this, [this](const QPoint& pos) {
        const QModelIndex idx = ui->tvCards->indexAt(pos);
        if (!idx.isValid()) return;

        const int row = idx.row();
        const auto* c = m_cardsModel->cardAtRow(row);
        if (!c) return;

        QMenu menu(this);
        QAction* actOpen = menu.addAction(QStringLiteral("Открыть"));
        QAction* actDelete = menu.addAction(QStringLiteral("Удалить"));

        QAction* chosen = menu.exec(ui->tvCards->viewport()->mapToGlobal(pos));
        if (!chosen) return;

        if (chosen == actOpen) {
            const QRect vr = ui->tvCards->visualRect(idx);
            const QPoint globalAnchor = ui->tvCards->viewport()->mapToGlobal(vr.center());
            openCardPopup(*c, globalAnchor);
            return;
        }

        if (chosen == actDelete) {
            if (!m_notify) {
                emit cardDeleteRequested(c->id);
                return;
            }

            auto* toast = m_notify->showConfirm(
                QStringLiteral("Удалить карточку?"),
                QStringLiteral("Удалить"),
                QStringLiteral("Отмена"));

            connect(toast, &rewise::ui::widgets::ToastWidget::accepted, this, [this, id = c->id] {
                emit cardDeleteRequested(id);
            });
        }
    });
}

LibraryPage::~LibraryPage() {
    delete ui;
}

void LibraryPage::setNotifier(rewise::ui::widgets::NotificationCenter* n) {
    m_notify = n;
}

void LibraryPage::setDatabase(const rewise::storage::Database& db) {
    const auto prevFolder = currentFolderId();

    m_db = db;

    m_foldersModel->setFolders(m_db.folders);

    m_cardsModel->setDatabase(m_db);
    m_cardsModel->setSearchQuery(ui->leSearch->text());
    m_cardsModel->setFolderFilter(prevFolder);

    // Restore folder selection if possible
    int row = m_foldersModel->rowForId(prevFolder);
    if (row < 0) row = 0;
    ui->lvFolders->setCurrentIndex(m_foldersModel->index(row, 0));
}

rewise::domain::Id LibraryPage::currentFolderId() const {
    const QModelIndex idx = ui->lvFolders->currentIndex();
    if (!idx.isValid()) return {};
    return m_foldersModel->idAtRow(idx.row()); // invalid => "All cards"
}

void LibraryPage::openFolderEditorCreate() {
    m_folderEditMode = FolderEditMode::Create;
    m_editFolderId = {};

    ui->folderEditor->setVisible(true);
    ui->leFolderName->clear();
    ui->leFolderName->setFocus();
}

void LibraryPage::openFolderEditorRename(const rewise::domain::Folder& f) {
    m_folderEditMode = FolderEditMode::Rename;
    m_editFolderId = f.id;

    ui->folderEditor->setVisible(true);
    ui->leFolderName->setText(f.name);
    ui->leFolderName->setFocus();
    ui->leFolderName->selectAll();
}

void LibraryPage::closeFolderEditor() {
    m_folderEditMode = FolderEditMode::None;
    m_editFolderId = {};

    ui->folderEditor->setVisible(false);
    ui->leFolderName->clear();
}

void LibraryPage::commitFolderEditor() {
    const QString name = ui->leFolderName->text().trimmed();
    if (name.isEmpty()) {
        showError(QStringLiteral("Название папки не может быть пустым."));
        return;
    }

    if (m_folderEditMode == FolderEditMode::Create) {
        emit folderCreateRequested(name);
        closeFolderEditor();
        return;
    }

    if (m_folderEditMode == FolderEditMode::Rename) {
        if (!m_editFolderId.isValid()) {
            closeFolderEditor();
            return;
        }
        emit folderRenameRequested(m_editFolderId, name);
        closeFolderEditor();
        return;
    }
}

void LibraryPage::openNewCardPopup() {
    const auto folderId = currentFolderId();

    // Anchor near + button
    const QPoint globalAnchor = ui->btnNewCard->mapToGlobal(QPoint(ui->btnNewCard->width() / 2,
                                                                 ui->btnNewCard->height() / 2));

    closeCardPopup();

    m_cardPopup = new rewise::ui::widgets::CardPopupDialog(window());
    m_cardPopup->card()->startCreate(folderId);

    // Save / delete routing
    connect(m_cardPopup->card(), &rewise::ui::widgets::CardWidget::saveRequested,
            this, [this](const rewise::domain::Id& cardId,
                         const rewise::domain::Id& folderCtx,
                         const QString& q,
                         const QString& a) {
                if (q.trimmed().isEmpty() || a.trimmed().isEmpty()) {
                    showError(QStringLiteral("Вопрос и ответ не должны быть пустыми."));
                    return;
                }

                if (!cardId.isValid()) {
                    emit cardCreateRequested(folderCtx, q, a);
                } else {
                    emit cardUpdateRequested(cardId, q, a);
                }

                closeCardPopup();
            });

    connect(m_cardPopup->card(), &rewise::ui::widgets::CardWidget::closeRequested,
            this, [this] { closeCardPopup(); });

    m_cardPopup->popupNear(globalAnchor);
}

void LibraryPage::openCardPopup(const rewise::domain::Card& card, const QPoint& globalAnchor) {
    closeCardPopup();

    m_cardPopup = new rewise::ui::widgets::CardPopupDialog(window());
    m_cardPopup->card()->setCard(card);

    connect(m_cardPopup->card(), &rewise::ui::widgets::CardWidget::saveRequested,
            this, [this](const rewise::domain::Id& cardId,
                         const rewise::domain::Id& folderCtx,
                         const QString& q,
                         const QString& a) {
                Q_UNUSED(folderCtx);

                if (!cardId.isValid()) return;
                if (q.trimmed().isEmpty() || a.trimmed().isEmpty()) {
                    showError(QStringLiteral("Вопрос и ответ не должны быть пустыми."));
                    return;
                }

                emit cardUpdateRequested(cardId, q, a);
                closeCardPopup();
            });

    connect(m_cardPopup->card(), &rewise::ui::widgets::CardWidget::deleteRequested,
            this, [this](const rewise::domain::Id& id) {
                if (!m_notify) {
                    emit cardDeleteRequested(id);
                    closeCardPopup();
                    return;
                }

                auto* toast = m_notify->showConfirm(
                    QStringLiteral("Удалить карточку?"),
                    QStringLiteral("Удалить"),
                    QStringLiteral("Отмена"));

                connect(toast, &rewise::ui::widgets::ToastWidget::accepted, this, [this, id] {
                    emit cardDeleteRequested(id);
                    closeCardPopup();
                });
            });

    connect(m_cardPopup->card(), &rewise::ui::widgets::CardWidget::closeRequested,
            this, [this] { closeCardPopup(); });

    m_cardPopup->popupNear(globalAnchor);
}

void LibraryPage::closeCardPopup() {
    if (!m_cardPopup) return;

    m_cardPopup->close();
    m_cardPopup->deleteLater();
    m_cardPopup = nullptr;
}

void LibraryPage::showError(const QString& text) {
    if (m_notify) {
        m_notify->showError(text);
        return;
    }
}

void LibraryPage::showInfo(const QString& text) {
    if (m_notify) {
        m_notify->showInfo(text);
        return;
    }
}

} // namespace rewise::ui::pages
