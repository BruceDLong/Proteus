# Avoiding empty Sets and repeated RHS representation checks

This continues the scalar-transfer measurements from `c0d49c9`, with the
ordinary Rubix turn definitions from World `00ca5c6`. No model-specific
execution paths were added.

## Changes

* Scalar discovery and execution share `GraphCopyPlan`. Discovery queues a Set
  only when a destination (including an intermediate scalar) might acquire a
  field or a type certificate. Compatibility is still checked. List counting
  uses the same approach, keeping extent validation even when no size is set.
* Record correspondence reads each member's identity links once within that
  comparison. The temporary sets disappear when the comparison returns; the
  next comparison reads the live POV/registry structure again.
* Field-source and predicate discovery reuse positive RHS representation
  proofs immediately, through the existing `representedWork` records. A proof
  means the RHS's current facts are represented, not that the infon is fully
  normalized. This is useful when partial colors and other values remain
  symbolic. No new graph, negative-result cache, or list of tried POVs is stored.

Proof reuse observes dependencies, requires the producing graph context's
adapter token, and handles both scoped and broad publications. Untracked calls
and mutation planners use fresh checks. Proof retention at search completion
uses the same helper as early discovery.

## Diagnostic observations

On the second front turn (clockwise, then counterclockwise):

* The baseline queued 59,093 scalar Sets; only 295 changed facts. The preflight
  experiment reduced this to 7,703, keeping uncertain certificate transfers.
* All 27,434 attempted counted-size Sets were empty. The final implementation
  filters these before scheduling them.
* The per-comparison identity snapshot reduced relationship accesses from
  3,167,509 to 1,942,903 and registry records scanned from 1,144,550,449 to
  639,556,776. This is a local consolidation, not a persistent registry index.
* RHS representation constructors cost about 12 seconds across callers.
  Reusing proofs with proper invalidation reduced that to about 3.2 seconds;
  field-source readiness fell from about 7 seconds to 0.55 seconds.
* The proof experiment reused 10,789 checks and created/replaced 720 proofs
  during the turn. Creation counts are cumulative, not simultaneous memory
  occupancy. Invalid records are discarded when new proofs are retained.

Each diagnostic variant checked all 26 occupants and frames against the
independent geometric oracle and verified earlier-state immutability. Timing
runs overlapped some compilations, so call counts establish the avoided work;
final measurements below use drivers without instrumentation.

Reading only proofs left by a completed search had zero useful hits during
these turns. Retaining successful checks during discovery was necessary.
The first early-retention probe lacked broad-update invalidation; it was not
accepted. The measured retained version includes that invalidation and requires
matching context evidence.

## Final validation and measurements

The regenerated `codeDog Proteus.Lib.dog` build succeeded. All **1,489 selected
graph tests passed**, including eleven new regressions. The previously excluded
`unit/graphDescriptive/modelTransition` remains excluded; expectations were not
changed. All twelve individual turns passed the independent geometry and
immutable-history checks.

New coverage includes no-op discovery, contradictions, intermediate filling,
late source values, retained/distinct type evidence, fresh identity snapshots,
proof reuse, shared-state publication, broad publication, proof consumers,
cross-context checks, untracked reads, and unpublished mutation-boundary checks.

The before/after runs used uninstrumented `g++ -O1` drivers, sequentially after
compilation and the other tests finished. Both validated front clockwise then
counterclockwise and all three immutable snapshots.

| Measurement | Before | After |
| --- | ---: | ---: |
| First turn | 3.698 s | 3.652 s |
| Second turn | 40.144 s | 31.652 s |
| Full process, including model load and snapshot inspection | 67.595 s | 55.270 s |
| Peak resident memory | 675.7 MiB | 676.8 MiB |
| Second-turn recorded search steps | 1,565,592 | 1,568,322 |

The second turn is about **21% faster**, with essentially unchanged peak memory
in this comparison. Most savings come from less work per visit. Reusing proofs
also lowers route costs, slightly changing traversal order and step count.
These are individual runs, not statistical timing guarantees.

Four front quarter-turns also passed: all 26 placements/frames agreed with the
oracle after every turn, the final state equaled the initial state, and all five
snapshots remained unchanged. Total process time was **230.7 seconds**,
including model load and snapshot verification; peak resident memory was
1200.6 MiB.

| Quarter-turn | Normalization time |
| --- | ---: |
| 1 | 3.656 s |
| 2 | 31.519 s |
| 3 | 53.279 s |
| 4 | 82.387 s |

The previous instrumented four-turn reference in
[scalar-transfer-performance.md](scalar-transfer-performance.md) took 515.4
seconds overall, with turn durations of 3.82, 45.46, 121.56 and 240.16 seconds.
That is a historical comparison, not the controlled pair above. The stronger
improvement on later turns is consistent with avoiding repeated nested proofs.
Longer histories still perform millions of search steps; this change does not
make their normalization constant-time.

Validation artifacts are under `/tmp/p4-next-final/`: `graph-audit/results.json`,
`moves.log`, the new focused test logs, and the sequence JSON/log files. These
are headless engine checks; Slipstream rendering was not rebuilt or inspected.
