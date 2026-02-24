#include "CardPopupDialog.h"
#include "CardWidget.h"

#include <QApplication>
#include <QGuiApplication>
#include <QGraphicsDropShadowEffect>
#include <QColor>
#include <QKeyEvent>
#include <QScreen>
#include <QVBoxLayout>
#include <QCursor>

namespace rewise::ui::widgets {

CardPopupDialog::CardPopupDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("CardPopupDialog");

    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(24, 24, 24, 24);
    outer->setSpacing(0);

    m_frame = new QWidget(this);
    m_frame->setObjectName("cardFrame");

    auto* shadow = new QGraphicsDropShadowEffect(m_frame);
    shadow->setBlurRadius(34);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 80));
    m_frame->setGraphicsEffect(shadow);

    auto* frameLayout = new QVBoxLayout(m_frame);
    frameLayout->setContentsMargins(18, 18, 18, 18);
    frameLayout->setSpacing(10);

    m_card = new CardWidget(m_frame);
    m_card->setWideLayout(true);

    frameLayout->addWidget(m_card);
    outer->addWidget(m_frame);

    m_frame->setStyleSheet(
        "#cardFrame {"
        "  background: rgba(255, 255, 255, 0.94);"
        "  border: 1px solid rgba(88, 110, 160, 0.22);"
        "  border-radius: 18px;"
        "}"
    );

    setFixedSize(900, 460);
}

void CardPopupDialog::openCentered() {
    adjustSize();

    QScreen* screen = nullptr;
    if (parentWidget()) screen = parentWidget()->screen();
    if (!screen) screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) screen = QGuiApplication::primaryScreen();

    const QRect avail = screen ? screen->availableGeometry() : QRect(0, 0, 1200, 800);
    const QSize sz = size();
    const QPoint topLeft(avail.center().x() - sz.width() / 2,
                         avail.center().y() - sz.height() / 2);
    move(topLeft);

    show();
    raise();
    activateWindow();
}

void CardPopupDialog::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Escape) {
        hide();
        e->accept();
        return;
    }
    QDialog::keyPressEvent(e);
}

} // namespace rewise::ui::widgets