#ifndef REWISE_DOMAIN_CARD_H
#define REWISE_DOMAIN_CARD_H

#include "Id.h"
#include "DomainJson.h"

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonValue>

namespace rewise::domain {

/// Flashcard: question + single reference answer.
struct Card final {
    Id id;
    Id folderId;

    QString question;
    QString answer;

    // UTC milliseconds since epoch (stable and timezone-safe in JSON).
    qint64 createdAtMsUtc = 0;
    qint64 updatedAtMsUtc = 0;

    static qint64 nowUtcMs() {
        return QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    }

    void touchCreatedNow() {
        const qint64 now = nowUtcMs();
        createdAtMsUtc = now;
        updatedAtMsUtc = now;
    }

    void touchUpdatedNow() {
        updatedAtMsUtc = nowUtcMs();
        if (createdAtMsUtc <= 0) createdAtMsUtc = updatedAtMsUtc;
    }

    bool isValid(QString* whyNot = nullptr) const {
        if (!id.isValid()) {
            if (whyNot) *whyNot = "Card.id is null/invalid.";
            return false;
        }
        if (!folderId.isValid()) {
            if (whyNot) *whyNot = "Card.folderId is null/invalid.";
            return false;
        }
        if (question.trimmed().isEmpty()) {
            if (whyNot) *whyNot = "Card.question is empty.";
            return false;
        }
        if (answer.trimmed().isEmpty()) {
            if (whyNot) *whyNot = "Card.answer is empty.";
            return false;
        }
        if (createdAtMsUtc <= 0) {
            if (whyNot) *whyNot = "Card.createdAtMsUtc must be > 0.";
            return false;
        }
        if (updatedAtMsUtc <= 0) {
            if (whyNot) *whyNot = "Card.updatedAtMsUtc must be > 0.";
            return false;
        }
        if (updatedAtMsUtc < createdAtMsUtc) {
            if (whyNot) *whyNot = "Card.updatedAtMsUtc must be >= createdAtMsUtc.";
            return false;
        }
        return true;
    }

    QJsonObject toJson() const {
        QJsonObject o;
        o.insert(json_keys::kId, id.toString());
        o.insert(json_keys::kFolderId, folderId.toString());
        o.insert(json_keys::kQuestion, question);
        o.insert(json_keys::kAnswer, answer);

        // JSON numbers are stored as double in Qt JSON, but epoch ms is safe in our range.
        o.insert(json_keys::kCreatedAtMs, static_cast<double>(createdAtMsUtc));
        o.insert(json_keys::kUpdatedAtMs, static_cast<double>(updatedAtMsUtc));
        return o;
    }

    static bool fromJson(const QJsonObject& o, Card* out, QString* error = nullptr) {
        if (!out) {
            if (error) *error = "Card::fromJson: out is null.";
            return false;
        }

        const QJsonValue idV = o.value(json_keys::kId);
        if (!idV.isString()) {
            if (error) *error = "Card.id is missing or not a string.";
            return false;
        }
        const Id parsedId = Id::fromString(idV.toString());
        if (!parsedId.isValid()) {
            if (error) *error = "Card.id is not a valid UUID string.";
            return false;
        }

        const QJsonValue folderIdV = o.value(json_keys::kFolderId);
        if (!folderIdV.isString()) {
            if (error) *error = "Card.folderId is missing or not a string.";
            return false;
        }
        const Id parsedFolderId = Id::fromString(folderIdV.toString());
        if (!parsedFolderId.isValid()) {
            if (error) *error = "Card.folderId is not a valid UUID string.";
            return false;
        }

        const QJsonValue qV = o.value(json_keys::kQuestion);
        if (!qV.isString()) {
            if (error) *error = "Card.question is missing or not a string.";
            return false;
        }

        const QJsonValue aV = o.value(json_keys::kAnswer);
        if (!aV.isString()) {
            if (error) *error = "Card.answer is missing or not a string.";
            return false;
        }

        auto readEpochMs = [&](const char* key, qint64* dst) -> bool {
            const QJsonValue v = o.value(key);
            if (!v.isDouble()) return false;
            *dst = static_cast<qint64>(v.toDouble());
            return (*dst > 0);
        };

        qint64 created = 0;
        qint64 updated = 0;

        // For forward/backward compatibility: if missing, fill with now().
        if (!readEpochMs(json_keys::kCreatedAtMs, &created)) created = nowUtcMs();
        if (!readEpochMs(json_keys::kUpdatedAtMs, &updated)) updated = created;

        Card tmp;
        tmp.id = parsedId;
        tmp.folderId = parsedFolderId;
        tmp.question = qV.toString();
        tmp.answer = aV.toString();
        tmp.createdAtMsUtc = created;
        tmp.updatedAtMsUtc = updated;

        QString why;
        if (!tmp.isValid(&why)) {
            if (error) *error = why;
            return false;
        }

        *out = tmp;
        return true;
    }

    friend bool operator==(const Card& a, const Card& b) { return a.id == b.id; }
    friend bool operator!=(const Card& a, const Card& b) { return !(a == b); }
};

} // namespace rewise::domain

#endif // REWISE_DOMAIN_CARD_H
