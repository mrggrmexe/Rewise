#ifndef REWISE_UI_WIDGETS_FOLDERPOPUPDIALOG_H
#define REWISE_UI_WIDGETS_FOLDERPOPUPDIALOG_H

#include <QDialog>

namespace rewise::ui::widgets {

class FolderCardWidget;

class FolderPopupDialog final : public QDialog {
    Q_OBJECT
public:
    explicit FolderPopupDialog(QWidget* parent = nullptr);

    FolderCardWidget* card() const { return m_card; }

    void openCentered();

signals:
    void submitted(const QString& name);
    void cancelled();

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    QWidget* m_frame = nullptr;
    FolderCardWidget* m_card = nullptr;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_FOLDERPOPUPDIALOG_H