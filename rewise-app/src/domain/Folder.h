#ifndef REWISE_DOMAIN_FOLDER_H
#define REWISE_DOMAIN_FOLDER_H

#include "Id.h"
#include "DomainJson.h"

#include <QString>
#include <QJsonObject>
#include <QJsonValue>

namespace rewise::domain {

/// Logical category for cards (one-level, no nesting).
struct Folder final {
    Id id;
    QString name;

    bool isValid(QString* whyNot = nullptr) const {
        if (!id.isValid()) {
            if (whyNot) *whyNot = "Folder.id is null/invalid.";
            return false;
        }
        if (name.trimmed().isEmpty()) {
            if (whyNot) *whyNot = "Folder.name is empty.";
            return false;
        }
        return true;
    }

    QJsonObject toJson() const {
        QJsonObject o;
        o.insert(json_keys::kId, id.toString());
        o.insert(json_keys::kName, name);
        return o;
    }

    static bool fromJson(const QJsonObject& o, Folder* out, QString* error = nullptr) {
        if (!out) {
            if (error) *error = "Folder::fromJson: out is null.";
            return false;
        }

        const QJsonValue idV = o.value(json_keys::kId);
        if (!idV.isString()) {
            if (error) *error = "Folder.id is missing or not a string.";
            return false;
        }
        const Id parsedId = Id::fromString(idV.toString());
        if (!parsedId.isValid()) {
            if (error) *error = "Folder.id is not a valid UUID string.";
            return false;
        }

        const QJsonValue nameV = o.value(json_keys::kName);
        if (!nameV.isString()) {
            if (error) *error = "Folder.name is missing or not a string.";
            return false;
        }

        Folder tmp;
        tmp.id = parsedId;
        tmp.name = nameV.toString();

        QString why;
        if (!tmp.isValid(&why)) {
            if (error) *error = why;
            return false;
        }

        *out = tmp;
        return true;
    }
};

} // namespace rewise::domain

#endif // REWISE_DOMAIN_FOLDER_H
