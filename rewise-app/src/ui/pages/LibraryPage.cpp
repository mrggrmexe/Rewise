#include "LibraryPage.h"
#include "ui_LibraryPage.h"

#include "ui/widgets/InlineMessageWidget.h"
#include "ui/widgets/FolderListModel.h"
#include "ui/widgets/CardTableModel.h"
#include "ui/widgets/FolderEditDialog.h"
#include "ui/widgets/CardEditDialog.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHeaderView>

namespace rewise::ui::pages {

LibraryPage::LibraryPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LibraryPage)
{
    ui->setupUi(this);
    ui->tvCards->horizontalHeader()->setVisible(false);
    ui->tvCards->verticalHeader()->setVisible(false);
    ui->tvCards->setCornerButtonEnabled(false);
    ui->tvCards->setShowGrid(false);
    ui->btnStartReview->setProperty("primary", true);

    // Inline message (banner) — держим в коде, чтобы не возиться с custom widget plugin.
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
    m_cardModel = new rewise::ui::widgets::CardTableModel(this);

    ui->lvFolders->setModel(m_folderModel);

    ui->tvCards->setModel(m_cardModel);
    ui->tvCards->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tvCards->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tvCards->setSortingEnabled(true);
    ui->tvCards->horizontalHeader()->setStretchLastSection(true);
    ui->tvCards->horizontalHeader()->setSectionResizeMode(rewise::ui::widgets::CardTableModel::QuestionCol, QHeaderView::Stretch);
    ui->tvCards->horizontalHeader()->setSectionResizeMode(rewise::ui::widgets::CardTableModel::AnswerCol, QHeaderView::Stretch);
    ui->tvCards->horizontalHeader()->setSectionResizeMode(rewise::ui::widgets::CardTableModel::FolderCol, QHeaderView::ResizeToContents);
    ui->tvCards->horizontalHeader()->setSectionResizeMode(rewise::ui::widgets::CardTableModel::UpdatedCol, QHeaderView::ResizeToContents);
    ui->tvCards->setShowGrid(false);
    ui->tvCards->setAlternatingRowColors(false);
    ui->tvCards->verticalHeader()->setVisible(false);



    // Splitter initial sizes (folders ~240px)
    ui->splitterMain->setSizes({240, 740});

    wireUi();
    refreshButtons();
    refreshPreview();
}

LibraryPage::~LibraryPage() {
    delete ui;
}

void LibraryPage::wireUi() {
    connect(ui->leSearch, &QLineEdit::textChanged, this, [this](const QString& t) {
        m_cardModel->setSearchQuery(t);
        refreshButtons();
        refreshPreview();
    });

    connect(ui->lvFolders->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
        m_cardModel->setFolderFilter(selectedFolderId());
        refreshButtons();
        refreshPreview();
    });

    connect(ui->tvCards->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
        refreshButtons();
        refreshPreview();
    });

    connect(ui->btnAddFolder, &QPushButton::clicked, this, [this] {
        QString name;
        if (!rewise::ui::widgets::FolderEditDialog::getFolderName(this, "Новая папка", "", &name)) return;
        emit folderCreateRequested(name);
    });

    connect(ui->btnRenameFolder, &QPushButton::clicked, this, [this] {
        const auto id = selectedFolderId();
        if (!id.isValid()) return; // "Все карточки" нельзя
        const auto* f = m_db.folderById(id);
        if (!f) return;

        QString name;
        if (!rewise::ui::widgets::FolderEditDialog::getFolderName(this, "Переименовать папку", f->name, &name)) return;
        emit folderRenameRequested(id, name);
    });

    connect(ui->btnDeleteFolder, &QPushButton::clicked, this, [this] {
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

    connect(ui->btnAddCard, &QPushButton::clicked, this, [this] {
        const auto folderId = selectedFolderId(); // invalid => mainwindow выберет default
        QString q, a;
        if (!rewise::ui::widgets::CardEditDialog::getCardText(this, "Новая карточка", "", "", &q, &a)) return;
        emit cardCreateRequested(folderId, q, a);
    });

    connect(ui->btnEditCard, &QPushButton::clicked, this, [this] {
        const auto cardId = selectedCardId();
        if (!cardId.isValid()) return;
        const auto* c = m_db.cardById(cardId);
        if (!c) return;

        QString q, a;
        if (!rewise::ui::widgets::CardEditDialog::getCardText(this, "Редактировать карточку", c->question, c->answer, &q, &a)) return;
        emit cardUpdateRequested(cardId, q, a);
    });

    connect(ui->btnDeleteCard, &QPushButton::clicked, this, [this] {
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

    connect(ui->btnStartReview, &QPushButton::clicked, this, [this] {
        emit startReviewRequested(selectedFolderId());
    });
}

void LibraryPage::setDatabase(rewise::storage::Database db) {
    const auto prevFolder = selectedFolderId();
    const auto prevCard = selectedCardId();

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

void LibraryPage::refreshButtons() {
    const auto folderId = selectedFolderId();
    const bool hasFolder = folderId.isValid();
    const bool hasAnyFolder = (m_db.folders.size() > 0);

    ui->btnRenameFolder->setEnabled(hasFolder);
    ui->btnDeleteFolder->setEnabled(hasFolder && hasAnyFolder);

    const auto cardId = selectedCardId();
    const bool hasCard = cardId.isValid();

    ui->btnEditCard->setEnabled(hasCard);
    ui->btnDeleteCard->setEnabled(hasCard);

    ui->btnStartReview->setEnabled(m_cardModel->rowCount() > 0);
}

void LibraryPage::refreshPreview() {
    const auto cardId = selectedCardId();
    const auto* c = m_db.cardById(cardId);

    if (!c) {
        ui->tbPreview->setHtml("<div style='opacity:0.7'>Выберите карточку, чтобы увидеть превью.</div>");
        return;
    }

    const QString html =
        "<div style='white-space: pre-wrap; line-height:1.35;'>"
        "<div style='font-weight:600; margin-bottom:4px;'>Вопрос</div>"
        "<div style='margin-bottom:12px;'>" + c->question.toHtmlEscaped() + "</div>"
        "<div style='font-weight:600; margin-bottom:4px;'>Ответ</div>"
        "<div>" + c->answer.toHtmlEscaped() + "</div>"
        "</div>";

    ui->tbPreview->setHtml(html);
}

} // namespace rewise::ui::pages
