#include "Repository.h"
#include "StorageJson.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

namespace rewise::storage {

using rewise::domain::Folder;
using rewise::domain::Card;
using rewise::domain::Id;

Repository::Repository(QString fileName)
    : m_fileName(std::move(fileName)) {}

QString Repository::databaseDirPath() const {
    // AppDataLocation is an application-specific persistent data directory.
    // It depends on QCoreApplication (app/organization name), but Qt guarantees non-empty.
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    // Some apps forget to set appName; still return something stable.
    // Usually base already includes app name, so we don’t append more folders here.
    return base;
}

QString Repository::databaseFilePath() const {
    QDir dir(databaseDirPath());
    return dir.filePath(m_fileName);
}

bool Repository::ensureDatabaseDir(QString* error) const {
    const QString dirPath = databaseDirPath();
    QDir dir(dirPath);
    if (dir.exists()) return true;

    if (!dir.mkpath(".")) {
        if (error) *error = "Failed to create database directory: " + dirPath;
        return false;
    }
    return true;
}

bool Repository::writeJsonAtomically(const QString& path, const QByteArray& utf8, QString* error) const {
    QFileInfo fi(path);
    QDir parent(fi.absolutePath());
    if (!parent.exists() && !parent.mkpath(".")) {
        if (error) *error = "Failed to create parent directory: " + fi.absolutePath();
        return false;
    }

    QSaveFile file(path);
    // If atomic rename isn't possible (permissions), allow direct write fallback.
    file.setDirectWriteFallback(true);

    if (!file.open(QIODevice::WriteOnly)) {
        if (error) *error = "Failed to open for writing: " + path + " (" + file.errorString() + ")";
        return false;
    }

    const qint64 written = file.write(utf8);
    if (written != utf8.size()) {
        if (error) *error = "Failed to write all bytes to: " + path + " (" + file.errorString() + ")";
        return false;
    }

    if (!file.commit()) {
        if (error) *error = "Failed to commit file: " + path + " (" + file.errorString() + ")";
        return false;
    }

    return true;
}

QByteArray Repository::serializeDatabaseJson(const Database& db) const {
    QJsonObject root;
    root.insert(json_keys::kVersion, db.version);

    QJsonArray foldersArr;
    foldersArr.reserve(db.folders.size());
    for (const Folder& f : db.folders) {
        foldersArr.append(f.toJson());
    }
    root.insert(json_keys::kFolders, foldersArr);

    QJsonArray cardsArr;
    cardsArr.reserve(db.cards.size());
    for (const Card& c : db.cards) {
        cardsArr.append(c.toJson());
    }
    root.insert(json_keys::kCards, cardsArr);

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
}

bool Repository::parseDatabaseJson(const QByteArray& utf8, Database* outDb, QString* error) const {
    if (!outDb) {
        if (error) *error = "parseDatabaseJson: outDb is null.";
        return false;
    }

    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(utf8, &pe);
    if (pe.error != QJsonParseError::NoError) {
        if (error) *error = QString("JSON parse error at offset %1: %2").arg(pe.offset).arg(pe.errorString());
        return false;
    }
    if (!doc.isObject()) {
        if (error) *error = "Database JSON root must be an object.";
        return false;
    }

    const QJsonObject root = doc.object();

    Database db;
    db.version = root.value(json_keys::kVersion).toInt(json_keys::kSchemaVersion);

    // folders
    {
        const QJsonValue v = root.value(json_keys::kFolders);
        if (v.isArray()) {
            const QJsonArray arr = v.toArray();
            db.folders.reserve(arr.size());

            for (int i = 0; i < arr.size(); ++i) {
                const QJsonValue item = arr.at(i);
                if (!item.isObject()) {
                    if (error) *error = QString("folders[%1] must be an object.").arg(i);
                    return false;
                }

                Folder f;
                QString why;
                if (!Folder::fromJson(item.toObject(), &f, &why)) {
                    if (error) *error = QString("folders[%1] invalid: %2").arg(i).arg(why);
                    return false;
                }
                db.folders.push_back(f);
            }
        } else if (!v.isUndefined() && !v.isNull()) {
            if (error) *error = "folders must be an array.";
            return false;
        }
    }

    // cards
    {
        const QJsonValue v = root.value(json_keys::kCards);
        if (v.isArray()) {
            const QJsonArray arr = v.toArray();
            db.cards.reserve(arr.size());

            for (int i = 0; i < arr.size(); ++i) {
                const QJsonValue item = arr.at(i);
                if (!item.isObject()) {
                    if (error) *error = QString("cards[%1] must be an object.").arg(i);
                    return false;
                }

                Card c;
                QString why;
                if (!Card::fromJson(item.toObject(), &c, &why)) {
                    if (error) *error = QString("cards[%1] invalid: %2").arg(i).arg(why);
                    return false;
                }
                db.cards.push_back(c);
            }
        } else if (!v.isUndefined() && !v.isNull()) {
            if (error) *error = "cards must be an array.";
            return false;
        }
    }

    *outDb = db;
    return true;
}

void Repository::ensureDefaults(Database* db) const {
    if (!db) return;

    // Ensure at least one folder exists.
    const Id defaultId = db->ensureDefaultFolder("Default");
    (void)defaultId;

    // Optional: ensure folder names are unique (nice to have).
    db->ensureUniqueFolderNames();
}

void Repository::repairOrphans(Database* db) const {
    if (!db) return;

    // Ensure we have at least one folder.
    ensureDefaults(db);

    // Build set of existing folder IDs.
    QSet<QString> folderIds;
    folderIds.reserve(db->folders.size());
    for (const Folder& f : db->folders) folderIds.insert(f.id.toString());

    // Find orphan cards.
    QVector<int> orphanIdx;
    orphanIdx.reserve(db->cards.size());
    for (int i = 0; i < db->cards.size(); ++i) {
        if (!folderIds.contains(db->cards[i].folderId.toString())) {
            orphanIdx.push_back(i);
        }
    }
    if (orphanIdx.isEmpty()) return;

    // Create "Orphaned" folder once.
    Folder orphanFolder;
    orphanFolder.id = Id::create();
    orphanFolder.name = "Orphaned";
    db->folders.push_back(orphanFolder);

    const Id orphanId = orphanFolder.id;
    for (int idx : orphanIdx) {
        db->cards[idx].folderId = orphanId;
        db->cards[idx].touchUpdatedNow();
    }
}

bool Repository::load(Database* outDb, QString* error) const {
    if (!outDb) {
        if (error) *error = "Repository::load: outDb is null.";
        return false;
    }

    if (!ensureDatabaseDir(error)) return false;

    const QString path = databaseFilePath();
    QFile file(path);

    if (!file.exists()) {
        // First run: create a fresh DB.
        Database db;
        ensureDefaults(&db);

        const QByteArray bytes = serializeDatabaseJson(db);
        if (!writeJsonAtomically(path, bytes, error)) return false;

        *outDb = db;
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = "Failed to open DB file: " + path + " (" + file.errorString() + ")";
        return false;
    }

    const QByteArray bytes = file.readAll();
    file.close();

    Database db;
    if (!parseDatabaseJson(bytes, &db, error)) {
        return false;
    }

    // Handle versioning (currently only v1).
    if (db.version != json_keys::kSchemaVersion) {
        if (error) *error = QString("Unsupported DB schema version: %1").arg(db.version);
        return false;
    }

    // Self-heal minimal invariants.
    ensureDefaults(&db);
    repairOrphans(&db);

    // Final validation.
    QString why;
    if (!db.validate(&why)) {
        if (error) *error = "Database validation failed: " + why;
        return false;
    }

    *outDb = db;
    return true;
}

bool Repository::save(const Database& db, QString* error) const {
    if (!ensureDatabaseDir(error)) return false;

    QString why;
    if (!db.validate(&why)) {
        if (error) *error = "Refusing to save invalid DB: " + why;
        return false;
    }

    const QByteArray bytes = serializeDatabaseJson(db);
    return writeJsonAtomically(databaseFilePath(), bytes, error);
}

} // namespace rewise::storage
