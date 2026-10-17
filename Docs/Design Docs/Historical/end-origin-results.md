# End-relative intersection origins

A negative inner extent is a source-end coordinate, not a negative list
cardinality. `*(-2)+[<_> _] <~ source` now supplies its pattern from the same
bounded source-window query used by bracketed slices. The pattern retains its
ordinary constraints and selected member. No source prefix is materialized.
Parenthesized signed literals share coordinate reading with positional paths.

The resulting equality uses normal list correspondence. Constraint-only walks
preserve their demand across relationships and check aligned member constraints
in the same walk. They do not recursively require full normalization of the
selected input, which would create a completion cycle. Grammar correspondence
continues to own its predicates; an exclusion predicate is not reinterpreted as
an asserted member value.

Unknown source ends subscribe. Cancellation prevents resumption; replay reuses
window pieces and identities. Fixed logical item extents are supported here;
conversion between typed traversal units remains outside this increment.

Validation:

- Source regeneration succeeded, followed by the native header rebuild.
- 817/817 focused checks, including 12 new end-origin cases: finite and embedded
  sources, selected spans, a billion-item source, suffix constraints/conflicts,
  unknown and late ends, cancellation, out-of-range input, and identity replay.
- Generic suites: 30 search, 220 span, and 18 list-structure cases passed.
- Original defaults: 177/270 passing, up from 172. All five end-relative slice
  fixtures recovered; no previous pass was lost. The suite remains unchanged.

Logs: `/tmp/p4-end-final-build.log`, `/tmp/p4-end-native-build.log`,
`/tmp/p4-end-verified-summary.log`, `/tmp/p4-end-verified-focused.json`,
`/tmp/p4-end-default/results.json`.
