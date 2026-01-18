#ifndef REWISE_UI_WIDGETS_FOLDERLISTMODEL_H
#define REWISE_UI_WIDGETS_FOLDERLISTMODEL_H

#include "domain/Folder.h"
#include "domain/Id.h"

#include <QAbstractListModel>
#include <QVector>

namespace rewise::ui::widgets {

class FolderListModel final : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        IsAllRole
    };

    explicit FolderListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void setFolders(QVector<rewise::domain::Folder> folders);
    const QVector<rewise::domain::Folder>& folders() const { return m_folders; }

    // Row 0 (если включено) — виртуальный пункт "Все карточки".
    void setIncludeAllItem(bool enabled);
    bool includeAllItem() const { return m_includeAll; }

    rewise::domain::Id idAtRow(int row) const;
    int rowForId(const rewise::domain::Id& id) const;

    bool isAllRow(int row) const;

private:
    QVector<rewise::domain::Folder> m_folders;
    bool m_includeAll = true;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_FOLDERLISTMODEL_H
