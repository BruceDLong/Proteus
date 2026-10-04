# Scoped record and identity publications

This implements the first step of the manual-normalization review: finish
precise invalidation for ordinary record copying. Completed-goal reuse, cost
repricing, traversal, and normalization semantics are unchanged.

## Changes

`ProteusRecordGoal` returns the changed header/state slots, member-plan
footprints, and old tail position when appending members. It uses the existing
definition plan's scoped application instead of discarding its publication data.
Named-occurrence imports and unconverted compound plans retain broad publication.

The small `WriteScope` collector records indirect identity insertions during the
atomic Set. `IdentityRegistry.recordIdentity` records endpoints and scoped
occurrence names only for new identities. The collector publishes nothing during
the operation; its footprint joins the Set result afterward. This uses existing
dependency keys, with no adjacency index or retained graph copy.

Direct relationship readers already observe endpoint POVs. Transitive
`areIdentical` reads now observe visited endpoints and occurrence names, including
when no connecting path exists yet. A link added to an intermediate endpoint or
another POV with the same occurrence name can invalidate that finding without
waking unrelated readers. Duplicate identity insertion records no write.

Definition-projected scalar facts notify the affected view and old/new state
slots, including shared-value aliases. Copying `%?` notifies the affected view.

## Comparable two-turn measurement

O1 integration drivers ran sequentially after compilers, the audit, and other cube
checks finished. Before is `c1684d1`; after includes only this publication change.
Both load the same World files, execute a front clockwise turn and its inverse,
validate all 26 placements/frames, and verify all three retained states remain
unchanged. Times are headless, one run each. The CodeDog test build uses O2.

| Measurement | Before | After |
| --- | ---: | ---: |
| First turn: initial to clockwise | 3.686 s | 1.616 s |
| Second turn: clockwise to inverse | 32.778 s | 5.488 s |
| Process including loading/snapshots | 57.474 s | 27.019 s |
| Peak RSS | 676.8 MiB | 222.8 MiB |
| First-turn recorded steps | 214,271 | 42,934 |
| Second-turn recorded steps | 1,568,322 | 106,770 |

Separate aggregate instrumentation counted the second turn:

| Measurement | Before | After |
| --- | ---: | ---: |
| Distinct goal keys | 7,006 | 7,006 |
| Distinct task/query combinations visited | 14,387 | 14,387 |
| Information-changing commands | 822 | 822 |
| Whole-model publications | 181 | 0 |
| Graph visits | 709,569 | 49,037 |
| Subgoal requests | 458,661 | 28,035 |
| Task restarts, excluding initial starts | 392,206 | 21,812 |
| Restarts attributed to record Sets | 358,562 | 3,810 |

The unchanged cardinalities do not prove equality of every internal trace, but
combined with the state oracle they support the intended reduction in repeated
work. Average visits per distinct task/query combination fall from 49.3 to 3.4.
Of the remaining restarts, 12,126 come from route-cost repricing, which still
clears task progress. This is a measured follow-up target, not changed here.

Four consecutive front clockwise turns pass the geometry oracle at every step
and preserve all five snapshots. Turn times: 1.624, 5.288, 7.326, 8.696
seconds; process time 82.527 seconds; peak RSS
358.4 MiB. This longer run has no fresh paired baseline.

## Validation and limits

- Regenerated `codeDog Proteus.Lib.dog` build: `Marker: Build Successful`.
- **1,498 selected graph tests pass**, retaining the existing
  `unit/graphDescriptive/modelTransition` exclusion. `modelPalette` is included.
- Nine new tests cover unrelated subscribers at 1/8/32 readers, transitive and
  named identities, deferred publication, duplicate insertion, shared record
  values and appended positions, projected scalar aliases, `%?` aliases, and a
  live unresolved reader acquiring a value through a newly added identity.
- All 12 separate cube turns pass the independent geometry oracle.
- Clockwise/inverse and four-clockwise sequences pass, including prior-state
  immutability checks.
- Slipstream was not rebuilt or visually tested. Other registry operations and
  unconverted compound writes can still use broad publication.

Temporary evidence: `/tmp/p4-record-publication/` contains before/after/four
logs and measurements, graph audit, move checks, and diagnostic log. The earlier
diagnostic baseline is `/tmp/p4-manual-review/analysis.json`.
