#ifndef REWISE_STORAGE_DATABASE_H
#define REWISE_STORAGE_DATABASE_H

#include "../domain/Card.h"
#include "../domain/Folder.h"
#include "StorageJson.h"

#include <QVector>
#include <QString>

namespace rewise::storage {

struct Database final {
    int version = json_keys::kSchemaVersion;

    QVector<rewise::domain::Folder> folders;
    QVector<rewise::domain::Card> cards;

    // --- Lookup helpers ---
    int folderIndexById(const rewise::domain::Id& id) const;
    int cardIndexById(const rewise::domain::Id& id) const;

    const rewise::domain::Folder* folderById(const rewise::domain::Id& id) const;
    rewise::domain::Folder* folderById(const rewise::domain::Id& id);

    const rewise::domain::Card* cardById(const rewise::domain::Id& id) const;
    rewise::domain::Card* cardById(const rewise::domain::Id& id);

    // Ensures there is at least one folder. Returns the default folder id.
    rewise::domain::Id ensureDefaultFolder(const QString& defaultName = "Default");

    // Validation: checks basic invariants.
    // - schema version supported
    // - folder ids unique & names non-empty
    // - card ids unique
    // - each card.folderId exists
    bool validate(QString* error = nullptr) const;

    // Convenience: make folder names unique (case-insensitive) by appending " (2)", " (3)", etc.
    // Returns true if any changes were made.
    bool ensureUniqueFolderNames();
};

} // namespace rewise::storage

#endif // REWISE_STORAGE_DATABASE_H
