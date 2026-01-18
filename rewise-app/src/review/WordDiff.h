#ifndef REWISE_REVIEW_WORDDIFF_H
#define REWISE_REVIEW_WORDDIFF_H

#include "ReviewTypes.h"

#include <QString>

namespace rewise::review {

class WordDiff final {
public:
    // Produces LCS-based diff by normalized word tokens.
    static DiffResult diffByWords(const QString& referenceText,
                                  const QString& userText,
                                  const NormalizeOptions& opt = {});

private:
    static DiffResult diffTokens(const QVector<QString>& refTokens,
                                 const QVector<QString>& userTokens);
};

} // namespace rewise::review

#endif // REWISE_REVIEW_WORDDIFF_H
