#ifndef REWISE_UI_WIDGETS_CARDTABLEMODEL_H
#define REWISE_UI_WIDGETS_CARDTABLEMODEL_H

#include "storage/Database.h"
#include "domain/Id.h"

#include <QAbstractTableModel>
#include <QVector>

namespace rewise::ui::widgets {

class CardTableModel final : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column {
        QuestionCol = 0,
        AnswerCol   = 1,
        FolderCol   = 2,
        UpdatedCol  = 3,
        ColCount
    };

    explicit CardTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void setDatabase(rewise::storage::Database db);

    void setFolderFilter(const rewise::domain::Id& folderId); // invalid => all
    rewise::domain::Id folderFilter() const { return m_filterFolderId; }

    void setSearchQuery(QString q);
    QString searchQuery() const { return m_search; }

    void sort(int column, Qt::SortOrder order) override;

    rewise::domain::Id cardIdAtRow(int row) const;
    const rewise::domain::Card* cardAtRow(int row) const;

    int rowForCardId(const rewise::domain::Id& id) const;

private:
    void rebuildView();

    static QString preview(const QString& s, int maxChars = 80);
    static QString folderNameById(const QVector<rewise::domain::Folder>& folders,
                                  const rewise::domain::Id& id);

    rewise::storage::Database m_db;

    QVector<int> m_view; // indices into m_db.cards
    rewise::domain::Id m_filterFolderId; // invalid => all
    QString m_search;

    int m_sortColumn = UpdatedCol;
    Qt::SortOrder m_sortOrder = Qt::DescendingOrder;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_CARDTABLEMODEL_H
