# Proteus4 graph foundation

This private experiment starts at Proteus commit
`73752c4b85ba1af6e956336e7d14385a2c3d4445`. The local branch is `codex/proteus4`.
It has no Git remote. Proteus4 is an independent redesign. Remove obsolete code
as its responsibilities move to the graph reasoner. Ordinary Agent normalization
now uses the graph reasoner. Legacy AItem rules remain compiled for unmigrated facilities and reference tests; normalization
has no agenda fallback.

## Start here

```text
// core.init(...) initializes its Agent and persistent graph context.
me NormalizationHandle: handle <- core.agent!.normalize(pov)
handle.waitForCompletion()
// Inspect handle flags and the resulting POV.
// handle.cancel() stops this search and its subscriptions.
core.deInit()  // after closing and joining any input streams
```

`Agent.normalize()` starts asynchronous work and returns a handle. Each Agent
owns one semantic context, registry, and worker for its lifetime. Each call
creates a separate search session. `Agent.normalizeAndWait()` is the explicit
synchronous convenience used by query/load APIs and ordinary language tests;
it retains unresolved sessions so their subscriptions survive the call.
Finished sessions are pruned on later calls, and `Agent.close()` cancels the
remaining sessions. `ProteusCore.deInit()` calls that shutdown.

Standalone `GraphNormalizer` contexts remain available for focused experiments.
One normalizer owns one semantic context, registry, and worker.
`initWithRegistry(existing)` explicitly adopts a pre-existing registry;
a POV already bound to another registry cannot silently be rebound. Do not run
the legacy normalizer concurrently against this context: it does not use this
runtime's synchronization boundary. Keep one manager per shared semantic context.

`GraphNormalizer.findItem(source, index)` starts a zero-based positional goal. Its handle's
`result()` is the selected POV (possibly an implicit sparse occurrence). A
normalization handle's result is the original target. Ordinary query, load, and
test normalization now enter through Agent's graph context.

Structural `%prev` and `%next` now use graph position goals, followed by ordinary
identity traversal. See [relative-position-results.md](relative-position-results.md)
for supported contexts, sparse lookup, partial results and deferred boundaries.
Logical navigation now crosses embedded spans and supports positive intersection
prefix selection; see [logical-navigation-results.md](logical-navigation-results.md).

The older language files are tracked separately from the focused graph tests.
See [language-compatibility-results.md](language-compatibility-results.md) for the
complete fixture inventory and the next compatibility priorities.
Run `python3 graph/audit_language_suite.py --output /tmp/proteus4-language-audit`
to preserve every original fixture and record failures, crashes and timeouts.

## Responsibilities

| Component | Responsibility |
|---|---|
| `GraphSearch.h` | Opaque-node search, lazy edges, route costs, sub-goals, revisions, subscriptions, cancellation, worker lifetime |
| `GraphFoundation.Lib.dog` | Public CodeDog API; Proteus graph access, compatibility checks and atomic Set batches |
| `POVDescription.h` | Live, goal-independent access to existing POV links, state slots, work entries and registry identities |
| `ProteusGraph.h` | Translate semantic access into graph edges and refinement opportunities; GoalSpec turns findings into Sets |
| `ListStructure.h` | Shared incremental list walk and layout evidence for embedded positions, gaps, and owner extents |
| `SpanAlignment.h` | Plan finite ordered correspondence from opaque member/gap evidence; no POV access or mutation |
| `GroupedListPlan.h` | Plan fixed-width repeated groups against whole ordered source spans; preserve occurrence identity |
| `RHSRepresentation.h` | Prove which RHS facts are already represented, independently of complete normalization |
| `IdentityRegistry` | Existing domain records, owned through the manager's Proteus adapter |

The generic runtime is a C++17 template, instantiated with `shared_ptr<POV>`.
It never dereferences a POV, reads a field, or manufactures a replacement graph
node. Semantic checks and Sets remain primarily CodeDog; structural descriptions
and planners also use native helpers. The thin native library binding
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

## Represented RHS information

A symbolic result may already contain every fact supplied by a particular RHS.
For example, `_ = 2` has acquired the value and overlay but still lacks a numeric
size. Marking it `isNormed` would be wrong; printing `2 = 2` is unnecessary.

The adapter advertises a separate `assessRepresentation` goal for work edges.
`RHSRepresentation.h` builds a read-only proof for scalar facts and closed,
ordered physical layouts, reusing the span equations for embedded members.
Every known RHS field must occur in the target. Unknown RHS fields impose no
additional fact. Layouts must agree without fills, and every mapped member must
pass the same check. Named RHS requirements, contextual meanings, open or
repeated spans, and cyclic proofs conservatively remain unproved.

The manager prices the proof's visits before accepting it. At settlement the
completion adapter publishes only derived rendering metadata; the printer can
then omit that RHS text. Work edges, member identities, and subscriptions are
unchanged. No numeric size is invented and `isNormed` retains its stricter rule.
Graph publications invalidate the shared proof tokens, including after a
handle is cancelled or the original POV is released. Copies sharing a token
cannot retain stale evidence after its invalidation.

See [rhs-representation-results.md](rhs-representation-results.md) for the
original-test recovery milestone and remaining language blockers.

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

## Shared POV description

`POVDescription.h` provides one goal-independent description of the stored
relationships: physical next/previous and first/last members, parent and view
links, size/value slots, member specifications, work entries, and registry
identities. It returns the existing objects. A work entry retains its original
POV and flags; a registry relation retains its actual evidence record and can
be read from either endpoint. They remain distinct even when their endpoints
match. Different POVs over shared infon storage retain their own structural
context and registry relationships.

This is a live view, not a second adjacency graph or a once-ever expansion
cache. Each read uses the current POV storage under the graph context lock;
registry enumeration additionally takes the registry lock. Relationship reads
produce temporary descriptors without adding nodes or records. New publications
can therefore expose new work, identities, or sparse routes through the same
view. Sparse inspection and existing-occurrence lookup never materialize items;
the selected graph edge still owns that operation and its cost.

Goals retain their semantic eligibility checks and required sub-goals. They
share this structural description and the simple member-or-boundary edge
construction used for normalization, concatenation, and intersection extent
walks. Unsupported or unfinished stored work is still described; recognizing a
link is separate from proving that it satisfies a goal. The generic graph
manager remains unaware of POV storage.

See [pov-description-results.md](pov-description-results.md) for validation and
remaining boundaries.

## Shared list structure

`ListStructure.h` defines the structural walk used by correspondence, finite
definition applications, RHS representation proofs, and grouping. It enters a
list, visits its positions, descends only into embedded members, and leaves
with the owning list's extent and open-tail evidence. Ordinary nested lists
remain one position. Each embedded boundary retains its owner in the layout's
extent equations, and cycles are checked against ancestor infon identity.

The walk is incremental: correspondence advertises one step per graph edge.
Existing finite planners drain the same walk through `readListLayout`; they
still perform that finite inspection inside their planning callbacks. This
change does not make those callbacks independently resumable or eliminate
their up-front planning cost.

`ListLayoutBuilder` translates events into the common `SpanLayout`, projecting
POVs to instance slots when needed by a definition application. Virtual tails
stay single gap segments. Owners remain existing nodes, so stored member
specifications and occurrence relations remain accessible through graph access.
There is no second persistent infon graph or eagerly generated member list.

Published stream traversal stops before unfinished members. Stored inspection
retains them so its consumer must account for their unresolved semantics.
Consumer checks still govern orderedness, definition constraints, and supported
relationships. Recognizing structure alone never establishes those facts.

See [list-structure-results.md](list-structure-results.md) for the consolidation
and verification boundaries.

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

## Definitions and type applications

`DefinitionApplications.h` owns a catalogue and reusable type evidence inside
each graph context. Normalizing a declaration registers its existing `funcArgs`
body through one POV; registration does not normalize or copy that body. A
declaration counts as one list item, and its completion means registration
succeeded, not that its meaning has a concrete value.

A named scalar/list, a list with a member template, or an implicit occurrence
contributes a required `typeRequirements` goal.
Read-only graph edges walk its unique definition, aliases, added type names,
body RHS constraints, and physical list members. Leave edges compile those
findings into requirements. This data is a validation certificate and Set plan;
it does not replace the POV graph. Scalar requirements include size, value, and
overlay. Finite list requirements additionally describe ordered positions.
Homogeneous templates describe one repeated member constraint.
Embedded spans flatten into positions; an ordinary nested list occupies one
position and has its own member requirements. Alias and containment cycles
remain symbolic before any instance members are allocated.

`DefinitionPlan.h` checks the instance against these requirements without
writing it. It reuses `SpanAlignment.h` to match existing members and uniquely
fill open instance tails. It rejects incompatible known facts or lengths and
defers unsupported/ambiguous layouts. A priced Set then fills missing fields,
establishes the proven order, and allocates only absent instance members. A
later incompatible member cannot cause a partial write from that batch. Other
goals remain independent; this is not rollback of earlier normalization Sets.

No application adds definition RHSs or bidirectional identities to shared
bodies. New members have independent infonViews and retain named member types.
Existing instance labels and work constraints are preserved. Member unknowns
can remain symbolic with subscriptions; `isNormed` still requires their complete
normalization. Bare parser-created names begin with unknown size, while an
explicit zero size remains a constraint.

Certificates record bindings, facts, RHSs, and the physical member POVs they
came from. Instance and member certificates share this evidence. Repeated
normalization and unrelated graph revisions do not rebuild an application.
Copies can inherit certificates only when their destination has all supporting
contents; a size-only copy cannot inherit proof of a missing list value. The
copy can also establish orderedness from checked evidence. Changed definitions,
replaced members, new ambiguity, and new type requirements force validation
again. These constraints are context-free; future contextual types need binding
context in the cache key. Witness and destination checks still cost work.

The catalogue currently admits exact, Unicode-normalized, untagged names.
Distinct definitions of one name stay ambiguous. Missing names retain
subscriptions and can resume after another normalization registers the meaning.
Publication uses graph locking and revisions, without populating the legacy
global `ModelManager`.

Supported bodies include closed physical ordered lists, empty open list bodies,
and homogeneous open member templates, with scalar, nested-list, alias, and
supported RHS constraints. Empty open bodies retain a virtual span whose extent
can come from their size slot or another compatible definition constraint.
An empty instance can acquire a compressed `listSpec` reference. Positional
lookup creates only the requested registry occurrence; normalization then reads
its source-list requirements. Existing closed finite instances receive the
member constraint in place, and parent normalization revisits already-demanded
occurrences. Untouched positions stay virtual even in a billion-item list.

Prefixed definition bodies, embedded repeated spans, unordered records,
contextual paths, computed definition expressions, and registry relationships
inside shared bodies remain deferred. Multiple writable positions sharing the
same list storage need a joint application plan and also remain deferred.

See [definition-results.md](definition-results.md) for the scalar checkpoint,
[finite-definition-results.md](finite-definition-results.md) for finite lists,
and [lazy-template-results.md](lazy-template-results.md) for the template slice.
Loaded instance prefixes now receive template requirements in place. Their
unrequested tail stays compressed. General sparse-tail indexing after a prefix
and record/field matching remain later boundaries.

## Finite extents over virtual positions

`DefinitionRequirement::memberLayout()` describes explicit positions and a
virtual tail using the same `SpanLayout` equations as ordinary correspondence.
An extent of a billion requires one gap descriptor. An unknown extent remains
unknown; it is never inferred as zero from an empty physical member list.

Applying `#row=*2+{...}` to an empty instance establishes a list and its size
without allocating members. Existing concrete contents must agree with that
size. A closed-list constraint can supply the missing positions; a homogeneous
constraint can supply their member type. Shared definition bodies remain
read-only, including when another RHS supplies the extent. The existing type
sub-goal follows those constraints and subscriptions reconsider new information.

Grouping uses the same extent to partition its loaded source. It prepares only
the positions supplied by those source spans, retaining existing group POVs and
canonical occurrences. A zero or unknown group width stays symbolic. Failed
extent or member checks publish no partial group batch. A closed list still
requires all members to normalize before `isNormed` can be set.

Definition evidence records whether the source tail is open, as well as its
facts and members, so a later closure cannot reuse proof of a virtual body.
Prefixed and embedded virtual definition bodies remain deferred. General indexed
access to an untemplated virtual tail is not introduced here; the existing
sparse lookup route still uses member templates.

See [virtual-shape-results.md](virtual-shape-results.md) for examples and checks.

## Fixed-width whole-list grouping

`{{_ _}|...} == {1 2 3 4}` maps two result members (pairs) to four input
members. The adapter advertises a `groupedList` goal for the whole loose RHS;
it does not turn that RHS into a strict identity between the two containers.
The source endpoint's value is inspected separately from its relationship flags.
Other loose relationships remain unsupported.

The definition walk supplies the read-only repeated member requirement.
Its shared span layout derives width from explicit positions or a known size,
including an empty virtual body such as `*2+{...}` or a sized member template.
`GroupedListPlan.h` uses that nonzero fixed width, checks the source's loaded
ordered layout and the target's group count, and prepares per-group definition
plans. It validates every group's known facts before offering a Set. The graph
manager prices visits, new member storage, and member identities before execution.
The Set appends missing groups, reuses existing prefix POVs and already-demanded
canonical occurrence POVs, applies the prepared fields, and records identities
between matching scalar positions. It closes the finite target at its own
member count. Neither shared templates nor source data are specialized.

A complete group mapping is distinct from fully concrete normalization. Bare
numbers can retain unknown sizes. At settlement, separate revision-scoped proofs
suppress a represented template and RHS; the work edge and template remain
searchable. New information invalidates that metadata and can still wake a
subscription or expose a conflict through member identities.

The route accepts a repeated finite scalar group and an already-closed,
ordered scalar source list (including finite embedded source spans). It also
accepts existing target prefixes and prior sparse lookups. It defers text,
variable-width/nested groups, `==:` prefix matching, unordered or root-constrained
sources, and distinct occurrence views of the same logical index that require a
joint alias plan. Size-only grouping and general virtual-tail navigation remain
later routes. See [grouping-results.md](grouping-results.md).

## Lifecycle and publication

- `waitForCompletion()` waits for current quiescence, failure, or cancellation,
  including delivery of captured completion effects. A symbolic quiescent
  result may retain a subscription and resume later. `active()` distinguishes
  a live search/subscription from a terminal handle.
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
  not recursively call the manager APIs while holding the context lock.
  Completion effects are captured under the lock and delivered outside it;
  they may publish new information, but must not wait for this worker or close
  its manager from the callback.
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

## Parser and reply lifecycle

Streaming parsers share their Agent's graph publication lock. They release it
around potentially blocking input-buffer operations and publish parse milestones
before yielding. The graph can visit finished children of a growing literal
list. It defers value writes and indexed materialization on unfinished
parser-owned infons. This avoids the old independent parser/agenda
wake-up mechanism and its initial timing delay.

`ThreadedNormalizer` retains its handle and uses an input-lifetime wait: ordinary
quiescence while subscribed does not mean that parsing has finished. Parser
completion publishes another revision; cancellation and failure also release
that wait. Both streaming and parse-first `InfonStream` paths use this mechanism.
`requestFinish()` cancels normalization and closes input; callers then join the
stream before closing the Agent. A cancelled normalization does not stop a
separately owned parser by itself.

Reply-marked goals queue a result when they settle, including symbolic results.
All settled reply markers are claimed before text is captured, so a containing
list does not echo already-consumed markers. Replies follow marker positions in
the input, regardless of goal completion order. Captured text is delivered via
`ProteusCore.submitProteusResult()` outside the graph lock, with the original
origin ID and item position. A second normalization does not resend a consumed
marker. Parser errors wake the handle as failures.

Strict ordered relationships may consume a growing RHS's completed prefix.
The existing correspondence goal walks only finished members and represents the
unreceived remainder as a gap. Its Set withholds writes to parser-owned tails;
it can fill independent LHS members without waiting for EOF or expanding sparse
remainders. Explicitly list-typed unknown targets can also adopt this evidence.
Final RHS extent and closure remain unavailable until that RHS finishes parsing.

Early prefix results carry weak semantic witnesses of the ordered source body.
These conditions propagate through result copying and correspondence and keep
completion symbolic while input is pending. If later input reinterprets the
body (for example with a comma or postfix function), dependent searches fail
explicitly; this implementation preserves partial Sets for inspection and does
not yet roll them back or replan the changed interpretation. Producer revisions
resume subscribed searches, and cancellation preserves the last partial result.
See [growing-rhs-results.md](growing-rhs-results.md) for validation.

Loose correspondence, path streaming queries, record/template types, and intersection/template
input traversal need further semantic work. Completed physical open lists
continue to use the existing graph span/subscription behavior.

## Deliberate initial limits

Supported normalization includes strict, positive numeric/string scalar
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
repetition spans. Embedded-string correspondence, general template/prefix mapping, nested indexed traversal, general intersection solving, record/contextual
definitions, `%prev`/`%tPrev`, range algebra, tentative branch transactions,
commutators and theorem discovery are not migrated.
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
g++ -std=c++17 -Wall -Wextra -Werror graph/test_list_structure.cpp -o /tmp/proteus4-list-structure-tests
/tmp/proteus4-list-structure-tests
g++ -std=c++17 -Wall -Wextra -Werror graph/test_span_alignment.cpp -o /tmp/proteus4-span-tests
/tmp/proteus4-span-tests
```

Run the CodeDog tests **from `LocalBuild`**, where the generated resource paths
resolve:

```sh
cd LocalBuild
./TestProteus -t unit/graphFoundation/
./TestProteus -t unit/graphDefinitions/
./TestProteus -t unit/graphListDefinitions/
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
selected existing language fixtures through the public `Agent.normalize()`
graph entry point.
The recorded first run is in [language-baseline-results.md](language-baseline-results.md).
See [entry-lifecycle-results.md](entry-lifecycle-results.md) for the entry-point
and streaming lifecycle migration. See [selection-results.md](selection-results.md) for preceding validation,
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
