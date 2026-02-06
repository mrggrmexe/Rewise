#include "LibraryPage.h"
#include "ui_LibraryPage.h"

#include "ui/widgets/InlineMessageWidget.h"
#include "ui/widgets/FolderListModel.h"
#include "ui/widgets/CardTableModel.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QMenu>
#include <QStyle>
#include <QToolButton>

#include <algorithm>

namespace rewise::ui::pages {

static QPoint menuAnchorBelow(QWidget* w) {
    return w->mapToGlobal(QPoint(0, w->height()));
}

LibraryPage::LibraryPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LibraryPage)
{
    ui->setupUi(this);

    // --- Styling hooks (QSS) ---
    ui->frameFolders->setProperty("card", true);
    ui->frameCards->setProperty("card", true);
    ui->detailsStack->setProperty("card", true);
    ui->tbPreview->setProperty("flat", true);
    ui->leSearch->setProperty("search", true);

    ui->btnStartReview->setProperty("primary", true);

    // Tool-buttons: icon-ish, no focus rectangle noise.
    const auto prepToolBtn = [](QToolButton* b) {
        if (!b) return;
        b->setAutoRaise(true);
        b->setFocusPolicy(Qt::NoFocus);
    };
    prepToolBtn(ui->btnAddFolder);
    prepToolBtn(ui->btnFolderMore);
    prepToolBtn(ui->btnFolderSave);
    prepToolBtn(ui->btnFolderCancel);

    prepToolBtn(ui->btnAddCard);
    prepToolBtn(ui->btnEditCard);
    prepToolBtn(ui->btnDeleteCard);
    prepToolBtn(ui->btnCardSave);
    prepToolBtn(ui->btnCardCancel);

    // Use a mix of glyphs + standard icons (stable on macOS/Fusion).
    ui->btnAddFolder->setText(QStringLiteral("＋"));
    ui->btnFolderMore->setText(QStringLiteral("⋯"));

    ui->btnAddCard->setText(QStringLiteral("＋"));
    ui->btnEditCard->setText(QStringLiteral("✎"));

    ui->btnDeleteCard->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    ui->btnDeleteCard->setText(QString());
    ui->btnDeleteCard->setToolButtonStyle(Qt::ToolButtonIconOnly);

    ui->btnFolderSave->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    ui->btnFolderSave->setText(QString());
    ui->btnFolderSave->setToolButtonStyle(Qt::ToolButtonIconOnly);

    ui->btnFolderCancel->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    ui->btnFolderCancel->setText(QString());
    ui->btnFolderCancel->setToolButtonStyle(Qt::ToolButtonIconOnly);

    ui->btnCardSave->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    ui->btnCardSave->setText(QString());
    ui->btnCardSave->setToolButtonStyle(Qt::ToolButtonIconOnly);

    ui->btnCardCancel->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    ui->btnCardCancel->setText(QString());
    ui->btnCardCancel->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // Splitters: keep layout tight, allow user-driven sizing.
    ui->splitMain->setChildrenCollapsible(false);
    ui->splitCards->setChildrenCollapsible(false);

    {
        QList<int> sizes;
        sizes << 280 << 900;
        ui->splitMain->setSizes(sizes);
    }
    {
        QList<int> sizes;
        sizes << 420 << 220;
        ui->splitCards->setSizes(sizes);
    }

    // Inline message (banner) — держим в коде (без custom widget plugin).
    m_msg = new rewise::ui::widgets::InlineMessageWidget(this);
    if (!ui->messageHost->layout()) {
        auto* l = new QVBoxLayout(ui->messageHost);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(0);
        ui->messageHost->setLayout(l);
    }
    ui->messageHost->layout()->addWidget(m_msg);

    // Models
    m_folderModel = new rewise::ui::widgets::FolderListModel(this);
    m_cardModel   = new rewise::ui::widgets::CardTableModel(this);

    ui->lvFolders->setModel(m_folderModel);

    ui->tvCards->setModel(m_cardModel);
    ui->tvCards->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tvCards->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tvCards->setSortingEnabled(true);
    ui->tvCards->setCornerButtonEnabled(false);
    ui->tvCards->setShowGrid(false);
    ui->tvCards->setAlternatingRowColors(false);
    ui->tvCards->verticalHeader()->setVisible(false);
    ui->tvCards->horizontalHeader()->setVisible(false);
    ui->tvCards->horizontalHeader()->setStretchLastSection(true);
    ui->tvCards->verticalHeader()->setDefaultSectionSize(44);

    ui->tvCards->horizontalHeader()->setSectionResizeMode(rewise::ui::widgets::CardTableModel::QuestionCol, QHeaderView::Stretch);
    ui->tvCards->horizontalHeader()->setSectionResizeMode(rewise::ui::widgets::CardTableModel::AnswerCol, QHeaderView::Stretch);

    // Убираем визуальный шум: папка и дата — в будущем можно включать опционально.
    ui->tvCards->setColumnHidden(rewise::ui::widgets::CardTableModel::FolderCol, true);
    ui->tvCards->setColumnHidden(rewise::ui::widgets::CardTableModel::UpdatedCol, true);

    // Context menus (чтобы убрать лишние кнопки с экрана).
    ui->lvFolders->viewport()->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tvCards->viewport()->setContextMenuPolicy(Qt::CustomContextMenu);

    wireUi();
    applyEditState();
    refreshButtons();
    refreshPreview();
}

LibraryPage::~LibraryPage() {
    delete ui;
}

bool LibraryPage::isEditing() const {
    return (m_folderEditMode != FolderEditMode::None) || (m_cardEditMode != CardEditMode::None);
}

void LibraryPage::wireUi() {
    // Search
    connect(ui->leSearch, &QLineEdit::textChanged, this, [this](const QString& t) {
        m_cardModel->setSearchQuery(t);
        refreshButtons();
        refreshPreview();
    });

    // Folder selection => filter cards
    connect(ui->lvFolders->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
        if (isEditing()) return; // не меняем контекст, пока открыт inline-editor
        m_cardModel->setFolderFilter(selectedFolderId());
        refreshButtons();
        refreshPreview();
    });

    // Card selection => preview
    connect(ui->tvCards->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
        if (m_cardEditMode != CardEditMode::None) return;
        refreshButtons();
        refreshPreview();
    });

    // --- Folder actions ---
    connect(ui->btnAddFolder, &QToolButton::clicked, this, [this] {
        openFolderCreate();
    });

    connect(ui->btnFolderMore, &QToolButton::clicked, this, [this] {
        showFolderMenu(menuAnchorBelow(ui->btnFolderMore));
    });

    connect(ui->lvFolders->viewport(), &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        const QModelIndex idx = ui->lvFolders->indexAt(pos);
        if (idx.isValid()) ui->lvFolders->setCurrentIndex(idx);
        showFolderMenu(ui->lvFolders->viewport()->mapToGlobal(pos));
    });

    connect(ui->lvFolders, &QListView::doubleClicked, this, [this](const QModelIndex& idx) {
        if (!idx.isValid()) return;
        const auto id = selectedFolderId();
        if (!id.isValid()) return; // "Все карточки" не переименовываем
        openFolderRename(id);
    });

    connect(ui->btnFolderSave, &QToolButton::clicked, this, [this] { commitFolderEditor(); });
    connect(ui->btnFolderCancel, &QToolButton::clicked, this, [this] { closeFolderEditor(); });
    connect(ui->leFolderName, &QLineEdit::returnPressed, this, [this] { commitFolderEditor(); });
    connect(ui->leFolderName, &QLineEdit::textChanged, this, [this] { refreshButtons(); });

    // --- Card actions ---
    connect(ui->btnAddCard, &QToolButton::clicked, this, [this] { openCardCreate(); });
    connect(ui->btnEditCard, &QToolButton::clicked, this, [this] {
        const auto id = selectedCardId();
        if (!id.isValid()) return;
        openCardEdit(id);
    });
    connect(ui->btnDeleteCard, &QToolButton::clicked, this, [this] {
        const auto id = selectedCardId();
        if (!id.isValid()) return;
        requestDeleteCard(id);
    });

    connect(ui->tvCards, &QTableView::doubleClicked, this, [this](const QModelIndex&) {
        const auto id = selectedCardId();
        if (!id.isValid()) return;
        openCardEdit(id);
    });

    connect(ui->tvCards->viewport(), &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        const QModelIndex idx = ui->tvCards->indexAt(pos);
        if (idx.isValid()) ui->tvCards->setCurrentIndex(idx);
        showCardMenu(ui->tvCards->viewport()->mapToGlobal(pos));
    });

    connect(ui->btnCardSave, &QToolButton::clicked, this, [this] { commitCardEditor(); });
    connect(ui->btnCardCancel, &QToolButton::clicked, this, [this] { closeCardEditor(); });

    connect(ui->leCardQuestion, &QLineEdit::textChanged, this, [this] { refreshButtons(); });
    connect(ui->pteCardAnswer, &QPlainTextEdit::textChanged, this, [this] { refreshButtons(); });

    connect(ui->btnStartReview, &QPushButton::clicked, this, [this] {
        if (isEditing()) return;
        emit startReviewRequested(selectedFolderId());
    });
}

void LibraryPage::setDatabase(rewise::storage::Database db) {
    const auto prevFolder = selectedFolderId();
    const auto prevCard   = selectedCardId();

    m_db = std::move(db);

    m_folderModel->setFolders(m_db.folders);
    m_cardModel->setDatabase(m_db);
    m_cardModel->setFolderFilter(prevFolder);

    // restore folder selection
    const int folderRow = m_folderModel->rowForId(prevFolder);
    if (folderRow >= 0) {
        ui->lvFolders->setCurrentIndex(m_folderModel->index(folderRow, 0));
    } else {
        if (m_folderModel->includeAllItem()) {
            ui->lvFolders->setCurrentIndex(m_folderModel->index(0, 0));
        } else if (m_folderModel->rowCount() > 0) {
            ui->lvFolders->setCurrentIndex(m_folderModel->index(0, 0));
        }
    }

    // restore card selection
    const int cardRow = m_cardModel->rowForCardId(prevCard);
    if (cardRow >= 0) {
        ui->tvCards->selectRow(cardRow);
        ui->tvCards->scrollTo(m_cardModel->index(cardRow, 0));
    }

    refreshButtons();
    refreshPreview();
}

void LibraryPage::showError(const QString& text) {
    if (m_msg) m_msg->showMessage(rewise::ui::widgets::InlineMessageWidget::Kind::Error, text);
}

void LibraryPage::showInfo(const QString& text) {
    if (m_msg) m_msg->showMessage(rewise::ui::widgets::InlineMessageWidget::Kind::Info, text);
}

void LibraryPage::clearMessage() {
    if (m_msg) m_msg->clearMessage();
}

rewise::domain::Id LibraryPage::selectedFolderId() const {
    const QModelIndex idx = ui->lvFolders->currentIndex();
    if (!idx.isValid()) return rewise::domain::Id{}; // all
    return m_folderModel->idAtRow(idx.row());
}

rewise::domain::Id LibraryPage::selectedCardId() const {
    const QModelIndex idx = ui->tvCards->currentIndex();
    if (!idx.isValid()) return rewise::domain::Id{};
    return m_cardModel->cardIdAtRow(idx.row());
}

void LibraryPage::applyEditState() {
    ui->folderEditor->setVisible(m_folderEditMode != FolderEditMode::None);

    if (m_cardEditMode != CardEditMode::None) {
        ui->detailsStack->setCurrentWidget(ui->editorPage);
    } else {
        ui->detailsStack->setCurrentWidget(ui->previewPage);
    }
}

void LibraryPage::refreshButtons() {
    const bool editingFolder = (m_folderEditMode != FolderEditMode::None);
    const bool editingCard   = (m_cardEditMode != CardEditMode::None);
    const bool editingAny    = editingFolder || editingCard;

    // Lock navigation while editing to reduce accidental context changes.
    ui->lvFolders->setEnabled(!editingAny);
    ui->tvCards->setEnabled(!editingAny);
    ui->leSearch->setEnabled(!editingAny);
    ui->btnStartReview->setEnabled(!editingAny && m_cardModel->rowCount() > 0);

    ui->btnAddFolder->setEnabled(!editingAny);
    ui->btnFolderMore->setEnabled(!editingAny);

    ui->btnAddCard->setEnabled(!editingAny);
    const auto cardId = selectedCardId();
    const bool hasCard = cardId.isValid();
    ui->btnEditCard->setEnabled(!editingAny && hasCard);
    ui->btnDeleteCard->setEnabled(!editingAny && hasCard);

    // Inline editors validation.
    if (editingFolder) {
        const bool ok = !ui->leFolderName->text().trimmed().isEmpty();
        ui->btnFolderSave->setEnabled(ok);
    }
    if (editingCard) {
        const bool ok = !ui->leCardQuestion->text().trimmed().isEmpty()
                        && !ui->pteCardAnswer->toPlainText().trimmed().isEmpty();
        ui->btnCardSave->setEnabled(ok);
    }
}

void LibraryPage::refreshPreview() {
    if (m_cardEditMode != CardEditMode::None) return;

    const auto cardId = selectedCardId();
    const auto* c = m_db.cardById(cardId);

    if (!c) {
        ui->tbPreview->setHtml("<div style='opacity:0.65'>Выберите карточку, чтобы увидеть детали.</div>");
        return;
    }

    const QString html =
        "<div style='white-space: pre-wrap; line-height:1.35;'>"
          "<div style='font-weight:700; font-size:15px; margin-bottom:6px;'>" + c->question.toHtmlEscaped() + "</div>"
          "<div style='opacity:0.75; font-weight:600; margin-bottom:4px;'>Эталонный ответ</div>"
          "<div>" + c->answer.toHtmlEscaped() + "</div>"
        "</div>";

    ui->tbPreview->setHtml(html);
}

// ---------------- Folder inline editor ----------------

void LibraryPage::openFolderCreate() {
    clearMessage();
    m_folderEditMode = FolderEditMode::Create;
    m_folderEditId = {};

    ui->leFolderName->clear();
    ui->leFolderName->setFocus();

    applyEditState();
    refreshButtons();
}

void LibraryPage::openFolderRename(const rewise::domain::Id& folderId) {
    if (!folderId.isValid()) return;
    const auto* f = m_db.folderById(folderId);
    if (!f) return;

    clearMessage();
    m_folderEditMode = FolderEditMode::Rename;
    m_folderEditId = folderId;

    ui->leFolderName->setText(f->name);
    ui->leFolderName->selectAll();
    ui->leFolderName->setFocus();

    applyEditState();
    refreshButtons();
}

void LibraryPage::closeFolderEditor() {
    m_folderEditMode = FolderEditMode::None;
    m_folderEditId = {};

    ui->leFolderName->clear();

    applyEditState();
    refreshButtons();
}

void LibraryPage::commitFolderEditor() {
    const QString name = ui->leFolderName->text().trimmed();
    if (name.isEmpty()) {
        showError("Название папки не может быть пустым.");
        return;
    }

    const auto mode = m_folderEditMode;
    const auto id   = m_folderEditId;

    closeFolderEditor();

    if (mode == FolderEditMode::Create) {
        emit folderCreateRequested(name);
    } else if (mode == FolderEditMode::Rename) {
        if (!id.isValid()) return;
        emit folderRenameRequested(id, name);
    }
}

void LibraryPage::showFolderMenu(const QPoint& globalPos) {
    if (m_folderEditMode != FolderEditMode::None || m_cardEditMode != CardEditMode::None) return;

    QMenu menu(this);

    auto* aNew = menu.addAction("Новая папка…");
    auto* aRename = menu.addAction("Переименовать…");
    auto* aDelete = menu.addAction("Удалить…");

    const auto folderId = selectedFolderId();
    const bool hasFolder = folderId.isValid(); // invalid = "Все карточки"
    aRename->setEnabled(hasFolder);
    aDelete->setEnabled(hasFolder);

    QAction* act = menu.exec(globalPos);
    if (!act) return;

    if (act == aNew) {
        openFolderCreate();
        return;
    }

    if (!hasFolder) return;

    if (act == aRename) {
        openFolderRename(folderId);
        return;
    }

    if (act == aDelete) {
        const auto* f = m_db.folderById(folderId);
        if (!f) return;

        const int cardCount = std::count_if(m_db.cards.begin(), m_db.cards.end(),
                                            [&](const rewise::domain::Card& c) { return c.folderId == folderId; });

        const auto ans = QMessageBox::question(this,
                                              "Удалить папку?",
                                              QString("Удалить папку \"%1\"?\nКарточек внутри: %2.\n\nКарточки будут перенесены в Default.")
                                                  .arg(f->name)
                                                  .arg(cardCount),
                                              QMessageBox::Yes | QMessageBox::No,
                                              QMessageBox::No);
        if (ans != QMessageBox::Yes) return;

        emit folderDeleteRequested(folderId);
    }
}

// ---------------- Card inline editor ----------------

void LibraryPage::openCardCreate() {
    clearMessage();
    m_cardEditMode = CardEditMode::Create;
    m_cardEditId = {};

    ui->lblEditorTitle->setText("Новая карточка");
    ui->leCardQuestion->clear();
    ui->pteCardAnswer->clear();
    ui->leCardQuestion->setFocus();

    applyEditState();
    refreshButtons();
}

void LibraryPage::openCardEdit(const rewise::domain::Id& cardId) {
    if (!cardId.isValid()) return;
    const auto* c = m_db.cardById(cardId);
    if (!c) return;

    clearMessage();
    m_cardEditMode = CardEditMode::Edit;
    m_cardEditId = cardId;

    ui->lblEditorTitle->setText("Редактирование");
    ui->leCardQuestion->setText(c->question);
    ui->leCardQuestion->selectAll();
    ui->pteCardAnswer->setPlainText(c->answer);

    ui->leCardQuestion->setFocus();

    applyEditState();
    refreshButtons();
}

void LibraryPage::closeCardEditor() {
    m_cardEditMode = CardEditMode::None;
    m_cardEditId = {};

    ui->leCardQuestion->clear();
    ui->pteCardAnswer->clear();

    applyEditState();
    refreshButtons();
    refreshPreview();
}

void LibraryPage::commitCardEditor() {
    const QString q = ui->leCardQuestion->text().trimmed();
    const QString a = ui->pteCardAnswer->toPlainText().trimmed();

    if (q.isEmpty()) {
        showError("Вопрос не может быть пустым.");
        return;
    }
    if (a.isEmpty()) {
        showError("Ответ не может быть пустым.");
        return;
    }

    const auto mode = m_cardEditMode;
    const auto id   = m_cardEditId;

    closeCardEditor();

    if (mode == CardEditMode::Create) {
        emit cardCreateRequested(selectedFolderId(), q, a);
    } else if (mode == CardEditMode::Edit) {
        if (!id.isValid()) return;
        emit cardUpdateRequested(id, q, a);
    }
}

void LibraryPage::requestDeleteCard(const rewise::domain::Id& cardId) {
    if (!cardId.isValid()) return;

    const auto ans = QMessageBox::question(this,
                                          "Удалить карточку?",
                                          "Удалить выбранную карточку?",
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No);
    if (ans != QMessageBox::Yes) return;

    emit cardDeleteRequested(cardId);
}

void LibraryPage::showCardMenu(const QPoint& globalPos) {
    if (m_folderEditMode != FolderEditMode::None || m_cardEditMode != CardEditMode::None) return;

    QMenu menu(this);

    auto* aNew   = menu.addAction("Новая карточка…");
    auto* aEdit  = menu.addAction("Редактировать…");
    auto* aDel   = menu.addAction("Удалить…");

    const auto cardId = selectedCardId();
    const bool hasCard = cardId.isValid();
    aEdit->setEnabled(hasCard);
    aDel->setEnabled(hasCard);

    QAction* act = menu.exec(globalPos);
    if (!act) return;

    if (act == aNew) {
        openCardCreate();
        return;
    }

    if (!hasCard) return;

    if (act == aEdit) {
        openCardEdit(cardId);
        return;
    }

    if (act == aDel) {
        requestDeleteCard(cardId);
        return;
    }
}

} // namespace rewise::ui::pages
