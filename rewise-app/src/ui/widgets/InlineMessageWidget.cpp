#include "InlineMessageWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QFontMetrics>
#include <QResizeEvent>

namespace rewise::ui::widgets {

InlineMessageWidget::InlineMessageWidget(QWidget* parent)
    : QWidget(parent)
{
    setVisible(false);

    m_label = new QLabel(this);
    m_label->setWordWrap(false);                 // ключ: не раздуваем высоту
    m_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_close = new QToolButton(this);
    m_close->setText("✕");
    m_close->setAutoRaise(true);
    m_close->setToolTip("Скрыть сообщение");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(8);
    layout->addWidget(m_label, 1);
    layout->addWidget(m_close, 0, Qt::AlignVCenter);

    // Высота баннера фиксированная (чтобы топбар не прыгал)
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    connect(m_close, &QToolButton::clicked, this, [this] {
        clearMessage();
        emit dismissed();
    });
}

void InlineMessageWidget::applyKindStyle(Kind kind) {
    QString border;
    QString bg;
    QString fg = "color: #1A1F2B;";

    switch (kind) {
        case Kind::Info:
            border = "border: 1px solid rgba(96, 120, 255, 0.45);";
            bg     = "background: rgba(214, 229, 255, 0.60);";
            break;
        case Kind::Warning:
            border = "border: 1px solid rgba(200, 150, 40, 0.45);";
            bg     = "background: rgba(255, 240, 200, 0.70);";
            break;
        case Kind::Error:
            border = "border: 1px solid rgba(190, 80, 120, 0.45);";
            bg     = "background: rgba(255, 220, 235, 0.70);";
            break;
    }

    setStyleSheet(QString("border-radius: 12px; %1 %2 %3").arg(border, bg, fg));
}

void InlineMessageWidget::showMessage(Kind kind, const QString& text) {
    m_kind = kind;
    m_fullText = text;
    applyKindStyle(kind);

    m_hasMessage = !m_fullText.trimmed().isEmpty();
    setVisible(m_hasMessage);

    updateElide();
}

void InlineMessageWidget::clearMessage() {
    m_fullText.clear();
    m_label->clear();
    m_label->setToolTip({});
    m_hasMessage = false;
    setVisible(false);
}

bool InlineMessageWidget::hasMessage() const {
    return m_hasMessage;
}

void InlineMessageWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    if (m_hasMessage) updateElide();
}

void InlineMessageWidget::updateElide() {
    if (!m_hasMessage) return;

    // Оставим место под кнопку закрытия и padding
    const int available = qMax(10, width() - 10 - 10 - 8 - m_close->sizeHint().width());
    const QFontMetrics fm(m_label->font());
    const QString elided = fm.elidedText(m_fullText, Qt::ElideRight, available);

    m_label->setText(elided);
    m_label->setToolTip(m_fullText); // полный текст по наведению
}

} // namespace rewise::ui::widgets
