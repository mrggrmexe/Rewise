#ifndef REWISE_REVIEW_TEXTNORMALIZE_H
#define REWISE_REVIEW_TEXTNORMALIZE_H

#include "ReviewTypes.h"

#include <QString>
#include <QVector>

namespace rewise::review {

class TextNormalize final {
public:
    // Produces normalized text according to options.
    // Guarantees: if simplifySpaces==true => no leading/trailing spaces and single spaces between tokens.
    static QString normalize(const QString& input, const NormalizeOptions& opt = {});

    // Tokenizes to normalized "word" tokens (letters/digits), punctuation ignored per opt.
    // Output tokens are lowercased (if opt.toLower).
    static QVector<QString> tokenizeWords(const QString& input, const NormalizeOptions& opt = {});

private:
    static bool isWordChar(const QChar& ch); // letter or number
};

} // namespace rewise::review

#endif // REWISE_REVIEW_TEXTNORMALIZE_H
