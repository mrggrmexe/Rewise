#include "ActionMenuPopup.h"
#include "ActionMenuWidget.h"

#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QWidget>

namespace rewise::ui::widgets {

ActionMenuPopup::ActionMenuPopup(QWidget* parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setObjectName("ActionMenuPopup");
    setAttribute(Qt::WA_TranslucentBackground, true);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(16, 16, 16, 16);
    outer->setSpacing(0);

    m_frame = new QWidget(this);
    m_frame->setObjectName("actionMenuFrame");

    auto* shadow = new QGraphicsDropShadowEffect(m_frame);
    shadow->setBlurRadius(28);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 72));
    m_frame->setGraphicsEffect(shadow);

    auto* frameLayout = new QVBoxLayout(m_frame);
    frameLayout->setContentsMargins(14, 14, 14, 14);
    frameLayout->setSpacing(0);

    m_menu = new ActionMenuWidget(m_frame);
    frameLayout->addWidget(m_menu);
    outer->addWidget(m_frame);

    connect(m_menu, &ActionMenuWidget::renameRequested, this, [this] {
        emit renameRequested();
        hide();
    });
    connect(m_menu, &ActionMenuWidget::deleteRequested, this, [this] {
        emit deleteRequested();
        hide();
    });
    connect(m_menu, &ActionMenuWidget::cancelled, this, [this] {
        emit cancelled();
        hide();
    });
}

void ActionMenuPopup::openBelow(QWidget* anchor, int yOffset) {
    if (!anchor) return;

    adjustSize();
    const QSize hint = sizeHint();
    resize(hint);

    const QPoint global = anchor->mapToGlobal(QPoint(0, anchor->height() + yOffset));
    move(global);
    show();
    raise();
    activateWindow();
}

} // namespace rewise::ui::widgets