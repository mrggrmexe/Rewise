#include "Database.h"

#include <QSet>
#include <QHash>

namespace rewise::storage {

using rewise::domain::Id;
using rewise::domain::Folder;
using rewise::domain::Card;

static QString normNameKey(const QString& s) {
    return s.trimmed().toLower();
}

int Database::folderIndexById(const Id& id) const {
    for (int i = 0; i < folders.size(); ++i) {
        if (folders[i].id == id) return i;
    }
    return -1;
}

int Database::cardIndexById(const Id& id) const {
    for (int i = 0; i < cards.size(); ++i) {
        if (cards[i].id == id) return i;
    }
    return -1;
}

const Folder* Database::folderById(const Id& id) const {
    const int idx = folderIndexById(id);
    return (idx >= 0) ? &folders[idx] : nullptr;
}

Folder* Database::folderById(const Id& id) {
    const int idx = folderIndexById(id);
    return (idx >= 0) ? &folders[idx] : nullptr;
}

const Card* Database::cardById(const Id& id) const {
    const int idx = cardIndexById(id);
    return (idx >= 0) ? &cards[idx] : nullptr;
}

Card* Database::cardById(const Id& id) {
    const int idx = cardIndexById(id);
    return (idx >= 0) ? &cards[idx] : nullptr;
}

Id Database::ensureDefaultFolder(const QString& defaultName) {
    if (!folders.isEmpty()) {
        // First folder is treated as default by convention.
        return folders.first().id;
    }

    Folder f;
    f.id = Id::create();
    f.name = defaultName.trimmed().isEmpty() ? QString("Default") : defaultName.trimmed();

    folders.push_back(f);
    return f.id;
}

bool Database::ensureUniqueFolderNames() {
    bool changed = false;
    QHash<QString, int> used; // normalized -> count

    for (Folder& f : folders) {
        QString base = f.name.trimmed();
        if (base.isEmpty()) {
            base = "Folder";
            f.name = base;
            changed = true;
        }

        const QString key = normNameKey(base);
        int count = used.value(key, 0);
        if (count == 0) {
            used.insert(key, 1);
            continue;
        }

        // Need to rename with suffix.
        int suffix = count + 1;
        QString candidate;
        while (true) {
            candidate = QString("%1 (%2)").arg(base).arg(suffix);
            const QString candKey = normNameKey(candidate);
            if (!used.contains(candKey)) {
                used.insert(candKey, 1);
                break;
            }
            ++suffix;
        }

        f.name = candidate;
        used[key] = count + 1;
        changed = true;
    }

    return changed;
}

bool Database::validate(QString* error) const {
    if (version != json_keys::kSchemaVersion) {
        if (error) *error = QString("Unsupported DB schema version: %1").arg(version);
        return false;
    }

    // Folder ids unique
    {
        QSet<QString> ids;
        for (const Folder& f : folders) {
            QString why;
            if (!f.isValid(&why)) {
                if (error) *error = QString("Invalid folder: %1").arg(why);
                return false;
            }
            const QString idStr = f.id.toString();
            if (ids.contains(idStr)) {
                if (error) *error = "Duplicate folder id: " + idStr;
                return false;
            }
            ids.insert(idStr);
        }
    }

    // Card ids unique + folderId exists
    {
        QSet<QString> cardIds;
        QSet<QString> folderIds;
        folderIds.reserve(folders.size());
        for (const Folder& f : folders) folderIds.insert(f.id.toString());

        for (const Card& c : cards) {
            QString why;
            if (!c.isValid(&why)) {
                if (error) *error = QString("Invalid card: %1").arg(why);
                return false;
            }

            const QString cid = c.id.toString();
            if (cardIds.contains(cid)) {
                if (error) *error = "Duplicate card id: " + cid;
                return false;
            }
            cardIds.insert(cid);

            if (!folderIds.contains(c.folderId.toString())) {
                if (error) *error = "Card references missing folderId: " + c.folderId.toString();
                return false;
            }
        }
    }

    return true;
}

} // namespace rewise::storage
