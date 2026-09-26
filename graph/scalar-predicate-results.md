# Scalar exclusion in sequence correspondence

The sequence matcher can now use scalar `!pattern` templates without mutating
those templates or enumerating failed parses. This is a prerequisite of generic
dot selection, whose Proteus3 expansion skips `!path` and then selects `path`.
Typed scalar matching is added in the subsequent
[typed-predicate-results.md](typed-predicate-results.md) checkpoint. Path lowering
itself is not implemented by either change.

Examples from the unchanged older suite:

```proteus
[&{!$|...} $]<~{2 {'Hi' 3} 4 'Cat' 'Hat'}
// 'Cat'

[&{!_|...} _]<~{'A' {1 2} 3 4}
// 3

[&{!*12+5|...} *12+5]<~{*10+5 *12+4 *12+5}
// *12+5
```

## Rule and implementation

The positive facts of a scalar predicate are its overlay, stated size, and
value. A known disagreement in any field proves a mismatch. If no field
contradicts the predicate but a required fact is missing, the result is unknown.
Only a proved mismatch advances a negated match. This follows Proteus3's
scalar-template polarity rules in `InfonViewReasoner.planConsolidation`.

`ProteusGraphAccess.predicateFacts` reads the positive facts of a complemented
template. It shares the existing structural guards, while ordinary `typeFacts`
continues to reject complemented values as positive facts. There is no mutation
of the pattern, and the generic graph scheduler is unchanged.

`SequenceCorrespondence` performs this check before exposing a continuation.
Only an accepted complete correspondence supplies a Set. The existing source
member refinement and graph publication/subscription mechanism allow unknown
values or sizes to arrive later. Cancellation still stops that search.

A successful atomic list input contributes an identity to the existing source
POV. Ordinary graph correspondence resolves its members. On replay, that identity
keeps the list one consumed input member rather than interpreting the copied
list value as new grammar. Scalar and compound matches have separate completion
edges so this distinction survives advancement of the source cursor. Shared
templates remain read-only, and replay reuses the same output POVs and identities.

## Boundaries

This handles scalar predicates used to instantiate sequence templates. Composite
negation must prove the whole subpattern, so negated lists/intersections remain
deferred. Standalone complemented destinations, named predicates, recursive
grammars, and structural matching inside a list-valued input remain future work.
No named lookup shortcut, new definition RHS, or legacy agenda fallback was added.

Known output alone is not proof of successful selection: the focused tests check
that the inner/source relationship was proved, in addition to the printed value.
Unknown numeric sizes still leave results symbolic; they are not fabricated just
to set `isNormed`.

## Validation

The older default suite improves from **157/270 to 159/270**, with no prior pass
lost and no process errors, crashes, or timeouts. The recovered assertions are
`query/firstStrC` and `notTemplate/notNum`. The previously printable selected
literals in the compound/late-value examples now have actual predicate proofs.
No original fixture or expected output was changed.

CodeDog source regeneration and the native build succeeded. All **18 predicate,
24 source-injection, 36 text, and 519 other focused checks pass (597 total)**.
The other language suites and standalone streaming executable were not refreshed.

Focused checks cover overlay/value/size exclusion, partial facts, late value and
size publication, cancellation, equal candidates, read-only templates, atomic
list identity, replay, completion flags, and deferred composite exclusion.

Logs: `/tmp/p4-predicate-final-build.log`, `/tmp/p4-predicate-final-tests.log`,
`/tmp/p4-predicate-inner.log`, `/tmp/p4-predicate-text.log`,
`/tmp/p4-predicate-regression.log`, and `/tmp/p4-predicate-default/results.json`.

Validation correction: the subsequent full source regeneration exposed an
accidentally copied predicate-completion assertion in `graphNavigation/repeat`.
It contradicted the same test's non-symbolic result assertion. That assertion is
corrected in the typed-predicate checkpoint; see its regenerated test results.
The earlier executable's passing result did not cover that final source edit.
