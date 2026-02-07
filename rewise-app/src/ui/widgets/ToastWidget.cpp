#include "ToastWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QStyle>

namespace rewise::ui::widgets {

ToastWidget::ToastWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setProperty("toast", true);
    setKind(Kind::Info);

    m_label = new QLabel(this);
    m_label->setWordWrap(true);

    m_btnClose = new QToolButton(this);
    m_btnClose->setAutoRaise(true);
    m_btnClose->setFocusPolicy(Qt::NoFocus);
    m_btnClose->setText(QStringLiteral("✕"));
    m_btnClose->setToolTip(QStringLiteral("Закрыть"));

    m_btnYes = new QPushButton(QStringLiteral("Да"), this);
    m_btnNo  = new QPushButton(QStringLiteral("Нет"), this);

    m_btnYes->setProperty("primary", true);

    connect(m_btnClose, &QToolButton::clicked, this, &ToastWidget::closeClicked);
    connect(m_btnYes, &QPushButton::clicked, this, &ToastWidget::accepted);
    connect(m_btnNo,  &QPushButton::clicked, this, &ToastWidget::rejected);

    rebuildUiForKind();
}

void ToastWidget::setKind(Kind k) {
    if (m_kind == k) return;
    m_kind = k;

    switch (m_kind) {
        case Kind::Info:
            setProperty("kind", "info");
            break;
        case Kind::Error:
            setProperty("kind", "error");
            break;
        case Kind::Confirm:
            setProperty("kind", "confirm");
            break;
    }

    style()->unpolish(this);
    style()->polish(this);

    rebuildUiForKind();
}

void ToastWidget::setText(const QString& text) {
    m_label->setText(text);
}

QString ToastWidget::text() const {
    return m_label->text();
}

void ToastWidget::setConfirmButtons(const QString& yesText, const QString& noText) {
    m_btnYes->setText(yesText);
    m_btnNo->setText(noText);
}

void ToastWidget::rebuildUiForKind() {
    if (layout()) {
        delete layout();
    }

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(12, 10, 10, 10);
    root->setSpacing(10);

    root->addWidget(m_label, 1);

    if (m_kind == Kind::Confirm) {
        auto* btns = new QHBoxLayout();
        btns->setContentsMargins(0, 0, 0, 0);
        btns->setSpacing(8);
        btns->addWidget(m_btnYes);
        btns->addWidget(m_btnNo);
        root->addLayout(btns);
    }

    root->addWidget(m_btnClose, 0, Qt::AlignTop);
    setLayout(root);

    m_btnYes->setVisible(m_kind == Kind::Confirm);
    m_btnNo->setVisible(m_kind == Kind::Confirm);
}

} // namespace rewise::ui::widgets
