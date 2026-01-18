#include "CardTableModel.h"

#include <QDateTime>

namespace rewise::ui::widgets {

CardTableModel::CardTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{}

int CardTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_view.size();
}

int CardTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return ColCount;
}

QVariant CardTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
    switch (section) {
        case QuestionCol: return "Вопрос";
        case AnswerCol:   return "Ответ (превью)";
        case FolderCol:   return "Папка";
        case UpdatedCol:  return "Обновлено";
        default:          return {};
    }
}

Qt::ItemFlags CardTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QString CardTableModel::preview(const QString& s, int maxChars) {
    QString t = s;
    t.replace('\n', ' ');
    t = t.simplified();
    if (t.size() <= maxChars) return t;
    return t.left(maxChars - 1) + "…";
}

QString CardTableModel::folderNameById(const QVector<rewise::domain::Folder>& folders,
                                      const rewise::domain::Id& id)
{
    for (const auto& f : folders) {
        if (f.id == id) return f.name;
    }
    return QStringLiteral("?");
}

QVariant CardTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const int row = index.row();
    const int col = index.column();
    if (row < 0 || row >= m_view.size()) return {};
    const int cardIdx = m_view[row];
    if (cardIdx < 0 || cardIdx >= m_db.cards.size()) return {};

    const auto& c = m_db.cards[cardIdx];

    if (role == Qt::DisplayRole) {
        switch (col) {
            case QuestionCol: return preview(c.question, 120);
            case AnswerCol:   return preview(c.answer, 120);
            case FolderCol:   return folderNameById(m_db.folders, c.folderId);
            case UpdatedCol: {
                const auto dt = QDateTime::fromMSecsSinceEpoch(c.updatedAtMsUtc, Qt::UTC).toLocalTime();
                return dt.toString("yyyy-MM-dd HH:mm");
            }
            default: return {};
        }
    }

    // Полезно: id карточки в UserRole.
    if (role == Qt::UserRole) {
        return c.id.toString();
    }

    return {};
}

void CardTableModel::setDatabase(rewise::storage::Database db) {
    beginResetModel();
    m_db = std::move(db);
    endResetModel();
    rebuildView();
}

void CardTableModel::setFolderFilter(const rewise::domain::Id& folderId) {
    if (m_filterFolderId == folderId) return;
    m_filterFolderId = folderId;
    rebuildView();
}

void CardTableModel::setSearchQuery(QString q) {
    q = q.simplified();
    if (m_search == q) return;
    m_search = std::move(q);
    rebuildView();
}

void CardTableModel::sort(int column, Qt::SortOrder order) {
    if (column < 0 || column >= ColCount) return;
    m_sortColumn = column;
    m_sortOrder = order;
    rebuildView();
}

void CardTableModel::rebuildView() {
    beginResetModel();
    m_view.clear();
    m_view.reserve(m_db.cards.size());

    const QString s = m_search.toLower();

    for (int i = 0; i < m_db.cards.size(); ++i) {
        const auto& c = m_db.cards[i];

        if (m_filterFolderId.isValid() && c.folderId != m_filterFolderId) continue;

        if (!s.isEmpty()) {
            const QString q = c.question.toLower();
            const QString a = c.answer.toLower();
            if (!q.contains(s) && !a.contains(s)) continue;
        }

        m_view.push_back(i);
    }

    auto cmp = [&](int aIdx, int bIdx) -> bool {
        const auto& a = m_db.cards[aIdx];
        const auto& b = m_db.cards[bIdx];

        auto less = [&](auto x, auto y) { return (m_sortOrder == Qt::AscendingOrder) ? (x < y) : (x > y); };

        switch (m_sortColumn) {
            case QuestionCol: return less(a.question.toLower(), b.question.toLower());
            case AnswerCol:   return less(a.answer.toLower(), b.answer.toLower());
            case FolderCol:   return less(folderNameById(m_db.folders, a.folderId).toLower(),
                                          folderNameById(m_db.folders, b.folderId).toLower());
            case UpdatedCol:
            default:          return less(a.updatedAtMsUtc, b.updatedAtMsUtc);
        }
    };

    std::sort(m_view.begin(), m_view.end(), cmp);
    endResetModel();
}

rewise::domain::Id CardTableModel::cardIdAtRow(int row) const {
    const auto* c = cardAtRow(row);
    return c ? c->id : rewise::domain::Id{};
}

const rewise::domain::Card* CardTableModel::cardAtRow(int row) const {
    if (row < 0 || row >= m_view.size()) return nullptr;
    const int idx = m_view[row];
    if (idx < 0 || idx >= m_db.cards.size()) return nullptr;
    return &m_db.cards[idx];
}

int CardTableModel::rowForCardId(const rewise::domain::Id& id) const {
    if (!id.isValid()) return -1;
    for (int r = 0; r < m_view.size(); ++r) {
        const int idx = m_view[r];
        if (idx >= 0 && idx < m_db.cards.size() && m_db.cards[idx].id == id) return r;
    }
    return -1;
}

} // namespace rewise::ui::widgets
