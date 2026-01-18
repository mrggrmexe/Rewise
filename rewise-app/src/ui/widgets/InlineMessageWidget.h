#ifndef REWISE_UI_WIDGETS_INLINEMESSAGEWIDGET_H
#define REWISE_UI_WIDGETS_INLINEMESSAGEWIDGET_H

#include <QWidget>

class QLabel;
class QToolButton;

namespace rewise::ui::widgets {

class InlineMessageWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Kind { Info, Warning, Error };

    explicit InlineMessageWidget(QWidget* parent = nullptr);

    void showMessage(Kind kind, const QString& text);
    void clearMessage();
    bool hasMessage() const;

signals:
    void dismissed();

private:
    void applyKindStyle(Kind kind);

    QLabel* m_label = nullptr;
    QToolButton* m_close = nullptr;
    bool m_hasMessage = false;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_INLINEMESSAGEWIDGET_H
