#include "FolderPopupDialog.h"
#include "FolderCardWidget.h"

#include <QApplication>
#include <QColor>
#include <QCursor>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QScreen>
#include <QVBoxLayout>

namespace rewise::ui::widgets {

FolderPopupDialog::FolderPopupDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("FolderPopupDialog");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(24, 24, 24, 24);
    outer->setSpacing(0);

    m_frame = new QWidget(this);
    m_frame->setObjectName("folderPopupFrame");

    auto* shadow = new QGraphicsDropShadowEffect(m_frame);
    shadow->setBlurRadius(34);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 80));
    m_frame->setGraphicsEffect(shadow);

    auto* frameLayout = new QVBoxLayout(m_frame);
    frameLayout->setContentsMargins(18, 18, 18, 18);
    frameLayout->setSpacing(10);

    m_card = new FolderCardWidget(m_frame);
    frameLayout->addWidget(m_card);
    outer->addWidget(m_frame);

    setFixedSize(520, 220);

    connect(m_card, &FolderCardWidget::submitted, this, [this](const QString& name) {
        emit submitted(name);
        hide();
    });
    connect(m_card, &FolderCardWidget::cancelled, this, [this] {
        emit cancelled();
        hide();
    });
}

void FolderPopupDialog::openCentered() {
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

    open();
    raise();
    activateWindow();
}

void FolderPopupDialog::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Escape) {
        hide();
        emit cancelled();
        e->accept();
        return;
    }
    QDialog::keyPressEvent(e);
}

} // namespace rewise::ui::widgets