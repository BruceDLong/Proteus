# Older language compatibility, 2026-09-26

Every active row in the six pre-existing top-level `.tst` files was run separately
through the public test entry point, preserving its original mode, input and
expected text. Each process had a five-second limit. All 363 fixtures completed
without a crash, timeout or incomplete test summary at their recorded checkpoints.
The default suite has been refreshed for this change; the other five rows retain
the logical-navigation checkpoint. This audit does not include all legacy unit
tests compiled into `TestProteus`.

| Suite | Passing | Failing | Total |
|---|---:|---:|---:|
| `defaultTests.tst` | 164 | 106 | 270 |
| `demoTests.tst` | 1 | 21 | 22 |
| `sparseTests.tst` | 1 | 22 | 23 |
| `timeTests.tst` | 0 | 36 | 36 |
| `worldLoadTests.tst` | 0 | 2 | 2 |
| `endRelativeR1Tests.tst` | 0 | 10 | 10 |

The R1 file records an experimental contract and explicitly excludes itself from
the default tests; its failures are not evidence of regressions from a previously
passing implementation. Other suites also overlap in coverage, so the combined
166/363 pass count is a fixture inventory, not a measure of language completeness.

The default row was refreshed after chained field selection, recovering
`query/chain` and `query/chain2` with no prior pass lost. See
[chained-field-results.md](chained-field-results.md) for the latest change.

The default suite was audited before and after logical navigation. It improved
from 135/270 to 138/270, with no formerly passing case lost. The recovered cases
are `query/first`, `query/getNth`, and `query/getNthSub`. They all use the common
positive intersection prefix endpoint rule. No original fixture or expected
output was edited. The other suite counts are current snapshots, not before/after
regression comparisons. Numeric normalization subsequently recovered `parse/frac`,
`neg/simple`, and `fracs/simple`, bringing the default suite to 141/270 without
losing any previous pass. Text correspondence subsequently recovered five parsing
fixtures and four whole-string scalar cases, reaching 150/270 with no previous pass lost.
Source injection raises the assertion count to 157/270, again losing no prior pass.
Five newly supported examples are `concat/parseSrc`, `query/firstStrB`,
`outr/everyOther1`, `outr/everyOther2`, and `marked/lookahead`.
At that checkpoint, `notTemplate/compoundNum` and `notTemplate/resolvedUnknown`
passed only their rendered-result assertions while retaining unresolved predicates.
Scalar exclusion now proves those predicates, with focused tests checking the
relationship proof as well as output. It also restores `query/firstStrC` and
`notTemplate/notNum`, bringing the default count to 159/270 with no previous pass
lost and no crashes or timeouts. Scalar field selection then recovered
`read/readStrA` and `temporal/sBeginField`, reaching 161/270 without losing any
previous pass. The latter is an ordinary `.begin` field query, not temporal
boundary support. Its rendered-result assertion passes while the bare numeric
size remains unknown; this does not establish `isNormed`. Nominal and finite
structured field selection then recovered `query/byType1`, reaching 162/270
without losing any previous pass or changing an original fixture. Chained field
selection subsequently recovered `query/chain` and `query/chain2`, reaching
164/270 without losing a prior pass.

The complete per-fixture status and input-file hashes are in
[language-compatibility-status.json](language-compatibility-status.json).
A failure means the original assertion failed; classification into missing
semantics, printing differences, outdated expectations or environment issues
still requires inspecting the actual result. Test-name grouping alone is not a
root-cause analysis.

## Compatibility work next

The constrained ambiguous output example in
[source-injection-results.md](source-injection-results.md) intentionally remains
symbolic and subscribed until more information arrives, per the user's
clarification. Its waiting state is an expected result.

Use the failing original examples to choose the next semantic unit, starting
with field updates and retained-parent paths, such as `query/chain3` and
`query/chain4`. Read-only `.wheel.spoke.length` now resolves without validating
every field of each intermediate object's definition.
Dot paths lower to negated skip plus positive selection in Proteus3, so the scalar
predicate prerequisite was implemented first, followed by read-only scalar type
meaning compilation, single-step dot selection, and finite ordered structural
predicates. Nominal selection now checks the type word and explicit query
constraints without demanding full definition validation first. Contextual
predicates still require careful composition with type evidence. The user clarified that omitted items
in an unordered list are non-relevant to that instance; they must not become
missing-field subscriptions or force insertion of every definition member.
The finite ordered-list requirement planner cannot be reused unchanged for that
case. List size still counts the items the instance holds.

The user clarified that ordered lists can have typed fields, as in a C struct.
Keep the space-separated `query/chain*` fixtures: selection by type does not
imply unorderedness or require punctuation changes. Selecting an existing member
and validating the entire container against a positional definition are separate
goals; a printed selected value alone does not establish full container validation.

These are typed members, not string-keyed fields. Nominal identity, additional
types, and explicit alias relationships establish compatibility; equal field
layout alone does not. Structural/property constraints further restrict a query.
A literal name is represented as `field^{name=X}`. Follow existing POV relations
and reusable type evidence without inserting repeated type RHSs.

The user clarified ordered-first and unordered-oneOf selection, then retracted
the assumption that equal structure establishes type compatibility. A `.wheel`
query must not select a `pedals` member based only on equal structure. The old
`query/byType1` and `query/chain` expectations remain valid under this correction.
See [field-selection-contract.md](field-selection-contract.md).

The latest unordered-list clarification gives partial descriptions an implied
open tail. End of stored members is not proof that other parts do not exist.
The current complete-candidate collection does not yet implement that implicit
tail or partial continuation through a oneOf. The role of explicit size in
closing membership is pending clarification.

For each semantic change, run its original fixtures alongside focused graph
checks, then compare the default inventory so recovered behavior stays recovered.
Keep unresolved cases visible; do not relax expected outputs or normalization
flags merely to improve the pass count.

## Reproduce

Build from the fork root with `codeDog Proteus.Lib.dog`, then run:

```sh
python3 graph/audit_language_suite.py --output /tmp/proteus4-default-audit
python3 graph/audit_language_suite.py --suite timeTests.tst --output /tmp/proteus4-time-audit
python3 graph/audit_language_suite.py --select query/ --output /tmp/proteus4-query-audit
```

Use a separate output directory per suite. The script records incremental
`results.json` and a numbered log for each fixture. It returns nonzero for an
assertion failure, process error, timeout or incomplete summary. This compensates
for the underlying harness returning zero even when an assertion fails.

No sanitizer, GUI integration or full compiled legacy-unit pass is claimed for
this checkpoint.
