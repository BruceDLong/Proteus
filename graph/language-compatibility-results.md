# Older language compatibility, 2026-09-26

Every active row in the six pre-existing top-level `.tst` files was run separately through the
current public test entry point, preserving its original mode, input and expected
text. Each process had a five-second limit. All 363 fixtures completed without a
crash, timeout or incomplete test summary. This audit does not include all legacy
unit tests compiled into `TestProteus`.

| Suite | Passing | Failing | Total |
|---|---:|---:|---:|
| `defaultTests.tst` | 157 | 113 | 270 |
| `demoTests.tst` | 1 | 21 | 22 |
| `sparseTests.tst` | 1 | 22 | 23 |
| `timeTests.tst` | 0 | 36 | 36 |
| `worldLoadTests.tst` | 0 | 2 | 2 |
| `endRelativeR1Tests.tst` | 0 | 10 | 10 |

The R1 file records an experimental contract and explicitly excludes itself from
the default tests; its failures are not evidence of regressions from a previously
passing implementation. Other suites also overlap in coverage, so the combined
159/363 pass count is a fixture inventory, not a measure of language completeness.

The default row was refreshed after source injection; the other
five rows retain the logical-navigation checkpoint. See
[source-injection-results.md](source-injection-results.md) for the latest change.

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
`notTemplate/compoundNum` and `notTemplate/resolvedUnknown` also pass their
rendered-result assertions, but their negated matching predicates remain
unresolved. Their selected literals were already known; these two rows are not
claimed as completed predicate implementations.

The complete per-fixture status and input-file hashes are in
[language-compatibility-status.json](language-compatibility-status.json).
A failure means the original assertion failed; classification into missing
semantics, printing differences, outdated expectations or environment issues
still requires inspecting the actual result. Test-name grouping alone is not a
root-cause analysis.

## Compatibility work next

The constrained ambiguous output example in
[source-injection-results.md](source-injection-results.md) needs a design pass:
carry the selected-output demand through matching before widening grammar support.

Use the failing original examples to choose the next semantic unit, starting
with named part/path selection and its relationship to definition requirements.
For example, `query/chain2` currently leaves `.wheel.spoke.length` unresolved.
Follow existing POV/definition relationships and reuse checked type evidence;
do not add repeated type RHSs just to make each query pass.

Then examine contextual repeated templates and temporal recurrence: the original
`prev/plus1` remains symbolic despite structural `%prev` working. Other clusters
include symbolic span alignment, alternatives/ranges, ordered slicing, and world
loading. For example, `merge/dots/align2a` already gains some member information
but retains a different symbolic layout from the expected output; diagnose that
before deciding it needs an entirely new operation.

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
