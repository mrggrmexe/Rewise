#include "ReviewPage.h"

#include "review/ReviewEngine.h"
#include "ui/widgets/DiffTextWidget.h"
#include "ui/widgets/InlineMessageWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRandomGenerator>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace rewise::ui::pages {

ReviewPage::ReviewPage(QWidget* parent)
    : QWidget(parent)
{
    buildUi();
    wireUi();
}

void ReviewPage::buildUi() {
    m_msg = new rewise::ui::widgets::InlineMessageWidget(this);

    m_back = new QPushButton("← Назад", this);
    m_title = new QLabel("Повторение", this);
    m_title->setStyleSheet("font-weight: 700;");

    auto* top = new QHBoxLayout();
    top->addWidget(m_back);
    top->addSpacing(10);
    top->addWidget(m_title, 1);

    m_question = new QTextBrowser(this);
    m_question->setReadOnly(true);
    m_question->setMinimumHeight(110);

    m_answer = new QPlainTextEdit(this);
    m_answer->setPlaceholderText("Введите ваш ответ…");
    m_answer->setMinimumHeight(90);
    m_answer->setTabChangesFocus(true);

    m_percent = new QLabel("", this);
    m_percent->setStyleSheet("font-size: 18px; font-weight: 700;");

    m_check = new QPushButton("Проверить", this);
    m_reveal = new QPushButton("Показать ответ", this);
    m_next = new QPushButton("Следующая", this);

    auto* btns = new QHBoxLayout();
    btns->addWidget(m_check);
    btns->addWidget(m_reveal);
    btns->addStretch(1);
    btns->addWidget(m_next);

    m_reference = new QTextBrowser(this);
    m_reference->setReadOnly(true);
    m_reference->setMinimumHeight(110);

    m_diff = new rewise::ui::widgets::DiffTextWidget(this);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);
    root->addLayout(top);
    root->addWidget(m_msg);
    root->addWidget(new QLabel("Вопрос:", this));
    root->addWidget(m_question);
    root->addWidget(new QLabel("Ваш ответ:", this));
    root->addWidget(m_answer);
    root->addLayout(btns);
    root->addWidget(m_percent);
    root->addWidget(new QLabel("Эталонный ответ:", this));
    root->addWidget(m_reference);
    root->addWidget(m_diff, 1);

    stopSession();
}

void ReviewPage::wireUi() {
    connect(m_back, &QPushButton::clicked, this, [this] {
        stopSession();
        emit exitRequested();
    });

    connect(m_check, &QPushButton::clicked, this, [this] {
        if (m_current < 0 || m_current >= m_cards.size()) return;
        const auto& card = m_cards[m_current];

        const QString user = m_answer->toPlainText();
        const auto res = rewise::review::ReviewEngine::evaluate(card.answer, user);

        m_checked = true;

        m_percent->setText(QString("Совпадение: %1%").arg(res.similarity.percent));
        m_diff->setReviewResult(res);

        // Если ответ ещё не раскрывали — оставим кнопку раскрытия
        if (!m_revealed) {
            m_reference->setHtml("<div style='opacity:0.7'>Нажмите “Показать ответ”.</div>");
        } else {
            m_reference->setHtml("<div style='white-space:pre-wrap;'>" + card.answer.toHtmlEscaped() + "</div>");
        }

        m_next->setEnabled(true);
    });

    connect(m_reveal, &QPushButton::clicked, this, [this] {
        if (m_current < 0 || m_current >= m_cards.size()) return;
        const auto& card = m_cards[m_current];
        m_revealed = true;
        m_reference->setHtml("<div style='white-space:pre-wrap;'>" + card.answer.toHtmlEscaped() + "</div>");
    });

    connect(m_next, &QPushButton::clicked, this, [this] {
        pickNextCard();
        showCard();
    });
}

void ReviewPage::startSession(QVector<rewise::domain::Card> cards, const QString& title) {
    m_cards = std::move(cards);
    m_titleText = title;

    m_msg->clearMessage();

    if (m_cards.isEmpty()) {
        m_title->setText("Повторение — пусто");
        m_question->setHtml("<div style='opacity:0.7'>В выбранной папке нет карточек.</div>");
        m_answer->setEnabled(false);
        m_check->setEnabled(false);
        m_reveal->setEnabled(false);
        m_next->setEnabled(false);
        return;
    }

    m_title->setText(QString("Повторение: %1").arg(m_titleText));
    m_answer->setEnabled(true);
    m_check->setEnabled(true);
    m_reveal->setEnabled(true);

    m_last = -1;
    pickNextCard();
    showCard();
}

void ReviewPage::stopSession() {
    m_cards.clear();
    m_titleText.clear();
    m_current = -1;
    m_last = -1;

    m_title->setText("Повторение");
    m_question->setHtml("<div style='opacity:0.7'>Запустите повторение из библиотеки.</div>");
    m_answer->clear();
    m_answer->setEnabled(false);

    m_check->setEnabled(false);
    m_reveal->setEnabled(false);
    m_next->setEnabled(false);

    clearResultUi();
}

void ReviewPage::pickNextCard() {
    if (m_cards.isEmpty()) {
        m_current = -1;
        return;
    }

    if (m_cards.size() == 1) {
        m_current = 0;
        return;
    }

    // Избегаем немедленного повтора
    int idx = m_last;
    for (int guard = 0; guard < 20 && idx == m_last; ++guard) {
        idx = static_cast<int>(QRandomGenerator::global()->bounded(m_cards.size()));
    }
    m_current = idx;
    m_last = idx;
}

void ReviewPage::clearResultUi() {
    m_checked = false;
    m_revealed = false;
    m_percent->clear();
    m_reference->setHtml("<div style='opacity:0.7'>Нажмите “Показать ответ”.</div>");
    m_diff->clear();
}

void ReviewPage::showCard() {
    clearResultUi();

    if (m_current < 0 || m_current >= m_cards.size()) return;

    const auto& card = m_cards[m_current];
    m_question->setHtml("<div style='white-space:pre-wrap;'>" + card.question.toHtmlEscaped() + "</div>");
    m_answer->clear();
    m_answer->setFocus();

    m_next->setEnabled(true);
}

} // namespace rewise::ui::pages
