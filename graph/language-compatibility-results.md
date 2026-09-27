# Older language compatibility, 2026-09-26

The default suite passes **194/270** fixtures after the user-approved canonical
expectations for `merge/dots/align3` and `align4`. Those two assertions now pass;
all other fixtures retain their prior status. There were no crashes, timeouts,
or incomplete summaries. This update changes expected printing only.

| Suite | Passing | Failing | Total |
|---|---:|---:|---:|
| `defaultTests.tst` | 194 | 76 | 270 |
| `demoTests.tst` | 1 | 21 | 22 |
| `sparseTests.tst` | 1 | 22 | 23 |
| `timeTests.tst` | 0 | 36 | 36 |
| `worldLoadTests.tst` | 0 | 2 | 2 |
| `endRelativeR1Tests.tst` | 0 | 10 | 10 |

Only the default row has been refreshed. The other five rows retain the earlier
logical-navigation snapshot; they were not rerun for this change. The R1 file
records an experimental contract and excludes itself from the default tests.
This table is a fixture inventory, not a measure of language completeness, and
it does not include all compiled legacy unit tests.

## Latest behavior

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

The exact original assertions for `merge/dots/align2a` and `align2b` still fail
because they expect the older nested `&{ ... }` spelling. Both spellings parse
to the current canonical form in focused round-trip checks. They are still
counted as failures; those two fixtures remain unchanged. Likewise,
a passing printed assertion alone does not prove all normalization obligations
resolved: unknown numeric sizes and subscribed open tails remain symbolic.

The complete fixture status and input hashes are recorded in
[language-compatibility-status.json](language-compatibility-status.json).
Detailed logs for this checkpoint are in `/tmp/p4-canonical-expectations-final/`.

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
```

The auditor runs each active fixture separately through the public entry point,
preserving its mode, input, and expected text, with a five-second limit per
process. It records incremental JSON and one log per fixture and returns nonzero
for failures, errors, timeouts, or missing summaries. The underlying test harness
can exit zero even when assertions fail.

No sanitizer, GUI integration, or full compiled legacy-unit pass is claimed.
