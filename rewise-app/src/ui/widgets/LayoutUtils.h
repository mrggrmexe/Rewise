#ifndef REWISE_UI_WIDGETS_LAYOUTUTILS_H
#define REWISE_UI_WIDGETS_LAYOUTUTILS_H

#include <QLayout>
#include <QWidget>
#include <QStyle>

namespace rewise::ui::widgets {

inline void clearLayout(QLayout* layout) {
    if (!layout) return;
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* w = item->widget()) {
            w->setParent(nullptr);
            w->deleteLater();
        }
        if (QLayout* child = item->layout()) {
            clearLayout(child);
        }
        delete item;
    }
}

inline void enableStyledBackground(QWidget* w) {
    if (!w) return;
    // Required for QWidget backgrounds/border-radius via QSS.
    w->setAttribute(Qt::WA_StyledBackground, true);
}

inline void repolish(QWidget* w) {
    if (!w) return;
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_LAYOUTUTILS_H
