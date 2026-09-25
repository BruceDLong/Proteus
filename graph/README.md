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
| `SpanAlignment.h` | Plan finite ordered correspondence from opaque member/gap evidence; no POV access or mutation |
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
Relational queries also carry a second endpoint and, for embedded traversal, a
stack of existing owner POVs. Both participate in query identity, preserving
different contexts even when an embedded list shares its physical members.

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
An edge is materialized only when selected. Proposed Sets enter the same
frontier with their own work estimate. A finite fill is therefore priced before
it creates members; a cheaper route can win or a policy can exclude the fill
without mutating the list. A semantic revision discards stale queued plans.
Estimates must be nonnegative and
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

A refinement with `alternative=false` is a required child goal. Completion is
computed per goal from its own evidence and these dependencies. A resolved
child can complete while a sibling stays symbolic; the parent cannot complete
until all required children do. Shared goals are searched once at their cheapest
discovered base cost. An initially excluded goal can be reconsidered if a
cheaper route reaches it. Completion is published from children to parents;
cycles remain symbolic rather than serving as their own proof. These are
session-local goal dependencies, not a second persistent graph of infons.

`fullyNormed` means the current reasonable search has settled under its policy.
It can coexist with `symbolicValues` and `withSubscribers`. It does not assert
that all information is concrete or that unsupported constraints are proved.
`symbolicValues` also records residual symbolic obligations. The caller inspects
the result to see exactly what remains unresolved.

The infonView's `isNormed` flag is stricter than the handle's `fullyNormed`:
a supported scalar is marked only when its overlay type, value, and size are
known and its required search has settled without deferred constraints. A supported physical
list must also have a closed extent that agrees with its members, and every
required member must finish normalization. Each GoalSpec receives its own
completion callback under the context lock; the Proteus adapter sets the flag.
The existing printer then hides the retained work list. Failed, cancelled,
cost-limited, or otherwise symbolic goals are not marked. Successful children
may be marked independently of a symbolic parent.

A new normalization pass clears the target flag. Each semantic publication
also invalidates completion flags conservatively across the context, including
flags whose process handles have been released. The adapter tracks weak POV
references for this metadata; it does not retain another graph or keep nodes
alive. Completion metadata does not itself publish a semantic revision.
Completed handles remain records of their finished run; call `normalize()`
again to revalidate a result invalidated by later information.

## Physical list normalization

A size/value query advertises a required `listContents` subgoal. Its graph walk
visits existing physical POVs one edge at a time. Each visit advertises a
required member-normalization goal and contributes size evidence:

- An ordinary member contributes one parent item, including a nested list.
- An embedded member (`&...`) contributes its known nonnegative integer extent.
- A missing or unsupported extent leaves the count symbolic. Child Sets can
  expose that extent and trigger a new pass through the original graph.

`GraphListMeasure` holds the temporary count evidence in CodeDog. At the end of
the walk, its GoalSpec proposes a counted-size Set for a closed list whose size
is unknown and whose size mode permits counting. Explicit `fromGiven` unknown
sizes stay unknown; an explicit size disagreement stays symbolic. Inner `~size`
and outer `*size+` syntax use the existing shared size slot and retain their
presentation metadata. No legacy recursive `countSize()` or agenda call runs.

Members of an open list still receive goals, but its unfinished tail prevents
parent completion. A size-only count needs only embedded extents; it does not
normalize every ordinary nested value just to determine the parent's length.
Template and sparse regions remain symbolic in normalization, and no implicit
occurrences are eagerly generated. Existing lazy sparse indexed access remains
available.

## Embedded spans and open list RHS correspondence

A supported list relationship advertises a required relational goal containing
both existing parent POVs. `ProteusCorrespondenceGoal` walks the loaded positions
of each ordered physical list. Ordinary nested lists occupy one position;
embedded list spans contribute their positions. The query's opaque context
carries return points through embedded spans, with containment cycles deferred.

The walk collects temporary layout evidence: loaded member POVs, a symbolic gap
for each open tail, and interval constraints from declared list sizes. A gap
refers to its existing owner POV; it does not allocate unknown occurrences.
`SpanAlignment.h` uses this evidence to plan correspondence:

- Match closed layouts across embedded-list boundaries.
- Infer a single missing gap from the other side's extent and known positions.
- Fill a gap from finite loaded members on the other side, closing it only when
  its end is proved. New members have independent views and identities to their
  sources, including for ordinary nested lists.
- Extend an open tail with an already-loaded prefix while preserving its
  symbolic remainder. A billion-item gap never creates a billion members.
- Match anchored prefixes/suffixes when multiple gaps remain ambiguous. Do not
  enumerate possible partitions; zero remaining width can close multiple gaps.

The complete supported layout must be inspected before publishing a plan.
Declared-size conflicts and unsupported later spans cannot publish an earlier
prefix. A supported but ambiguous layout may publish its proved relationships
and remain symbolic. Embedded strings, repetition templates, and unordered
matching remain deferred. Extents use exact checked integer conversion;
unsupported extents remain symbolic.

The GoalSpec offers one Set containing parent-size acquisition, member identities
and any finite tail fills. Its creation work is priced by the manager before
execution. Registry affiliations and new-member support are checked before the
batch publishes under the context lock. Identities use the existing registry's
`iekListCorrespondence` evidence and are bidirectional and deduplicated. Original
work lists remain attached. Child goals populate the new views through ordinary
size/value Sets; each revision exposes their new edges before settlement.

Cancellation takes effect between traversal steps or atomic Set batches. These
are monotone tail extensions and closures, with conservative revision restarts
and registry scans. Structural reordering and retraction need their own
invalidation design; large-list performance optimization remains future work.

## Field expressions

A `concatField` goal works on either the size or value slot of the existing POV.
It follows the expression's operand POVs and requests their values, with size
subgoals where a numeric radix is needed. The Proteus adapter plans string,
numeric, and physical-list concatenation; the priced Set publishes the result.
A computed size can consequently expose sparse indexed access on the next pass.

The original state remains in `sizeExpression` or `valueExpression` after the
result is installed. These retain the original operand POVs as provenance, so
later normalization still validates their constraints. No field-wrapper POVs
are created and operands are not destructively combined. List results receive
independent member views through registry identities. A final open list operand
can propagate its loaded prefix and later resume; templates and multiple open
chunks remain symbolic. Invalid or mixed interpretations are not guessed.

See [expression-results.md](expression-results.md) for verified examples.

## Intersection result selection

`selectIntersection` is a required goal for an ordinary first/last/marked
intersection. Its graph walk gathers extent evidence from the existing inner
POVs without normalizing their values. It rejects proved extent conflicts and
then follows the selected position. Last selection requires a known end; an
explicit extent can prove that an open tail contributes zero positions. Marked
selection does not require an unrelated tail to close. Embedded list spans are
traversed and empty spans skipped; an empty open span cannot be skipped when
searching for the first position.

The GoalSpec publishes an identity from the result POV to the selected POV.
Scalar Sets or list adoption then acquire the requested fields through ordinary
graph traversal. The original `innr` is retained as `selectionExpression`, along
with the selection kind and selected POV, so later constraints can be checked.
Other inner values are not forced to normalize. Alternative filtering, function
arguments, templates, positional selectors, and embedded-string traversal remain
deferred rather than being treated as ordinary result selection.

The parser now initializes the outer size as unknown when it moves the inner
extent into `innr`; clearing only the payload previously left a stale literal
flag. Selecting a result preserves actual outer size constraints.

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
constraints, with independent size/value/overlay-type acquisition, unresolved RHS
chains, and registry identities. An unknown overlay (`tUnknown` or `tUnset`),
including a plain `?` view, can acquire `NUM` or `STR`. The value demand includes
its interpretation: value evidence is complete only with a known overlay type.
Type-only evidence is useful even while the payload remains unknown; it leaves
the result symbolic and eligible for subscription. A compatible source supplies
all available requested fields in one Set. The Set establishes the type before
calling the matching-overlay copy primitive. It never overwrites a known type.
A plain unknown can also adopt an ordered physical list: it first becomes an
empty open list, then the existing correspondence goal supplies independent
member views. Other unsupported interpretations remain deferred.

Untyped physical literal lists support member
normalization, nested list values, embedded spans, and size counting/checking.
Ordered physical lists also propagate RHS information through corresponding
member identities, including nested lists, embedded list spans, finite open-tail
fills, and multiple sources. Literal list
size acquisition is also supported for the indexed-access refinement example. The adapter checks literal scalar agreement
directly, within its explicitly supported subset. Sets use
`infonState.copyOverlayFrom`; the graph adapter has no dependency on
`WorldManager`, `DirectConsolidationStrategy`, or `AItem`. Each candidate is
checked against its path before applying a batch. Size mismatches requiring
richer semantics remain symbolic rather than being silently overwritten.

Indexed access supports flat ordered physical lists and existing compressed
repetition spans. Embedded-string correspondence, template normalization, nested indexed traversal, general intersection solving, word
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
g++ -std=c++17 -Wall -Wextra -Werror graph/test_span_alignment.cpp -o /tmp/proteus4-span-tests
/tmp/proteus4-span-tests
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

## Existing language fixtures and next priority

Run `python3 graph/run_language_baseline.py` from the fork to normalize the
selected existing language fixtures exclusively through `GraphNormalizer`.
The recorded first run is in [language-baseline-results.md](language-baseline-results.md).
See [selection-results.md](selection-results.md) for the latest validation,
including computed sizes, concatenation, list adoption, and result selection.
Earlier checkpoints are recorded in
[expression-results.md](expression-results.md),
[span-alignment-results.md](span-alignment-results.md),
[list-correspondence-results.md](list-correspondence-results.md),
[list-normalization-results.md](list-normalization-results.md) and
[isnormed-results.md](isnormed-results.md).
The selection lives in `graph/language-baseline.txt`; input and expected output
are read from `defaultTests.tst` each time, with no copied expectations.
`--select merge/num/` restricts the selection. The runner exits nonzero on
mismatches, missing tests, timeout, or process failure; its exploratory baseline
is not a requirement that every selected case pass immediately.

The harness's `graph` test mode prints the resulting text, size state, and
settlement/symbolic flags. Selected `parse` fixtures are deliberately normalized
too. A matching printout alone does not prove list normalization: the parser
can already supply literal contents and a counted size, while unsupported list
work remains symbolic. The normal printer and expected strings are retained
so presentation differences are visible as well.

The list foundation covers ordinary and empty lists,
nested list values, embedded spans, counted and explicitly given sizes
(including inner `~size` syntax), child RHSs, list identities, open tails, and
computed sizes. Keep size evidence and child normalization in Proteus graph
access/GoalSpecs, with generic scheduling and subscriptions in the manager.
Do not expand an entire sparse list merely to normalize its representation.

The ordinary size/value goal applies a single Set batch when a compatible RHS
supplies both, including overlay type when needed. The focused `oneRHS` case
checks that it takes one such Set; `twoRHSs` continues to cover partial information
from separate sources. Known target constraints must still be validated.

## Scalar overlay acquisition validation (2026-09-24)

Scalar normalization now acquires unknown overlay types as part of its existing
Set batch. Both `tUnknown` and `tUnset` are covered, including parser-created
`?` views and independent members created by span filling. The manager's
scheduling and generic graph API did not change.

- CodeDog regeneration and C++ build: passed.
- Graph integration: **105/105 passed**, including 15 overlay scenarios covering
  numeric/string evidence, type-only and size-only partial results, transitive
  work, registry identities, cost exclusion, cancellation, subscription resumption,
  known-type preservation, and list members.
- The original `merge/directAny/scalar` and `merge/unknown/typed` fixtures changed
  from **0/2** to **2/2** exact output matches through the graph API:
  `? = 'Hello'` now prints `'Hello'`, with `symbolic=0`.
- The unchanged 38-fixture baseline remains **25/38** output matches.
- Full legacy suite and native scheduler sanitizers were not rerun for this
  adapter change. `git diff --check` passed.


## Unknown-to-list adoption validation (2026-09-24)

A scalar/value search preserves its original goal endpoint while traversing
unknown RHSs. When it encounters a concrete list, it can expose correspondence
to that original endpoint without normalizing every intermediate unknown.
Adoption changes only the target's interpretation and available size. The next
priced span Set handles member creation; source member links are never aliased.

- Full CodeDog regeneration and C++ build passed.
- Graph integration: **133/133 passed**, including 11 adoption scenarios.
- Coverage includes empty/closed/open lists, nested values, registry identities,
  transitive unresolved RHSs, subscription resumption, creation cost exclusion,
  concatenation RHSs, and a billion-item extent with zero expanded members.
- `git diff --check` passed. Full legacy suite not run.
