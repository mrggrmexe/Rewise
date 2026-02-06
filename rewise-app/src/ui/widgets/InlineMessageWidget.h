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

    // Backward-compatible API (как в твоём существующем коде)
    void showMessage(Kind kind, const QString& text);
    void clearMessage();
    bool hasMessage() const;

    // Optional convenience (на будущее): можно вызывать и так
    void showInfo(const QString& text)    { showMessage(Kind::Info, text); }
    void showWarning(const QString& text) { showMessage(Kind::Warning, text); }
    void showError(const QString& text)   { showMessage(Kind::Error, text); }

signals:
    void dismissed();

protected:
    void resizeEvent(QResizeEvent* e) override;

private:
    void applyKindStyle(Kind kind);
    void updateElide(); // не даём баннеру раздувать высоту

    QLabel* m_label = nullptr;
    QToolButton* m_close = nullptr;

    bool m_hasMessage = false;
    QString m_fullText;
    Kind m_kind = Kind::Info;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_INLINEMESSAGEWIDGET_H
