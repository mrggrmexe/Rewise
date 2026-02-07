#ifndef REWISE_UI_WIDGETS_NOTIFICATIONCENTER_H
#define REWISE_UI_WIDGETS_NOTIFICATIONCENTER_H

#include <QObject>
#include <QPointer>
#include <QVector>

class QWidget;

namespace rewise::ui::widgets {

class ToastWidget;

class NotificationCenter final : public QObject {
    Q_OBJECT
public:
    explicit NotificationCenter(QWidget* host, QObject* parent = nullptr);

    // Non-blocking messages.
    ToastWidget* showInfo(const QString& text, int autoCloseMs = 3200);
    ToastWidget* showError(const QString& text, int autoCloseMs = 0);

    // Confirmations (Yes/No). Caller connects to accepted()/rejected().
    ToastWidget* showConfirm(const QString& text,
                             const QString& yesText = QStringLiteral("Да"),
                             const QString& noText  = QStringLiteral("Нет"));

    void dismissAll();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    ToastWidget* createToast();

    void attachAndShow(ToastWidget* toast);
    void dismissToast(ToastWidget* toast);

    void relayout();
    void animateIn(ToastWidget* toast);
    void animateOutAndDelete(ToastWidget* toast);

private:
    QWidget* m_host = nullptr;

    QVector<QPointer<ToastWidget>> m_toasts;

    int m_margin = 14;
    int m_spacing = 10;
    int m_maxToasts = 4;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_NOTIFICATIONCENTER_H
