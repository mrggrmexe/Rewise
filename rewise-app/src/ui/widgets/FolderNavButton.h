#ifndef REWISE_UI_WIDGETS_FOLDERNAVBUTTON_H
#define REWISE_UI_WIDGETS_FOLDERNAVBUTTON_H

#include "domain/Id.h"
#include <QPushButton>

namespace rewise::ui::widgets {

class FolderNavButton final : public QPushButton {
    Q_OBJECT
public:
    explicit FolderNavButton(QWidget* parent = nullptr);

    void setFolder(const rewise::domain::Id& id, const QString& name, bool isAll = false);

    rewise::domain::Id folderId() const { return m_id; }
    bool isAll() const { return m_isAll; }

private:
    rewise::domain::Id m_id;
    bool m_isAll = false;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_FOLDERNAVBUTTON_H
