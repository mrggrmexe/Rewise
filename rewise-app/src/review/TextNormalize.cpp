#include "TextNormalize.h"

#include <QChar>

namespace rewise::review {

bool TextNormalize::isWordChar(const QChar& ch) {
    // Unicode-aware: true for Letter_* and Number_* categories.
    return ch.isLetterOrNumber();
}

QString TextNormalize::normalize(const QString& input, const NormalizeOptions& opt) {
    QString s = input;

    if (opt.removePunctuation) {
        // Convert anything that isn't a letter/number or whitespace into a space.
        // This effectively "ignores punctuation" while preserving word boundaries.
        QString out;
        out.reserve(s.size());

        for (QChar ch : s) {
            if (isWordChar(ch) || ch.isSpace()) {
                out.append(ch);
            } else {
                out.append(QChar(' '));
            }
        }
        s.swap(out);
    }

    if (opt.toLower) {
        s = s.toLower();
    }

    if (opt.simplifySpaces) {
        // simplified() trims + collapses internal whitespace to single spaces.
        s = s.simplified();
    }

    return s;
}

QVector<QString> TextNormalize::tokenizeWords(const QString& input, const NormalizeOptions& opt) {
    const QString norm = normalize(input, opt);
    if (norm.isEmpty()) return {};

    // After normalize(simplified), tokens are separated by single spaces.
    const QStringList parts = norm.split(QChar(' '), Qt::SkipEmptyParts);

    QVector<QString> tokens;
    tokens.reserve(parts.size());
    for (const QString& p : parts) tokens.push_back(p);

    return tokens;
}

} // namespace rewise::review
