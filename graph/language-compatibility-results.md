# Older language compatibility, 2026-09-27

Consolidated-RHS cleanup preserves all previous language results: **203/270**
default fixtures and **218/363** across all six suites. The source build and
**1,002/1,002** focused graph/parser checks pass, including 18 new RHS lifecycle
checks. No language input or expected output changed in this increment. See
[rhs-consolidation-results.md](rhs-consolidation-results.md).

The preceding repetition change made `range/select1` expose its completed
`2 3 4 5` prefix with an unresolved tail, following the user's clarification.
Its old closed-result expectation was updated in that earlier increment.

| Suite | Passing | Failing | Total |
|---|---:|---:|---:|
| `defaultTests.tst` | 203 | 67 | 270 |
| `demoTests.tst` | 8 | 14 | 22 |
| `sparseTests.tst` | 2 | 21 | 23 |
| `timeTests.tst` | 3 | 33 | 36 |
| `worldLoadTests.tst` | 0 | 2 | 2 |
| `endRelativeR1Tests.tst` | 2 | 8 | 10 |

All six rows were rerun for consolidated-RHS cleanup: **218/363**
language fixtures pass, with **145** failures and no crashes, timeouts, or
incomplete summaries. The R1 file records an experimental contract and excludes
itself from the default tests. Fixture counts describe tested examples, not
unrestricted support for a language feature.

The **1,002/1,002 focused graph/parser checks** are a separate baseline. They
include some of the language fixtures above, so these counts must not be added.
The 125 additional registered legacy helper and parser-error checks were also
rerun: **102 pass, 23 fail**, without process errors or timeouts. Their exact
inventory is [legacy-helper-status.json](legacy-helper-status.json). Some call
legacy reasoners or assert old registry bookkeeping; inspect those contracts
before treating a failure as missing graph behavior or a pass as graph coverage.

At the preceding checkpoint, unchanged standalone checks passed: graph search **30/30**, list structure **18/18**, span
alignment **374/374**, and numeric ranges **1,225 interval pairs** plus boundary
checks. The documented span-test command exposed a signed-comparison warning in
its test assertion; explicit size casts fixed that build without changing the
span algorithm or its expectations.

## Compatibility work order

The priority is dependable existing behavior before new natural-language work.

1. Completed repetition results are implemented for loaded sources and later
   revisions. Unknown result tails remain subscribed. Parser-owned open input
   needs a separate pending-cursor extension; it remains deferred.
2. Closed, context-free strict equalities now move out of pending work into
   the registry, retaining their owner and identity. Partial/qualified/typed
   relationships remain pending. Top-level rejection lifetime needs the
   separate decision recorded in [open-questions.md](open-questions.md).
3. Work through remaining failures by the semantic constructs they exercise:
   scalar/sequence relationships, contextual and external-property resolution,
   definition composition, and temporal/sparse navigation. Validate the first
   actual failure against Proteus3 before selecting a fix.

Keep the original semantic expectations unless an explicit language decision
changes them. Update obsolete internal assertions to test the graph contract;
do not reinstate unused agenda machinery solely to satisfy those assertions.
Natural-language expansion is deferred. A failing fixture remains documented
until its behavior works or its contract is explicitly revised.

## Latest behavior

- Direct numeric matching, interval intersection, and range predicates share
  exact half-open bounds. See [numeric-range-results.md](numeric-range-results.md).
- `align2a` and `align2b` now use the same shorter canonical spelling already
  approved for `align3` and `align4`. Only their expected text changed.

- Missing scalar fields can acquire facts from the corresponding parent-definition
  field without expanding omitted parts. See
  [definition-projection-results.md](definition-projection-results.md).
- Crossing spans with proven relative placement now share one unresolved middle.
  See [crossing-span-results.md](crossing-span-results.md).
- `merge/dots/align3` and `align4` now expect the shorter `...` spelling in place
  of `&{ ... }` and pass. Focused checks retain coverage of equivalent legacy
  parsing. Ambiguous `align4b` remains unresolved.

## Earlier recovered behavior

- Strict oneOf filtering recovers `merge/num/literalSelect`,
  `merge/str/literalSelect`, and `altSelect/simple`.
- Open-span correspondence and representation recover `merge/list/dotsC`.
- Normalizing supplied typed descriptions without enforcing the full parent
  definition recovers `query/chain3` and `query/chain4`. The existing assignment
  succeeds and `;` or `;;` returns the appropriately normalized partial parent.

See [choice-constraint-results.md](choice-constraint-results.md),
[open-span-correspondence-results.md](open-span-correspondence-results.md), and
[partial-type-results.md](partial-type-results.md). Earlier checkpoints remain
in the per-feature reports linked from [README.md](README.md).

`merge/directRange/endReject` still rejects the endpoint but retains the RHS,
whereas the old assertion expects it removed. This is separate from consolidation
of accepted information. `range/select1` now passes with the approved open-tail
expectation; it does not claim the unfinished repetition is complete.

A passing printed assertion alone does not prove all normalization obligations
resolved: unknown numeric sizes and subscribed open tails remain symbolic.

The complete fixture status and input hashes are recorded in
[language-compatibility-status.json](language-compatibility-status.json).
Detailed language and helper audit logs are in `/tmp/p4-rhs-audit-*`.

## Remaining work

Ambiguous crossing spans need additional placement evidence. General nested or
contextual parent requirements remain beyond scalar field projection. Contextual
type meaning, unordered structural correspondence, recursive grammars, fuller stream
publication, and temporal operations retain their documented boundaries.
Ambiguous writes also need the semantic decision tracked in
[open-questions.md](open-questions.md).

Use each failing fixture as evidence to investigate. Do not add a rule for its
name or relax its expected output to improve the count. Typed fields remain
nominal members: equal structure does not make `.pedal` match `.wheel`, and
space-separated typed fields do not become unordered merely because selection
uses a type word. See [field-selection-contract.md](field-selection-contract.md).

## Reproduce

Build with `codeDog Proteus.Lib.dog`, then run:

```sh
python3 graph/audit_language_suite.py --output /tmp/proteus4-default-audit
python3 graph/audit_language_suite.py --suite timeTests.tst --output /tmp/proteus4-time-audit
python3 graph/audit_language_suite.py --select query/ --output /tmp/proteus4-query-audit
python3 graph/audit_language_suite.py --compiled --exclude unit/graph --output /tmp/proteus4-helper-audit
```

The auditor runs each active fixture separately through the public entry point,
preserving its mode, input, and expected text, with a five-second limit per
process. It records incremental JSON and one log per fixture and returns nonzero
for failures, errors, timeouts, or missing summaries. The underlying test harness
can exit zero even when assertions fail.

The auditor now also accepts `--compiled` to read registered names from
`ProteusTests.dog`; `--exclude` can omit a name prefix. Compiled checks use exact
names and the same per-process bound and summary validation.

No sanitizer or GUI integration pass is claimed. Legacy helper failures remain
part of the recorded compatibility work.
