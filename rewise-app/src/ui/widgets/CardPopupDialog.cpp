#include "CardPopupDialog.h"
#include "CardWidget.h"

#include <QGuiApplication>
#include <QPropertyAnimation>
#include <QScreen>
#include <QShowEvent>
#include <QVBoxLayout>

namespace rewise::ui::widgets {

CardPopupDialog::CardPopupDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(0);

    m_card = new CardWidget(this);
    m_card->setHeaderVisible(true);
    m_card->setElevated(true);
    m_card->setCloseButtonVisible(true);

    connect(m_card, &CardWidget::closeRequested, this, &QDialog::reject);

    root->addWidget(m_card);
    setLayout(root);

    resize(620, 520);
}

void CardPopupDialog::popupNear(const QPoint& globalAnchor) {
    adjustSize();

    const QSize s = size();

    QScreen* screen = QGuiApplication::screenAt(globalAnchor);
    if (!screen) screen = QGuiApplication::primaryScreen();

    QRect area = screen ? screen->availableGeometry() : QRect(0, 0, 1440, 900);

    QPoint pos = globalAnchor - QPoint(s.width() / 2, s.height() / 2);

    if (pos.x() < area.left() + 8) pos.setX(area.left() + 8);
    if (pos.y() < area.top() + 8) pos.setY(area.top() + 8);
    if (pos.x() + s.width() > area.right() - 8) pos.setX(area.right() - 8 - s.width());
    if (pos.y() + s.height() > area.bottom() - 8) pos.setY(area.bottom() - 8 - s.height());

    move(pos);

    setWindowOpacity(0.0);
    m_animateOnShow = true;
    show();
    raise();
    activateWindow();
}

void CardPopupDialog::showEvent(QShowEvent* e) {
    QDialog::showEvent(e);
    if (m_animateOnShow) {
        m_animateOnShow = false;
        runShowAnimation();
    }
}

void CardPopupDialog::runShowAnimation() {
    const QRect endG = geometry();
    const QRect startG = QRect(endG.center() - QPoint(endG.width() * 0.47, endG.height() * 0.47),
                               QSize(static_cast<int>(endG.width() * 0.94),
                                     static_cast<int>(endG.height() * 0.94)));

    auto* aOpacity = new QPropertyAnimation(this, "windowOpacity", this);
    aOpacity->setDuration(160);
    aOpacity->setStartValue(0.0);
    aOpacity->setEndValue(1.0);

    auto* aGeom = new QPropertyAnimation(this, "geometry", this);
    aGeom->setDuration(180);
    aGeom->setStartValue(startG);
    aGeom->setEndValue(endG);

    aOpacity->start(QAbstractAnimation::DeleteWhenStopped);
    aGeom->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace rewise::ui::widgets
