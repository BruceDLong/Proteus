# Infon transfer and copy taxonomy

Source snapshot: `main` at `d10cad662bc0a2a7985ddf08c265f5a62ef5a8f6`

Companion inventories:

- `Docs/Design Docs/Historical/normalization-mutation-matrix.md` records each mutation unit reachable from
  `processAgendaItem()`.
- `Docs/Design Docs/Historical/infon-operation-catalog.md` proposes the broader normalization-planner
  output vocabulary.

## Purpose

The current normalizer uses the word "copy" for operations with very different
meanings.  Conversely, the same semantic intent is sometimes implemented by
different physical mutations.  A planner cannot safely return one generic
`copy` operation because its executor would not know which identity,
allocation, topology, preservation, or obligation postconditions are intended.

This taxonomy separates four questions:

1. **Why is the transfer authorized?**
2. **What part of the source is transferred?**
3. **Where and how is the result installed?**
4. **What identity and state postconditions must hold?**

Equality of printed or scalar content answers none of those questions.  In
particular, equal `cat` infons do not authorize any identity operation.  An
identity operation requires a particular identity assertion; a selection
operation requires the POV/provenance that selected the source.

## Terms and postconditions

The following states must remain distinguishable:

| Term | Required postcondition |
| --- | --- |
| Exact identity | `target_pov.pItem === source_infon` after the operation. |
| Target-storage preservation | The object originally held by the target POV still occupies that slot, but some or all of its state changed. |
| Detached clone | A newly allocated object has copied state but is not handle-identical to the source. |
| Shallow state equivalence | Selected scalar fields and handles match; referenced subobjects or list topology may be shared. |
| Source-backed view | A POV or wrapper explicitly points to selected source objects using `outerPOV`, `sourcePOV`, or `accessMode=aRefTo`. |
| Provenance link | `copySource`, `proxy`, predecessors, or another link records where state came from without asserting handle identity. |
| Obligation equivalence | Two targets have equivalent pending constraints; this says nothing about their infon identity. |

"Preserve target identity" below always means preserve the target object's
storage handle.  It does not mean that the target becomes identical to the
source.

## The four independent axes

### 1. Authorization

| Authorization kind | Evidence required | Examples in current code |
| --- | --- | --- |
| `asserted_identity` | The exact Proteus assertion/merge obligation relating target and source | Strict merge branches that reach `copyIdentity()`; the current AItem is only implicit evidence. |
| `selected_source` | The cursor, path, marked-return, alternative, or ordered-span POV chain that selected the object | `%world`, direct parts lookup, marked returns, unique alternatives, source-backed span views. |
| `selected_template` | Definition/list-spec reference plus its selection/provenance | `derefInfon()`, `seedTypeSpecs()`, definition-derived fields and ordered-span shapes. |
| `compatible_merge` | The exact merge obligation and the checks that accepted it | Value, prefix, size, type, and list-shape copying. Compatibility is authorization for those content effects, not identity evidence. |
| `branch_construction` | The RHS intersection and the alternative POV for this branch | `mergeRHSIntersect()`. |
| `representation_rewrite` | The source node and the normalization rule whose representation is being removed or split | Singleton concat collapse, re-functionization, sparse-span splitting. |
| `external_replacement` | The include/parse request and returned parse object | Include-stream target replacement. |
| `force_flag` | The user's explicit `<<` assertion syntax | This is `user_override` authority with replacement conflict policy. The invalidation and worklist replacement contract is incomplete, but authorization is explicit. |

Authorization must be carried in `evidence_refs[]`.  It must never be recovered
later by comparing source and target contents.

### 2. Extent

| Extent | State included |
| --- | --- |
| `whole_infon` | Flags, provenance, type/order, size, value, children, args, intersection state, and optionally paths/work. |
| `value` | `pureInfon` value format, size mode, and number/string/list payload. |
| `value_kind` | `value.fType` and the owning infon's `infMode`. |
| `size` | The `infSize` pure infon. |
| `word_type` | The `WordUse` handle in `type`. |
| `list_shape` | List spec, order mode, size/size mode, open-tail state, and sometimes type; never concrete members. |
| `slot_metadata` | Request-local type/list shape, item mode, reply, path, origin, or tentative state named by the caller. |
| `constraints` | Worklist POVs and their constraint infons. |
| `pov_and_provenance` | Item handle plus selection/outer/source/predecessor/topology fields. |

### 3. Placement and topology

| Placement/topology | Physical result |
| --- | --- |
| `rebind_exact` | Change a POV to the exact existing source object. |
| `overwrite_deep` | Recursively copy state into the existing target object. |
| `overwrite_shallow` | Struct-assign state into the existing target; referenced handles may be shared. |
| `allocate_deep` | Allocate a distinct target and recursively copy the source graph. |
| `allocate_shallow` | Allocate a distinct target, then struct-copy selected state. |
| `copy_members` | Mutate only the named extent in the existing target. |
| `copy_prefix` | Copy a bounded prefix and produce or retain a remainder. |
| `make_view` | Allocate POV/wrapper structure that refers to existing source objects. |
| `share_topology` | Alias existing POVs or a `PovList` head/tail. |
| `move_references` | Attach the exact existing POVs to a new owner and clear the old owner. |

### 4. Preservation and invalidation

A complete operation must name, rather than imply, at least the applicable
items below:

- whether the target object handle and target POV survive;
- whether concrete children, list spec, type, item mode, paths, reply/origin,
  tentative state, `thisPtr`, and provenance survive;
- whether worklists are copied, moved, shared, cleared, or dropped;
- whether list topology and child objects are deep-copied or shared;
- which normalization, word-resolution, alternative, size, and index caches
  become invalid;
- what new merge, normalization, or branch obligations are generated.

## Observed authorization/transfer combinations

These are the combinations actually used by the main snapshot.  A missing
combination is not implicitly supported.

| Authorization | Placement | Extent/topology | Identity postcondition | Representative rows |
| --- | --- | --- | --- | --- |
| Asserted identity | Exact POV rebind | Whole infon, exact reference | Exact source handle | `G-006`, `H-003` |
| Asserted identity plus source-backed target | Deep overwrite | Whole infon, recursive clone | Target storage preserved; no source-handle identity | `H-002`, `S-015` |
| Asserted identity with open definition shape | Selective member copy | Type/list shape only | Target storage and concrete children preserved | `H-001`, reached through `G-006` |
| Explicit cursor/path/branch selection | Exact POV rebind | Whole infon, exact reference | Exact selected handle | `I-018`, `I-033`, `X-014` |
| Explicit selection requiring detached result | Deep clone then overwrite | Whole infon, recursive clone | Target storage preserved and detached from source | `I-019` |
| Explicit selection requiring source view | Allocate view/wrapper | Exact source handles under new topology | Wrapper distinct; children/source exact | `X-010`, `L-012`, `S-005` through `S-009` |
| Explicit selection closing a source-backed outer result | Shallow overwrite | Whole infon, shared referenced state possible | Outer storage preserved; no source-handle identity | `I-040` |
| Selected definition/list spec | Deep overwrite | Whole infon, recursive clone | Target storage preserved; definition provenance recorded | `I-011`, `L-001` |
| Selected definition/list spec | Deep allocation | Whole constraint infon, recursive clone | Distinct template-derived object | `L-002` |
| Selected definition | Shallow allocation | Whole field state | Distinct field; referenced subobjects may be shared | `M-007` |
| Compatible merge | Selective member copy | Value, size, word type, or list shape | Content/metadata only | `G-001`, `G-002`, `G-003`, `G-005`, `G-007`, `G-014`, `G-017`, `G-021` |
| Compatible loose string merge | Prefix copy | String value prefix | Content only plus optional remainder | `G-022`, `G-023`, `H-006` |
| Accepted range merge | Shallow overwrite | Whole infon state | Target storage preserved; no identity | `G-032`, `G-033`, `G-039`, `G-040` |
| RHS intersection branch | Deep allocation | Whole LHS clone per alternative | Distinct tentative branches | `H-005` and its generated call sites |
| Representation rewrite | Shallow overwrite/allocation | Whole infon or value | Wrapper/target storage rules are operation-specific; no source identity | `I-038`, `I-042` |
| Ordered-span split | Deep allocation plus count rewrite | Sparse span remainder | Distinct remainder with copied shape | `L-010`, `S-011` through `S-014` |
| Constraint propagation | Share exact POV/infon | Constraint reference | Constraint identity preserved | `M-006`, `P-011`, `F-005` through `F-007` |
| Constraint cloning | Deep allocation | Whole constraint graph | Distinct but state-equivalent obligation | `G-013`, `G-016`, `F-008` |
| Constraint transfer | Move or share exact POVs | Worklist topology | Constraint identity preserved; ownership differs | `I-032`, `X-003`, `X-012`, `F-009` |
| External parse result | Exact POV rebind | Whole newly parsed graph | Exact returned parse object | `I-003`, `E-005` |
| User-forced identity assertion | Deep overwrite | Whole infon, recursive clone | Target head preserved with forced replacement state; no RHS-handle identity | `M-005`; invalidation/worklist contract unresolved |

The table makes the central asymmetry explicit: authorization does not choose
placement, and placement does not supply authorization.  An asserted identity
may lead to an exact rebind or to materialization in a source-backed slot, but
only the first has an exact source-handle postcondition.

### Combinations that must be rejected

| Requested conclusion/action | Why it is invalid |
| --- | --- |
| Rebind or identity materialization authorized by equal content | Content equality is not a Proteus identity assertion. |
| Exact identity after a deep or shallow overwrite | The target handle remains distinct from the source handle. |
| Independent clone after struct assignment or `PovList` assignment | Referenced objects/topology may be shared. |
| Whole-infon normalization after only `copy_value`, `copy_size`, or `copy_word_type` | Those operations satisfy only named member obligations. |
| Identity between a source-view wrapper and its selected children | The wrapper is new; its child POVs retain exact references to the selected source objects but do not assert identity with the wrapper. |
| `copy_constraints` postconditions after `takeWrkListFrom(..., false)` | That path shares the exact constraint POVs rather than cloning them. |
| Identity proof from `copySource`, `proxy`, `outerPOV`, or `sourcePOV` alone | These are provenance/selection links. An assertion is still required to prove two distinct infons identical; a selection trace only identifies the selected location. |

## Physical mechanisms in the main snapshot

These are executor mechanisms, not sufficient planner outputs.

| Mechanism | Actual physical behavior | Matrix rows | Contract hazards |
| --- | --- | --- | --- |
| `POV.pItem <- source` | Exact handle rebind | `F-004`, used by `H-003` and selection rows | Old target state and work may become unreachable. Requires assertion or explicit selection evidence. |
| `infon.copyAsTypeTo(target, copyWork, copyParts)` | Recursive graph copy into caller-supplied storage | `F-001`, `F-002` | Clears `doReply`; shares `WordUse` and some other handles; work/path policy is boolean and call-site dependent; cycle handling is incomplete as a general graph-copy contract. |
| `target! <- source!` | Shallow struct/state assignment while retaining target storage | `F-003` | Referenced subobjects may be shared. Never establishes identity. |
| `DO_COPY(from, to, sizeToCopy)` | Member copy for number/string/list pure infons | `H-007` | List copy aliases `PovList` topology; string may copy a prefix; type mismatch silently does nothing. |
| `copyWrkListFrom()` | Deep-copies each constraint infon into a fresh POV | `F-008` | Leaves source constraints; does not itself clear target `isNormed`. |
| `takeWrkListFrom()` | Attaches exact existing work POVs; optionally clears source | `F-009` | `doClear=false` shares obligations; `doClear=true` moves them. |
| POV/list append and assignment | Creates POVs to existing infons or aliases list head/tail | `F-005`, `F-006`, `F-011`, `F-012`, `F-014`, `F-016` | Sharing is often intentional selection/provenance, but cannot be called a deep copy. |
| `copyIdentity()` | Dispatches among selective list-shape merge, deep overwrite, and exact rebind | `H-001` through `H-004` | One name has three incompatible identity postconditions. |
| `proxy`, overloaded source uses of `outerPOV`, `sourcePOV`, predecessors | Records a link to a source or selection path | `M-013`, `H-009`, `H-010` and traversal rows | `outerPOV` originally describes intersection structure; recent source-view code overloads it. None of these links is new identity evidence. |

Allocation is performed by each caller.  Therefore `copyAsTypeTo()` alone does
not reveal whether the result is an overwrite of an existing slot, a detached
clone, a template instance, a branch, or a sparse remainder.

## Semantic operation families

The planner should expose intent-specific operations.  Several may share one
internal transfer executor, but they must retain distinct authorization and
postconditions.

### Identity realization

| Planner operation | Authorization | Placement and extent | Required postcondition | Current rows |
| --- | --- | --- | --- | --- |
| `rebind_identity` | `asserted_identity` | `rebind_exact`, `whole_infon` | Target POV and source refer to the exact same object. | Generated call sites `G-006`, `G-013`, `G-016`, `G-019`; helper `H-003`; primitive `F-004`. |
| `materialize_identity_into_slot` | `asserted_identity` plus proof that the target is a selected/source-backed slot that must retain its object | `overwrite_deep`, `whole_infon` | Target storage is preserved and has the required source state; **no** source/target handle-identity claim. | `G-006`, `G-013`, `G-016`, `G-019`, `S-015`; helper `H-002`; deep-copy primitives `F-001`, `F-002`. |
| `merge_open_list_shape` | The current assertion plus an open definition/list-shape source | `copy_members`, `list_shape` | Concrete LHS members and storage survive; only missing shape fields are installed. | `G-006`, `G-019`, `G-024`; helper `H-001`. |
| `record_proxy_source` | Accepted non-loose merge and its assertion evidence | Link only | `target.proxy=source`; no copied state or new proof. | `M-013`. |

`assert_identity` is a prerequisite fact/obligation, not a mutation inferred by
these operations.  The current code does not carry an explicit evidence object
from the assertion into `copyIdentity()`; matrix code `A?` records that gap.

### Selection and source views

| Planner operation | Authorization | Placement | Required postcondition | Current rows |
| --- | --- | --- | --- | --- |
| `select_cursor_source` | Cursor resolution POV/anchor | `rebind_exact` for `%world` with no remaining path | Result is the specifically selected world object. Detached neighbor results use `copy_selected_cursor_result` below. | `I-018`, plus metadata repair `I-021`. |
| `select_path_result` | Direct or synthetic path selection POV | `rebind_exact` and, when known, `aRefTo` provenance | Result is the specifically selected object, never a content match. | `I-028`, `I-033`, `H-009`, `H-010`. |
| `select_unique_alternative` | The sole accepted branch and predecessor trace | Selection only; produces a selected source reference | The chosen branch is explicit and justified by rejection/predecessor state. | `X-006`; installation is `X-014`. |
| `install_selected_result` | A preceding selection operation | `rebind_exact` plus named metadata transfer | Target POV names the exact selected object. Any writes to the selected source are explicit effects. | `X-014`. |
| `materialize_selected_result_into_slot` | A preceding selection/closure operation and a source-backed outer slot | `overwrite_shallow`, `whole_infon` | Outer target storage survives with selected state; no handle-identity claim with the selected branch. | `I-040`. |
| `return_marked_source_view` | Marked-return POV | `make_view` | Wrapper and child POVs refer back to the exact marked source. | `X-002`, `X-009` through `X-011`, `P-006`. |
| `make_source_view` | Ordered-span or traversal selection | `make_view` or `share_topology` | View children point to exact selected source items; wrapper identity is distinct. | `P-001`, `L-012`, `S-005` through `S-009`. |
| `flatten_singleton_sublist` | Explicit containment plus flattening rule | `rebind_exact` to sole child | Wrapper POV now names the contained child and retains `isSubItm`. | `I-041`. |

Related provenance/link rows are `P-009` through `P-011`, `L-007` through
`L-009`, and `L-015`.  They preserve the traversal path needed to justify a
later selection; they do not themselves copy an infon.

### Whole-state materialization and cloning

| Planner operation | Authorization | Placement | Required postcondition | Current rows |
| --- | --- | --- | --- | --- |
| `copy_from_template` | `selected_template` | `overwrite_deep` for a blank target or `allocate_deep` for a constraint | Distinct state materialization with `copySource` and `copySourceType`; no identity with template. | `I-011`, `L-001`, `L-002`; primitives `F-001`, `F-002`. |
| `clone_branch` | `branch_construction` | One `allocate_deep` LHS clone per RHS alternative | Branches are distinct, tentative, carry branch-specific constraints and predecessor/outer links. | Call sites `G-008`, `G-009`, `G-025` through `G-031`, `G-035` through `G-038`; helper `H-005`. |
| `copy_selected_cursor_result` | `%prev/%next` selection | `allocate_deep` followed by overwrite of target storage | Target is detached from the neighbor even though contents were copied. | `I-019`. |
| `copy_infon` | Explicit non-identity copy authority, or a user-forced identity replacement | `overwrite_deep` or `allocate_deep`, `whole_infon` | Target has copied state, with work/path policy and invalidations stated; no RHS-handle identity. | `M-005` (`forceID`), `F-001`, `F-002`. `forceID` is user-authorized; its invalidation contract remains incomplete. |
| `overwrite_infon_state` | Cursor/range/representation rule | `overwrite_shallow`, `whole_infon` | Target storage survives; referenced state may be shared; no identity. | `I-020`, `G-032`, `G-033`, `G-039`, `G-040`; primitive `F-003`. |
| `create_field_from_definition` | Missing unordered field plus definition provenance | `allocate_shallow` then append | New field is distinct and records the definition source; current shallow assignment may share subobjects. | `M-007`. |
| `resolve_word_definition` | Definition lookup result | `allocate_deep` for chosen/alternative models | Cached model or alternative graph is distinct and records its definition route. | `I-044`. |

### Partial-state copy

| Planner operation | Authorization | Extent/topology | Current rows and important effects |
| --- | --- | --- | --- |
| `copy_value` | Accepted strict/loose value merge | `value`, `copy_members` | `G-007`, `G-014`, `G-017`, `G-021`; helper `H-007`. `G-007` also initializes `value_kind`; `G-021` also copies negation. |
| `consume_string_prefix` | Accepted loose string-prefix merge | `value`, `copy_prefix` | `G-022`, `G-023`, helper `H-006`; may create an RHS remainder. |
| `copy_size` | Accepted strict size merge | `size`, `copy_members` | Dispatcher `M-004`, generated `G-003`, helper `H-007`. |
| `copy_word_type` | Type-compatible merge with missing target type | `word_type`, shared handle | `M-009`, `G-005`. This is type information, not infon identity. |
| `copy_list_shape` | Selected definition/list template | `list_shape`, selective member copy | `I-017`, dispatcher `M-004`, `G-001`, `G-002`; overlaps `H-001` when invoked through identity handling. |
| `preserve_slot_metadata` | A selected/rebound target slot | Named `slot_metadata` fields | `I-021`, `H-004`, `X-014`. These are part of another operation's atomic postcondition, not a free-standing copy permission. |
| `collapse_singleton_concat` | `representation_rewrite` | Either `value` shallow copy or `whole_infon` shallow overwrite | `I-038`; preserves wrapper storage and removes concat representation. |
| `normalize_size_expression` | `representation_rewrite` | Wraps the existing size expression as temporary value state | `I-022`; creates a normalization obligation rather than satisfying the size itself. |
| `derive_type_or_shape_metadata` | Definition/function/list-spec derivation | Selected type/order/provenance fields | `I-004`, `I-005`, `I-009`, `I-012`, `M-018`, `L-004`, `L-005`, `S-001`. These are better named by the fact derived than by `copy`. |

`DO_COPY()` is not a sufficient executor for a general `copy_value`: its list
case aliases the `PovList` head/tail, while its scalar cases copy value data,
and its string case can copy only a prefix.  Those topology/prefix differences
must be explicit in the operation parameters or split into separate internal
executors.

### Structural clones and representation transfer

| Planner operation | Authorization | Transfer behavior | Current rows |
| --- | --- | --- | --- |
| `make_intersection` | `representation_rewrite` | Shallow-copies the old infon into a new inner object, then rewrites the original object as an intersection. | `I-042`, primitive `F-010`. |
| `compile_selector` | Path/index request | Builds source-derived pattern graphs and transfers remaining path/work state. | `I-029`, `I-030`, `I-031`. |
| `split_sparse_span` | Ordered-span alignment rule | Deep-copies a sparse span into a distinct remainder, rewrites counts, and links the remainder. | `L-010`, `S-011` through `S-014`. |
| `expose_sparse_head` | Traversal/materialization rule | Creates a template/type-derived head, inserts it, and decrements/removes the sparse span. | `L-003` through `L-005`, `L-011`, `S-001`, `S-002`. |
| `concatenate_list` | Concatenation rule | Appends new POVs to the exact RHS child objects; children are shared, not cloned. | `I-036`. |
| `install_parsed_graph` | `external_replacement` | Rebinds target POV to the newly parsed graph, then schedules its normalization. | `I-003`, `E-005`. |

### Constraint transfer

Constraints are infons, but transferring a constraint is not copying the
normalized value of the constrained infon.

| Planner operation | Physical policy | Current rows | Required distinction |
| --- | --- | --- | --- |
| `add_constraint` | Add a POV to the exact source constraint | `I-010`, `I-014`, `I-023`, `M-006`, `X-001`, `P-011`; primitives `F-005` through `F-007` | Shares the constraint object and creates a merge obligation. |
| `copy_constraints` | `allocate_deep` for every constraint infon | Alternatives in `G-013`, `G-016`; primitive `F-008` | Source retains constraints; target gets distinct constraint graphs. |
| `move_constraints` | Attach exact existing POVs and clear source | `I-032`, `X-012`; primitive `F-009` with `doClear=true` | Ownership transfers; constraint identity is preserved. |
| `share_constraints` | Attach exact existing POVs without clearing source | `X-003`; primitive `F-009` with `doClear=false` | Both owners refer to the same obligations. |
| `dequeue_constraint` | Move exact POV from pending to used work | `I-024` | Agenda state transition, not a content copy. |
| `filter_constraints` | Retain all exact POVs except a consumed constraint | `S-003` | Rebuilds a worklist by reference; no constraint clones. |

`L-002` is deliberately composite: it first performs `copy_from_template` to
create a distinct constraint infon and then performs `add_constraint`.

## Call-site coverage from the mutation matrix

This section is the completeness index.  It maps every mutation row whose
own effect transfers infon state, a member, a POV/source reference,
provenance, or constraints.  Operation-specific dispatcher/helper/primitive
rows are listed even when that duplicates the same runtime mutation at
different levels of the call tree. Generic dispatch-only rows such as `M-010`
and `P-004` remain delegated to their concrete rows.

| Transfer family | Matrix rows |
| --- | --- |
| Exact object installation or selected-object rebind | `I-003`, `I-018`, `I-028`, `I-033`, `I-041`, `H-003`, `X-006`, `X-014`, `F-004`, `E-005` |
| Recursive whole-infon copy | `I-011`, `I-019`, `M-005`, `H-002`, `F-001`, `F-002` |
| Shallow whole-infon/state overwrite | `I-020`, `I-037`, `I-038`, `I-040`, `I-042`, `M-007`, `G-032`, `G-033`, `G-039`, `G-040`, `F-003`, `F-010` |
| Identity-copy dispatch and selective identity realization | `G-006`, `G-013`, `G-016`, `G-019`, `G-024`, `H-001` through `H-004`, `S-015` |
| Template, definition, branch, or structural clone | `I-044`, `G-008`, `G-009`, `G-025` through `G-031`, `G-035` through `G-038`, `H-005`, `L-001` through `L-005`, `L-010`, `L-011`, `S-001`, `S-002`, `S-011` through `S-014` |
| Value, size, type, shape, or slot-metadata transfer | `I-004`, `I-005`, `I-009`, `I-012`, `I-017`, `I-021`, `I-022`, `M-004`, `M-009`, `M-018`, `G-001` through `G-005`, `G-007`, `G-014`, `G-017`, `G-021` through `G-023`, `H-006`, `H-007`, `L-004`, `L-005`, `S-001` |
| Source views, shared child handles, and selection provenance | `I-036`, `H-009`, `H-010`, `X-002`, `X-003`, `X-005`, `X-009` through `X-011`, `P-001`, `P-006`, `P-009` through `P-011`, `L-007` through `L-009`, `L-012`, `L-015`, `S-005` through `S-009`, `F-011`, `F-012`, `F-014`, `F-016` |
| Constraint reference, clone, move, share, or filter | `I-010`, `I-014`, `I-023`, `I-024`, `I-032`, `M-006`, `G-013`, `G-016`, `X-001`, `X-003`, `X-012`, `P-011`, `L-002`, `S-003`, `F-005` through `F-009` |
| Provenance/proxy link without a copy | `M-013`, `H-009`, `H-010`, `P-006`, `P-010`, `L-015` |
| Source-derived selector/path construction | `I-026`, `I-029`, `I-030`, `I-031` |

Rows may appear in more than one family when the source operation is
composite.  For example, `X-003` both shares outer constraints and installs
selection provenance on each branch; `S-001` builds a new graph while copying
definition shape; `G-013` either realizes source-backed identity or clones
constraints.

The remaining mutation-matrix rows perform arithmetic or textual
normalization, list insertion/removal/closure without source-state transfer,
compatibility/rejection, agenda scheduling, lifecycle flags, indexing, or
external emission.  They remain required normalization operations, but they
are not variants of copying one infon to another.

## Proposed planner/executor boundary

There should be no public planner opcode named only `copy`.  A compact initial
semantic vocabulary that covers the current combinations is:

```text
rebind_identity
materialize_identity_into_slot
merge_open_list_shape
select_cursor_source
select_path_result
select_unique_alternative
install_selected_result
materialize_selected_result_into_slot
make_source_view
copy_from_template
clone_branch
copy_selected_cursor_result
copy_infon
overwrite_infon_state
copy_value
consume_string_prefix
copy_size
copy_word_type
copy_list_shape
preserve_slot_metadata
split_sparse_span
add_constraint
copy_constraints
move_constraints
share_constraints
record_proxy_source
```

These are the direct transfer operations. Source-derived construction remains
under its semantic operation (`make_intersection`, `compile_selector`,
`expose_sparse_head`, `install_parsed_graph`, and so on) with an internal
transfer policy describing the copied parts. The executor may share that
lower-level policy:

```text
TransferPolicy {
    placement          // rebind_exact, overwrite_deep, allocate_deep, ...
    extent             // whole_infon, value, size, type, shape, ...
    topology           // exact_reference, shallow_share, recursive_clone, prefix
    include_work
    include_paths
    preserve[]
    invalidate[]
}
```

But the policy is mechanics only.  A planner operation must also provide:

```text
TransferOperation {
    opcode
    target_ref
    source_refs[]
    authorization_kind
    evidence_refs[]
    policy
    expected_identity_postcondition
    effects[]
    creates[]
    generated_obligations[]
}
```

The executor must reject any operation whose evidence does not authorize its
semantic opcode, even if the requested physical policy is mechanically
possible.

## Coverage result and unresolved contracts

The proposed axes can describe every copy-like mutation combination found in
the matrix.  That is a coverage result, not yet proof that every operation is
correct.  The audit exposes these contracts that must be resolved before an
executor can safely replay plans:

1. `copyIdentity()` has three incompatible postconditions and must be split at
   the planner boundary.
2. The ordinary rebind branch drops access to the old target and its pending
   work unless another operation explicitly transfers it.
3. The source-backed branch realizes required state in a selected slot but
   does not make source and target handles identical.
4. `forceID` is explicitly authorized by the user's `<<` syntax and performs a
   deep replacement into the existing target. Its invalidation, stale-proof,
   and worklist replacement contract is incomplete.
5. Whole-infon deep-copy policy is controlled by two booleans and has several
   implicit field exceptions, including clearing `doReply`.
6. `DO_COPY()` has three topology modes hidden behind one function: scalar
   value copy, string-prefix copy, and list-topology aliasing.
7. Direct struct assignment is shallow for handle fields and therefore cannot
   promise an independent clone.
8. Constraint copy, sharing, and movement currently have different
   `isNormed`, source-clearing, and source-flag side effects.
9. Selection installation currently writes reply/path/origin metadata into the
   selected source object; `X-014` even marks the origin write as wrong in the
   source.
10. `copyWrkListFrom()` deep-copies constraint infons, whereas several other
    worklist paths preserve exact constraint handles.  A plan must choose
    deliberately between those postconditions.
11. List-shape inheritance and `merge_open_list_shape` overlap but do not copy
    exactly the same field set.
12. `copySource`, `proxy`, `sourcePOV`, and predecessors represent different
    kinds of provenance and must not be treated as interchangeable identity
    assertions. `outerPOV` should retain its original intersection-structure
    meaning; current source-view uses are an implementation overloading that a
    general connection model should replace.

The next design check should map each proposed planner opcode to an explicit
precondition/effect schema and then replay representative matrix rows against
those schemas.  The simplest mandatory discriminator is whether the expected
result is exact handle identity, preserved target storage, a distinct clone,
or a source-backed view.
