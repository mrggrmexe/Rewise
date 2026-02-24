#ifndef REWISE_UI_WIDGETS_CARDPOPUPDIALOG_H
#define REWISE_UI_WIDGETS_CARDPOPUPDIALOG_H

#include <QDialog>

namespace rewise::ui::widgets {

class CardWidget;

class CardPopupDialog final : public QDialog {
    Q_OBJECT
public:
    explicit CardPopupDialog(QWidget* parent = nullptr);

    CardWidget* card() const { return m_card; }

    // Opens a wide, centered popup (horizontal card format).
    void openCentered();

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    QWidget* m_frame = nullptr;
    CardWidget* m_card = nullptr;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_CARDPOPUPDIALOG_H