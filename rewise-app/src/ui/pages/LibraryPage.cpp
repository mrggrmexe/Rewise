#include "LibraryPage.h"
#include "ui_LibraryPage.h"

#include "ui/widgets/FolderListModel.h"
#include "ui/widgets/CardTableModel.h"
#include "ui/widgets/CardPopupDialog.h"
#include "ui/widgets/CardWidget.h"
#include "ui/widgets/FolderPopupDialog.h"
#include "ui/widgets/FolderCardWidget.h"
#include "ui/widgets/ActionMenuPopup.h"
#include "ui/widgets/ActionMenuWidget.h"
#include "ui/widgets/NotificationCenter.h"
#include "ui/widgets/ToastWidget.h"

#include <QApplication>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QMetaObject>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QStyledItemDelegate>

namespace {

static QColor bgHover()    { return QColor(214, 229, 255, 150); }
static QColor bgSelected() { return QColor(206, 200, 255, 190); }

static QColor mutedText(const QColor& base) {
    QColor c = base;
    c.setAlphaF(0.62);
    return c;
}

static QString oneLine(QString s) {
    s.replace('\n', ' ');
    return s.simplified();
}

static void paintRounded(QPainter* p, const QRect& rect, const QColor& fill, int radius) {
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);
    p->fillPath(path, fill);
    p->restore();
}

static void neutralizeNativeSelection(QAbstractItemView* v) {
    QPalette pal = v->palette();
    pal.setColor(QPalette::Highlight, QColor(0, 0, 0, 0));
    pal.setColor(QPalette::HighlightedText, pal.color(QPalette::Text));
    pal.setColor(QPalette::Base, QColor(0, 0, 0, 0));
    pal.setColor(QPalette::AlternateBase, QColor(0, 0, 0, 0));
    v->setPalette(pal);

    if (v->viewport()) v->viewport()->setAutoFillBackground(false);
}

class FolderRowDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);

        opt.state &= ~QStyle::State_HasFocus;

        const bool selected = opt.state & QStyle::State_Selected;
        const bool hover    = opt.state & QStyle::State_MouseOver;
        const bool pressed  = opt.state & QStyle::State_Sunken;

        QRect bg = opt.rect.adjusted(6, 2, -6, -2);
        if (selected || pressed) paintRounded(p, bg, bgSelected(), 12);
        else if (hover)          paintRounded(p, bg, bgHover(), 12);

        const QString name = opt.text;
        const QColor base = opt.palette.color(QPalette::Text);

        p->save();
        p->setPen(base);
        QFont f = opt.font;
        f.setPointSizeF(std::max(11.0, opt.font.pointSizeF()));
        p->setFont(f);

        QRect tr = opt.rect.adjusted(16, 0, -12, 0);
        p->drawText(tr, Qt::AlignVCenter | Qt::AlignLeft, name);
        p->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const override {
        return QSize(0, 34);
    }
};

class CardRowDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);

        opt.state &= ~QStyle::State_HasFocus;

        const bool selected = opt.state & QStyle::State_Selected;
        const bool hover    = opt.state & QStyle::State_MouseOver;
        const bool pressed  = opt.state & QStyle::State_Sunken;

        QRect bg = opt.rect.adjusted(10, 4, -10, -4);
        if (selected || pressed) paintRounded(p, bg, bgSelected(), 14);
        else if (hover)          paintRounded(p, bg, bgHover(), 14);

        const QModelIndex aIdx = index.sibling(index.row(), rewise::ui::widgets::CardTableModel::AnswerCol);

        QString q = oneLine(index.data(Qt::DisplayRole).toString());
        QString a = oneLine(aIdx.data(Qt::DisplayRole).toString());

        const QColor base = opt.palette.color(QPalette::Text);
        const QColor muted = mutedText(base);

        QRect r = opt.rect.adjusted(22, 10, -18, -10);

        QFont qFont = opt.font;
        qFont.setBold(true);
        QFontMetrics qFm(qFont);

        QFont aFont = opt.font;
        aFont.setBold(false);
        aFont.setPointSizeF(std::max(11.0, opt.font.pointSizeF() - 0.5));
        QFontMetrics aFm(aFont);

        const int gap = 4;
        QRect qRect(r.left(), r.top(), r.width(), qFm.height());
        QRect aRect(r.left(), r.top() + qFm.height() + gap, r.width(), aFm.height());

        p->save();
        p->setPen(base);
        p->setFont(qFont);
        p->drawText(qRect, Qt::AlignLeft | Qt::AlignVCenter, qFm.elidedText(q, Qt::ElideRight, qRect.width()));

        if (!a.isEmpty()) {
            p->setPen(muted);
            p->setFont(aFont);
            p->drawText(aRect, Qt::AlignLeft | Qt::AlignVCenter, aFm.elidedText(a, Qt::ElideRight, aRect.width()));
        }
        p->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const override {
        return QSize(0, 56);
    }
};

} // namespace

namespace rewise::ui::pages {

LibraryPage::LibraryPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LibraryPage)
{
    ui->setupUi(this);

    auto prepTool = [](QToolButton* b) {
        if (!b) return;
        b->setAutoRaise(false);
        b->setFocusPolicy(Qt::NoFocus);
        b->setProperty("toolIcon", true);
        b->setFixedSize(44, 36);
    };
    prepTool(ui->btnAddFolder);
    prepTool(ui->btnFolderMenu);
    prepTool(ui->btnAddCard);

    m_folderModel = new rewise::ui::widgets::FolderListModel(this);
    m_folderModel->setIncludeAllItem(true);
    ui->lvFolders->setModel(m_folderModel);

    m_cardModel = new rewise::ui::widgets::CardTableModel(this);
    ui->tvCards->setModel(m_cardModel);

    ui->lvFolders->setMouseTracking(true);
    ui->lvFolders->setUniformItemSizes(true);
    ui->lvFolders->setSpacing(4);
    ui->lvFolders->setItemDelegate(new FolderRowDelegate(ui->lvFolders));
    neutralizeNativeSelection(ui->lvFolders);

    ui->tvCards->setMouseTracking(true);
    ui->tvCards->setShowGrid(false);
    ui->tvCards->setAlternatingRowColors(false);
    ui->tvCards->verticalHeader()->setVisible(false);
    ui->tvCards->horizontalHeader()->setVisible(false);
    ui->tvCards->setSortingEnabled(false);
    ui->tvCards->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tvCards->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->tvCards->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tvCards->verticalHeader()->setDefaultSectionSize(56);

    ui->tvCards->setItemDelegateForColumn(rewise::ui::widgets::CardTableModel::QuestionCol,
                                          new CardRowDelegate(ui->tvCards));

    ui->tvCards->setColumnHidden(rewise::ui::widgets::CardTableModel::AnswerCol, true);
    ui->tvCards->setColumnHidden(rewise::ui::widgets::CardTableModel::FolderCol, true);
    ui->tvCards->setColumnHidden(rewise::ui::widgets::CardTableModel::UpdatedCol, true);
    ui->tvCards->horizontalHeader()->setSectionResizeMode(rewise::ui::widgets::CardTableModel::QuestionCol,
                                                          QHeaderView::Stretch);

    neutralizeNativeSelection(ui->tvCards);

    m_cardPopup = new rewise::ui::widgets::CardPopupDialog(this);
    m_folderPopup = new rewise::ui::widgets::FolderPopupDialog(this);
    m_folderActionsPopup = new rewise::ui::widgets::ActionMenuPopup(this);

    wireUi();
}

LibraryPage::~LibraryPage() {
    delete ui;
}

void LibraryPage::setNotifier(QObject* notify) {
    m_notify = notify;
}

void LibraryPage::setDatabase(rewise::storage::Database db) {
    m_db = std::move(db);

    m_folderModel->setFolders(m_db.folders);

    m_cardModel->setDatabase(m_db);
    m_cardModel->setSearchQuery(ui->leSearch->text());
    m_cardModel->setFolderFilter(selectedFolderId());

    if (!ui->lvFolders->selectionModel()->hasSelection()) {
        ui->lvFolders->setCurrentIndex(m_folderModel->index(0, 0));
    }
}

rewise::domain::Id LibraryPage::selectedFolderId() const {
    const QModelIndex idx = ui->lvFolders->currentIndex();
    if (!idx.isValid()) return {};
    const int row = idx.row();
    if (m_folderModel->isAllRow(row)) return {};
    return m_folderModel->idAtRow(row);
}

rewise::domain::Id LibraryPage::selectedCardId() const {
    const QModelIndex idx = ui->tvCards->currentIndex();
    if (!idx.isValid()) return {};
    return m_cardModel->cardIdAtRow(idx.row());
}

void LibraryPage::showError(const QString& text) {
    if (m_notify && QMetaObject::invokeMethod(m_notify, "showError", Qt::QueuedConnection, Q_ARG(QString, text))) {
        return;
    }
    QMessageBox::critical(this, tr("Ошибка"), text);
}

void LibraryPage::showInfo(const QString& text) {
    if (m_notify && QMetaObject::invokeMethod(m_notify, "showInfo", Qt::QueuedConnection, Q_ARG(QString, text))) {
        return;
    }
    QMessageBox::information(this, tr("Информация"), text);
}

void LibraryPage::openCreateFolderPopup() {
    m_pendingFolderRenameId = {};
    m_folderPopup->card()->setTitle(tr("Новая папка"));
    m_folderPopup->card()->setActionText(tr("Создать"));
    m_folderPopup->card()->setFolderName(QString());
    m_folderPopup->openCentered();
}

void LibraryPage::openRenameFolderPopup() {
    const rewise::domain::Id fid = selectedFolderId();
    if (!fid.isValid()) {
        showError(tr("Нельзя переименовать виртуальный раздел «Все карточки»."));
        return;
    }

    const auto* f = m_db.folderById(fid);
    if (!f) {
        showError(tr("Папка не найдена."));
        return;
    }

    m_pendingFolderRenameId = fid;
    m_folderPopup->card()->setTitle(tr("Переименовать папку"));
    m_folderPopup->card()->setActionText(tr("Сохранить"));
    m_folderPopup->card()->setFolderName(f->name);
    m_folderPopup->openCentered();
}

void LibraryPage::openFolderActionsPopup() {
    const bool enabled = selectedFolderId().isValid();
    m_folderActionsPopup->menu()->setActionsEnabled(enabled, enabled);
    m_folderActionsPopup->openBelow(ui->btnFolderMenu, 8);
}

void LibraryPage::requestDeleteFolder() {
    const rewise::domain::Id fid = selectedFolderId();
    if (!fid.isValid()) {
        showError(tr("Нельзя удалить виртуальный раздел «Все карточки"));
        return;
    }

    if (auto* center = qobject_cast<rewise::ui::widgets::NotificationCenter*>(m_notify)) {
        auto* toast = center->showConfirm(tr("Удалить папку? Карточки будут перенесены в Default."),
                                          tr("Удалить"), tr("Отмена"));
        connect(toast, &rewise::ui::widgets::ToastWidget::accepted, this, [this, fid] {
            emit folderDeleteRequested(fid);
        });
        return;
    }

    const auto res = QMessageBox::question(this,
                                           tr("Удалить папку"),
                                           tr("Удалить папку? Карточки будут перенесены в Default."),
                                           QMessageBox::Yes | QMessageBox::No,
                                           QMessageBox::No);
    if (res == QMessageBox::Yes) {
        emit folderDeleteRequested(fid);
    }
}

void LibraryPage::wireUi() {
    connect(ui->lvFolders->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex&, const QModelIndex&) {
                m_cardModel->setFolderFilter(selectedFolderId());
                ui->tvCards->clearSelection();
            });

    connect(ui->leSearch, &QLineEdit::textChanged, this, [this](const QString& t) {
        m_cardModel->setSearchQuery(t);
    });

    connect(ui->btnAddFolder, &QToolButton::clicked, this, [this] {
        openCreateFolderPopup();
    });

    connect(ui->btnFolderMenu, &QToolButton::clicked, this, [this] {
        openFolderActionsPopup();
    });

    connect(m_folderPopup, &rewise::ui::widgets::FolderPopupDialog::submitted,
            this, [this](const QString& name) {
                const QString trimmed = name.trimmed();
                if (trimmed.isEmpty()) {
                    showError(tr("Название не может быть пустым."));
                    return;
                }

                if (m_pendingFolderRenameId.isValid()) {
                    emit folderRenameRequested(m_pendingFolderRenameId, trimmed);
                    m_pendingFolderRenameId = {};
                } else {
                    emit folderCreateRequested(trimmed);
                }
            });

    connect(m_folderPopup, &rewise::ui::widgets::FolderPopupDialog::cancelled,
            this, [this] {
                m_pendingFolderRenameId = {};
            });

    connect(m_folderActionsPopup, &rewise::ui::widgets::ActionMenuPopup::renameRequested,
            this, [this] {
                openRenameFolderPopup();
            });

    connect(m_folderActionsPopup, &rewise::ui::widgets::ActionMenuPopup::deleteRequested,
            this, [this] {
                requestDeleteFolder();
            });

    connect(ui->btnAddCard, &QToolButton::clicked, this, [this] {
        rewise::domain::Id folderId = selectedFolderId();
        if (!folderId.isValid()) folderId = m_db.ensureDefaultFolder();
        m_cardPopup->card()->startCreate(folderId);
        m_cardPopup->openCentered();
    });

    connect(ui->tvCards, &QTableView::doubleClicked, this, [this](const QModelIndex& idx) {
        if (!idx.isValid()) return;
        const rewise::domain::Id cid = m_cardModel->cardIdAtRow(idx.row());
        const auto* c = m_db.cardById(cid);
        if (!c) {
            showError(tr("Карточка не найдена."));
            return;
        }
        m_cardPopup->card()->setCard(*c);
        m_cardPopup->openCentered();
    });

    connect(ui->btnStartReview, &QPushButton::clicked, this, [this] {
        emit startReviewRequested(selectedFolderId());
    });

    auto* cw = m_cardPopup->card();
    connect(cw, &rewise::ui::widgets::CardWidget::saveRequested,
            this, [this](const rewise::domain::Card& card) {
                const bool exists = (m_db.cardById(card.id) != nullptr);
                if (exists) emit cardUpdateRequested(card.id, card.question, card.answer);
                else emit cardCreateRequested(card.folderId, card.question, card.answer);
            });

    connect(cw, &rewise::ui::widgets::CardWidget::deleteRequested,
            this, [this](const rewise::domain::Id& id) {
                if (!id.isValid()) return;
                emit cardDeleteRequested(id);
            });

    connect(cw, &rewise::ui::widgets::CardWidget::closeRequested,
            this, [this] {
                m_cardPopup->hide();
            });
}

} // namespace rewise::ui::pages