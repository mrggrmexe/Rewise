#ifndef REWISE_UI_WIDGETS_ACTIONMENUPOPUP_H
#define REWISE_UI_WIDGETS_ACTIONMENUPOPUP_H

#include <QFrame>

namespace rewise::ui::widgets {

class ActionMenuWidget;

class ActionMenuPopup final : public QFrame {
    Q_OBJECT
public:
    explicit ActionMenuPopup(QWidget* parent = nullptr);

    ActionMenuWidget* menu() const { return m_menu; }
    void openBelow(QWidget* anchor, int yOffset = 8);

signals:
    void renameRequested();
    void deleteRequested();
    void cancelled();

private:
    QWidget* m_frame = nullptr;
    ActionMenuWidget* m_menu = nullptr;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_ACTIONMENUPOPUP_H