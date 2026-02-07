#ifndef REWISE_UI_WIDGETS_CARDPOPUPDIALOG_H
#define REWISE_UI_WIDGETS_CARDPOPUPDIALOG_H

#include <QDialog>
#include <QPoint>

namespace rewise::ui::widgets {

class CardWidget;

class CardPopupDialog final : public QDialog {
    Q_OBJECT
public:
    explicit CardPopupDialog(QWidget* parent = nullptr);
    ~CardPopupDialog() override = default;

    CardWidget* card() const { return m_card; }

    // Shows popup close to an anchor point in global coordinates.
    void popupNear(const QPoint& globalAnchor);

protected:
    void showEvent(QShowEvent* e) override;

private:
    void runShowAnimation();

    CardWidget* m_card = nullptr;
    bool m_animateOnShow = true;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_CARDPOPUPDIALOG_H
