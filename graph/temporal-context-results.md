# Temporal context reads, support, and cuts

The relative-context rule is `LHS.current = RHS.prev`, equivalently
`LHS.next = RHS.current`. This increment exposes temporal reads as graph
relationships. It does not redefine the general assignment parser or implement
new word/context binding rules.

## Navigation and value identity

`%tprev`/`%tnext` now use the same neighbor walk as `%prev`/`%next`, extracted
into `RelativeNavigation.h`. Its temporal policy enters nested T-spans and
explicit embedded spans, and stops at ordinary S-list contexts. An S-list used
as a T-list member supplies one state value. The normal structural policy still
stops at an ordinary nested list, including a nested T-list.

The walk reuses the existing sparse/indexed routes, open-end requirements,
subscriptions, costs, and cancellation. An open preceding span cannot be
skipped to reach an earlier value. Nested normalization retains the temporal
owner chain. Streaming reads retain evidence of the parser's ordered T-context;
a later incompatible reinterpretation fails the dependent request.

A temporal read records a `temporalValueSource`, exposed by `POVDescription`
for value acquisition but excluded from occurrence-identity traversal. This
allows a later state to acquire the preceding value while their beginning cuts
remain distinct. S-context cursor behavior is unchanged. Existing Sets and
constraint comparison still detect conflicting values.

## Support and interior cuts

`TemporalGraph.h` implements graph goals over existing POVs and registry records:

- `findTemporalSupport(source)` and `.asTList` return a stable temporal support
  reference. An existing T-list is its own span. Other occurrences have a lazy
  open T-span with unknown member count, linked to their source boundaries.
- `findTemporalCut(source)` creates a fresh symbolic strict-interior cut.
  `.cut` names one such cut per syntax occurrence and reuses it on replay.
  Prefix and suffix witnesses stay virtual and retain the three concatenation
  boundary identities. Independent cut requests are not automatically identical.
- `findTemporalSide(cut, after)` follows the cut's source support. For a known
  S-state, both sides return the exact same source POV without copying a state
  or identifying the two witness spans. For an unlocated cut in a changing
  T-history, the result remains symbolic and subscribed.

Unknown support remains an obligation even after a cut obtains its name. A
known null or empty T-span cannot contain a strict interior cut; later discovery
of an empty source rejects subscribed requests. Equal-valued but distinct state
occurrences retain separate supports. Explicit occurrence aliases relate their
support spans through the existing identity registry.

All semantic allocation and identity publication is done by priced Sets.
Inspection does not allocate virtual members. There is no temporal branch in
`GraphSearch.h` and no new scheduling system.

## Validation

- Full CodeDog regeneration and C++ build passed.
- **36/36** new temporal tests passed, covering direct/nested reads, normal
  structural boundaries, sparse billion-item access, open-end waiting, replay,
  cancellation, cost exclusion, aliases, distinct supports for repeated values,
  contradictions, late information, and streaming context preservation.
- Compiled audit: **1,177 pass / 8 existing fail**, out of 1,185.
  All **1,127 graph tests pass**. The 58 retained helper tests remain
  **50 pass / 8 fail**, and every pre-existing compiled status is unchanged.
- Default language suite: **247 pass / 13 fail / 8 documented skips**.
  `temporal/directTPrev`, `temporal/directTNext`, `temporal/nestedBoundaryPrev`,
  and `temporal/nestedBoundaryNext` now pass without changing their fixtures.
- All six language suites: **265 pass / 88 existing fail / 8 skips**.
  Apart from those four recoveries, every individual status is unchanged.
- Standalone streaming source build passed; supported selections
  `-f 1`, `2`, `4`, `5`, `6`, and `8` passed **33/33 checks**.
  Selections 3 and 7 remain outside the migrated streaming coverage.
- No process errors, timeouts, or incomplete audit summaries. No old fixture,
  expectation, registration, or ambiguity skip was changed. `git diff --check`
  passed. No new sanitizer or GUI integration pass was run.

Audits: `/tmp/p4-temporal-compiled-final`, `/tmp/p4-temporal-language-final`, and
`/tmp/p4-temporal-stream-*.log`. The exact inventories are
`language-compatibility-status.json` and `legacy-helper-status.json`.

## Remaining implementation boundaries

General interval ordering/overlap constraints, coordinates and units, locating
a cut in a changing history, supported parent/part-role and schema mappings,
and temporal word-reference binding are not implemented here. A child history
may contain several states; this increment does not infer its alignment with a
parent interval or synchronize their transitions. Intersections that collect
multiple temporal supports/cuts remain a subsequent step.

The temporal-side API selects the support retained by that cut's reference. It
is not a global search for every system obtaining at an identical cut. The
existing `.begin`/`.end` typed-field naming question remains deferred.

## Later increment: located cuts

[Temporal cut results](temporal-cut-results.md) adds side lookup through known
boundary identities, nested T-spans, explicit history aliases, and existing sparse
occurrences. Unlocated cuts remain unresolved. General parent/part projection and
verb application are still outside the implemented boundary.


## Explicit value independence (2026-10-02)

`%?` declares that the later value must not be obtained through default temporal
preservation. It supplies neither a value nor an inequality. For example,
`{T *10+7 %?}` leaves the later value unresolved, while an additional explicit
constraint may supply either `*10+7` or `*10+8`. Ordinary `?` and `_` retain their
existing meanings; their construction history has no temporal significance.
The two characters must be contiguous: `% ?` is still two infons. No new `!%`
behavior or change to bare `%` cursor binding is part of this increment.

The declaration is stored on the occurrence's view, separately from its value
and size. POVDescription exposes it to the graph. The existing relationship
walk requests a small declaration goal when a strict same-occurrence constraint
supplies the marker; it performs no additional graph scan. A temporal value-only
reference does not transfer the previous occurrence's declaration. Compiled
definitions retain the declaration as part of their requirements and evidence.

The printer retains `%?` after resolution, for example `%? = *10+7`, and keeps
an explicit embedded span when collapsing it to `...` would erase the marker.
RHS consolidation requires the target to retain any declaration supplied by its
RHS. Re-normalization reuses the result without republishing the declaration.

This establishes the language and graph representation for the preservation
exception. General adjacency-based preservation, causal precedence, and the
cube's remaining inherited fields are still unimplemented. `%?` does not erase
an already established value or disable an explicit temporal read.

Validation for this increment: final CodeDog/C++ source build succeeded;
**1,347/1,347 graph tests** across 63 suites and **57/57 parser tests** passed.
The 15 new graph cases cover token separation, preflight, ordinary unknowns,
pending and late values, equal/different explicit values, contradictions, RHS
transfer, prior-value reads, definitions, cloning/clearing, selected fields,
stream fragmentation, embedded rendering, replay, and print/parse round trips.
Four language parser cases also cover the new spelling. The shared preflight
helpers now accept a single-letter identifier and an integer ending at closed
input; ordinary `b:?` and `? = *10+7` are included as controls.

Logs: `/tmp/p4-temporal-marker-final-build.log`,
`/tmp/p4-temporal-marker-audit/results.json`, and
`/tmp/p4-temporal-marker-parser.log`. No full legacy-language or GUI run was
performed for this syntax/declaration increment.
