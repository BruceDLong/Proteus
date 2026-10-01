# Incremental normalization measurements

## 1. Narrower invalidation (2026-10-01)

Baseline: `3b5fd5a` / functional engine `926d579`. Intermediate result handling
was unchanged during this comparison.

Path preparation and result publication now report the objects they change.
The adapter records reads of POV bindings, shared views, path-selection state,
and intersection bodies. Choice publication includes reused aliases and both
endpoints of new registry identities. A named occurrence binding retains the
broad fallback because it can introduce additional registry relationships.

The scheduler still uses its existing dependency subscriptions. The new object
index stores opaque evidence tokens, not nodes or adjacency, and does not own
model objects. Duplicate reads are suppressed within each callback; nested
proof captures have independent read sets. Definition validation advances its
validation epoch on data writes without waking unrelated goals. Other model
producers retain their existing conservative footprints.

Most header changes are mechanical replacements of direct `pItem` access with
one live-view reader. The CodeDog adapter records its POV arguments and indirect
sparse source reads at the same boundary. No Rubix-specific code was added.

### Visible cube benchmark

The real Slipstream Rubix and Kanban models were run in isolated displays with
fresh application data. The milestone is the first visibly colored cube, not
completion of every symbolic goal. Polling brackets are roughly 1.1 seconds.
No build or test process ran concurrently with these measurements.

| Pair | Baseline visible cube (s) | Narrower invalidation (s) | Baseline visits | Narrower visits |
| --- | ---: | ---: | ---: | ---: |
| 1 | 22.72–23.85 | 12.21–13.31 | 201,594 | 72,811 |
| 2 | 20.51–21.65 | 12.66–13.84 | 184,473 | 71,025 |
| 3 | 22.62–23.79 | 13.96–15.12 | 201,618 | 79,966 |
| Median upper bound | **23.79** | **13.84** | **201,594** | **72,811** |

That is approximately **42% less elapsed time** and **64% fewer accepted graph
visits** at this milestone. The earlier first-known-hue milestone did not improve:
its median was 5.82 seconds before and 6.49 seconds after. The benefit is chiefly
in finishing the remaining cube work without replaying unrelated searches.
The sequence/chart counters stayed zero on this workload.

Both widgets were visually checked. The normal Slipstream executable was not
replaced. The candidate used regenerated `ProteusGraphAccess` declarations and
all 119 methods from the test build, with the remaining GUI source, assets,
compiler flags and first-hue probe unchanged from the control.

An initial undeduplicated reader reduced visits but regressed wall time to
39.57 seconds. Deduplicating callback reads removed that overhead. The final
measurements above also include precise alternative-result publication.

### Validation

- 1,313/1,313 checks across 60 graph groups.
- 13/13 insertion checks and 14/14 legacy parsing checks: **1,340/1,340 total**.
- 62/62 native scheduler scenarios.
- New tests cover 1, 8, and 32 independent subscribers while eight paths resolve,
  a closed two-result oneOf, preserved unrelated completion evidence, and a real
  later RHS waking its consumer. Unrelated subscriber step counts stay unchanged.
- Native coverage checks context-local object keys, nested captures, relevant
  invalidation, and fallback outside a tracked context.
- CodeDog regeneration/build and `git diff --check` passed. The full older default
  suite was not rerun for this performance change.

Artifacts: `/tmp/p4-cube-chart-bench/narrow-control-{1,2,3}/` and
`narrow-final-{1,2,3}/` contain results, logs and screenshots. The candidate build
is `narrow-invalidation-all/`; the control is `definition-transfer/`.
`/tmp/p4-stage-bench.py` refreshes the isolated candidate from regenerated source;
`/tmp/p4-stage-tests.py` runs the behavior groups. Test logs are in
`/tmp/p4-cube-chart-bench/narrow-all-tests/`.

At this checkpoint, intermediate scalar discoveries and list-walk reuse were
unchanged. They are measured separately below.


## 2. Preserve intermediate results (2026-10-01)

Control: `5996034`, measured again immediately before each candidate run.
The candidate contains that change plus the following two forms of retained work:

- A scalar finding is applied to the requested POV and plain scalar intermediate
  POVs on its validated RHS route. The existing Set primitive fills only missing
  fields. The whole batch is checked before any write, and type-check evidence
  is transferred as before. Existing RHS consolidation removes work now fully
  represented, while preserving registry identities for later contradictions.
  Merely filling fields does not certify all of an intermediate's constraints.
- List discovery depends on positions and their count contributions. Its required
  member goals still resolve values and validate constraints. Replacing an
  ordinary member's payload no longer throws away the completed walk. Embedded
  spans still read their size; a newly known extent must wake the count. Changes
  to adjacency, membership classification, or parse errors also invalidate it.

The position token uses a stable slot in the existing POV. It adds neither a
new graph node nor a persistent expansion cache. Path publication compares the
relevant position facts and publishes that token only when those facts change.
Other model producers continue to use the conservative fallback.

A publication audit also found that a distinct view can share the mutable state
inside a choice without sharing its inner-view address. Choice rebuilding now
uses broad publication in that case. A regression explicitly exercises it; the
extra benchmark counter stayed **zero in all three cube runs**, so this safeguard
was not a contributor to the measured result.

### Controlled list traversal

Each fixture is an ordinary list of independent paths resolving to a scalar.
The count below includes accepted `listMember` visits of the outer list only,
not all work needed to resolve its members.

| Members | Control position visits | Retained walk position visits |
| --- | ---: | ---: |
| 16 | 522 | 16 |
| 64 | 6,909 | 64 |

The control used the same regenerated test program with the first-stage graph
headers and original counting method. Both control outputs resolved correctly but exceeded the new single-walk
regression expectation. The candidate also
checks that a symbolic embedded extent wakes counting and computes the outer
size when it becomes known.

### Visible cube benchmark

The same isolated Slipstream procedure was used, with builds and tests stopped
while timing. The candidate includes the regenerated `GraphListMeasure::add`
as well as all 119 `ProteusGraphAccess` methods. GUI code and assets are unchanged.

| Pair | Narrower invalidation only (s) | Plus retained results (s) | Control visits | Candidate visits |
| --- | ---: | ---: | ---: | ---: |
| 1 | 12.55–13.63 | 12.62–13.71 | 83,818 | 72,969 |
| 2 | 12.04–13.15 | 12.80–13.98 | 70,977 | 69,195 |
| 3 | 13.96–15.08 | 12.83–14.01 | 72,815 | 68,680 |
| Median upper bound | **13.63** | **13.98** | **72,815** | **69,195** |

This second change reduced median visits by about **5%**, but did **not demonstrate
an elapsed-time improvement** on the cube. The observed time ranges overlap and
the median upper bound is slightly slower. First-known-hue medians were 5.78 and
6.05 seconds, respectively. Sequence/chart counters remained zero. Both the
colored cube and Kanban were visually checked.

The first-stage control here is a fresh set of runs, which is why its median
is 13.63 rather than the 13.84 reported in the earlier comparison. The large
cube improvement is attributable to narrower invalidation; the second stage
principally removes repeated list walks and preserves useful scalar discoveries.

### Validation

- **1,347/1,347** checks: 1,320 across 61 graph groups, 13 insertion checks, and
  14 legacy parsing checks.
- **62/62** native scheduler scenarios.
- Six new progress tests cover list scaling, embedded extent resumption, retained
  scalar intermediates, partial size/value findings, immediate conflicts, and
  later contradictions through consolidated identities.
- One additional publication test covers distinct views sharing choice state.
- Three existing expectations changed deliberately: `graphFoundation/throughRHS`,
  `graphFoundation/overlay/throughUnknown`, and `graphConsolidation/lateSize` now
  expect intermediate facts to be stored instead of left unknown. Their result,
  identity, and contradiction assertions remain in place.
- Final CodeDog regeneration/build and `git diff --check` passed. The full older
  default suite was not rerun. Normal Slipstream binaries were not replaced.

Artifacts: `preserve-control-{1,2,3}/` and `preserve-final-{1,2,3}/` under
`/tmp/p4-cube-chart-bench/`; the GUI candidate is `preserved-results/`. The isolated
list control is `progress-control/`, with logs `/tmp/p4-progress-control-16.log`
and `/tmp/p4-progress-control-64.log`. Final behavior logs are in
`/tmp/p4-cube-chart-bench/preserve-final-tests/`.
