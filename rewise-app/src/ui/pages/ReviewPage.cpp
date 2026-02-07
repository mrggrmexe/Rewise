#include "ReviewPage.h"
#include "ui_ReviewPage.h"

#include "ui/widgets/CardWidget.h"
#include "ui/widgets/NotificationCenter.h"

#include <QRandomGenerator>

namespace rewise::ui::pages {

ReviewPage::ReviewPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ReviewPage)
{
    ui->setupUi(this);

    // Start in "empty" state
    ui->stack->setCurrentWidget(ui->pageEmpty);

    // Create shared CardWidget for the session.
    m_cardWidget = new rewise::ui::widgets::CardWidget(ui->cardHost);
    m_cardWidget->setHeaderVisible(false);
    m_cardWidget->setCloseButtonVisible(false);
    m_cardWidget->setElevated(false);

    ui->cardHostLayout->addWidget(m_cardWidget);

    connect(ui->btnBack, &QToolButton::clicked, this, [this] {
        stopSession();
        emit exitRequested();
    });

    connect(m_cardWidget, &rewise::ui::widgets::CardWidget::nextRequested,
            this, &ReviewPage::pickAndShowNextCard);
}

ReviewPage::~ReviewPage() {
    delete ui;
}

void ReviewPage::setNotifier(rewise::ui::widgets::NotificationCenter* n) {
    m_notify = n;
}

void ReviewPage::startSession(const rewise::domain::Folder& folder,
                              const QVector<rewise::domain::Card>& cards) {
    m_folder = folder;
    m_cards = cards;
    m_index = -1;

    if (m_cards.isEmpty()) {
        ui->lblEmptyHint->setText(QStringLiteral("В этой папке пока нет карточек для повторения."));
        ui->stack->setCurrentWidget(ui->pageEmpty);

        if (m_notify) {
            m_notify->showInfo(QStringLiteral("В папке нет карточек."));
        }
        return;
    }

    ui->lblTitle->setText(QStringLiteral("Повторение · %1").arg(m_folder.name));
    ui->stack->setCurrentWidget(ui->pageCard);

    pickAndShowNextCard();
}

void ReviewPage::stopSession() {
    m_folder = {};
    m_cards.clear();
    m_index = -1;

    ui->lblTitle->setText(QStringLiteral("Повторение"));
    ui->lblEmptyHint->setText(QStringLiteral("Выберите папку в библиотеке и нажмите «Повторять»."));
    ui->stack->setCurrentWidget(ui->pageEmpty);
}

void ReviewPage::pickAndShowNextCard() {
    if (m_cards.isEmpty()) return;

    // Random walk: prevents "same card repeat" a bit.
    int next = QRandomGenerator::global()->bounded(m_cards.size());
    if (m_cards.size() > 1 && next == m_index) {
        next = (next + 1) % m_cards.size();
    }

    m_index = next;
    m_cardWidget->setReviewCard(m_cards[m_index]);
}

} // namespace rewise::ui::pages
