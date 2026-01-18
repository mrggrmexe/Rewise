#include "LibraryPage.h"

#include "ui/widgets/InlineMessageWidget.h"
#include "ui/widgets/FolderListModel.h"
#include "ui/widgets/CardTableModel.h"
#include "ui/widgets/FolderEditDialog.h"
#include "ui/widgets/CardEditDialog.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTableView>
#include <QTextBrowser>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLineEdit>

namespace rewise::ui::pages {

LibraryPage::LibraryPage(QWidget* parent)
    : QWidget(parent)
{
    buildUi();
    wireUi();
}

void LibraryPage::buildUi() {
    m_msg = new rewise::ui::widgets::InlineMessageWidget(this);

    // Models
    m_folderModel = new rewise::ui::widgets::FolderListModel(this);
    m_cardModel = new rewise::ui::widgets::CardTableModel(this);

    // Left: folders
    auto* foldersBox = new QGroupBox("Папки", this);
    m_foldersView = new QListView(foldersBox);
    m_foldersView->setModel(m_folderModel);
    m_foldersView->setSelectionMode(QAbstractItemView::SingleSelection);

    m_addFolder = new QPushButton("Добавить", foldersBox);
    m_renameFolder = new QPushButton("Переименовать", foldersBox);
    m_deleteFolder = new QPushButton("Удалить", foldersBox);

    auto* folderBtns = new QHBoxLayout();
    folderBtns->addWidget(m_addFolder);
    folderBtns->addWidget(m_renameFolder);
    folderBtns->addWidget(m_deleteFolder);

    auto* foldersLayout = new QVBoxLayout(foldersBox);
    foldersLayout->addWidget(m_foldersView, 1);
    foldersLayout->addLayout(folderBtns);

    // Right: cards
    auto* cardsBox = new QGroupBox("Карточки", this);

    m_search = new QLineEdit(cardsBox);
    m_search->setPlaceholderText("Поиск по вопросу/ответу…");

    m_cardsView = new QTableView(cardsBox);
    m_cardsView->setModel(m_cardModel);
    m_cardsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_cardsView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_cardsView->setSortingEnabled(true);
    m_cardsView->horizontalHeader()->setStretchLastSection(true);
    m_cardsView->horizontalHeader()->setSectionResizeMode(rewise::ui::widgets::CardTableModel::QuestionCol, QHeaderView::Stretch);
    m_cardsView->horizontalHeader()->setSectionResizeMode(rewise::ui::widgets::CardTableModel::AnswerCol, QHeaderView::Stretch);
    m_cardsView->horizontalHeader()->setSectionResizeMode(rewise::ui::widgets::CardTableModel::FolderCol, QHeaderView::ResizeToContents);
    m_cardsView->horizontalHeader()->setSectionResizeMode(rewise::ui::widgets::CardTableModel::UpdatedCol, QHeaderView::ResizeToContents);

    m_addCard = new QPushButton("Добавить", cardsBox);
    m_editCard = new QPushButton("Редактировать", cardsBox);
    m_deleteCard = new QPushButton("Удалить", cardsBox);
    m_review = new QPushButton("Начать повторение", cardsBox);

    auto* cardBtns = new QHBoxLayout();
    cardBtns->addWidget(m_addCard);
    cardBtns->addWidget(m_editCard);
    cardBtns->addWidget(m_deleteCard);
    cardBtns->addStretch(1);
    cardBtns->addWidget(m_review);

    m_preview = new QTextBrowser(cardsBox);
    m_preview->setReadOnly(true);
    m_preview->setMinimumHeight(140);

    auto* cardsLayout = new QVBoxLayout(cardsBox);
    cardsLayout->addWidget(m_search);
    cardsLayout->addWidget(m_cardsView, 1);
    cardsLayout->addLayout(cardBtns);
    cardsLayout->addWidget(m_preview);

    // Split
    auto* split = new QSplitter(this);
    split->setOrientation(Qt::Horizontal);
    split->addWidget(foldersBox);
    split->addWidget(cardsBox);
    split->setStretchFactor(0, 0);
    split->setStretchFactor(1, 1);
    split->setSizes({240, 560});

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);
    root->addWidget(m_msg);
    root->addWidget(split, 1);
}

void LibraryPage::wireUi() {
    connect(m_search, &QLineEdit::textChanged, this, [this](const QString& t) {
        m_cardModel->setSearchQuery(t);
        refreshButtons();
        refreshPreview();
    });

    connect(m_foldersView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this] {
                m_cardModel->setFolderFilter(selectedFolderId());
                refreshButtons();
                refreshPreview();
            });

    connect(m_cardsView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this] {
                refreshButtons();
                refreshPreview();
            });

    connect(m_addFolder, &QPushButton::clicked, this, [this] {
        QString name;
        if (!rewise::ui::widgets::FolderEditDialog::getFolderName(this, "Новая папка", "", &name)) return;
        emit folderCreateRequested(name);
    });

    connect(m_renameFolder, &QPushButton::clicked, this, [this] {
        const auto id = selectedFolderId();
        if (!id.isValid()) return; // "Все карточки" переименовывать нельзя
        const auto* f = m_db.folderById(id);
        if (!f) return;

        QString name;
        if (!rewise::ui::widgets::FolderEditDialog::getFolderName(this, "Переименовать папку", f->name, &name)) return;
        emit folderRenameRequested(id, name);
    });

    connect(m_deleteFolder, &QPushButton::clicked, this, [this] {
        const auto id = selectedFolderId();
        if (!id.isValid()) return;
        const auto* f = m_db.folderById(id);
        if (!f) return;

        const int cardCount = std::count_if(m_db.cards.begin(), m_db.cards.end(),
                                            [&](const rewise::domain::Card& c) { return c.folderId == id; });

        const auto ans = QMessageBox::question(this,
                                              "Удалить папку?",
                                              QString("Удалить папку \"%1\"?\nКарточек внутри: %2.\n\nКарточки будут перенесены в Default.")
                                                .arg(f->name)
                                                .arg(cardCount),
                                              QMessageBox::Yes | QMessageBox::No,
                                              QMessageBox::No);
        if (ans != QMessageBox::Yes) return;

        emit folderDeleteRequested(id);
    });

    connect(m_addCard, &QPushButton::clicked, this, [this] {
        const auto folderId = selectedFolderId(); // invalid => mainwindow сам выберет default
        QString q, a;
        if (!rewise::ui::widgets::CardEditDialog::getCardText(this, "Новая карточка", "", "", &q, &a)) return;
        emit cardCreateRequested(folderId, q, a);
    });

    connect(m_editCard, &QPushButton::clicked, this, [this] {
        const auto cardId = selectedCardId();
        if (!cardId.isValid()) return;
        const auto* c = m_db.cardById(cardId);
        if (!c) return;

        QString q, a;
        if (!rewise::ui::widgets::CardEditDialog::getCardText(this, "Редактировать карточку", c->question, c->answer, &q, &a)) return;
        emit cardUpdateRequested(cardId, q, a);
    });

    connect(m_deleteCard, &QPushButton::clicked, this, [this] {
        const auto cardId = selectedCardId();
        if (!cardId.isValid()) return;

        const auto ans = QMessageBox::question(this,
                                              "Удалить карточку?",
                                              "Удалить выбранную карточку?",
                                              QMessageBox::Yes | QMessageBox::No,
                                              QMessageBox::No);
        if (ans != QMessageBox::Yes) return;

        emit cardDeleteRequested(cardId);
    });

    connect(m_review, &QPushButton::clicked, this, [this] {
        emit startReviewRequested(selectedFolderId());
    });
}

void LibraryPage::setDatabase(rewise::storage::Database db) {
    // Попробуем сохранить выбор
    const auto prevFolder = selectedFolderId();
    const auto prevCard = selectedCardId();

    m_db = std::move(db);

    // Обновляем модели
    m_folderModel->setFolders(m_db.folders);
    m_cardModel->setDatabase(m_db);
    m_cardModel->setFolderFilter(prevFolder);

    // Восстановим selection папки
    const int folderRow = m_folderModel->rowForId(prevFolder);
    if (folderRow >= 0) {
        m_foldersView->setCurrentIndex(m_folderModel->index(folderRow, 0));
    } else {
        // По умолчанию — "Все карточки"
        if (m_folderModel->includeAllItem()) {
            m_foldersView->setCurrentIndex(m_folderModel->index(0, 0));
        } else if (m_folderModel->rowCount() > 0) {
            m_foldersView->setCurrentIndex(m_folderModel->index(0, 0));
        }
    }

    // Восстановим selection карточки (если осталась)
    const int cardRow = m_cardModel->rowForCardId(prevCard);
    if (cardRow >= 0) {
        m_cardsView->selectRow(cardRow);
        m_cardsView->scrollTo(m_cardModel->index(cardRow, 0));
    }

    refreshButtons();
    refreshPreview();
}

void LibraryPage::showError(const QString& text) {
    m_msg->showMessage(rewise::ui::widgets::InlineMessageWidget::Kind::Error, text);
}

void LibraryPage::showInfo(const QString& text) {
    m_msg->showMessage(rewise::ui::widgets::InlineMessageWidget::Kind::Info, text);
}

void LibraryPage::clearMessage() {
    m_msg->clearMessage();
}

rewise::domain::Id LibraryPage::selectedFolderId() const {
    const QModelIndex idx = m_foldersView->currentIndex();
    if (!idx.isValid()) return rewise::domain::Id{}; // all
    return m_folderModel->idAtRow(idx.row());
}

rewise::domain::Id LibraryPage::selectedCardId() const {
    const QModelIndex idx = m_cardsView->currentIndex();
    if (!idx.isValid()) return rewise::domain::Id{};
    const int row = idx.row();
    return m_cardModel->cardIdAtRow(row);
}

void LibraryPage::refreshButtons() {
    const auto folderId = selectedFolderId();
    const bool hasFolder = folderId.isValid();
    const bool hasAnyFolder = (m_db.folders.size() > 0);

    m_renameFolder->setEnabled(hasFolder);
    m_deleteFolder->setEnabled(hasFolder && hasAnyFolder);

    const auto cardId = selectedCardId();
    const bool hasCard = cardId.isValid();

    m_editCard->setEnabled(hasCard);
    m_deleteCard->setEnabled(hasCard);

    // Review доступен, если есть хотя бы 1 карточка под текущим фильтром/поиском
    m_review->setEnabled(m_cardModel->rowCount() > 0);
}

void LibraryPage::refreshPreview() {
    const auto cardId = selectedCardId();
    const auto* c = m_db.cardById(cardId);

    if (!c) {
        m_preview->setHtml("<div style='opacity:0.7'>Выберите карточку, чтобы увидеть превью.</div>");
        return;
    }

    const QString html =
        "<div style='white-space: pre-wrap; line-height:1.35;'>"
        "<div style='font-weight:600; margin-bottom:4px;'>Вопрос</div>"
        "<div style='margin-bottom:12px;'>" + c->question.toHtmlEscaped() + "</div>"
        "<div style='font-weight:600; margin-bottom:4px;'>Ответ</div>"
        "<div>" + c->answer.toHtmlEscaped() + "</div>"
        "</div>";

    m_preview->setHtml(html);
}

} // namespace rewise::ui::pages
