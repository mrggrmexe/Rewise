#ifndef REWISE_REVIEW_REVIEWENGINE_H
#define REWISE_REVIEW_REVIEWENGINE_H

#include "ReviewTypes.h"

namespace rewise::review {

class ReviewEngine final {
public:
    // Main entry point: normalize -> percent -> word diff.
    static ReviewResult evaluate(const QString& referenceAnswer,
                                 const QString& userAnswer,
                                 const NormalizeOptions& opt = {});
};

} // namespace rewise::review

#endif // REWISE_REVIEW_REVIEWENGINE_H
