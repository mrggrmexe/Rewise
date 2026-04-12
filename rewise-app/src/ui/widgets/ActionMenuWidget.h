#ifndef REWISE_UI_WIDGETS_ACTIONMENUWIDGET_H
#define REWISE_UI_WIDGETS_ACTIONMENUWIDGET_H

#include <QWidget>

class QPushButton;

namespace rewise::ui::widgets {

class ActionMenuWidget final : public QWidget {
    Q_OBJECT
public:
    explicit ActionMenuWidget(QWidget* parent = nullptr);

    void setActionsEnabled(bool renameEnabled, bool deleteEnabled);

signals:
    void renameRequested();
    void deleteRequested();
    void cancelled();

private:
    QPushButton* m_btnRename = nullptr;
    QPushButton* m_btnDelete = nullptr;
    QPushButton* m_btnCancel = nullptr;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_ACTIONMENUWIDGET_H