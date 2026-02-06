#include "ReviewPage.h"
#include "ui_ReviewPage.h"

#include "review/ReviewEngine.h"
#include "ui/widgets/DiffTextWidget.h"
#include "ui/widgets/InlineMessageWidget.h"

#include <QRandomGenerator>
#include <QVBoxLayout>

namespace rewise::ui::pages {

ReviewPage::ReviewPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ReviewPage)
{
    ui->setupUi(this);
    ui->btnCheck->setProperty("primary", true);
    ui->btnNext->setProperty("primary", true);


    // Встраиваем баннер сообщений (как в твоём ручном UI)
    // Он не в .ui: добавляем сверху, чтобы не ломать Designer.
    m_msg = new rewise::ui::widgets::InlineMessageWidget(this);
    ui->rootLayout->insertWidget(1, m_msg); // после topBar

    // Встраиваем DiffTextWidget внутрь diffHost
    m_diff = new rewise::ui::widgets::DiffTextWidget(ui->diffHost);
    if (!ui->diffHost->layout()) {
        auto* l = new QVBoxLayout(ui->diffHost);
        l->setContentsMargins(0, 0, 0, 0);
        ui->diffHost->setLayout(l);
    }
    ui->diffHost->layout()->addWidget(m_diff);

    wireUi();
    stopSession();
}

ReviewPage::~ReviewPage() {
    delete ui;
}

void ReviewPage::wireUi() {
    connect(ui->btnBack, &QPushButton::clicked, this, [this] {
        stopSession();
        emit exitRequested();
    });

    connect(ui->btnCheck, &QPushButton::clicked, this, [this] {
        if (m_current < 0 || m_current >= m_cards.size()) return;
        const auto& card = m_cards[m_current];

        const QString user = ui->pteAnswer->toPlainText();
        const auto res = rewise::review::ReviewEngine::evaluate(card.answer, user);

        m_checked = true;

        ui->lblPercent->setText(QString("Совпадение: %1%").arg(res.similarity.percent));
        m_diff->setReviewResult(res);

        if (!m_revealed) {
            ui->tbReference->setHtml("<div style='opacity:0.7'>Нажмите “Показать ответ”.</div>");
        } else {
            ui->tbReference->setHtml("<div style='white-space:pre-wrap;'>" + card.answer.toHtmlEscaped() + "</div>");
        }

        ui->btnNext->setEnabled(true);
    });

    connect(ui->btnReveal, &QPushButton::clicked, this, [this] {
        if (m_current < 0 || m_current >= m_cards.size()) return;
        const auto& card = m_cards[m_current];
        m_revealed = true;
        ui->tbReference->setHtml("<div style='white-space:pre-wrap;'>" + card.answer.toHtmlEscaped() + "</div>");
    });

    connect(ui->btnNext, &QPushButton::clicked, this, [this] {
        pickNextCard();
        showCard();
    });
}

void ReviewPage::startSession(QVector<rewise::domain::Card> cards, const QString& title) {
    m_cards = std::move(cards);
    m_titleText = title;

    if (m_msg) m_msg->clearMessage();

    if (m_cards.isEmpty()) {
        ui->lblTitle->setText("Повторение — пусто");
        ui->tbQuestion->setHtml("<div style='opacity:0.7'>В выбранной папке нет карточек.</div>");
        ui->pteAnswer->setEnabled(false);
        ui->btnCheck->setEnabled(false);
        ui->btnReveal->setEnabled(false);
        ui->btnNext->setEnabled(false);
        return;
    }

    ui->lblTitle->setText(QString("Повторение: %1").arg(m_titleText));
    ui->pteAnswer->setEnabled(true);
    ui->btnCheck->setEnabled(true);
    ui->btnReveal->setEnabled(true);

    m_last = -1;
    pickNextCard();
    showCard();
}

void ReviewPage::stopSession() {
    m_cards.clear();
    m_titleText.clear();
    m_current = -1;
    m_last = -1;

    ui->lblTitle->setText("Повторение");
    ui->tbQuestion->setHtml("<div style='opacity:0.7'>Запустите повторение из библиотеки.</div>");
    ui->pteAnswer->clear();
    ui->pteAnswer->setEnabled(false);

    ui->btnCheck->setEnabled(false);
    ui->btnReveal->setEnabled(false);
    ui->btnNext->setEnabled(false);

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
    ui->lblPercent->clear();
    ui->tbReference->setHtml("<div style='opacity:0.7'>Нажмите “Показать ответ”.</div>");
    if (m_diff) m_diff->clear();
}

void ReviewPage::showCard() {
    clearResultUi();
    if (m_current < 0 || m_current >= m_cards.size()) return;

    const auto& card = m_cards[m_current];
    ui->tbQuestion->setHtml("<div style='white-space:pre-wrap;'>" + card.question.toHtmlEscaped() + "</div>");
    ui->pteAnswer->clear();
    ui->pteAnswer->setFocus();

    ui->btnNext->setEnabled(true);
}

} // namespace rewise::ui::pages
