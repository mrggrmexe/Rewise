#include "WordDiff.h"
#include "TextNormalize.h"

#include <QVector>
#include <algorithm>

namespace rewise::review {

DiffResult WordDiff::diffByWords(const QString& referenceText,
                                 const QString& userText,
                                 const NormalizeOptions& opt) {
    const QVector<QString> refTokens = TextNormalize::tokenizeWords(referenceText, opt);
    const QVector<QString> userTokens = TextNormalize::tokenizeWords(userText, opt);
    return diffTokens(refTokens, userTokens);
}

DiffResult WordDiff::diffTokens(const QVector<QString>& refTokens,
                                const QVector<QString>& userTokens) {
    const int n = refTokens.size();
    const int m = userTokens.size();

    // DP table for LCS lengths.
    QVector<QVector<int>> dp(n + 1, QVector<int>(m + 1, 0));
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (refTokens[i - 1] == userTokens[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    // Backtrack to build token roles.
    QVector<StyledToken> refOut;
    QVector<StyledToken> userOut;
    refOut.reserve(n);
    userOut.reserve(m);

    int i = n;
    int j = m;
    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && refTokens[i - 1] == userTokens[j - 1]) {
            refOut.push_back({refTokens[i - 1], DiffRole::Common});
            userOut.push_back({userTokens[j - 1], DiffRole::Common});
            --i; --j;
        } else if (i > 0 && (j == 0 || dp[i - 1][j] >= dp[i][j - 1])) {
            refOut.push_back({refTokens[i - 1], DiffRole::Removed});
            --i;
        } else if (j > 0) {
            userOut.push_back({userTokens[j - 1], DiffRole::Added});
            --j;
        }
    }

    // We built reversed order.
    std::reverse(refOut.begin(), refOut.end());
    std::reverse(userOut.begin(), userOut.end());

    DiffResult result;
    result.reference = std::move(refOut);
    result.user = std::move(userOut);
    return result;
}

} // namespace rewise::review
