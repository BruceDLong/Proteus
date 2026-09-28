# Older language compatibility, 2026-09-27

The obsolete AItem engine has been removed. The source build and
**1,091/1,091 registered graph tests** pass. All six older language suites retain
exactly their previous individual statuses: **261 passed, 92 failed, and 8
explicit ambiguity skips**, out of 361. No language fixture or expectation
changed during this cleanup. See [aitem-cleanup-results.md](aitem-cleanup-results.md).

The preceding boundary increment introduced `NULL:<ID>` and universal
`.begin`/`.end` access, with 26 graph tests. See
[boundary-results.md](boundary-results.md).

Named grammar choices and basic plural/text rules were recovered in the preceding
increment; see [word-grammar-results.md](word-grammar-results.md).

Recovered default fixtures: `parse/lang`, `word/pluralize`, and `lang/splitText`.
The bounded plural-lookup regression passes; the old `word/findPlural` fixture
remains unchanged and is skipped pending its open-unordered result contract.
The user-requested removal deletes `prev/plus1`, `tprev/plus1`, and two compiled
arithmetic-relative tests. Direct `%prev`/`%next` navigation remains supported.
Two additional unchanged fixtures now pass: `unit/day/fromHours/readFirstMinute`
and `r1/correspondence/contentConstrainedSuffix`.

The earlier repetition change made `range/select1` expose its completed
`2 3 4 5` prefix with an unresolved tail, following the user's clarification.
Its old closed-result expectation was updated in that earlier increment.

| Suite | Passing | Failing | Ambiguous skips | Total |
|---|---:|---:|---:|---:|
| `defaultTests.tst` | 243 | 17 | 8 | 268 |
| `demoTests.tst` | 8 | 14 | 0 | 22 |
| `sparseTests.tst` | 2 | 21 | 0 | 23 |
| `timeTests.tst` | 4 | 32 | 0 | 36 |
| `worldLoadTests.tst` | 0 | 2 | 0 | 2 |
| `endRelativeR1Tests.tst` | 4 | 6 | 0 | 10 |

All six language suites were rerun without crashes, timeouts, or incomplete
summaries. The 58 retained helper checks report **50 pass / 8 existing fail**;
every retained status is unchanged. The cleanup retired 67 registrations that
directly tested the deleted engine (52 previously passed, 15 failed), plus one
unregistered declaration. These are recorded in
[retired-aitem-tests.json](retired-aitem-tests.json), not counted as semantic
recoveries. The R1 file records an experimental contract and excludes itself
from the default tests. Helper passing counts do not establish graph coverage.
Focused checks include some language fixtures, so the counts must not be added.
Exact current helper results are in [legacy-helper-status.json](legacy-helper-status.json).

The standalone streaming build also passed, with **33/33 checks** across the
supported `-f 1`, `2`, `4`, `5`, `6`, and `8` selections. Cases 3 and 7 remain
outside this migrated coverage.

The unchanged standalone scheduler passed **38/38** at the preceding checkpoint,
also under address, undefined-behavior, and leak sanitizers. Earlier unchanged
standalone checks passed: list structure **18/18**, span alignment
**374/374**, and numeric ranges **1,225 interval pairs** plus boundary checks. The documented span-test command exposed a signed-comparison warning in
its test assertion; explicit size casts fixed that build without changing the
span algorithm or its expectations.

## Compatibility work order

The priority is dependable existing behavior before new natural-language work.

1. Completed repetition results are implemented for loaded sources and later
   revisions. Unknown result tails remain subscribed. Parser-owned open input
   needs a separate pending-cursor extension; it remains deferred.
2. Closed, context-free strict equalities now move out of pending work into
   the registry, retaining their owner and identity. Partial/qualified/typed
   relationships remain pending. Rejected sources and contexts are preserved;
   contradictions stop their owning goals and required parents, while
   exploratory parents keep other routes. See [goal-rejection-results.md](goal-rejection-results.md).
3. Work through remaining failures by the semantic constructs they exercise:
   remaining scalar/sequence relationships and contextual definitions,
   definition composition, and temporal/sparse navigation. Validate the first
   actual failure against Proteus3 before selecting a fix.

Keep the original semantic expectations unless an explicit language decision
changes them. Update obsolete internal assertions to test the graph contract;
do not reinstate unused agenda machinery solely to satisfy those assertions.
Natural-language expansion is deferred. A failing fixture remains documented
until its behavior works or its contract is explicitly revised.

## Latest behavior

- Strict scalar exclusions share a three-way compatibility decision; definite
  overlay mismatches reject without changing either operand. Correspondence
  checks explicit member contradictions before publishing identities.
- `%ctx` and unary `~word` reuse path selection over the Agent's topic history.
  Counted context sizes are invalidated on membership edits. Context-dependent
  definitions must preserve their original binding; implementation is deferred.

- Arguments use shared first/last navigation. Open intersection results can use
  symbolic registry endpoints; qualified RHS dependencies are explicit graph
  prerequisites. Explicit-source `~` lookup gives direct fields precedence over
  external `^` associations. See [input-resolution-results.md](input-resolution-results.md).

- Qualified scalar strings share the scalar-relation goal with numeric ranges.
  A colon supplies prefix value evidence without inferring a missing target size;
  a known target width selects that prefix. See
  [qualified-text-results.md](qualified-text-results.md).

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

`merge/directRange/endReject` rejects the endpoint and retains the RHS; its
expectation was updated at the previous checkpoint. This is separate from
consolidation of accepted information. `range/select1` now passes with the approved open-tail
expectation; it does not claim the unfinished repetition is complete.

A passing printed assertion alone does not prove all normalization obligations
resolved: unknown numeric sizes and subscribed open tails remain symbolic.

The complete fixture status and input hashes are recorded in
[language-compatibility-status.json](language-compatibility-status.json).
Current language logs are in `/tmp/p4-aitem-language-final`; compiled logs are
in `/tmp/p4-aitem-compiled-final`. The previous compiled baseline is preserved
in `/tmp/p4-null-compiled-final`.

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

The auditor reports exact, hash-checked ambiguity skips separately. Use
`--include-ambiguous` to run those fixtures as well. The skip manifest is
[test-ambiguities.json](test-ambiguities.json).

The auditor now also accepts `--compiled` to read registered names from
`ProteusTests.dog`; `--exclude` can omit a name prefix. Compiled checks use exact
names and the same per-process bound and summary validation.

Only the standalone scheduler received sanitizer coverage; no full-engine
sanitizer or GUI integration pass is claimed. Legacy helper failures remain
part of the recorded compatibility work.
