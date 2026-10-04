# Repeated cube-turn normalization: scalar publications and retained evidence

Measured from the private Proteus4 baseline `84c2a9b` using the ordinary
Rubix turn definitions in World commit `00ca5c6`. No cube-specific solver or
permanent graph cache was added.

## Changes

1. `ProteusGoal` publishes the destination POV, infonView, size state and value
   state after `apply`. That operation edits those existing objects; it does
   not replace them or modify the registry. Readers of shared state slots still
   wake, while unrelated model readers need not retry. Registry-changing Sets,
   including equality consolidation, retain their conservative publication.
2. `ProteusDefinitions::inheritChanges` skips transferring an already-held
   scalar certificate with the exact same requirement and source evidence.
   This adds no new cache. It does not mark the instance checked, skip ordinary
   type goals, or reuse a stale instance-data check. List, choice and contextual
   requirements still use the existing transfer planner; distinct evidence
   remains distinct even for equal requirements.

## Measurements

Headless integration driver, `g++ -O1`, front clockwise then counterclockwise.
Every run verifies all 26 occupants and exact frames against an independent
geometric oracle and checks that earlier states remain unchanged.

| Variant | First turn | Second turn | First-turn steps | Second-turn steps |
| --- | ---: | ---: | ---: | ---: |
| Baseline | 15.43 s | 263.56 s | 327,949 | 2,117,321 |
| Scoped scalar Set publication | 8.32 s | 161.90 s | 214,271 | 1,565,592 |
| Also omit duplicate scalar evidence transfer | 3.35 s | 44.06 s | 214,271 | 1,565,592 |

These are diagnostic runs, not GUI timings or a timing guarantee. Concurrent
compilation and other runs affect wall time. Step and validation counts make
the avoided work explicit. Timers are cumulative through the end of each turn
and include preceding model/snapshot work, so they are not an exact breakdown
of the printed turn duration.

Through the second turn, the scoped variant performed 10,955,503 fresh source
witness checks. Avoiding duplicate transfers reduced that to 626,587 and
skipped 105,032 transfers. Search steps stayed the same for that second change:
the engine still discovers and schedules those routes, but no longer rechecks
a meaning solely to reinstall the same scalar certificate.

Diagnostic files:

- `/tmp/p4-opt-scalar/baseline.json`
- `/tmp/p4-opt-scalar/scoped.json`
- `/tmp/p4-opt-retained/retained.json`

## Rejected probes

Dense scheduler reachability storage reduced bookkeeping by roughly two seconds
across the sequence but did not materially change overall time. It was not
promoted. Grouping evidence checks within one transfer had no shared-evidence
hits. Reusing published meaning checks only during route discovery also left
the first-turn validation counts unchanged; the repeated work was in Sets.
These probes and their instrumentation remain outside the repository.

## Validation

The rebuilt engine passes the four new checks for scalar Set isolation, shared
value/size subscribers, and retained scalar evidence. The latter also verifies
that an invalid instance still fails its ordinary type check and that a second
proof with different source evidence is retained.

The scalar isolation test uses a value demand to exclude the separate registry
write caused by full equality consolidation. It requires the real scalar Set
to execute, the work relationship to remain, and zero additional steps in 1, 8,
and 32 unrelated subscribers. This is deliberately a boundary test of the
changed publisher, not a claim that all publications are now scoped.

The regenerated `codeDog Proteus.Lib.dog` build succeeded. The graph audit
passed 1,477 cases within its per-test limit; `unit/graphDescriptive/modelPalette`
then passed separately with a 300-second limit, for 1,478 passing cases in total.
The previously failing `unit/graphDescriptive/modelTransition` remains explicitly
excluded. No expectation was relaxed.

- `/tmp/p4-opt-retained/graph-audit/results.json`
- `/tmp/p4-opt-retained/palette.log`
- `/tmp/p4-opt-retained/build-production.log`

The final integration driver was also rebuilt against the regenerated engine,
without probe instrumentation or the uncommitted scheduler experiment. Its
first and second turns took 4.68 and 49.09 seconds, with the same 214,271 and
1,565,592 steps as the optimized probe. This remains a headless `-O1` driver;
the normal CodeDog test build uses `-O2`.

The longer sequences still schedule substantial work. The next optimization
should measure repeated traversal and the creation of commands which later
return no change. Broad registry-change publications also remain a separate
boundary; the scalar improvement does not claim to narrow them.

The optimized probe also passed four consecutive front quarter-turns: every
intermediate placement/frame matched the independent oracle, the fourth state
matched the initial state, and all five snapshots remained immutable. Total
runtime including snapshots was 515 seconds; the old run had timed out at 900
seconds. The four turn durations were 3.82, 45.46, 121.56 and 240.16 seconds,
with 214,271, 1,569,999, 2,105,555 and 2,604,652 steps respectively. This is a
correctness and completion improvement, not a claim that long-sequence
performance is solved. See `/tmp/p4-opt-retained/four.json`.

The final-driver inverse check passed all three immutable snapshots:
`/tmp/p4-opt-final/final-inverse.json`.

All twelve individual face-turn checks passed with the final driver, preserving
26 placements and all earlier states. Their output is in
`/tmp/p4-opt-final/moves.log`. These are semantic checks; no new Slipstream
rendering or animation timing is claimed here.
