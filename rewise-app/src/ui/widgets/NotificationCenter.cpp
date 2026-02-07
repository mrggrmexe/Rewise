#include "NotificationCenter.h"
#include "ToastWidget.h"

#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>

namespace rewise::ui::widgets {

static void setOpacity(QWidget* w, qreal opacity) {
    if (!w) return;
    auto* eff = qobject_cast<QGraphicsOpacityEffect*>(w->graphicsEffect());
    if (!eff) {
        eff = new QGraphicsOpacityEffect(w);
        eff->setOpacity(opacity);
        w->setGraphicsEffect(eff);
        return;
    }
    eff->setOpacity(opacity);
}

static qreal opacityOf(QWidget* w) {
    auto* eff = qobject_cast<QGraphicsOpacityEffect*>(w ? w->graphicsEffect() : nullptr);
    return eff ? eff->opacity() : 1.0;
}

NotificationCenter::NotificationCenter(QWidget* host, QObject* parent)
    : QObject(parent)
    , m_host(host)
{
    Q_ASSERT(m_host);
    m_host->installEventFilter(this);
}

ToastWidget* NotificationCenter::showInfo(const QString& text, int autoCloseMs) {
    auto* t = createToast();
    t->setKind(ToastWidget::Kind::Info);
    t->setText(text);
    attachAndShow(t);

    if (autoCloseMs > 0) {
        QTimer::singleShot(autoCloseMs, t, [this, t] { dismissToast(t); });
    }
    return t;
}

ToastWidget* NotificationCenter::showError(const QString& text, int autoCloseMs) {
    auto* t = createToast();
    t->setKind(ToastWidget::Kind::Error);
    t->setText(text);
    attachAndShow(t);

    if (autoCloseMs > 0) {
        QTimer::singleShot(autoCloseMs, t, [this, t] { dismissToast(t); });
    }
    return t;
}

ToastWidget* NotificationCenter::showConfirm(const QString& text,
                                             const QString& yesText,
                                             const QString& noText) {
    auto* t = createToast();
    t->setKind(ToastWidget::Kind::Confirm);
    t->setText(text);
    t->setConfirmButtons(yesText, noText);
    attachAndShow(t);
    return t;
}

void NotificationCenter::dismissAll() {
    const auto copy = m_toasts;
    for (const auto& p : copy) {
        if (p) dismissToast(p);
    }
}

bool NotificationCenter::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_host) {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Show) {
            relayout();
        }
    }
    return QObject::eventFilter(watched, event);
}

ToastWidget* NotificationCenter::createToast() {
    auto* t = new ToastWidget(m_host);
    t->setAttribute(Qt::WA_DeleteOnClose, true);
    t->setFixedWidth(360);

    connect(t, &ToastWidget::closeClicked, this, [this, t] { dismissToast(t); });

    // For confirm, close toast after decision as well.
    connect(t, &ToastWidget::accepted, this, [this, t] { dismissToast(t); });
    connect(t, &ToastWidget::rejected, this, [this, t] { dismissToast(t); });

    return t;
}

void NotificationCenter::attachAndShow(ToastWidget* toast) {
    if (!toast) return;

    // Limit stack size: dismiss the oldest first.
    if (m_toasts.size() >= m_maxToasts) {
        if (m_toasts.front()) dismissToast(m_toasts.front());
    }

    m_toasts.push_back(toast);

    toast->adjustSize();
    toast->show();
    toast->raise();

    // First place the toast, then animate from a small offset.
    relayout();
    animateIn(toast);
}

void NotificationCenter::dismissToast(ToastWidget* toast) {
    if (!toast) return;

    // If already closing (opacity 0), ignore.
    if (opacityOf(toast) <= 0.01) return;

    animateOutAndDelete(toast);

    // Remove from list (will be deleted later).
    for (int i = 0; i < m_toasts.size(); ++i) {
        if (m_toasts[i] == toast) {
            m_toasts.remove(i);
            break;
        }
    }

    // Relayout remaining toasts immediately.
    relayout();
}

void NotificationCenter::relayout() {
    if (!m_host) return;

    int y = m_margin;

    // Newest on top.
    for (int i = m_toasts.size() - 1; i >= 0; --i) {
        auto* t = m_toasts[i].data();
        if (!t || t->isHidden()) continue;

        t->adjustSize();
        const int w = t->width();
        const int h = t->height();

        const int x = m_host->width() - m_margin - w;

        t->move(x, y);
        t->raise();

        y += h + m_spacing;
    }
}

void NotificationCenter::animateIn(ToastWidget* toast) {
    if (!toast) return;

    setOpacity(toast, 0.0);

    const QPoint endPos = toast->pos();
    const QPoint startPos = endPos + QPoint(14, 0);

    toast->move(startPos);

    auto* eff = qobject_cast<QGraphicsOpacityEffect*>(toast->graphicsEffect());
    if (!eff) {
        eff = new QGraphicsOpacityEffect(toast);
        toast->setGraphicsEffect(eff);
    }
    eff->setOpacity(0.0);

    auto* animOpacity = new QPropertyAnimation(eff, "opacity", toast);
    animOpacity->setDuration(180);
    animOpacity->setStartValue(0.0);
    animOpacity->setEndValue(1.0);

    auto* animPos = new QPropertyAnimation(toast, "pos", toast);
    animPos->setDuration(180);
    animPos->setStartValue(startPos);
    animPos->setEndValue(endPos);

    auto* group = new QParallelAnimationGroup(toast);
    group->addAnimation(animOpacity);
    group->addAnimation(animPos);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void NotificationCenter::animateOutAndDelete(ToastWidget* toast) {
    if (!toast) return;

    auto* eff = qobject_cast<QGraphicsOpacityEffect*>(toast->graphicsEffect());
    if (!eff) {
        eff = new QGraphicsOpacityEffect(toast);
        toast->setGraphicsEffect(eff);
        eff->setOpacity(1.0);
    }

    const QPoint startPos = toast->pos();
    const QPoint endPos = startPos + QPoint(18, 0);

    auto* animOpacity = new QPropertyAnimation(eff, "opacity", toast);
    animOpacity->setDuration(160);
    animOpacity->setStartValue(eff->opacity());
    animOpacity->setEndValue(0.0);

    auto* animPos = new QPropertyAnimation(toast, "pos", toast);
    animPos->setDuration(160);
    animPos->setStartValue(startPos);
    animPos->setEndValue(endPos);

    auto* group = new QParallelAnimationGroup(toast);
    group->addAnimation(animOpacity);
    group->addAnimation(animPos);

    connect(group, &QParallelAnimationGroup::finished, toast, [toast] {
        toast->close();
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace rewise::ui::widgets
