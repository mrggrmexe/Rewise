#ifndef REWISE_DOMAIN_ID_H
#define REWISE_DOMAIN_ID_H

#include <QString>
#include <QUuid>

namespace rewise::domain {

/// Strong-ish ID wrapper based on QUuid.
/// Serializes to a compact UUID string (WithoutBraces).
struct Id final {
    QUuid value;

    Id() = default;
    explicit Id(const QUuid& v) : value(v) {}

    static Id create() { return Id{QUuid::createUuid()}; }

    bool isNull() const { return value.isNull(); }
    bool isValid() const { return !value.isNull(); }

    QString toString() const {
        // 00000000-0000-0000-0000-000000000000
        return value.toString(QUuid::WithoutBraces);
    }

    static Id fromString(const QString& s) {
        // Qt accepts UUID strings with or without braces; invalid => null UUID.
        return Id{QUuid{s}};
    }

    friend bool operator==(const Id& a, const Id& b) { return a.value == b.value; }
    friend bool operator!=(const Id& a, const Id& b) { return !(a == b); }
};

} // namespace rewise::domain

#endif // REWISE_DOMAIN_ID_H
