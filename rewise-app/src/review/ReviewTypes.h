#ifndef REWISE_REVIEW_REVIEWTYPES_H
#define REWISE_REVIEW_REVIEWTYPES_H

#include <QString>
#include <QVector>

namespace rewise::review {

struct NormalizeOptions final {
    bool toLower = true;
    bool simplifySpaces = true;      // collapse whitespace + trim (via QString::simplified)
    bool removePunctuation = true;   // treat non-letter/digit as separators
};

struct SimilarityResult final {
    int distance = 0;        // Levenshtein distance
    int maxLen = 0;          // max(len(a), len(b)) in UTF-16 code units
    double similarity = 0.0; // 0..1
    int percent = 0;         // 0..100
};

enum class DiffRole {
    Common,   // token exists in both (LCS)
    Added,    // present only in user answer
    Removed   // present only in reference answer
};

struct StyledToken final {
    QString text;     // token text (already normalized)
    DiffRole role = DiffRole::Common;
};

struct DiffResult final {
    QVector<StyledToken> reference; // tokens for reference answer (with Removed/Common)
    QVector<StyledToken> user;      // tokens for user answer (with Added/Common)
};

struct ReviewResult final {
    QString normalizedReference;
    QString normalizedUser;
    SimilarityResult similarity;
    DiffResult diff;
};

} // namespace rewise::review

#endif // REWISE_REVIEW_REVIEWTYPES_H
