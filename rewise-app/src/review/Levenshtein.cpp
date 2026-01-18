#include "Levenshtein.h"

#include <QVector>
#include <algorithm>

namespace rewise::review {

int Levenshtein::distance(const QString& a, const QString& b) {
    const int n = a.size();
    const int m = b.size();
    if (n == 0) return m;
    if (m == 0) return n;

    QVector<int> prev(m + 1);
    QVector<int> cur(m + 1);

    for (int j = 0; j <= m; ++j) prev[j] = j;

    for (int i = 1; i <= n; ++i) {
        cur[0] = i;
        const QChar ca = a.at(i - 1);

        for (int j = 1; j <= m; ++j) {
            const QChar cb = b.at(j - 1);
            const int cost = (ca == cb) ? 0 : 1;

            const int del = prev[j] + 1;
            const int ins = cur[j - 1] + 1;
            const int sub = prev[j - 1] + cost;

            cur[j] = std::min({del, ins, sub});
        }
        prev.swap(cur);
    }

    return prev[m];
}

SimilarityResult Levenshtein::similarityFromNormalized(const QString& normalizedA,
                                                       const QString& normalizedB) {
    SimilarityResult r;

    const int lenA = normalizedA.size();
    const int lenB = normalizedB.size();
    r.maxLen = std::max(lenA, lenB);

    // Define: if both empty => 100%, else if one empty => 0%.
    if (r.maxLen == 0) {
        r.distance = 0;
        r.similarity = 1.0;
        r.percent = 100;
        return r;
    }

    r.distance = distance(normalizedA, normalizedB);
    r.similarity = 1.0 - (static_cast<double>(r.distance) / static_cast<double>(r.maxLen));

    // Clamp due to any potential rounding.
    if (r.similarity < 0.0) r.similarity = 0.0;
    if (r.similarity > 1.0) r.similarity = 1.0;

    r.percent = static_cast<int>(std::lround(r.similarity * 100.0));
    if (r.percent < 0) r.percent = 0;
    if (r.percent > 100) r.percent = 100;

    return r;
}

} // namespace rewise::review
