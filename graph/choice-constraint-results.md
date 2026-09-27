# Constraints against oneOf alternatives

An equality between oneOf and a scalar or finite structural constraint now
lowers to candidate filtering. The existing field predicate reads the explicit
constraint, the shared list walk visits alternatives, and the path publisher
retains the compatible results. The equality may put the choice on either side.

Known mismatches supply no candidate. Unknown comparisons remain in a symbolic
choice and retain subscriptions; an open source also keeps the result open.
A sole proved candidate supplies its original value, size, and context. Multiple
candidates remain a oneOf, and no match yields an empty oneOf. A collected Set
publishes after traversal so cheaper findings cannot hide slower alternatives.

Additional RHS constraints on the published result feed the same predicate.
The shared candidate source is not overwritten to force a match. Normalization
can refine a positive explicit predicate, and result aliases reuse the original
cursor binding only through a registered correspondence with the same infonView.
A new cursor occurrence without that evidence still fails context revalidation.

This increment supports one strict equality with one explicit choice. It does
not implement Cartesian products of choices, repeated grammar alternatives,
choice-producing definitions, or loose text-prefix choice matching. The later test-completion checkpoint updates `merge/intersect/onRHS` to the
ordinary symbolic oneOf spelling without legacy tentative `t` markers. Its
three candidate values remain alternatives.

Validation:

- Full CodeDog source build succeeded.
- 866/866 focused checks, including 16 choice cases: both equality directions,
  empty/multiple results, unknown and late candidates, cancellation, open/late
  ends, structured/embedded/nominal candidates, new constraints, relative source
  context, and identity-stable replay.
- Original default suite: 189/270 passing, up from 186. Recovered
  `merge/num/literalSelect`, `merge/str/literalSelect`, and `altSelect/simple`.
  No prior pass was lost; no fixture expectations were changed.
- The generic search and span planners are unchanged in this increment.

Logs: `/tmp/p4-choice-final-build.log`, `/tmp/p4-choice-verified-summary.log`,
`/tmp/p4-choice-default/results.json`.
