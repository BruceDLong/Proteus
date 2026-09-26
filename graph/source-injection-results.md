# Source injection and repeated selection

`<~` now composes sequence correspondence with intersection selection:

```proteus
[*4+$ *10+$] <~ ('DO' 'gsTin' 'tinabulation')
// 'Tintinabul'

{[_ _]| ...} <~ {1 2 3 4 5 6 7 8}
// {2 4 6 8}

{*2+[...]| ...} <~ {1 2 3 4 5 6 7 8}
// {2 4 6 8}

{[<*1+$> *1+$]| ...} <~ 'ABCD'
// {'A' 'C'}
```

## Shared mechanism

`ProteusGraphAccess.innerSourceOwner/applyInnerSource` lowers the stored source
argument to a work constraint. The direct form constrains an inner prefix; the
repeated form supplies one advancing source to the `listSpec`. This follows
Proteus3's `IntersectionReasoner.prepareInnerContext` and
`WorkListReasoner.processWorkList` rules. The new implementation does not modify
`asWrkLstOutr` or `cnvtToFunc` on the shared template.

`SequenceCorrespondence.h` replaces the text-only header. Strings use offsets;
list input uses the existing `ListStructure` cursor, including embedded spans.
Both inputs share the same enter/member/end/repeat continuations. A scalar
mismatch contributes no successor. A successful intersection occurrence
contributes its last or marked result. The graph manager is unchanged.

Only an accepted complete path is compiled into existing `ProteusDefinitionPlan`
Sets. Scalar list input contributes persistent identities to selected output
members. Repeated normalization reuses those members and identities. Unknown
numeric sizes do not prevent matching known values; they keep the result
symbolic and prevent `isNormed`, as in existing grouping semantics.

A direct selection requires proof of the inner relationships, with full inner
normalization available as an exploratory subgoal. Requiring full normalization
would create a completion cycle through the selected-result identity. The
`relationshipConstraints` demand exposes the same relationship edges without
requiring all container members to be fully normalized.

## Boundary needing design work

An existing output constraint must participate in an ambiguous match:

```proteus
{[<{'A'|...}> {'A'|...}]|...} <~ 'AAA' = {{'A'}}
```

This has a possible split where the marked inner list takes one `A` and the
following list takes two. The current implementation leaves it symbolic. A `DefinitionConflict` rejects
only the speculative candidate and produces no Set; strict assertion goals still
report conflicts, and ownership/implementation errors are not caught here. It does
not yet carry the demanded selected output through the match state. Captures are
compiled from an accepted predecessor path, while visited-state sharing is keyed
by cursor and enclosing counts. Before claiming arbitrary constrained ambiguous
parsing, output demands must distinguish relevant continuations. Adding a final
result filter alone is insufficient if sharing already discarded another route.

Named/recursive grammar, negated patterns, source elements whose values are
lists, incomplete streaming-prefix publication, and demanded sparse aliases
remain outside this slice. In particular, no claim is made that all old parsing
tests pass or that this is a complete Earley chart.

## Validation

The focused `unit/graphInner/` tests cover direct and repeated injection, explicit
and virtual inner positions, marked results, variable-length groups, empty input,
embedded source spans, incomplete and rejected groups, finite output extents,
late values, cancellation, template preservation, identity reuse, and the
constrained-output boundary. Existing `unit/graphText/` cases remain unchanged.

See the compatibility inventory for exact older fixture counts. Those fixtures
assert rendered results; passing an assertion does not by itself prove every
predicate was resolved. The two negated-template rows can print their already
known selected literal while retaining unresolved matching work.

Final checks: CodeDog regeneration succeeded, followed by a successful native
header rebuild. All 24 injection, 36 existing text, and 519 other focused checks
pass (579 total). The unchanged default suite has 157 passing assertions and
113 failures, with no prior pass lost and no crashes/timeouts. Other language
suites and the standalone streaming executable were not rebuilt for this slice.

Logs: `/tmp/p4-inner-final-build.log`, `/tmp/p4-inner-candidate-build.log`,
`/tmp/p4-inner-checked24.log`, `/tmp/p4-inner-checked36.log`,
`/tmp/p4-inner-checked-regression.log`, and
`/tmp/p4-inner-checked-default/results.json`.
