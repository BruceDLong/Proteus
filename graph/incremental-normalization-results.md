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


## 3. Preservation without narrower invalidation (2026-10-01)

The experiment does **not** show that storing intermediate scalar results can
replace narrower invalidation on the cube. Preservation alone performs much like
the original control. The variants with narrower invalidation are faster and use
less total process memory at the visible-cube milestone.

### What was isolated

The preservation-only variant starts with `3b5fd5a` / functional engine `926d579`
and applies only the scalar Set batch from `b3fb3c3`. The new per-object dependency
index and per-callback address sets are absent, rather than merely disabling
notifications while continuing to allocate them. This variant fills scalar
intermediates and consolidates fully represented RHSs. Ordinary model writes
still cause broad invalidation; the older catalogue/proof subscriptions remain.

There is a dependency between the earlier two changes: retaining a completed
list walk across unrelated member writes requires selective invalidation. That
portion of the second change cannot operate with all those writes published
broadly. This is therefore a test of **facts stored in the model versus selective
search reuse**, not a claim that every line of the second commit is independent
of the first. It does not test a future implementation that preserves additional
kinds of intermediate list results.

The exact small patch is saved as
[experiments/preserve-with-broad-invalidation.patch](experiments/preserve-with-broad-invalidation.patch).
It applies to the earlier baseline, not the current branch. The working engine
continues to contain both accepted optimizations; this experiment used isolated
builds under `/tmp`.

### Four-way cube comparison

Three runs of each variant, with run order rotated. Every run used fresh data
and its own display. No build or test process ran during timing. Both the colored
cube and Kanban were visually checked for the experimental variant.

Time is the upper polling bound for first visible cube color, as before. Memory
is the process's Linux `VmHWM` through that milestone, converted from KiB to MiB.
`VmRSS` equaled that high-water mark in these runs. It includes GUI, model, search,
allocator and library memory; it is not a measurement of the index alone or of
steady-state memory after all normalization. Polling intervals are about 1.1 s.

| Variant | Median visible time (s) | Median graph visits | Median peak RSS (MiB) |
| --- | ---: | ---: | ---: |
| Neither change | 23.54 | 201,305 | 260.8 |
| Stored scalar results only | 23.15 | 198,148 | 264.1 |
| Narrower invalidation only | 13.80 | 75,340 | 215.6 |
| Both, including list-walk reuse | 13.96 | 68,987 | 215.3 |

Preservation alone reduces the median visits by only 1.6%; its timing differences
are within the observed run-to-run spread. Both changes use about **18.5% less
peak RSS** than preservation alone. This gives no evidence of a net memory
penalty from narrower invalidation on this workload. It does not prove a bound
for arbitrary models or attribute the whole memory difference to one container.

### What the memory bookkeeping actually stores

`SearchSession::Task::visited` is the pre-existing per-goal map of query and cost.
It distinguishes goals/contexts and prevents revisiting the same query within a
pass; it is cleared when that task restarts. It was not introduced by narrower
invalidation.

`ObjectDependencies` adds address-to-token entries per graph context. These
entries do not own POVs or copy adjacency. Tasks and cached proofs retain sets
of tokens they read; tokens hold weak subscriptions. The temporary address set
used to suppress repeated reads lives only for one callback. Unreferenced index
entries are reclaimed in batches when the insertion threshold is reached.

`SearchSession::steps` is a different, older source of retention: the handle's
history appends events across retries, including queries that reference POVs.
It has no configured history limit. Releasing the last handle releases that
session's visited-query and history containers; cached completion proofs can
outlive the handle by design.

### Controlled growth and repeated normalization

A separate diagnostic build counted bookkeeping under the graph context lock.
The fixtures are the same independent scalar paths used in section 2. These
counts are after normalization with one retained handle; they are not timing
measurements and introduce no production instrumentation.

| Members | Referenced object keys | Task-to-token entries | Cached proof read entries | Retained history events |
| --- | ---: | ---: | ---: | ---: |
| 16 | 162 | 374 | 473 | 407 |
| 64 | 642 | 1,478 | 1,865 | 1,607 |
| 256 | 2,562 | 5,894 | 7,433 | 6,407 |
| 1,024 | 10,242 | 23,558 | 29,705 | 25,607 |

All grow linearly in this fixture. These are addresses and read relationships,
not counts of distinct POVs: views, path state and position slots have keys too.
In general the storage cost depends on **goal-to-evidence relationships**, not
just the number of nodes. Many distinct goals reading many of the same facts can
still require many entries; this test does not rule out quadratic growth on a
dense dependency workload.

For the 64-member fixture, preservation with broad invalidation and the current
engine both end with **451 visited-query entries**. Their history sizes are
**65,728 versus 1,607 events**, respectively. The corresponding vector capacities
are 131,072 versus 2,048. On this build `Step<shared_ptr<POV>>` is 144 bytes: just
that array reserves **18 MiB versus 0.28 MiB**, before strings and query-context
allocations. This is a concrete example of retry history outweighing the added
tracking structures.

Repeating the same 64-member normalization 1,000 times, releasing each handle,
leaves **668 total index entries / 642 referenced entries** and **194 live
completion proofs** unchanged. Released sessions retain zero visited/history
entries. Expired weak subscriptions do consume spare capacity: it rose from
3,824 slots initially to 41,192 after 100 repetitions, and remained 41,192 after
1,000. At 16 bytes per weak pointer that final capacity is about **0.63 MiB**;
only 1,092 subscriptions were live at the final snapshot. The cleanup batches
prevented continued growth here, but their space overhead is worth reducing.

Closing the entire test context left 192 proof objects associated with the
64-member model in **both** the broad and current variants. This establishes
that the survival is not specific to the new index, but is not a lifetime audit
of model ownership. No claim of complete model reclamation is made.

### Validation and artifacts

The preservation-only variant passed **317/317 focused checks** covering scalar
foundations, consolidation, representation, streaming, finite definitions, open
spans, crossing, numeric ranges, definition reuse, sequence charts, and the new
scalar/conflict/embedded-size cases. Performance assertions that require selective
invalidation were not treated as semantic gates for a deliberately broad variant.
The full legacy suite was not rerun. The production engine source and normal
Slipstream binaries were not changed.

All artifacts are under `/tmp/p4-cube-chart-bench/`:

- `preserve-only/`: isolated source, patch, manifest and app/test builds.
- `ablation-<variant>-{1,2,3}/`: per-run screenshots, logs, memory and timing JSON.
- `ablation-summary.json`: four-way medians.
- `ablation-tests/`: focused behavior logs and summary.
- `memory-probe/` and `memory-probe-broad/`: instrumented sources and growth logs.
- `ablation-memory-scaling.json`: parsed memory snapshots.

Construction/runner scripts: `/tmp/p4-preserve-ablation.py`,
`/tmp/p4-cube-chart-bench/run-memory.py`,
`/tmp/p4-cube-chart-bench/run-ablation-matrix.py`, `/tmp/p4-memory-probe.py`, and
`/tmp/p4-ablation-tests.py`.

These results support retaining narrower invalidation. The next memory targets
suggested by the measurements are optional bounded event history and more compact
subscription storage, evaluated separately from model-result preservation.
