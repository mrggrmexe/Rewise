#include "CardWidget.h"

#include "review/ReviewEngine.h"
#include "ui/widgets/DiffTextWidget.h"

#include <QFrame>
#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QToolButton>
#include <QVBoxLayout>

namespace rewise::ui::widgets {

static QTextBrowser* makeFlatTextBrowser(QWidget* parent) {
    auto* tb = new QTextBrowser(parent);
    tb->setOpenExternalLinks(false);
    tb->setReadOnly(true);
    tb->setFrameShape(QFrame::NoFrame);
    tb->setProperty("flat", true);
    tb->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    return tb;
}

CardWidget::CardWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setProperty("card", true);

    buildUi();
    setCloseButtonVisible(false);
    setElevated(false);

    setMode(Mode::LibraryView);
    clearReview();
}

void CardWidget::setElevated(bool elevated) {
    if (m_elevated == elevated) return;
    m_elevated = elevated;

    if (!m_elevated) {
        setGraphicsEffect(nullptr);
        return;
    }

    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(26);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(20, 30, 55, 120));
    setGraphicsEffect(shadow);
}

void CardWidget::setHeaderVisible(bool visible) {
    if (m_header) m_header->setVisible(visible);
}

void CardWidget::setCloseButtonVisible(bool visible) {
    if (!m_btnClose) return;
    m_btnClose->setVisible(visible);
}

void CardWidget::setHeaderTitle(const QString& title) {
    if (m_headerTitle) m_headerTitle->setText(title);
}

void CardWidget::setMode(Mode m) {
    m_mode = m;
    switch (m_mode) {
        case Mode::LibraryView:
            enterLibraryViewMode();
            break;
        case Mode::LibraryEdit:
            if (m_stack) m_stack->setCurrentWidget(m_editPage);
            setHeaderTitle(m_cardId.isValid() ? QStringLiteral("Карточка") : QStringLiteral("Новая карточка"));
            break;
        case Mode::Review:
            if (m_stack) m_stack->setCurrentWidget(m_reviewPage);
            setHeaderTitle(QStringLiteral("Карточка"));
            break;
    }
}

void CardWidget::setCard(const rewise::domain::Card& card) {
    m_cardId = card.id;
    m_folderCtx = card.folderId;
    m_question = card.question;
    m_answer = card.answer;

    applyCardToUi();
    setMode(Mode::LibraryView);
}

void CardWidget::startEdit(const rewise::domain::Card& card) {
    m_cardId = card.id;
    m_folderCtx = card.folderId;
    m_question = card.question;
    m_answer = card.answer;

    applyCardToUi();
    enterLibraryEditMode(false);
}

void CardWidget::startCreate(const rewise::domain::Id& folderContext) {
    m_cardId = {};
    m_folderCtx = folderContext;
    m_question.clear();
    m_answer.clear();

    applyCardToUi();
    enterLibraryEditMode(true);
}

void CardWidget::setReviewCard(const rewise::domain::Card& card) {
    m_cardId = card.id;
    m_folderCtx = card.folderId;
    m_question = card.question;
    m_answer = card.answer;

    applyCardToUi();
    resetReviewUi();
    setMode(Mode::Review);

    if (m_reviewInput) {
        m_reviewInput->setFocus();
    }
}

void CardWidget::clearReview() {
    m_reviewChecked = false;
    if (m_reviewInput) m_reviewInput->clear();

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

void CardWidget::resetReviewUi() {
    clearReview();

    if (m_reviewInput) {
        m_reviewInput->setEnabled(true);
        m_reviewInput->setPlaceholderText(QStringLiteral("Введите ответ…"));
    }

    if (m_btnCheck) m_btnCheck->setEnabled(true);
    if (m_btnNext) m_btnNext->setEnabled(false);
}

void CardWidget::applyCardToUi() {
    const QString qHtml = "<div style='white-space:pre-wrap;'>" + m_question.toHtmlEscaped() + "</div>";
    const QString aHtml = "<div style='white-space:pre-wrap;'>" + m_answer.toHtmlEscaped() + "</div>";

    if (m_viewQuestion) m_viewQuestion->setHtml(qHtml);
    if (m_viewAnswer) m_viewAnswer->setHtml(aHtml);

    if (m_leQuestion) m_leQuestion->setText(m_question);
    if (m_pteAnswer) m_pteAnswer->setPlainText(m_answer);

    if (m_reviewQuestion) m_reviewQuestion->setHtml(qHtml);
}

void CardWidget::enterLibraryViewMode() {
    m_mode = Mode::LibraryView;

    if (m_stack) m_stack->setCurrentWidget(m_viewPage);
    setHeaderTitle(QStringLiteral("Карточка"));

    if (m_btnEdit) m_btnEdit->setEnabled(m_cardId.isValid());
    if (m_btnDelete) m_btnDelete->setEnabled(m_cardId.isValid());
}

void CardWidget::enterLibraryEditMode(bool isCreate) {
    m_mode = Mode::LibraryEdit;

    if (m_stack) m_stack->setCurrentWidget(m_editPage);
    if (m_editTitle) {
        m_editTitle->setText(isCreate ? QStringLiteral("Новая карточка") : QStringLiteral("Редактирование"));
    }
    setHeaderTitle(isCreate ? QStringLiteral("Новая карточка") : QStringLiteral("Карточка"));

    if (m_leQuestion) m_leQuestion->setFocus();
}

void CardWidget::onEdit() {
    if (!m_cardId.isValid()) return;
    enterLibraryEditMode(false);
}

void CardWidget::onDelete() {
    if (!m_cardId.isValid()) return;
    emit deleteRequested(m_cardId);
}

void CardWidget::onSave() {
    const QString q = m_leQuestion ? m_leQuestion->text().trimmed() : QString();
    const QString a = m_pteAnswer ? m_pteAnswer->toPlainText().trimmed() : QString();

    if (q.isEmpty() || a.isEmpty()) {
        return;
    }

    emit saveRequested(m_cardId, m_folderCtx, q, a);

    m_question = q;
    m_answer = a;
    applyCardToUi();
    enterLibraryViewMode();
}

void CardWidget::onCancel() {
    if (m_leQuestion) m_leQuestion->setText(m_question);
    if (m_pteAnswer) m_pteAnswer->setPlainText(m_answer);

    if (m_cardId.isValid()) {
        enterLibraryViewMode();
    } else {
        emit closeRequested();
    }
}

void CardWidget::onCheck() {
    if (m_mode != Mode::Review) return;

    const QString user = m_reviewInput ? m_reviewInput->toPlainText() : QString();
    const auto res = rewise::review::ReviewEngine::evaluate(m_answer, user);

    m_reviewChecked = true;

    if (m_lblPercent) {
        m_lblPercent->setText(QStringLiteral("Совпадение: %1%").arg(res.similarity.percent));
        m_lblPercent->setVisible(true);
    }
    if (m_diff) {
        m_diff->setReviewResult(res);
        m_diff->setVisible(true);
    }

    if (m_btnNext) m_btnNext->setEnabled(true);
}

void CardWidget::onNext() {
    emit nextRequested();
}

void CardWidget::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 14, 16, 14);
    root->setSpacing(12);

    // Header: title + close
    m_header = new QWidget(this);
    m_header->setAttribute(Qt::WA_StyledBackground, false);
    auto* head = new QHBoxLayout(m_header);
    head->setContentsMargins(0, 0, 0, 0);
    head->setSpacing(10);

    m_headerTitle = new QLabel(QStringLiteral("Карточка"), m_header);
    m_headerTitle->setProperty("heading", true);

    m_btnClose = new QToolButton(m_header);
    m_btnClose->setAutoRaise(true);
    m_btnClose->setFocusPolicy(Qt::NoFocus);
    m_btnClose->setText(QStringLiteral("✕"));
    m_btnClose->setToolTip(QStringLiteral("Закрыть"));

    head->addWidget(m_headerTitle, 1);
    head->addWidget(m_btnClose, 0, Qt::AlignTop);

    connect(m_btnClose, &QToolButton::clicked, this, &CardWidget::closeRequested);

    root->addWidget(m_header);

    // Stack
    m_stack = new QStackedWidget(this);
    root->addWidget(m_stack, 1);

    // -------- Library View --------
    m_viewPage = new QWidget(m_stack);
    auto* v = new QVBoxLayout(m_viewPage);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(10);

    auto* lblQ = new QLabel(QStringLiteral("Вопрос"), m_viewPage);
    lblQ->setProperty("muted", true);

    m_viewQuestion = makeFlatTextBrowser(m_viewPage);

    auto* lblA = new QLabel(QStringLiteral("Ответ"), m_viewPage);
    lblA->setProperty("muted", true);

    m_viewAnswer = makeFlatTextBrowser(m_viewPage);

    auto* actions = new QHBoxLayout();
    actions->setContentsMargins(0, 0, 0, 0);
    actions->setSpacing(10);
    actions->addStretch();

    m_btnEdit = new QPushButton(QStringLiteral("Редактировать"), m_viewPage);
    m_btnDelete = new QPushButton(QStringLiteral("Удалить"), m_viewPage);
    m_btnDelete->setProperty("destructive", true);

    actions->addWidget(m_btnEdit);
    actions->addWidget(m_btnDelete);

    connect(m_btnEdit, &QPushButton::clicked, this, &CardWidget::onEdit);
    connect(m_btnDelete, &QPushButton::clicked, this, &CardWidget::onDelete);

    v->addWidget(lblQ);
    v->addWidget(m_viewQuestion, 0);
    v->addWidget(lblA);
    v->addWidget(m_viewAnswer, 1);
    v->addLayout(actions);

    m_stack->addWidget(m_viewPage);

    // -------- Library Edit --------
    m_editPage = new QWidget(m_stack);
    auto* e = new QVBoxLayout(m_editPage);
    e->setContentsMargins(0, 0, 0, 0);
    e->setSpacing(10);

    m_editTitle = new QLabel(QStringLiteral("Редактирование"), m_editPage);
    m_editTitle->setProperty("heading", true);

    auto* lblQ2 = new QLabel(QStringLiteral("Вопрос"), m_editPage);
    lblQ2->setProperty("muted", true);

    m_leQuestion = new QLineEdit(m_editPage);
    m_leQuestion->setPlaceholderText(QStringLiteral("Например: Что такое ...?"));

    auto* lblA2 = new QLabel(QStringLiteral("Ответ"), m_editPage);
    lblA2->setProperty("muted", true);

    m_pteAnswer = new QPlainTextEdit(m_editPage);
    m_pteAnswer->setPlaceholderText(QStringLiteral("Например: ..."));

    auto* eActions = new QHBoxLayout();
    eActions->setContentsMargins(0, 0, 0, 0);
    eActions->setSpacing(10);
    eActions->addStretch();

    m_btnCancel = new QPushButton(QStringLiteral("Отмена"), m_editPage);
    m_btnSave = new QPushButton(QStringLiteral("Сохранить"), m_editPage);
    m_btnSave->setProperty("primary", true);

    eActions->addWidget(m_btnCancel);
    eActions->addWidget(m_btnSave);

    connect(m_btnCancel, &QPushButton::clicked, this, &CardWidget::onCancel);
    connect(m_btnSave, &QPushButton::clicked, this, &CardWidget::onSave);

    // Enable Save only when both fields are non-empty.
    auto updateSaveEnabled = [this] {
        const bool okQ = !m_leQuestion->text().trimmed().isEmpty();
        const bool okA = !m_pteAnswer->toPlainText().trimmed().isEmpty();
        m_btnSave->setEnabled(okQ && okA);
    };
    connect(m_leQuestion, &QLineEdit::textChanged, this, [updateSaveEnabled](const QString&) { updateSaveEnabled(); });
    connect(m_pteAnswer, &QPlainTextEdit::textChanged, this, updateSaveEnabled);
    updateSaveEnabled();

    e->addWidget(m_editTitle);
    e->addWidget(lblQ2);
    e->addWidget(m_leQuestion);
    e->addWidget(lblA2);
    e->addWidget(m_pteAnswer, 1);
    e->addLayout(eActions);

    m_stack->addWidget(m_editPage);

    // -------- Review --------
    m_reviewPage = new QWidget(m_stack);
    auto* r = new QVBoxLayout(m_reviewPage);
    r->setContentsMargins(0, 0, 0, 0);
    r->setSpacing(10);

    auto* lblQ3 = new QLabel(QStringLiteral("Вопрос"), m_reviewPage);
    lblQ3->setProperty("muted", true);

    m_reviewQuestion = makeFlatTextBrowser(m_reviewPage);

    auto* lblYour = new QLabel(QStringLiteral("Ваш ответ"), m_reviewPage);
    lblYour->setProperty("muted", true);

    m_reviewInput = new QPlainTextEdit(m_reviewPage);
    m_reviewInput->setPlaceholderText(QStringLiteral("Введите ответ…"));

    auto* rActions = new QHBoxLayout();
    rActions->setContentsMargins(0, 0, 0, 0);
    rActions->setSpacing(10);
    rActions->addStretch();

    m_btnCheck = new QPushButton(QStringLiteral("Проверить"), m_reviewPage);
    m_btnCheck->setProperty("primary", true);
    m_btnNext = new QPushButton(QStringLiteral("Следующая"), m_reviewPage);

    rActions->addWidget(m_btnCheck);
    rActions->addWidget(m_btnNext);

    connect(m_btnCheck, &QPushButton::clicked, this, &CardWidget::onCheck);
    connect(m_btnNext, &QPushButton::clicked, this, &CardWidget::onNext);

    m_lblPercent = new QLabel(m_reviewPage);
    m_lblPercent->setProperty("muted", true);

    m_diff = new DiffTextWidget(m_reviewPage);
    m_diff->setFramed(false);

    r->addWidget(lblQ3);
    r->addWidget(m_reviewQuestion, 0);
    r->addWidget(lblYour);
    r->addWidget(m_reviewInput, 1);
    r->addLayout(rActions);
    r->addWidget(m_lblPercent);
    r->addWidget(m_diff, 1);

    m_stack->addWidget(m_reviewPage);
}

} // namespace rewise::ui::widgets
