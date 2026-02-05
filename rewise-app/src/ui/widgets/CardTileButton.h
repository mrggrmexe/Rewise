#ifndef REWISE_UI_WIDGETS_CARDTILEBUTTON_H
#define REWISE_UI_WIDGETS_CARDTILEBUTTON_H

#include "domain/Id.h"
#include <QPushButton>

class QLabel;

namespace rewise::ui::widgets {

class CardTileButton final : public QPushButton {
    Q_OBJECT
public:
    explicit CardTileButton(QWidget* parent = nullptr);

    void setCard(const rewise::domain::Id& cardId,
                 const QString& question,
                 const QString& answer,
                 const QString& metaLine = {});

    rewise::domain::Id cardId() const { return m_id; }

private:
    static QString makePreview(const QString& s, int maxChars);

    rewise::domain::Id m_id;
    QLabel* m_title = nullptr;
    QLabel* m_preview = nullptr;
    QLabel* m_meta = nullptr;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_CARDTILEBUTTON_H
