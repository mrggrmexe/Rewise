#include "CardWidget.h"

#include "review/ReviewEngine.h"
#include "ui/widgets/DiffTextWidget.h"

#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

namespace rewise::ui::widgets {

static QString clipOneLine(QString s) {
    s.replace('\n', ' ');
    s = s.simplified();
    if (s.size() > 42) s = s.left(42) + QStringLiteral("…");
    return s;
}

CardWidget::CardWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("CardWidget");
    setAttribute(Qt::WA_StyledBackground, true);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(12);

    // Header
    m_headerWidget = new QWidget(this);
    auto* header = new QHBoxLayout(m_headerWidget);
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(10);

    m_title = new QLabel(tr("Карточка"), m_headerWidget);
    QFont tf = m_title->font();
    tf.setPointSize(14);
    tf.setBold(true);
    m_title->setFont(tf);

    m_btnClose = new QToolButton(m_headerWidget);
    m_btnClose->setText(QStringLiteral("✕"));
    m_btnClose->setAutoRaise(true);
    m_btnClose->setCursor(Qt::PointingHandCursor);

    header->addWidget(m_title);
    header->addStretch(1);
    header->addWidget(m_btnClose);

    root->addWidget(m_headerWidget);

    connect(m_btnClose, &QToolButton::clicked, this, [this] { emit closeRequested(); });

    // Stack: editor/view vs review
    m_stack = new QStackedWidget(this);
    root->addWidget(m_stack, 1);

    // Page: Library (View/Edit/Create) - based on splitter
    auto* pageLibrary = new QWidget(m_stack);
    auto* libL = new QVBoxLayout(pageLibrary);
    libL->setContentsMargins(0, 0, 0, 0);
    libL->setSpacing(12);

    m_split = new QSplitter(Qt::Horizontal, pageLibrary);

    auto* left = new QWidget(m_split);
    auto* leftL = new QVBoxLayout(left);
    leftL->setContentsMargins(0, 0, 0, 0);
    leftL->setSpacing(6);

    m_lblQuestion = new QLabel(tr("Вопрос"), left);
    QFont lf = m_lblQuestion->font();
    lf.setBold(true);
    m_lblQuestion->setFont(lf);

    m_question = new QPlainTextEdit(left);
    m_question->setPlaceholderText(tr("Введите вопрос…"));
    m_question->setTabChangesFocus(true);

    leftL->addWidget(m_lblQuestion);
    leftL->addWidget(m_question);

    auto* right = new QWidget(m_split);
    auto* rightL = new QVBoxLayout(right);
    rightL->setContentsMargins(0, 0, 0, 0);
    rightL->setSpacing(6);

    m_lblAnswer = new QLabel(tr("Ответ"), right);
    m_lblAnswer->setFont(lf);

    m_answer = new QPlainTextEdit(right);
    m_answer->setPlaceholderText(tr("Введите эталонный ответ…"));
    m_answer->setTabChangesFocus(true);

    rightL->addWidget(m_lblAnswer);
    rightL->addWidget(m_answer);

    m_split->addWidget(left);
    m_split->addWidget(right);
    m_split->setStretchFactor(0, 1);
    m_split->setStretchFactor(1, 1);

    libL->addWidget(m_split, 1);

    // Footer (library buttons)
    m_footerWidget = new QWidget(pageLibrary);
    auto* footer = new QHBoxLayout(m_footerWidget);
    footer->setContentsMargins(0, 0, 0, 0);
    footer->setSpacing(10);

    m_btnDelete = new QPushButton(tr("Удалить"), m_footerWidget);
    m_btnEdit = new QPushButton(tr("Редактировать"), m_footerWidget);
    m_btnSave = new QPushButton(tr("Сохранить"), m_footerWidget);
    m_btnCancel = new QPushButton(tr("Отмена"), m_footerWidget);

    footer->addWidget(m_btnDelete);
    footer->addStretch(1);
    footer->addWidget(m_btnEdit);
    footer->addWidget(m_btnCancel);
    footer->addWidget(m_btnSave);

    libL->addWidget(m_footerWidget);

    connect(m_btnCancel, &QPushButton::clicked, this, [this] {
        if (m_mode == Mode::Create) {
            emit closeRequested();
            return;
        }
        setMode(Mode::View);
        applyCardToEditors();
        rebuildUiForMode();
    });

    connect(m_btnEdit, &QPushButton::clicked, this, [this] {
        setMode(Mode::Edit);
        applyCardToEditors();
        rebuildUiForMode();
    });

    connect(m_btnSave, &QPushButton::clicked, this, [this] {
        collectEditorsToCard();
        if (!m_card.id.isValid()) m_card.id = rewise::domain::Id::create();
        m_card.touchUpdatedNow();
        emit saveRequested(m_card);
        setMode(Mode::View);
        rebuildUiForMode();
    });

    connect(m_btnDelete, &QPushButton::clicked, this, [this] {
        if (m_card.id.isValid()) emit deleteRequested(m_card.id);
        emit closeRequested();
    });

    // Page: Review
    m_reviewPage = new QWidget(m_stack);
    auto* r = new QVBoxLayout(m_reviewPage);
    r->setContentsMargins(0, 0, 0, 0);
    r->setSpacing(10);

    // In review: show question (read-only) on left/top, input instead of answer, diff+percent below
    m_lblYourAnswer = new QLabel(tr("Ваш ответ"), m_reviewPage);
    QFont rf = m_lblYourAnswer->font();
    rf.setBold(true);
    m_lblYourAnswer->setFont(rf);

    m_input = new QPlainTextEdit(m_reviewPage);
    m_input->setPlaceholderText(tr("Введите ответ…"));
    m_input->setTabChangesFocus(true);

    m_lblPercent = new QLabel(m_reviewPage);
    m_lblPercent->setVisible(false);

    m_diff = new DiffTextWidget(m_reviewPage);
    m_diff->setVisible(false);

    m_reviewButtons = new QWidget(m_reviewPage);
    auto* rb = new QHBoxLayout(m_reviewButtons);
    rb->setContentsMargins(0, 0, 0, 0);
    rb->setSpacing(10);

    rb->addStretch(1);
    m_btnCheck = new QPushButton(tr("Проверить"), m_reviewButtons);
    m_btnNext = new QPushButton(tr("Следующая"), m_reviewButtons);

    rb->addWidget(m_btnCheck);
    rb->addWidget(m_btnNext);

    connect(m_btnCheck, &QPushButton::clicked, this, [this] { doCheck(); });
    connect(m_btnNext, &QPushButton::clicked, this, [this] { emit nextRequested(); });

    // Reuse left side (question) from splitter: but in review we don’t want split; simplest: keep question editor read-only.
    // We'll use the same m_question as read-only question view inside review too:
    // To avoid duplicating widgets, we’ll show a read-only question block using the existing question editor.
    // We'll place it at top of review page:
    auto* lblQ = new QLabel(tr("Вопрос"), m_reviewPage);
    lblQ->setFont(rf);

    auto* qView = new QPlainTextEdit(m_reviewPage);
    qView->setReadOnly(true);
    qView->setObjectName("reviewQuestion");
    qView->setMinimumHeight(90);

    // Keep qView synced with m_card.question
    // (we update it in setReviewCard)
    qView->setPlainText(QString());

    r->addWidget(lblQ);
    r->addWidget(qView);
    r->addWidget(m_lblYourAnswer);
    r->addWidget(m_input, 1);
    r->addWidget(m_reviewButtons);
    r->addWidget(m_lblPercent);
    r->addWidget(m_diff, 1);

    // Store qView pointer via dynamic property so we don't add new member:
    m_reviewPage->setProperty("_qview_ptr", QVariant::fromValue<void*>(qView));

    m_stack->addWidget(pageLibrary);
    m_stack->addWidget(m_reviewPage);

    // Defaults
    setHeaderVisible(true);
    setCloseButtonVisible(true);
    setElevated(false);
    setWideLayout(true);

    m_card = rewise::domain::Card{};
    setMode(Mode::View);
    rebuildUiForMode();
}

void CardWidget::setHeaderVisible(bool v) {
    if (m_headerWidget) m_headerWidget->setVisible(v);
}

void CardWidget::setCloseButtonVisible(bool v) {
    if (m_btnClose) m_btnClose->setVisible(v);
}

void CardWidget::setElevated(bool v) {
    if (m_elevated == v) return;
    m_elevated = v;

    if (!v) {
        setGraphicsEffect(nullptr);
        return;
    }

    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(32);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 85));
    setGraphicsEffect(shadow);
}

void CardWidget::setWideLayout(bool enabled) {
    if (m_split) m_split->setOrientation(enabled ? Qt::Horizontal : Qt::Vertical);
}

void CardWidget::setMode(Mode m) {
    m_mode = m;
    if (m_stack) {
        if (m_mode == Mode::Review) m_stack->setCurrentWidget(m_reviewPage);
        else m_stack->setCurrentIndex(0);
    }
}

void CardWidget::startCreate(const rewise::domain::Id& folderId) {
    m_card = rewise::domain::Card{};
    m_card.id = rewise::domain::Id::create();
    m_card.folderId = folderId;
    m_card.touchCreatedNow();

    setMode(Mode::Create);
    applyCardToEditors();
    rebuildUiForMode();
}

void CardWidget::setCard(const rewise::domain::Card& card) {
    m_card = card;
    setMode(Mode::View);
    applyCardToEditors();
    rebuildUiForMode();
}

void CardWidget::setReviewCard(const rewise::domain::Card& card) {
    m_card = card;
    setMode(Mode::Review);

    // Fill question view
    void* ptr = m_reviewPage->property("_qview_ptr").value<void*>();
    auto* qView = static_cast<QPlainTextEdit*>(ptr);
    if (qView) qView->setPlainText(m_card.question);

    resetReviewUi();
    rebuildUiForMode();

    if (m_input) m_input->setFocus();
}

void CardWidget::rebuildUiForMode() {
    const bool editable = (m_mode == Mode::Edit || m_mode == Mode::Create);
    const bool inReview = (m_mode == Mode::Review);

    // Library editor widgets
    if (m_question) {
        m_question->setReadOnly(!editable);
        m_question->setVisible(!inReview);
    }
    if (m_answer) {
        m_answer->setReadOnly(!editable);
        m_answer->setVisible(!inReview);
    }
    if (m_lblQuestion) m_lblQuestion->setVisible(!inReview);
    if (m_lblAnswer) m_lblAnswer->setVisible(!inReview);

    if (m_split) m_split->setVisible(!inReview);
    if (m_footerWidget) m_footerWidget->setVisible(!inReview);

    // Review page is handled by stack (visible only in review)

    if (m_mode == Mode::Create) {
        m_title->setText(tr("Новая карточка"));
    } else if (m_mode == Mode::Edit) {
        m_title->setText(tr("Редактирование"));
    } else if (m_mode == Mode::Review) {
        m_title->setText(tr("Карточка"));
    } else {
        const QString q = clipOneLine(m_card.question);
        m_title->setText(q.isEmpty() ? tr("Карточка") : q);
    }

    // Buttons visibility
    if (m_btnSave) m_btnSave->setVisible(editable);
    if (m_btnCancel) m_btnCancel->setVisible(editable);

    if (m_btnEdit) m_btnEdit->setVisible(!editable && !inReview);
    if (m_btnDelete) m_btnDelete->setVisible(!editable && !inReview && m_card.id.isValid());
}

void CardWidget::applyCardToEditors() {
    if (m_question) m_question->setPlainText(m_card.question);
    if (m_answer) m_answer->setPlainText(m_card.answer);
}

void CardWidget::collectEditorsToCard() {
    if (m_question) m_card.question = m_question->toPlainText().trimmed();
    if (m_answer) m_card.answer = m_answer->toPlainText().trimmed();
}

void CardWidget::resetReviewUi() {
    if (m_input) m_input->clear();
    if (m_lblPercent) {
        m_lblPercent->clear();
        m_lblPercent->setVisible(false);
    }
    if (m_diff) {
        m_diff->clear();
        m_diff->setVisible(false);
    }
    if (m_btnNext) m_btnNext->setEnabled(false);
}

void CardWidget::doCheck() {
    const QString user = m_input ? m_input->toPlainText() : QString{};
    const auto res = rewise::review::ReviewEngine::evaluate(m_card.answer, user);

    if (m_lblPercent) {
        m_lblPercent->setText(tr("Совпадение: %1%").arg(res.similarity.percent));
        m_lblPercent->setVisible(true);
    }
    if (m_diff) {
        m_diff->setReviewResult(res);
        m_diff->setVisible(true);
    }
    if (m_btnNext) m_btnNext->setEnabled(true);
}

} // namespace rewise::ui::widgets