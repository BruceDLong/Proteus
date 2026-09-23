# Proteus4 graph foundation

This private experiment starts at Proteus commit
`73752c4b85ba1af6e956336e7d14385a2c3d4445`. The local branch is `codex/proteus4`.
It has no Git remote. Proteus4 is an independent redesign. Remove obsolete code
as its responsibilities move to the graph reasoner. The remaining agenda entry
points are migration work; the graph API does not call them.

## Start here

```text
me GraphNormalizer: reasoner
reasoner.init()
me NormalizationHandle: handle <- reasoner.normalize(pov)
handle.waitForCompletion()
// Inspect handle flags and the resulting POV.
// handle.cancel() stops this search and its subscriptions.
reasoner.close()
```

`normalize()` starts asynchronous work and returns a handle. One normalizer owns
one semantic context, registry, and worker. Each call creates a separate search
session. `initWithRegistry(existing)` explicitly adopts a pre-existing registry;
a POV already bound to another registry cannot silently be rebound. Do not run
the legacy normalizer concurrently against this context: it does not use this
runtime's synchronization boundary. Keep one manager per shared semantic context.

`findItem(source, index)` starts a zero-based positional goal. Its handle's
`result()` is the selected POV (possibly an implicit sparse occurrence). A
normalization handle's result is the original target. This is a foundation API;
the parser's ordinary `Agent.normalize()` entry point has not been redirected.

## Responsibilities

| Component | Responsibility |
|---|---|
| `GraphSearch.h` | Opaque-node search, lazy edges, route costs, sub-goals, revisions, subscriptions, cancellation, worker lifetime |
| `GraphFoundation.Lib.dog` | Public CodeDog API; Proteus graph access, compatibility checks and atomic Set batches |
| `ProteusGraph.h` | Translate semantic access into graph edges and refinement opportunities; GoalSpec turns findings into Sets |
| `IdentityRegistry` | Existing domain records, owned through the manager's Proteus adapter |

The generic runtime is a C++17 template, instantiated with `shared_ptr<POV>`.
It never dereferences a POV, reads a field, or manufactures a replacement graph
node. The semantic methods remain CodeDog. The thin native library binding
provides RAII thread shutdown and interoperates with the existing C++ backend.
Other CodeDog backends are not implemented in this experiment.

The adapter reads existing work lists and registry identities. A query carries
an opaque kind and argument. This lets the same POV participate in different
demands without storing a single global "visited" bit on it. Search bookkeeping
is local to each goal/session, and the work list is never consumed by traversal.

## One example, step by step

Consider an ordered sparse list whose size is unknown, with an RHS that supplies
a billion-item size. `findItem(listPOV, 999999999)` proceeds as follows:

1. Graph access cannot yet provide an indexed sparse edge. It advertises a
   refinement opportunity: resolve the size of this same POV.
2. The manager starts that size sub-goal. Its traversal follows the existing RHS
   without calling the legacy normalizer on it.
3. The size GoalSpec proposes a Set. The adapter validates it and applies the
   existing state-copy primitive under the context lock.
4. The Set publishes a graph revision. The original query is reconsidered.
5. Graph access can now expose a virtual indexed edge. Following it obtains the
   registry's canonical implicit POV for occurrence 999999999.
6. The handle reports completion. The physical list still has no expanded
   members; the registry retains only the demanded occurrence.

This is exercised by `unit/graphFoundation/refinement`, including the actual
parser-created list and existing repetition registry.

## Cost and settlement

Edges describe immediate work and an optional lower bound on remaining work.
The manager's `CostPolicy` prices visits and creations. The frontier compares
accumulated cost plus the remaining estimate; an estimate is not charged twice.
An edge is materialized only when selected. Estimates must be nonnegative and
must not overestimate remaining work if optimal route ordering is required.

`maximumRoute` excludes routes whose estimate exceeds the threshold. It does
not count down, stop the whole search, or get replenished for sub-goals. Native
tests check both a million-step versus sparse shortcut and many individually
cheap routes whose combined work exceeds the threshold.

A finding does not end conjunction checking. A work-list edge is a constraint,
whereas alternate ways of reaching a selected occurrence are marked as routes
that may be skipped after satisfaction. Sub-goals advertised as alternatives
are also unnecessary once another route supplies the result. A conflicting
scalar constraint fails the handle; previously applied Sets and every original
constraint remain available for inspection.

`fullyNormed` means the current reasonable search has settled under its policy.
It can coexist with `symbolicValues` and `withSubscribers`. It does not assert
that all information is concrete or that unsupported constraints are proved.
`symbolicValues` also records residual symbolic obligations. The caller inspects
the result to see exactly what remains unresolved.

## Lifecycle and publication

- `waitForCompletion()` waits for current quiescence, failure, or cancellation.
  A symbolic quiescent result may retain a subscription and resume later.
- `addRHS()` and `recordIdentity()` publish under the same context lock used to
  install subscriptions. There is no gap in which a wakeup can be lost.
- A Set publishes a revision before settlement. Lazy edge materialization also
  reports whether it changed the graph, even when it yields no Set. The first implementation
  conservatively restarts affected sessions across the whole context. Finer
  dependency indexing is a later optimization.
- `cancel()` stops this session and its child goals, removes its subscription,
  and releases its pending frontier. It takes effect between bounded graph
  steps/atomic Set batches; it preserves established writes. Other sessions
  remain active. `cancelled` is distinct from `fullyNormed`.
- All access callbacks must be bounded and nonblocking. External I/O must
  publish later through the context, not block an edge callback. Callbacks must
  not recursively call the manager APIs while holding the context lock. General parser
  stream integration and I/O adapters are future work.
- `close()` stops live sessions and joins the worker. C++ destruction does this
  too. Surviving handles can still report cancellation and return from waits.
- `stepCount()`, `stepDescription(index)`, `stepCost(index)` and `stepNode(index)`
  expose the trace. The native handle additionally exposes structured `steps()`.
- `stringifyResult(pov)` reads under the context lock. Do not directly mutate
  graph objects while the worker is running. A caller inspecting fields after
  a quiescent wait must ensure no other publisher can restart the search, or
  cancel/close first. Wait is not a permanent lock on a live result.

POV registry pointers are non-owning, as in the existing engine. Keep the
context or a handle retaining it alive while using registry-bound POVs. Calling
`close()` stops the worker but keeps the context alive.

The manager holds weak session references. Dropping every copy of a handle
releases its session after any current bounded step, including its subscription.

## Deliberate initial limits

Supported normalization is strict, positive, untyped numeric/string scalar
constraints, with independent size/value acquisition, unresolved RHS chains,
and registry identities. Literal list size acquisition is supported for the
indexed-access refinement example. The adapter checks literal scalar agreement
directly, within its explicitly supported subset. Sets use
`infonState.copyOverlayFrom`; the graph adapter has no dependency on
`WorldManager`, `DirectConsolidationStrategy`, or `AItem`. Each candidate is
checked against its path before applying a batch. Size mismatches requiring
richer semantics remain symbolic rather than being silently overwritten.

Indexed access supports flat ordered physical lists and existing compressed
repetition spans. Nested span traversal, intersection result selection, word
definitions, `%prev`/`%tPrev`, range algebra, tentative branch transactions,
stream-parser integration, commutators and theorem discovery are not migrated.
Unsupported work stays attached and visible as symbolic obligations. Existing
language tests are examples for future migration. Their pass count is not a
requirement for each development step and does not prove feature migration.

The extension points for those features are graph edges, demand translation,
explicit refinement sub-goals, and GoalSpec/Set semantics. No module class is
required for each keyword. Exploratory branches must gain an explicit isolated
mutation/evidence scope before speculative Sets or registry facts are allowed;
current sub-goals use established constraints only.

The initial search retains path traces and uses conservative revision restarts;
this is not yet a large-graph performance implementation. Cost is an ordering
policy, not a guarantee that an infinite cheap graph terminates. Cancellation
provides caller control for such a graph.

## Build and verify

From the Proteus4 directory:

```sh
codeDog Proteus.Lib.dog
g++ -std=c++17 -Wall -Wextra -Werror -pthread graph/test_graph_search.cpp -o /tmp/proteus4-graph-search-tests
/tmp/proteus4-graph-search-tests
```

Run the CodeDog tests **from `LocalBuild`**, where the generated resource paths
resolve:

```sh
cd LocalBuild
./TestProteus -t unit/graphFoundation/
```

Use focused tests for the behavior being changed. The full legacy suite is
available as `./TestProteus` when it is useful; it is not a gate for every step.
Remove obsolete implementations and tests rather than adding compatibility code
solely to preserve that suite.

For historical reference, the untouched starting commit's baseline was **8 failures / 456 tests**:
`concat/parseSrc`, `innr/oneOf`, `this/lvl1`, `search/adj1`,
`unit/world/dayFromSeconds/writeNestedSecond`,
`unit/world/dayFromSeconds/writeNestedLastSecondNegative`,
`world/load/kernel`, and `world/load/publicKB`. The final two report missing
`assets/public/KernelKB.pr`. When comparing historical runs, compare failure names as well as counts.


## Initial foundation validation (commit 36bbae4, 2026-09-23)

- CodeDog generation and C++ build succeeded.
- POV integration: **18/18** scenarios passed.
- Independent graph runtime: **16/16** scenarios passed under AddressSanitizer,
  UndefinedBehaviorSanitizer and LeakSanitizer.
- Full suite: **8 failures / 474 tests**. Failure names exactly match the
  **8 / 456** untouched baseline above; all 18 added integration tests pass.
- ThreadSanitizer compiled but could not run on this host: it terminated with
  `FATAL: ThreadSanitizer: unexpected memory mapping`. No ThreadSanitizer result
  is claimed.
- `git diff --check` passed. The original Proteus checkout was not changed.

The existing test harness can return exit status zero when assertions fail;
check its `Total Failures/Tests` summary rather than only the process status.

## Agenda dependency cleanup validation (2026-09-23)

- CodeDog generation and C++ build succeeded.
- POV integration: **20/20** scenarios passed, including conflicting string
  constraints and prefix copying with retained state metadata.
- The full legacy suite was not rerun; it is not a gate for this redesign.
