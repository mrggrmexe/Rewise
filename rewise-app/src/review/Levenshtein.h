#ifndef REWISE_REVIEW_LEVENSHTEIN_H
#define REWISE_REVIEW_LEVENSHTEIN_H

#include "ReviewTypes.h"

#include <QString>

namespace rewise::review {

class Levenshtein final {
public:
    // Standard Levenshtein distance (insert/delete/replace).
    // Uses UTF-16 code units (QString indexing). Good for typical short texts.
    static int distance(const QString& a, const QString& b);

    // Convenience: compute SimilarityResult from already-normalized strings.
    static SimilarityResult similarityFromNormalized(const QString& normalizedA,
                                                     const QString& normalizedB);
};

} // namespace rewise::review

#endif // REWISE_REVIEW_LEVENSHTEIN_H
