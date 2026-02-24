#include "LibraryPage.h"
#include "ui_LibraryPage.h"

#include "ui/widgets/FolderListModel.h"
#include "ui/widgets/CardTableModel.h"
#include "ui/widgets/CardPopupDialog.h"
#include "ui/widgets/CardWidget.h"

#include <QHeaderView>
#include <QItemSelectionModel>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPainterPath>
#include <QMetaObject>
#include <QApplication>
#include <QPalette>

namespace {

// soft tints
static QColor bgHover()    { return QColor(214, 229, 255, 150); }
static QColor bgSelected() { return QColor(206, 200, 255, 190); }
static QColor bgAltRow()   { return QColor(16, 24, 40, 10); }

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
    // Make native selection/focus invisible; we paint our own selection in delegates.
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

        // remove focus state to avoid native focus rect
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

        // kill native focus rect
        opt.state &= ~QStyle::State_HasFocus;

        const bool selected = opt.state & QStyle::State_Selected;
        const bool hover    = opt.state & QStyle::State_MouseOver;
        const bool pressed  = opt.state & QStyle::State_Sunken;

        QRect bg = opt.rect.adjusted(10, 4, -10, -4);
        if (selected || pressed) paintRounded(p, bg, bgSelected(), 14);
        else if (hover)          paintRounded(p, bg, bgHover(), 14);

        // Question + answer preview
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

    ui->wTopBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->wTopBar->setFixedHeight(56);

    // Tool buttons: stable geometry
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

    // Models
    m_folderModel = new rewise::ui::widgets::FolderListModel(this);
    m_folderModel->setIncludeAllItem(true);
    ui->lvFolders->setModel(m_folderModel);

    m_cardModel = new rewise::ui::widgets::CardTableModel(this);
    ui->tvCards->setModel(m_cardModel);

    // Folder list: rounded selection via delegate
    ui->lvFolders->setMouseTracking(true);
    ui->lvFolders->setUniformItemSizes(true);
    ui->lvFolders->setSpacing(4);
    ui->lvFolders->setItemDelegate(new FolderRowDelegate(ui->lvFolders));
    neutralizeNativeSelection(ui->lvFolders);

    // Cards list: list-like table view
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
    ui->tvCards->horizontalHeader()->setSectionResizeMode(rewise::ui::widgets::CardTableModel::QuestionCol, QHeaderView::Stretch);

    neutralizeNativeSelection(ui->tvCards);

    // Popup
    m_cardPopup = new rewise::ui::widgets::CardPopupDialog(this);

    wireUi();
}

LibraryPage::~LibraryPage() { delete ui; }

void LibraryPage::setNotifier(QObject* notify) { m_notify = notify; }

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
    if (m_notify && QMetaObject::invokeMethod(m_notify, "showError", Qt::QueuedConnection, Q_ARG(QString, text))) return;
    QMessageBox::critical(this, tr("Ошибка"), text);
}

void LibraryPage::showInfo(const QString& text) {
    if (m_notify && QMetaObject::invokeMethod(m_notify, "showInfo", Qt::QueuedConnection, Q_ARG(QString, text))) return;
    QMessageBox::information(this, tr("Информация"), text);
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
        bool ok = false;
        const QString name = QInputDialog::getText(this, tr("Новая папка"),
                                                   tr("Название папки:"), QLineEdit::Normal,
                                                   QString(), &ok);
        if (!ok) return;
        const QString trimmed = name.trimmed();
        if (trimmed.isEmpty()) { showError(tr("Название не может быть пустым.")); return; }
        emit folderCreateRequested(trimmed);
    });

    connect(ui->btnFolderMenu, &QToolButton::clicked, this, [this] {
        QMenu menu(this);
        QAction* actRename = menu.addAction(tr("Переименовать…"));
        QAction* actDelete = menu.addAction(tr("Удалить…"));

        const rewise::domain::Id fid = selectedFolderId();
        const bool enabled = fid.isValid();

        actRename->setEnabled(enabled);
        actDelete->setEnabled(enabled);

        QAction* chosen = menu.exec(ui->btnFolderMenu->mapToGlobal(QPoint(0, ui->btnFolderMenu->height())));
        if (!chosen) return;

        if (chosen == actRename) {
            const auto* f = m_db.folderById(fid);
            const QString current = f ? f->name : QString();

            bool ok = false;
            const QString name = QInputDialog::getText(this, tr("Переименовать папку"),
                                                       tr("Новое название:"), QLineEdit::Normal,
                                                       current, &ok);
            if (!ok) return;
            const QString trimmed = name.trimmed();
            if (trimmed.isEmpty()) { showError(tr("Название не может быть пустым.")); return; }
            emit folderRenameRequested(fid, trimmed);
        } else if (chosen == actDelete) {
            const auto res = QMessageBox::question(this, tr("Удалить папку"),
                                                   tr("Удалить папку? Карточки будут перенесены в Default."),
                                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (res != QMessageBox::Yes) return;
            emit folderDeleteRequested(fid);
        }
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
        if (!c) { showError(tr("Карточка не найдена.")); return; }
        m_cardPopup->card()->setCard(*c);
        m_cardPopup->openCentered();
    });

    connect(ui->btnStartReview, &QPushButton::clicked, this, [this] {
        emit startReviewRequested(selectedFolderId());
    });

    auto* cw = m_cardPopup->card();
    connect(cw, &rewise::ui::widgets::CardWidget::saveRequested, this,
            [this](const rewise::domain::Card& card) {
                const bool exists = (m_db.cardById(card.id) != nullptr);
                if (exists) emit cardUpdateRequested(card.id, card.question, card.answer);
                else emit cardCreateRequested(card.folderId, card.question, card.answer);
            });

    connect(cw, &rewise::ui::widgets::CardWidget::deleteRequested, this,
            [this](const rewise::domain::Id& id) {
                if (!id.isValid()) return;
                emit cardDeleteRequested(id);
            });

    connect(cw, &rewise::ui::widgets::CardWidget::closeRequested, this, [this] {
        m_cardPopup->hide();
    });
}

} // namespace rewise::ui::pages
