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

This does not yet preserve intermediate scalar discoveries or separate list
membership walks from member-result completion. Those are the next experiment.
