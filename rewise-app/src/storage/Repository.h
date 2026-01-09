#ifndef REWISE_STORAGE_REPOSITORY_H
#define REWISE_STORAGE_REPOSITORY_H

#include "Database.h"

#include <QString>

namespace rewise::storage {

class Repository final {
public:
    explicit Repository(QString fileName = "db.json");

    // Load database from disk. If missing, it will create a fresh DB with a default folder.
    bool load(Database* outDb, QString* error = nullptr) const;

    // Save database atomically.
    bool save(const Database& db, QString* error = nullptr) const;

    // Absolute full path to the DB file (AppDataLocation/<fileName>).
    QString databaseFilePath() const;

    // Absolute directory that contains the DB.
    QString databaseDirPath() const;

private:
    QString m_fileName;

    bool ensureDatabaseDir(QString* error = nullptr) const;
    bool writeJsonAtomically(const QString& path, const QByteArray& utf8, QString* error) const;

    // Parses JSON -> Database (no disk I/O).
    bool parseDatabaseJson(const QByteArray& utf8, Database* outDb, QString* error) const;

    // Serializes Database -> JSON bytes.
    QByteArray serializeDatabaseJson(const Database& db) const;

    // If there are orphaned cards (folderId missing), move them into a generated "Orphaned" folder.
    void repairOrphans(Database* db) const;

    // If folders empty, create "Default".
    void ensureDefaults(Database* db) const;
};

} // namespace rewise::storage

#endif // REWISE_STORAGE_REPOSITORY_H
