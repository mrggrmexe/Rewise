#ifndef REWISE_UI_WIDGETS_TOASTWIDGET_H
#define REWISE_UI_WIDGETS_TOASTWIDGET_H

#include <QWidget>

class QLabel;
class QToolButton;
class QPushButton;

namespace rewise::ui::widgets {

class ToastWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Kind { Info, Error, Confirm };

    explicit ToastWidget(QWidget* parent = nullptr);

    void setKind(Kind k);
    Kind kind() const { return m_kind; }

    void setText(const QString& text);
    QString text() const;

    // For confirm toasts.
    void setConfirmButtons(const QString& yesText = QStringLiteral("Да"),
                           const QString& noText  = QStringLiteral("Нет"));

signals:
    void closeClicked();
    void accepted();
    void rejected();

private:
    void rebuildUiForKind();

    Kind m_kind = Kind::Info;

    QLabel* m_label = nullptr;
    QToolButton* m_btnClose = nullptr;

    QPushButton* m_btnYes = nullptr;
    QPushButton* m_btnNo = nullptr;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_TOASTWIDGET_H
