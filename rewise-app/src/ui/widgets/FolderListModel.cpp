#include "FolderListModel.h"

namespace rewise::ui::widgets {

FolderListModel::FolderListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

int FolderListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_folders.size() + (m_includeAll ? 1 : 0);
}

QVariant FolderListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const int row = index.row();
    const int base = (m_includeAll ? 1 : 0);

    if (m_includeAll && row == 0) {
        if (role == Qt::DisplayRole) return QStringLiteral("Все карточки");
        if (role == IdRole) return QString(); // invalid id
        if (role == IsAllRole) return true;
        return {};
    }

    const int folderIdx = row - base;
    if (folderIdx < 0 || folderIdx >= m_folders.size()) return {};

    const auto& f = m_folders[folderIdx];
    if (role == Qt::DisplayRole) return f.name;
    if (role == IdRole) return f.id.toString();
    if (role == IsAllRole) return false;

    return {};
}

Qt::ItemFlags FolderListModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void FolderListModel::setFolders(QVector<rewise::domain::Folder> folders) {
    beginResetModel();
    m_folders = std::move(folders);
    endResetModel();
}

void FolderListModel::setIncludeAllItem(bool enabled) {
    if (m_includeAll == enabled) return;
    beginResetModel();
    m_includeAll = enabled;
    endResetModel();
}

rewise::domain::Id FolderListModel::idAtRow(int row) const {
    if (m_includeAll && row == 0) return rewise::domain::Id{}; // invalid = "All"
    const int base = (m_includeAll ? 1 : 0);
    const int idx = row - base;
    if (idx < 0 || idx >= m_folders.size()) return rewise::domain::Id{};
    return m_folders[idx].id;
}

int FolderListModel::rowForId(const rewise::domain::Id& id) const {
    if (!id.isValid()) return (m_includeAll ? 0 : -1);

    const int base = (m_includeAll ? 1 : 0);
    for (int i = 0; i < m_folders.size(); ++i) {
        if (m_folders[i].id == id) return i + base;
    }
    return -1;
}

bool FolderListModel::isAllRow(int row) const {
    return (m_includeAll && row == 0);
}

} // namespace rewise::ui::widgets
