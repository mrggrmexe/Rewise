#include "ReviewEngine.h"
#include "TextNormalize.h"
#include "Levenshtein.h"
#include "WordDiff.h"

namespace rewise::review {

ReviewResult ReviewEngine::evaluate(const QString& referenceAnswer,
                                    const QString& userAnswer,
                                    const NormalizeOptions& opt) {
    ReviewResult r;

    r.normalizedReference = TextNormalize::normalize(referenceAnswer, opt);
    r.normalizedUser = TextNormalize::normalize(userAnswer, opt);

    r.similarity = Levenshtein::similarityFromNormalized(r.normalizedReference,
                                                         r.normalizedUser);

    r.diff = WordDiff::diffByWords(referenceAnswer, userAnswer, opt);

    return r;
}

} // namespace rewise::review
