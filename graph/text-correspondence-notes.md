# Text correspondence through successful continuations

This slice supports finite list-to-text and whole-string scalar results using the graph frontier. It
restores fixed-width and repeated parsing without an AItem fallback.

## Mechanism

`TextCorrespondence.h` exposes enter/member/boundary/accepted edges over existing
POVs. Query coordinates hold input progress and enclosing list counts. The
generic graph manager still owns the frontier, cost selection, cancellation,
subscriptions and visited-state sharing; it has no text-matching algorithm.

A definite scalar mismatch exposes no continuation. Repetition exposes its end
boundary and next occurrence. Missing facts leave the goal symbolic, so ordinary
publication can restart it. Matching never installs tentative occurrences.
An accepted path builds a finite `DefinitionRequirement`; existing
`ProteusDefinitionPlan` Sets populate instances. Accepted embedded extents are
plan-local evidence, so they need not be published before the whole Set.
Qualified RHSs expose their stored size/value expressions before choosing a
correspondence route. Existing concatenation goals traverse operand identities,
subscribe to unknown operands, and publish the resulting payload. No recursive
normalizer or extra expression evaluator is introduced.

String copies reuse Proteus3's `mergeLooseStrings` size-mode rule.

Source reference: `Reasoner.dog` `mergeLooseStrings`,
`MergeReasoner.processStep1`, and `ParentMemberReasoner.propagate`. Failed
speculative matches do not mean terminal graph-engine exceptions. Existing
conjunctive scalar-conflict handling is unchanged.

## Ambiguity and open-prefix policy

The user chose the first complete match for now. For example,
`{{'A'|...} {'A'|...}} == 'AAA'` may settle as `{{} {'A' 'A' 'A'}}`.
The scheduler's cost and edge order choose that first route; equivalent cursor
states share the first predecessor path. This is deliberately not a complete
Earley chart or a representation of every parse.

A future `oneOf` result will need shared continuation/predecessor evidence so
semantically different alternatives are retained without enumerating failed
paths. That work is deferred by the current first-match policy.

A prefix relationship does not prove the end of an unsized open root list.
`{'A'|...} ==: 'AAA'` therefore stays symbolic in this finite-result slice.
It must eventually gain partial-prefix Sets; selecting an empty match and closing
the list would discard that unresolved extent. Finite outer lists can still use
prefix correspondence, including a repeated inner list followed by a delimiter.

Other deferred boundaries include named/recursive grammar, intersections inside
matches, unspecified string widths, partial publication within a string,
demanded sparse aliases, and joint conflicting writes to aliased destinations.
The source currently supports subscriptions to later publication of a complete
string or a member's size; it does not yet stream committed parse prefixes.

## Evidence

- Full CodeDog regeneration and C++ build succeeded.
- Focused correspondence: 36/36 pass after the first-match/open-prefix policy update.
- All 519 pre-existing focused graph/parser/numeric checks pass.
- Standalone graph search: 28/28; list traversal: 18/18; span alignment: 200/200.
- Unchanged older default fixtures: 150/270 pass, 120 fail, versus 141/270 at
  `ae03ed3`; no previously passing row regressed.
- Recovered: `parse/parse3n4`, `parse/catHatDogPig`, `parse/simple2`,
  `parse/reps1`, `parse/reps2`, `merge/str/loose`,
  `merge/directLooseStr/exactWidthCopy`, `merge/directLooseStr/exactLiteralAccept`,
  and `merge/unknown/loose`.
- The historical Proteus3 executable does not pass every newly proposed nullable
  or embedded example; those are not claimed as historical regressions restored.
- Original fixture inputs and expectations were not edited.

Logs: `/tmp/p4-text-expression-default/results.json`, `/tmp/p4-text-expression-regression.log`,
`/tmp/p4-text-expression-build.log`. Focused tests are in `unit/graphText/`; log: `/tmp/p4-text-expression-tests.log`.
