# Sparse correspondence and definition-derived units

A list's size still counts its own members. A four-cell list can supply a view
of two pairs without becoming identical to that two-member container. Unit
conversion follows homogeneous ordered definitions, with no time-unit names or
requirement that the storage or definitions be temporal.

## Shared semantic operations

- `UnitMeasure` reads the finite multiplicative extent of a homogeneous
  definition. Cycles, incompatible leaf types, nonintegral conversions and
  unsupported value constraints cannot supply this route.
- `UnitLayout` walks stored structure into leaf-unit coordinates. Ordinary
  nesting is entered only with unit evidence. Sparse gaps retain their original
  source and offset; they are never enumerated by this inspection.
- `UnitProjection` plans member views and compressed runs. Both unit selection
  and fixed sequence correspondence use it. The priced graph action creates
  views; matching leaf occurrences retain source identities.
- A virtual unit span records a source, member description, base offset and
  count. Indexed graph access exposes that relationship before creating an
  unrelated sparse occurrence. Looking into a previously skipped range works
  through the same graph goal, including subsequent writes to its source.
- `FixedSpanCorrespondence` handles bounded positional constraints and
  compressed spans. It preflights the entire supported route before applying
  identities or fills. Output-producing repeated grammars continue to use
  their existing sequence matcher; an empty output prefix is not a fixed input
  correspondence.
- `SparseOccurrence` publishes a supplied value at its canonical original
  position, splitting only the touched span. Untouched ranges remain compressed.

End-relative intersection lowering preserves two independent facts: its window
has fixed endpoints, and its input relationship may permit unit conversion.
Qualified RHS paths must be searchable before choosing a correspondence route.
Repeated structural predicates can compare a fixed template with supplied
members without allocating one predicate node per position.

The context retains only demanded unit views and virtual span relationships.
These are source-backed relationships, not a second expanded list graph.
Applying a different unit definition does not mutate the shared declaration.

## Retained fixture discrepancies

The original `sparseTests.tst` and `timeTests.tst` inputs and expected outputs are
preserved for comparison. Not every remaining textual failure is a missing
unit conversion:

- Some expect unresolved template text even though all supplied members now
  normalize, or omit the derived plural collection type.
- A bare `second` now normalizes to the explicit unknown numeric `second:_`.
- `sparse/implicit/readAfterSparsePrefixInSizedCompositeList` uses `<_>` to
  select a list-valued minute. `_` is numeric, whereas `?` has an unknown
  overlay. Substituting `<?>` returns `minute:*60+{second| ... }`; the old
  expectation also omits its known size. Generic regressions cover both the
  unknown-overlay selection and the numeric mismatch.
- The old unit writes replace known numbers, such as 8 with 80. Existing path
  assignment rejects contradictory known values. No overwrite exception is
  introduced for units; the desired assignment policy is awaiting clarification.

## Validation at this checkpoint

The ordinary `cd LocalBuild && ./TestProteus` run improves from **82/1610**
failures/tests at `f21e170` to **45/1638**. All 45 remaining failures were already
present: 37 original failures recovered, no new failure names. The extra 28
checks exercise units without time-specific names, ordinary and temporal lists,
source identity, unknown-value writes, cost limits, cycles, mismatch rejection,
replay, and billion-position sparse runs.

| Coverage | Result |
| --- | --- |
| New sparse/unit regressions | 28/28 pass |
| All graph regressions, isolated processes | 1252/1252 pass |
| Streaming selections 1, 2, 4, 5, 6, 8 | 33/33 pass |
| Original sparse suite | 20/23 pass, previously 2/23 |
| Original time/unit suite, ordinary run | 23/36 pass, previously 4/36 |

The five-second isolated language audit records 23 passes, 12 assertion failures,
and one timeout in `timeTests.tst`. The timed-out
`unit/nestedDay/readMinutesAcrossHourNegativeIntersection` completes in 5.18
seconds when run separately: its only assertion difference is the `minutes:`
label. Keep the timing issue visible; do not classify the five-second audit as
13 ordinary assertion failures. Other language suites retain their baseline
results, including eight explicitly documented ambiguity skips in the default
audit. The ordinary full command still executes those eight fixtures.

The original `.tst` fixtures are unchanged. One compiled expectation changes:
`unit/graphSlice/templateDeferred` is now `templateMismatch`. Its fixed predicate
`{*10+1 *10+2}@[0:*2+{*10+2|...}]` can prove the first member contradicts the
repeated `2`, so it returns an empty oneOf instead of staying deferred.

Commands used, from the repository root unless noted:

```sh
codeDog Proteus.Lib.dog
python3 graph/audit_language_suite.py --compiled --select unit/graph --output /tmp/p4-graph-audit
python3 graph/audit_language_suite.py --suite sparseTests.tst --output /tmp/p4-sparse-audit
python3 graph/audit_language_suite.py --suite timeTests.tst --output /tmp/p4-unit-audit
# From LocalBuild:
./TestProteus
./TestProteus -t unit/graphOrderedUnits/
# Build from the root, then run each supported selection from LinuxBuild:
codeDog streamReplyTest.dog
./streamReplyTest -f 1
```

All fixture names, remaining assertion details, recovered failures, and the
other language-suite counts are retained in [sparse-unit-status.json](sparse-unit-status.json).
The bounded language audit is also recorded in
[language-compatibility-status.json](language-compatibility-status.json).

## Remaining sparse/unit fixtures

Six differences concern normalized output:

- `sparse/scarcity/readSecondLastByNegativeIndex`
- `unit/hour/fromSeconds/readTwoMinutesNegativeIntersection`
- `unit/day/fromMinutes/readFirstHour`
- `unit/day/fromMinutes/readSecondHour`
- `unit/day/fromMinutes/readLastHourNegative`
- `unit/nestedDay/readMinutesAcrossHourNegativeIntersection` (also slow above)

One uses a numeric pattern to request a list:

- `sparse/implicit/readAfterSparsePrefixInSizedCompositeList`

Nine require a decision about replacement of known values:

- `sparse/pending/writeSpanAcrossSparseConcreteSparse`
- `unit/nestedDay/writeMinutesAcrossHourNegativeSpan`
- `unit/nestedDay/writeMinutesAcrossHourNegativeIntersection`
- `unit/world/dayFromSeconds/writeNestedSecond`
- `unit/world/dayFromSeconds/writeNestedLastSecondNegative`
- `unit/world/dayFromSeconds/writeLastMinuteNegativeSpan`
- `unit/world/dayFromSeconds/writeLastMinuteNegativeIntersection`
- `unit/world/dayFromSeconds/writeTwoMinutesNegativeSpan`
- `unit/world/dayFromSeconds/writeTwoMinutesNegativeIntersection`

Writes that supply unknown values are covered and preserve source identities.
No replacement exception is attached to unit words or temporal lists. The shared
assignment question is in [open-questions.md](open-questions.md).

## Supported boundary

The conversion route requires finite homogeneous ordered unit definitions with
a compatible nominal leaf. It declines cycles, nonintegral widths, heterogeneous
or unordered unit definitions, and unsupported value/range constraints. Ordinary
list order supplies coordinates; temporal meaning is not a prerequisite.
Existing graph routes remain available where this compressed route has no proof.
General regrouping inside unknown composite-unit occurrences is still deferred;
known nested members and virtual runs of scalar leaves are covered. Unsupported
routes must not erase constraints or allocate the full virtual domain.
