#ifndef REWISE_DOMAIN_ID_H
#define REWISE_DOMAIN_ID_H

#include <QString>
#include <QUuid>
#include <QHashFunctions>

namespace rewise::domain {

/// Strong-ish ID wrapper on top of QUuid.
/// - String form: "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx" (WithoutBraces)
/// - Parsing: QUuid(QString) accepts with/without braces; invalid -> null UUID.
struct Id final {
    QUuid value;

    Id() = default;
    explicit Id(const QUuid& v) : value(v) {}

    static Id create() { return Id{QUuid::createUuid()}; }

    bool isNull()  const { return value.isNull(); }
    bool isValid() const { return !value.isNull(); }

    QString toString() const {
        // WithoutBraces yields 36-char canonical UUID string.
        return value.toString(QUuid::WithoutBraces);
    }

    static Id fromString(const QString& s) {
        // If conversion fails, QUuid becomes null.
        return Id{QUuid{s}};
    }

    friend bool operator==(const Id& a, const Id& b) { return a.value == b.value; }
    friend bool operator!=(const Id& a, const Id& b) { return !(a == b); }
    friend bool operator<(const Id& a, const Id& b)  { return a.toString() < b.toString(); }
};

} // namespace rewise::domain

// Allow Id as a key in QHash/QSet.
inline uint qHash(const rewise::domain::Id& id, uint seed = 0) noexcept {
    // QUuid has qHash in Qt; if not, this still works because QUuid is hashable in Qt6.
    return qHash(id.value, seed);
}

#endif // REWISE_DOMAIN_ID_H
