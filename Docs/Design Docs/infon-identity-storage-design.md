# Infon identity, connection, and storage design

Design basis: legacy normalizer behavior audited from `main` at
`d10cad662bc0a2a7985ddf08c265f5a62ef5a8f6`, plus the operation and mutation
inventories in this directory.

## Goal

Provide one coherent system for applying and tracing Proteus identity while
allowing the executor to choose among:

- one shared whole-infon handle;
- distinct infon heads with live-shared `value`, `infSize`, type, or other
  components;
- a materialized copy when no future connection is observable;
- a POV or mapped view when updates or writes must reach an original location;
- branch-local speculative state for contradiction checking;
- independently mutable template instances, with an eager-copy implementation
  initially and copy-on-write storage later.

The design must not infer identity from equal contents, shared storage, or a
selection. Only a Proteus identity assertion, including a user-forced
assertion, supplies evidence that two infons are identical. A selection can
retain or recover a reference to the exact object selected; it does not prove
that a different infon is identical to it.

## Central distinction

Three relations that are currently mixed together must be represented
separately:

| Relation | Meaning |
| --- | --- |
| Semantic identity | Proteus has evidence that two specified information locations are identical at a specified extent. |
| Live connection | Future reads or writes must follow a source, target, or mapping. |
| Storage sharing | Two runtime objects happen to share physical backing, possibly only until the first write. |

Semantic identity may exist without a retained live connection. Storage may be
shared without semantic identity, as with two COW instances of one template.

This gives the fundamental rule:

> Pointer equality may realize an already authorized identity, but pointer
> equality or equal contents must never create identity evidence.

## Existing representation and feasibility

The generated C++ representation already gives `infon` independent shared
handles for `value` and `infSize`:

```text
infon
  shared_ptr<pureInfon> value
  shared_ptr<pureInfon> infSize
  shared_ptr<WordUse>   type

pureInfon
  shared_ptr<infon>     listSpec
```

Therefore the runtime can, in principle:

- share one complete infon;
- retain a distinct infon head while sharing only `value`;
- retain a distinct head while sharing only `infSize`;
- share both components while keeping other head state separate;
- share an immutable `listSpec` graph between instances.

Current source code does not provide the required semantics. Direct field and
struct writes can replace shared component handles, and no link records whether
sharing means live identity or COW backing. `pureInfon.copyAsTypeTo()` also
starts with a struct copy and does not recursively clone `listSpec`, so a
supposedly deep infon copy can retain a shared mutable list-spec handle.

COW is therefore feasible without changing the language-level shape of an
infon, but it requires all planned writes to pass through a component/link
abstraction. It cannot be implemented safely while arbitrary direct writes
bypass that abstraction.

## Identity and storage identifiers

Address identity alone is insufficient once COW and mapped views exist. The
runtime should distinguish these identifiers:

| Identifier | Identifies |
| --- | --- |
| `POVId` | One structural occurrence/slot in a container. |
| `InfonId` | One semantic infon head/entity. |
| `CellId` | One semantic component such as value, size, type, or list shape. |
| `StorageId` | Physical backing memory, which may be shared without semantic identity. |
| `EvidenceId` | The ordinary or user-forced assertion that establishes an identity relation. |
| `SelectionTraceId` | The path and operation that retained or recovered an already existing location; this is provenance, not identity evidence. |
| `Revision` | The observed version of an infon, component, mapping, or work state. |

Two COW template instances have different `InfonId` and `CellId` values even
while their components refer to the same immutable `StorageId`. Two live-
aliased value components share a `CellId` and mutable storage because writes
must be observed by both heads.

For in-process execution these identifiers may resolve to shared pointers. For
cross-process updates they must be opaque registry handles plus revisions; a
raw C++ address is not a stable cross-process identity.

## Runtime objects

### Infon head

The infon remains the independently addressable head:

```text
InfonHead {
    infon_id
    value_link
    size_link
    type_link
    list_shape_link       // may initially remain value.listSpec
    structural_state
    provenance
    work_state
    revision
}
```

Keeping the head separate allows an operation to preserve item mode,
selection metadata, paths, reply/origin, work state, or an occurrence-specific
role while sharing or copying only selected information components.

### Component link

Each independently shareable component is accessed through a link with an
explicit mode:

```text
ComponentLink<T> {
    cell_id
    mode                 // owned, live_alias, cow, mapped
    storage_ref
    source_location
    mapping_ref
    revision
}
```

| Mode | Read behavior | Write behavior |
| --- | --- | --- |
| `owned` | Read local storage | Mutate local storage and advance its revision. |
| `live_alias` | Read the shared semantic cell | Mutate the shared cell so every alias observes the update. |
| `cow` | Read immutable shared backing plus any local overlay | Detach/path-copy the affected storage, then write locally. |
| `mapped` | Resolve through an explicit source mapping | Translate writes through the mapping if it is writable. |

`cow` and `live_alias` may both use shared pointers, but they have opposite
write semantics. They must never be distinguished only by reference count.

### Location reference

Operations act on locations, not merely infon pointers:

```text
LocationRef {
    root_ref             // POVId, InfonId, or external handle
    extent               // whole, value, size, type, list_shape, path, view
    path_or_mapping
}
```

The reference level determines what later changes remain observable:

| Location | Observes/supports |
| --- | --- |
| POV slot | Item replacement, list topology, and writes to the selected occurrence. |
| Infon head | Mutations to that object, but not replacement of the POV containing it. |
| Value/size/type cell | Only that component; parent-head replacement is not automatically observed. |
| Mapped view | A defined projection over one or more source locations. |

### Identity trace

Identity authorization and realization are recorded separately:

```text
IdentityTrace {
    evidence_id
    evidence_kind        // assertion, user_override
    left_location
    right_location
    extent
    established_revision
    realization_receipt
}
```

The trace may outlive a transient source for explanation, replay, and audit.
It need not keep the source object alive when no future operation can refer to
that object.

Selections and mapped views instead retain a `SelectionTraceId` with their
`LocationRef`. That trace explains why the result names an existing location,
but it cannot be used to merge a different infon with that location.

## Connection requirements

Before selecting storage mechanics, the planner/executor must state the future
behavior that normalization must preserve:

```text
ConnectionRequirements {
    observe_source_updates
    write_back_to_source
    preserve_target_head
    preserve_target_pov
    identity_may_be_observed_later
    source_escapes
    target_escapes
    mapping_required
}
```

A source is transient only when reachability analysis shows that no agenda
item, world/context object, retained POV, reply, external reference, or future
operation can name it after the identity application.

A result being fully known is evidence that normalization will not need to
write it again, but does not by itself make copying safe. A persistent external
source may still change later.

## Identity application operation

The solver should ask for a semantic identity application rather than select a
raw copy primitive:

```text
ApplyIdentity {
    target_location
    source_location
    extent
    evidence_id
    authority             // assertion or user_override
    conflict_policy       // reject or replace
    requirements
}
```

The executor chooses a safe realization and returns it explicitly:

```text
IdentityRealizationReceipt {
    strategy
    retired_locations[]
    adopted_identity_ids[]
    component_link_changes[]
    mapping_changes[]
    constraint_changes[]
    invalidated_revisions[]
    resulting_revisions[]
}
```

The choice is not arbitrary. Two strategies are interchangeable only when no
permitted future observation can distinguish them.

## Realization strategies

| Strategy | Use when | Result |
| --- | --- | --- |
| `alias_head` | Whole-infon identity is live and one shared head is acceptable | Target POV refers to the exact source infon. Updates and work state are naturally shared. |
| `relocate_identity` | Source is transient, target head must survive, and the entire identity can move | State/identity ownership moves into target; source is retired. No lasting source pointer is required. |
| `alias_components` | Heads must differ but value, size, type, or shape must remain live-identical | Selected component cells use `live_alias`; unrelated head state remains independent. |
| `materialize` | No later read, write, or identity observation can distinguish a copy | Target receives owned component state; trace/receipt retains authorization. |
| `mapped_view` | Result is a sub-view, projection, reorder, or composite over source locations | Reads and permitted writes use an explicit mapping. |
| `read_proxy` | Future reads must follow a source but writeback is unnecessary | Reads resolve dynamically; writes are rejected unless a write mapping is supplied. |
| `speculative_overlay` | Candidate state is being tried for contradiction/branch search | Base state remains unchanged; all tentative writes live in a branch-local delta. |
| `cow_instance` | Template-derived state must be independently writable but is expensive to clone eagerly | Instance has distinct semantic IDs and shares immutable backing until a write. |

### Strategy selection

Choose the least-coupled strategy that preserves every required capability:

1. If this is speculative, use `speculative_overlay`.
2. If a projection/reordering is required, use `mapped_view`; reject writeback
   if the mapping is not invertible for the requested write.
3. If whole-infon live identity is required and a common head is legal, use
   `alias_head`.
4. If heads must differ but particular components require live updates, use
   `alias_components` for exactly those components.
5. If no future observation requires a live relation, use `materialize`.
6. If the source is transient, the target must survive, and the source identity
   itself must remain observable, use `relocate_identity`.
7. If the source is a template, use eager materialization initially or
   `cow_instance` when supported.
8. If no strategy provides the required read/write behavior, return
   `blocked`/`unsupported`; do not silently copy.

## Representative cases

### `_ = 5`

The RHS parsed literal is transient and cannot be named after this assertion.
No future update or writeback to that RHS is required.

```text
operation: ApplyIdentity
authority: assertion
conflict_policy: reject
strategy: materialize
extent: whole infon, or the explicitly required value/size/type cells
```

The executor copies the required RHS state into target-owned LHS components and
preserves the LHS head. The assertion remains in the `IdentityTrace`, but no
runtime link to the RHS is retained. The physical transfer may adopt uniquely
owned immutable backing as an optimization, provided that this is not
observable as a live RHS connection.

If RHS normalization has completed, no RHS worklist is transferred.

### Distinct head with identical value and size

If two heads have different roles but future writes to value and size must be
observed by both:

```text
target.value_link = live_alias(source.value_link)
target.size_link  = live_alias(source.size_link)
target.infon_id   remains distinct
```

If only a snapshot is required, use owned copies instead. If they are
template-derived independent instances, use distinct COW cells sharing only
the physical backing.

### Selected field writeback

A selected field has two separate relationships:

1. The selector result maps back to the original field POV.
2. A transient RHS supplies the new information.

The target is a writable POV/path `LocationRef`; the RHS can be materialized
without retaining an RHS link. This avoids using intersection `outerPOV` as a
writeback pointer.

### Fully known read-only result

If the source does not escape and the complete result will not be written or
observed through another handle, materialization is sufficient. The identity
trace can remain in the operation receipt without retaining live storage
coupling.

### Forced correction

`forceID` is explicitly authorized by the user's `<<` assertion syntax. It is
not an authorization gap:

```text
authority: user_override
conflict_policy: replace
```

For a correction such as `days-in-week <<= 7`, the target head should normally
survive so all existing references to the modeled `days-in-week` entity see
the correction. The transient RHS supplies replacement component state.

The operation must explicitly:

- invalidate contradictory target value/size facts and their proof receipts;
- resolve, invalidate, or remove conflicting target work;
- transfer any still-relevant RHS obligations;
- invalidate derived indexes, normalization flags, proxies, and revisions;
- publish a new target revision and a forced-identity receipt.

Force changes conflict policy, not the rules for selecting a live connection.

## Speculative contradiction checking

Scratch infons should not normally be ordinary mutable clones. Use a
branch-local overlay:

```text
SpeculativeState {
    base_revision_set[]
    component_deltas{}
    structural_deltas{}
    created_objects[]
    pending_obligations[]
    identity_traces[]
}
```

Reads consult the overlay and then the immutable base. Writes affect only the
overlay. Rejecting a branch discards it; accepting a branch produces an
ordered operation path whose revisions are checked before commit.

The logic engine can own scalar facts, identity evidence, contradictions, and
branch selection. Host-side overlays are still required for operations the
logic model does not yet express, such as POV topology, mapped list views, and
external effects. Both must use the same `LocationRef` and revision scheme so
they form one transaction rather than two independent models.

This approach is also the natural implementation foundation for later COW:
both read a base plus a local delta and materialize only changed state.

## Template and list-spec COW

Template instantiation has different semantics from live identity:

- each applied definition/list-spec instance receives distinct `InfonId` and
  `CellId` values;
- the definition/template graph is frozen;
- instances may share its immutable `StorageId` backing;
- a write detaches only the modified component or graph path;
- worklists, agenda state, tentative flags, reply/origin, paths, and executor
  caches are always instance-local and are never inherited as COW state;
- `copySource`/template provenance records derivation, not a writeback link.

For a large list spec, graph-level COW should use path copying:

```text
instance head
  -> local overlay for changed nodes
  -> immutable template graph for all other nodes
```

On first write to a path, clone the nodes along that path and preserve shared
immutable subgraphs elsewhere. Graph cycles and shared subgraphs require a
memo table keyed by template node ID; naive recursive tree copying is not
sufficient.

### Staged implementation compatibility

The semantic interface does not require COW on day one:

1. Implement `cow_instance` using the existing eager deep copy.
2. Require all mutations to obtain a writable component/location through the
   transaction API.
3. Freeze templates and reject direct template writes.
4. Later replace eager copying with shared immutable backing and detach-on-write.

Because the planner emits `instantiate_template`/`copy_from_template`, not
`memcpy`, the storage strategy can change without changing solver semantics.

## Worklist and obligation policy

Worklists describe incomplete normalization, not passive infon value. They are
head/transaction state by default and should not be hidden inside a generic
copy flag.

```text
ConstraintPolicy {
    existing_target      // retain, invalidate_conflicts, clear
    source_pending       // none, share, move, clone, translate
}
```

| Realization | Default policy |
| --- | --- |
| `alias_head` | One head and one worklist; do not duplicate work. |
| `relocate_identity` from transient source | Move relevant source obligations, merge with non-conflicting target obligations, then retire source work. |
| `alias_components` | Keep head worklists separate; share only explicitly scoped semantic cells. Generate obligations when their component revisions change. |
| `materialize` from normalized source | Transfer no work. |
| `materialize` from unresolved persistent source | Clone only the obligations required for the materialized extent. |
| `mapped_view` | Translate obligations by the mapping or route them to the source location. |
| `speculative_overlay` | Keep obligations branch-local until commit. |
| `cow_instance` | Start with instance-local obligations derived from the template contract; never share template worklist objects. |
| forced replacement | Invalidate conflicting target work and move/clone only relevant RHS obligations. |

An operation must never lose pending work merely because it rebinds a POV, nor
copy all work merely because it copied value state.

## Intersection structure versus source connection

`outerPOV` retains its original structural meaning: the POV outside an
intersection alternative. It is not identity evidence and is not the general
writeback connection.

Current ordered-span and marked-source code overloads `outerPOV` when paired
with `aRefTo`. The new design replaces those uses with `LocationRef` plus a
`mapped` component/view connection. `sourcePOV` can remain temporarily as
selection provenance during migration, but must not be promoted into an
intersection field to obtain writeback behavior.

## Transaction and concurrency model

Every mutating operation executes as a revisioned transaction:

1. Resolve all `LocationRef` values.
2. Verify identity-assertion evidence and selection provenance.
3. Record source, target, component, mapping, and work revisions.
4. Select a realization satisfying connection requirements.
5. Preflight conflict and COW-detach behavior without publishing changes.
6. Apply component, mapping, topology, and work mutations atomically.
7. Advance revisions and invalidate derived caches/indexes.
8. Return an `IdentityRealizationReceipt`.
9. Re-observe the requested normal-form postconditions.

An update received from another process uses opaque IDs and expected revisions.
A stale plan fails preflight and is replanned; it must not partially overwrite
newer state.

## Required invariants

1. Identity evidence comes only from an ordinary or user-forced Proteus
   identity assertion.
2. Equal content never creates an identity trace.
3. Selection provenance and shared template storage never create identity
   between distinct infons.
4. A `live_alias` write is visible through every alias of that cell.
5. A `cow` write is visible only in the writing instance after detachment.
6. A materialized copy is allowed only when no required future observation can
   distinguish it from a retained connection.
7. A mapped write is allowed only when the mapping defines an unambiguous
   destination.
8. `outerPOV` is intersection structure, not a source/writeback link.
9. Worklist disposition is explicit and scoped to the affected extent.
10. Forced identity preserves user authority and explicitly invalidates
    overridden state.
11. Scratch/speculative state cannot mutate live infons before commit.
12. Every operation records enough revisions and effects for replay and
    postcondition verification.

## Initial implementation boundary

The first implementation need not add general COW or replace every existing
normalizer mutation. It must establish the abstractions that prevent another
redesign:

1. Introduce opaque `LocationRef`, identity evidence, and realization receipts.
2. Introduce explicit connection requirements and strategy selection.
3. Support `alias_head`, `relocate_identity`, `alias_components`,
   `materialize`, and a basic mapped POV view.
4. Make value/size/type writes go through revisioned component operations.
5. Implement branch-local scalar/component overlays for solver trials.
6. Make worklist disposition explicit for each identity operation.
7. Treat template instantiation as a storage-policy boundary; use eager copy
   initially while guaranteeing templates are not mutated.
8. Reserve `cow` mode and immutable backing IDs so COW can replace eager copy
   without changing planner operations.

Before production routing, representative cases must verify exact head alias,
component-only alias, transient identity relocation, selected writeback,
mapped-view read/write behavior, speculative rollback, forced correction,
worklist movement/cloning, template independence, and stale-revision rejection.
