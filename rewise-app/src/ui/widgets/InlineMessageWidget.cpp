#include "InlineMessageWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

namespace rewise::ui::widgets {

InlineMessageWidget::InlineMessageWidget(QWidget* parent)
    : QWidget(parent)
{
    setVisible(false);

    m_label = new QLabel(this);
    m_label->setWordWrap(true);

    m_close = new QToolButton(this);
    m_close->setText("✕");
    m_close->setAutoRaise(true);
    m_close->setToolTip("Скрыть сообщение");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(8);
    layout->addWidget(m_label, 1);
    layout->addWidget(m_close, 0, Qt::AlignTop);

    connect(m_close, &QToolButton::clicked, this, [this] {
        clearMessage();
        emit dismissed();
    });
}

void InlineMessageWidget::applyKindStyle(Kind kind) {
    // Нейтральные “безопасные” стили: везде читаемо, без зависимости от тем.
    QString border;
    QString bg;
    QString fg = "color: #111;";

    switch (kind) {
        case Kind::Info:
            border = "border: 1px solid rgba(60, 120, 180, 0.35);";
            bg = "background: rgba(60, 120, 180, 0.10);";
            break;
        case Kind::Warning:
            border = "border: 1px solid rgba(200, 140, 0, 0.35);";
            bg = "background: rgba(200, 140, 0, 0.12);";
            break;
        case Kind::Error:
            border = "border: 1px solid rgba(200, 60, 60, 0.35);";
            bg = "background: rgba(200, 60, 60, 0.10);";
            break;
    }

    setStyleSheet(QString("border-radius: 8px; %1 %2 %3").arg(border, bg, fg));
}

void InlineMessageWidget::showMessage(Kind kind, const QString& text) {
    m_label->setText(text);
    applyKindStyle(kind);
    m_hasMessage = true;
    setVisible(true);
}

void InlineMessageWidget::clearMessage() {
    m_label->clear();
    m_hasMessage = false;
    setVisible(false);
}

bool InlineMessageWidget::hasMessage() const {
    return m_hasMessage;
}

} // namespace rewise::ui::widgets
