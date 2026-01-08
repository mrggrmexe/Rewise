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

    // UTC milliseconds since epoch (stable, timezone-safe in JSON).
    qint64 createdAtMsUtc = 0;
    qint64 updatedAtMsUtc = 0;

    static qint64 nowUtcMs() {
        return QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
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

    /// Call when creating a new card.
    void touchCreatedNow() {
        const qint64 now = nowUtcMs();
        createdAtMsUtc = now;
        updatedAtMsUtc = now;
    }

    /// Call when modifying an existing card.
    void touchUpdatedNow() { updatedAtMsUtc = nowUtcMs(); }

    QJsonObject toJson() const {
        QJsonObject o;
        o.insert(json_keys::kId, id.toString());
        o.insert(json_keys::kFolderId, folderId.toString());
        o.insert(json_keys::kQuestion, question);
        o.insert(json_keys::kAnswer, answer);
        o.insert(json_keys::kCreatedAtMs, static_cast<double>(createdAtMsUtc));
        o.insert(json_keys::kUpdatedAtMs, static_cast<double>(updatedAtMsUtc));
        return o;
    }

    static bool fromJson(const QJsonObject& o, Card* out, QString* error = nullptr) {
        if (!out) {
            if (error) *error = "Card::fromJson: out is null.";
            return false;
        }

        const QString idStr = o.value(json_keys::kId).toString();
        if (idStr.isNull() || idStr.isEmpty()) {
            if (error) *error = "Card.id is missing or not a string.";
            return false;
        }
        const Id parsedId = Id::fromString(idStr);
        if (!parsedId.isValid()) {
            if (error) *error = "Card.id is not a valid UUID string.";
            return false;
        }

        const QString folderIdStr = o.value(json_keys::kFolderId).toString();
        if (folderIdStr.isNull() || folderIdStr.isEmpty()) {
            if (error) *error = "Card.folderId is missing or not a string.";
            return false;
        }
        const Id parsedFolderId = Id::fromString(folderIdStr);
        if (!parsedFolderId.isValid()) {
            if (error) *error = "Card.folderId is not a valid UUID string.";
            return false;
        }

        const QString q = o.value(json_keys::kQuestion).toString();
        if (q.isNull()) {
            if (error) *error = "Card.question is missing or not a string.";
            return false;
        }

        const QString a = o.value(json_keys::kAnswer).toString();
        if (a.isNull()) {
            if (error) *error = "Card.answer is missing or not a string.";
            return false;
        }

        auto readMs = [&](const char* key, qint64* dst) -> bool {
            const QJsonValue v = o.value(key);
            if (!v.isDouble()) return false;
            // JSON numbers are double; for epoch ms this is safe in our range.
            *dst = static_cast<qint64>(v.toDouble());
            return (*dst > 0);
        };

        qint64 created = 0;
        qint64 updated = 0;
        if (!readMs(json_keys::kCreatedAtMs, &created)) {
            // For forward/backward compat: if missing, set to "now".
            created = nowUtcMs();
        }
        if (!readMs(json_keys::kUpdatedAtMs, &updated)) {
            updated = created;
        }

        Card tmp;
        tmp.id = parsedId;
        tmp.folderId = parsedFolderId;
        tmp.question = q;
        tmp.answer = a;
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
