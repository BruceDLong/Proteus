#pragma once

// Both sequence predicates and field queries need proof of exclusion. Missing
// facts are not a mismatch, even when another candidate is already available.
enum class ScalarMatch { yes, no, unknown };
inline ScalarMatch compareScalar(GraphTypeFacts required, const GraphTypeFacts& actual) {
    if (!required.valid || !actual.valid) return ScalarMatch::unknown;
    if (required.accepts(actual)) return ScalarMatch::yes;
    return required.combine(actual) ? ScalarMatch::unknown : ScalarMatch::no;
}
