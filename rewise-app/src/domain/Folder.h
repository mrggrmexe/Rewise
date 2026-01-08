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
        const QString trimmed = name.trimmed();
        if (!id.isValid()) {
            if (whyNot) *whyNot = "Folder.id is null/invalid.";
            return false;
        }
        if (trimmed.isEmpty()) {
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

        const QString idStr = o.value(json_keys::kId).toString();
        if (idStr.isNull() || idStr.isEmpty()) {
            if (error) *error = "Folder.id is missing or not a string.";
            return false;
        }

        const Id parsedId = Id::fromString(idStr);
        if (!parsedId.isValid()) {
            if (error) *error = "Folder.id is not a valid UUID string.";
            return false;
        }

        const QString nameStr = o.value(json_keys::kName).toString();
        if (nameStr.isNull()) {
            if (error) *error = "Folder.name is missing or not a string.";
            return false;
        }

        Folder tmp;
        tmp.id = parsedId;
        tmp.name = nameStr;

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
