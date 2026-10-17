# Infon normalization operation catalog

Source snapshot: `main` at `d10cad662bc0a2a7985ddf08c265f5a62ef5a8f6`

## Purpose and scope

This catalogs the operations reachable from `Agent.normalize()` through the
agenda/normalizer on the source snapshot above. It is intended to define the
output vocabulary of a constructive normalization planner.

The catalog includes:

- semantic mutations to infons and POVs;
- creation, removal, or selection of infons;
- constraint propagation and alternative construction;
- terminal results and externally visible effects.

It excludes parser-only construction, indexes used only as caches, logging,
debug graphs, and AItem counters except where they create a dependency that a
planner must represent. Low-level mutations are listed separately from the
semantic operations so that the planner does not emit implementation details
such as `append()` when its intended operation is “instantiate a list member.”

An infon must be addressed by an opaque infon ID or POV/path ID. Its printed
content is not an identity. In particular, two independently located `cat`
infons are not the same infon unless an assertion or an identity-preserving
selection establishes that fact.

## Identity-affecting mechanisms in the current code

These mechanisms must not be collapsed into content equality.

| Current mechanism | Physical effect | What it does and does not establish |
| --- | --- | --- |
| `copyIdentity(aItem)`, ordinary target | `LHS_item.pItem <- RHS.pItem` | Rebinds the LHS POV to the exact RHS object. The strict assertion being processed is the evidence for doing this. |
| `copyIdentity(aItem)`, source-backed target | `RHS.pItem.copyAsTypeTo(LHS_item.pItem, ...)` | Overwrites the existing source object with a deep copy. It does **not** make the two C++ handles identical even though the function is named `copyIdentity`. |
| `copyIdentity(aItem)`, open list shape | Copies missing type/list-spec/order metadata into an existing concrete list | Preserves the concrete LHS list and does not rebind or overwrite it. |
| `copyAsTypeTo()` or `target! <- source!` | Copies state | Content/state copy only; never evidence of identity. `forceID` also uses this mechanism despite its name. |
| `outerPOV`, `sourcePOV`, and `accessMode=aRefTo` | Builds a view that points back to an existing source location | Preserves an identity already obtained by selection; it does not prove identity between independent infons. |
| `LHS.proxy <- RHS` after an accepted concrete strict merge | Adds a dereference/provenance link followed by `dpItem()` | Records the accepted source link. It should be emitted with the assertion/merge evidence that authorized it, not inferred from equal content. |

Sources: `Proteus.Lib.dog:408-463`, `Proteus.Lib.dog:2104-2168`,
`Proteus.Lib.dog:2842-2853`, `WorldManagerOrderedSpans.dog:199-243`.

## Proposed planner operation record

Every emitted operation should have enough information to check and execute it:

```text
Operation {
    op_id
    opcode
    target_ref          // opaque POV/infon/path ID, never serialized content
    source_refs[]       // RHS, template, selected POV, etc.
    evidence_refs[]     // especially the assertion authorizing identity
    preconditions[]
    effects[]
    preserves[]
    invalidates[]       // normalization flags, indexes, sizes, cached choices
    creates[]
    generated_obligations[]
    cost
}
```

The overall result should contain an ordered operation path, not just a proved
relation:

```text
NormalizationResult {
    status              // complete, blocked, rejected, undefined, error
    result_ref
    operations[]
    remaining_obligations[]
    alternatives[]
    external_effects[]
    diagnostics[]
}
```

`complete` means that the requested normal-form postconditions hold, not merely
that the engine found two compatible terms.

## Semantic model-changing operations

### Binding and copying

| Proposed opcode | Preconditions | Direct effect | Preservation/follow-up | Current implementation |
| --- | --- | --- | --- | --- |
| `assert_identity` | A specific strict identity assertion relates target and source | Records the asserted identity fact used by later steps | Must carry `evidence_ref`; content equality is insufficient | Implicit in the strict merge that reaches `copyIdentity()` |
| `rebind_identity` | `assert_identity` is available and the target POV may be rebound | Makes `target.pItem` the exact source object | Preserves selected target slot metadata such as an explicit type/list shape/item mode where required | `copyIdentity()` ordinary branch, `Proteus.Lib.dog:2104-2127` |
| `materialize_identity_into_slot` | `assert_identity` is available but the target is source-backed and cannot be rebound | Deep-copies source state into the existing target object | Preserves the target object's storage identity; must not claim handle identity with source | `copyIdentity()` source-backed branch, `Proteus.Lib.dog:2116-2122` |
| `merge_open_list_shape` | RHS is a definition-style open list and LHS is a concrete list | Fills absent type, list-spec, order, and item mode without replacing LHS items | Concrete LHS members are preserved | `mergeOpenListShapeIntoConcreteList()`, `Proteus.Lib.dog:2141-2147` |
| `copy_infon` | Source exists; copy policy says whether to include work and pending part paths | Deep-copies the infon graph | Clears `doReply`; optionally copies worklist/path state; does not establish identity | `pureInfon.copyAsTypeTo()` and `infon.copyAsTypeTo()`, `Proteus.Lib.dog:77-95`, `408-463` |
| `overwrite_infon_state` | A merge/range rule authorizes replacing all current LHS state while retaining the LHS storage handle | Assigns the RHS infon struct state into the existing LHS object | This may share referenced subobjects depending on field representation; it is neither a deep clone nor identity evidence | Direct `LHS.pItem! <- RHS.pItem!` range branches in `WorldManager.dog:263-275`, `361-382` |
| `copy_value_kind` | LHS is wholly unknown and loose copying is authorized | Copies `value.fType` and `infMode` | Followed by `copy_value` | `WorldManager.dog:107-109` |
| `copy_value` | Source and target pure values have the same `fType` | Copies number, string, or list value and format; string copy may consume only a prefix; current list case assigns the POV-list state rather than deep-cloning children | Does not copy word type or infon identity; may produce a remainder | `DO_COPY()`, `Proteus.Lib.dog:2023-2037`; merge call sites in `WorldManager.dog` |
| `copy_size` | Strict size, LHS size unknown, RHS size literal, and list-output guard passes | Copies `infSize` | May be preceded by definition-shape inheritance | `mergeSizeRules()`, `WorldManager.dog:76-85` |
| `copy_word_type` | Target lacks a Proteus word type and source has one | Copies the `WordUse` handle into `target.type` | Does not copy value kind, value, or identity | `WorldManager.dog:96-100`; `Proteus.Lib.dog:2823-2827` |
| `copy_list_shape` | Definition/template has a literal open-list shape and target is compatible | Copies missing list-spec, order mode, size, size mode, and open-tail state | Does not copy members | `inheritDefinitionListShape*()`, `WorldManager.dog:2-74` |
| `copy_constraints` | Source has pending constraints that must also apply to target | Deep-copies each worklist POV/infon into target worklist | Marks target non-normal; constraints remain on source | `copyWrkListFrom()`, `Proteus.Lib.dog:557-566` |
| `move_constraints` | Ownership of pending constraints transfers to a result/selection | Adds work POVs to target and optionally clears source worklist | Marks target non-normal | `takeWrkListFrom()`, `Proteus.Lib.dog:568-573` |
| `add_type_fact` | Definition lookup or a compatible merge establishes an additional Proteus type | Adds the `WordUse` to `addedTypes`, or marks an existing type fact initialized | This is type knowledge, not identity knowledge | `derefInfon()`, `Proteus.Lib.dog:1193`; accepted type-example merge at `Proteus.Lib.dog:2833-2843` |
| `record_proxy_source` | A non-loose merge with a concrete RHS has been accepted | Sets target generality concrete and records `target.proxy=source` | Must retain the strict merge/assertion as evidence; this affects `dpItem()` dereferencing | `Proteus.Lib.dog:2849-2853`; `POV.dpItem()`, `infonList.dog:51-59` |
| `preserve_slot_metadata` | A rebind/copy would otherwise erase selector-local metadata | Restores explicit type, request list-spec, item mode, reply/path/origin metadata | Must name each preserved field rather than silently copying all flags | `preserveLHSSlotMetadata()`, cursor and intersection return code at `Proteus.Lib.dog:2149-2154`, `1347-1373`, `3066-3076` |

### Template and definition materialization

| Proposed opcode | Preconditions | Direct effect | Preservation/follow-up | Current implementation |
| --- | --- | --- | --- | --- |
| `copy_from_template` | A particular definition or list-spec template has been selected | Deep-copies the template into a target or creates a template copy as a constraint | Records `copySource` and `copySourceType`; source kind must be `definition` or `list_spec` | `derefInfon()`, `Proteus.Lib.dog:1154-1203`; `seedTypeSpecs()`, `infonList.dog:281-316`; plural definition construction via `setSelfAsAPluralOf()`, `Proteus.Lib.dog:625-634` |
| `add_template_constraint` | Target already has concrete state, so its definition cannot replace it | Adds the selected definition model to the worklist with priority | Target is marked concrete; generated merge must still run | `derefInfon()`, `Proteus.Lib.dog:1166-1177` |
| `derive_plural_list_type` | List has a singular typed list-spec but no container type | Creates and assigns the plural container `WordUse` | Marks word dereferenced | `ModelManager.completeItemsType()`, `ModelManager.dog:281-297` |
| `derive_order_mode` | Definition/function-argument type supplies ordered or unordered semantics | Sets the infon's `orderMode`, and may propagate it to function arguments | Subsequent merge dispatch and field matching depend on it | `infon.setOrderMode()` and `getOrderMode()`, `Proteus.Lib.dog:286-320` |
| `instantiate_list_member` | Open/tentative list traversal needs another member and size permits it | Creates a blank or list-spec-derived member and appends it | May be tentative and generate template constraints | `makePotentialMemberInfon()` and `tryAddingTentativeChild()`, `infonList.dog:318-333`, `464-489` |
| `instantiate_sparse_tail` | Typed list has more than one unmaterialized member with known/ranged capacity | Appends one sparse sub-list representing all remaining members | Stores count in `infSize`; later traversal splits/exposes it | `tryAddingListSpecSparseTail()`, `infonList.dog:443-462` |
| `create_field_from_definition` | Unordered merge cannot find a compatible LHS field and RHS field came from a definition | Appends a copied definition field and increments list size | Records definition provenance | `processUnorderedInfons()`, `Proteus.Lib.dog:2538-2547` |
| `make_list` | A new literal list representation is required | Clears target and initializes literal-list/open-size state | Existing target state is discarded | `infon.setToList()`, `Proteus.Lib.dog:603-623` |

### Scalar and sequence normalization

| Proposed opcode | Preconditions | Direct effect | Generated result | Current implementation |
| --- | --- | --- | --- | --- |
| `normalize_number` | Literal numeric value | Reduces the `FlexNum` representation | Same mathematical value | `rangeRules()`, `Proteus.Lib.dog:1102-1111` |
| `resolve_inverted_offset` | Literal numeric value is inverted and literal size is known | Replaces value with `size - value` and clears `inverted` | Concrete positive offset | `rangeRules()`, `Proteus.Lib.dog:1112-1118` |
| `normalize_size_expression` | `infSize` is a non-string concat expression | Wraps the size value in a temporary infon and schedules normal normalization on it | Parent waits for the generated obligation | `normSize()`, `Proteus.Lib.dog:1379-1399` |
| `recompute_size` | Literal string/list size is count-derived, or structure changed | Computes `infSize`; may close `tailUnfinished` when the size is verified | Generated whenever concatenation, flattening, or list closure changes structure | `infon.countSize()`, `Proteus.Lib.dog:248-282` |
| `intersect_numeric_ranges` | Both operands can be evaluated as ranges | Rewrites LHS to `max(low bounds)..min(high bounds)` | A narrower LHS range; empty-range detection is not implemented here | `mergeANDRanges()` and `resetNumRange()`, `Proteus.Lib.dog:1515-1540` |
| `check_numeric_range` | LHS/RHS are evaluable numeric literal/range forms | Accepts if RHS lies in LHS range; otherwise rejects | No state copy unless paired with `copy_value`/`copy_infon` | `checkNumRange()`, `Proteus.Lib.dog:1542-1556`; generated combinations in `WorldManager.dog` |
| `consume_string_prefix` | Loose string merge and prefixes are compatible | Copies/retains the matched LHS prefix and creates an RHS remainder POV when needed | `sizeToCopy` and optional remainder for continued propagation | `mergeLooseStrings()`, `Proteus.Lib.dog:1972-2020` |
| `concatenate` | Adjacent literal operands have the same value kind | Combines numbers, strings, or list members and removes the consumed RHS child | May update numeric encoding or string size | `joinOnRight()` and `JoinIfConcat()`, `Proteus.Lib.dog:1558-1594`, `1651-1670` |
| `collapse_singleton_concat` | A concat has one remaining child | Replaces concat value/size with its child representation | Sets `sizeMode=fromGiven` as appropriate | `collapseSingletonConcat()`, `Proteus.Lib.dog:1596-1608` |
| `tokenize_language_text` | Type is `english-text` with a literal string | Retypes as `english-phrase`, changes value kind to list, sets `word` list-spec, appends word infons | Original scalar string field is not explicitly cleared by current code | `splitLangText()`, `Proteus.Lib.dog:1121-1152` |

### Selection, intersections, and alternatives

| Proposed opcode | Preconditions | Direct effect | Identity rule | Current implementation |
| --- | --- | --- | --- | --- |
| `resolve_cursor` | Cursor mode and anchor are available | Selects world/context/user/args/this/previous/next, or marks undefined | Reuses a known object for world-without-path; previous/next are deliberately detached copies | `lookUpCursorRules()`, `Proteus.Lib.dog:1312-1377` |
| `attach_function_arguments` | `argPos` names a first/last item and that item exists | Moves `funcArgs` to the selected item's worklist as an internal-identity constraint, then clears the argument fields | Generates a merge obligation on the selected item | `funcArgRules()`, `Proteus.Lib.dog:1221-1241` |
| `compile_path_selector` | Direct field/first/last lookup did not resolve a path, or an index selector is used | Builds a skip/match/index intersection and transfers remaining path/work | This creates a query pattern, not an identity result | `partsRules()`, `Proteus.Lib.dog:1678-1839` |
| `select_path_result` | Direct lookup or a normalized synthetic selector has one result | Replaces the current POV's infon with the selected result and transfers reply/path/work metadata | If source POV information exists, preserves it with `aRefTo`/`sourcePOV` (including `carryPassiveScalarSource()`); must not infer by content | `partsRules()` and `selectReturnedInfonFromPOV()`, `Proteus.Lib.dog:1678-1839`, `2156-2174` |
| `make_intersection` | A function-like or alternative form is required | Converts target to `[inner]`, setting intersection mode and inner size policy | Old state may first be copied into the inner infon | `convertThisToIntersection()` and `reFunctionize()`, `Proteus.Lib.dog:465-473`, `666-670` |
| `branch_rhs_intersection` | RHS is an OR-like intersection | Clones LHS once per RHS alternative, adds each RHS alternative as work, and replaces LHS with the resulting intersection | Each branch remains tentative until validated | `mergeRHSIntersect()`, `Proteus.Lib.dog:2069-2102` |
| `add_alternative` | Propagation crosses an alternative line | Adds a unique alternative or constructs an OR work item | Tracks predecessor and line ID; generated branch must be normalized | `addToOrInfon()`, `pushAltIntoLineAlt()`, `Proteus.Lib.dog:475-513` |
| `select_unique_alternative` | A closed OR has exactly one non-rejected, resolved alternative | Rebinds the outer result to that alternative and validates predecessor state; source-backed closure may overwrite the outer infon in place | Selection is by branch provenance, not equal content | `closeIntersectAt()` and intersection step 1, `Proteus.Lib.dog:1842-1848`, `2971-3003`, `3066-3076` |
| `return_first_last_marked` | Function-like intersection is complete | Returns first, last, or marked result; marked ranges may create a source-backed list wrapper | Carries `outerPOV/accessMode` when the selected source is known | `Proteus.Lib.dog:3017-3062` |
| `mark_undefined` | Cursor/selection has no result or all closed alternatives fail | Sets `value.undefined`; zero-alternative case also empties inner list and clears paths | Terminal status should be `undefined`, distinct from `rejected` | `Proteus.Lib.dog:1334-1344`, `3004-3014`, `3063` |

### List and ordered-span structure

| Proposed opcode | Preconditions | Direct effect | Generated result | Current implementation |
| --- | --- | --- | --- | --- |
| `append_child`, `prepend_child`, `insert_child` | Parent list and insertion position exist | Adds a POV referring to an infon | Parent size is updated only by some callers; the executor must state size effects explicitly | `appendPov()/append()/prepend()/insertBefore()/insertAfter()`, `infonList.dog:705-799` |
| `remove_child` | Child POV exists in parent list | Unlinks the POV | Low-level primitive does not update size | `erase()/popFirst()/popLast()`, `infonList.dog:759-825` |
| `truncate_list` | A validated last item has been selected | Removes all later children and exploratory tentative tails | May recurse through predecessor structure | `closeItemsAtPOV()` and `trimTowardInfonFront()`, `infonList.dog:251-279`, `Proteus.Lib.dog:698-706` |
| `close_list` | Last-item validation succeeds | Clears tentative flags, optionally inserts child dots, closes tail, recounts size | Produces a concrete list prefix/normal form | `validateItem()`, `closeItemsAtPOV()`, and `insertChildDotsBefore()`, `Proteus.Lib.dog:1850-1892`, `infonList.dog:241-279` |
| `remove_null_intersection` | Closed function-like intersection contains zero items | Erases it from its parent list | Parent size repair is indirect | `cleanUpInfon()`, `Proteus.Lib.dog:1894-1910` |
| `flatten_singleton_sublist` | Untyped, closed, constraint-free sub-list has exactly one member and is not a marked return | Replaces wrapper POV with the sole child's infon and sub-item flag | Recounts list size | `unwrapSingletonSubItems()`, `Proteus.Lib.dog:1610-1649` |
| `split_sparse_span` | Sparse span count exceeds the requested prefix | Shrinks original to prefix and inserts a copied remainder span | Returns the remainder POV for continued propagation | `makeSparseSpanCopy()` / `splitSparseSpanPrefix()`, `infonList.dog:114-149`; ordered merge at `WorldManagerOrderedSpans.dog:432-482` |
| `expose_sparse_head` | Sparse span count is positive | Inserts one concrete/template-derived head before the span, then decrements or removes the span | Head may be type-only (`makeSparseSpanMemberTypeOnlyInfon()`) or a recursively constructed ordered-span composite | `infonList.dog:151-173`, `335-354`; `WorldManagerOrderedSpans.dog:26-71` |
| `make_span_view` | Requested unit can be converted to the source unit and enough source siblings exist | Builds an `aRefTo` list/unit view over source POVs, recursively if needed | Does not copy the underlying selected items; records source POVs; readiness flags are executor bookkeeping | `makeConcreteSiblingSpanView()`, `infonList.dog:198-239`; `makeDirectOrderedSpanSourceView()` / `makeOrderedSpanUnitView()` / `tryBuildOrderedSpanTraversalWindow()`, `WorldManagerOrderedSpans.dog:233-412` |
| `align_ordered_spans` | LHS/RHS sparse units have a valid common-base conversion | Compares base counts and splits the larger span to an exactly aligned prefix | Produces `sizeToCopy` and possibly an RHS remainder; equal spans need no structural mutation | `orderedSpanMergeRules()`, `WorldManagerOrderedSpans.dog:432-482` |
| `consume_ordered_span_skip` | Negative typed skip is immediately followed by a compatible positive request | Removes the consumed negative constraint from the positive request worklist | Then constructs the positive traversal window | `removeConsumedOrderedSpanSkipWork()`, `WorldManagerOrderedSpans.dog:73-95` |
| `map_marked_return_to_source` | Propagation matches a marked selector item to an RHS/source POV | Replaces the marked-return POV with the source-backed matched POV | May preserve dots for a tentative match | `propagate()`, `Proteus.Lib.dog:2320-2354` |

## Constraint and agenda operations

These are essential planner outputs, but they are obligations/control flow rather
than completed model changes.

| Proposed opcode/status | Effect | Current implementation |
| --- | --- | --- |
| `add_constraint` | Adds an RHS POV to an infon's worklist and marks the target non-normal | `addPOVToWrkList()` / `addInfonToWrkList()`, `Proteus.Lib.dog:531-555` |
| `dequeue_constraint` | Moves the next work item to `usedWrkList` | `wrkLstRules()`, `Proteus.Lib.dog:1401-1453` |
| `schedule_normalize` | Creates an `rsInfon` AItem, optionally subscribed to a parent | `processInfonViaAgenda()`, `Proteus.Lib.dog:960-988` |
| `schedule_merge` | Creates an `rsMerge` AItem for LHS/RHS and records dependency counts | `processMergeViaAgenda()`, `Proteus.Lib.dog:990-1018` |
| `schedule_intersection` | Creates an `rsIntersect` AItem | `processIntersectViaAgenda()`, `Proteus.Lib.dog:1020-1044` |
| `subscribe` / `fulfill` | Blocks a parent on generated work, then requeues it when the child finishes | `process*ViaAgenda()` and `fulfillSubscriptions()`, `Proteus.Lib.dog:960-1078` |
| `start_sequence_propagation` | A list/list or list/scalar merge needs element-wise work | Creates LHS/RHS iterators and changes merge status to unknown until element obligations finish | `startPropRules()` / `initListIterators()`, `WorldManager.dog:459-489`, `Proteus.Lib.dog:2041-2067` |
| `propagate_constraint` | Traverses LHS/RHS sequences, materializes traversal nodes as needed, and attaches RHS work to the next LHS node | `propagate()`, `Proteus.Lib.dog:2176-2465` |
| `branch_constraint` | For LHS/RHS alternatives, adds branch-specific work rather than merging all alternatives into one state | `propagate()`, `Proteus.Lib.dog:2390-2443` |
| `wait` / `blocked` | Leaves an obligation pending on parsing, nested normalization, or a subscription | AItem `pendingTasks`, streaming checks, and queued propagation records |
| `accept` | Merge constraints were compatible; may mark type initialized, concrete state, and proxy source | `processAgendaItem()` merge step 1/2, `Proteus.Lib.dog:2810-2899` |
| `reject` | Type/value/size/range constraints contradict; marks the relevant POV rejected | `WorldManager.mergeRules()`, compatibility checks, and merge finalization |
| `complete` | Cleans structure, marks processing state, emits any reply, and fulfills subscribers | `cleanUpInfon()` and `processAgendaItem()`, `Proteus.Lib.dog:2637-2695` |

## External effects produced by normalization

These should be returned separately from the infon operation path because they
change a knowledge base or consumer rather than the target infon alone.

| Proposed effect | Current behavior | Source |
| --- | --- | --- |
| `register_definition` | Stores a word definition/model and its locale/POS metadata | `wordDefRules()` and `ModelManager.indexDefinition()`, `Proteus.Lib.dog:1089-1100`, `ModelManager.dog:299-330` |
| `normalize_include_stream` | Parses/normalizes a named sub-stream into the target POV | `processAgendaItem()`, `Proteus.Lib.dog:2602-2627` |
| `record_context` | Moves a normalized top-level noun infon to the front of topic history and culls history | `AgentContext.addContextItem()`, `ModelManager.dog:120-143`; call at `Proteus.Lib.dog:2667-2669` |
| `emit_reply` | Stringifies the normalized infon and submits it using origin and character position | `processAgendaItem()`, `Proteus.Lib.dog:2648-2655`, `2670-2677` |
| `append_world` | Appends an infon to the world list | `Agent.appendToWorld()`, `Proteus.Lib.dog:3173-3176` |
| `error` | Reports undefined words, missing merge rules, or other fatal/incomplete cases | `derefInfon()`, `WorldManager.mergeRules()`, and `Agent.setErrorState()` |

## Bookkeeping that should not be planner opcodes

The current implementation also mutates `isNormed`, `wasProcessed`,
`wordDerefed`, `sizeNormed`, `alreadyTypeChecked`, `altRulesApplied`,
`hasPropagated`, iterator pointers, candidate lists, priorities, subscriber
counts, indexes, and debug pedigree. These are executor caches/lifecycle state.
They belong in operation invalidations or execution metadata, not in the
semantic plan.

Similarly, raw `PovList.append/erase`, struct assignment, and flag writes are
mechanical realizations. A plan should normally emit `instantiate_list_member`,
`select_unique_alternative`, or `close_list`, with the executor choosing those
mechanical mutations.

## Current gaps, not operations

The generated merge tree contains 60 `TODO: unfinished` branches and six
explicit “Do Nothing” branches on this snapshot. `sizesAreCompatable()` also
currently returns `true` unconditionally, and `mergeLooseNums()` is empty.
Those cases must be represented as `blocked`/`unsupported` (or supplied with a
new general operation rule); they must not silently become successful no-op
plans.

The main gap exposed by the catalog is that `copyIdentity()` combines three
different physical effects. The planner output should therefore carry both:

1. the logical authorization: `assert_identity(..., evidence_ref)`; and
2. the chosen realization: `rebind_identity`,
   `materialize_identity_into_slot`, or `merge_open_list_shape`.

This prevents copied content from being mistaken for proof that two infon
handles are identical.

## Sanity check: `_ = 5`

For a strict asserted identity whose LHS POV is an ordinary unknown slot and
whose RHS is the literal infon `5`, the constructive result is:

```text
1. schedule_normalize(rhs_5)
2. assert_identity(lhs_slot, rhs_5, evidence=the_written_assertion)
3. rebind_identity(lhs_slot, rhs_5)
4. complete(lhs_slot)
```

Postcondition: `lhs_slot.pItem` is the exact RHS infon object. If the LHS is a
source-backed slot, step 3 is instead `materialize_identity_into_slot`; the
result must explicitly state that storage identity was preserved and that a
deep copy, not a handle rebind, was performed.

## Coverage map

The inventory was checked against each mutation-producing phase called by
`processAgendaItem()`:

- word definition, numeric cleanup, text splitting, definition dereference;
- function arguments, intersections, cursor lookup, size normalization;
- worklists, path selection, concat joining, and cleanup;
- generated scalar/list merge actions and unordered field merge;
- propagation, alternatives, sparse lists, and ordered-span views;
- terminal completion, context, include-stream, and reply effects.

Primary dispatch is at `Proteus.Lib.dog:2580-3090`; the generated merge action
table is `WorldManager.dog:76-489`; ordered-span mutations are in
`WorldManagerOrderedSpans.dog`; list materialization and structural primitives
are in `infonList.dog`.

The implemented generator actions in `ruleMgr.py` were also checked: rejection,
RHS value/kind/size/type copying, identity copying, list propagation, loose
string consumption, RHS-intersection branching, range intersection/checking,
and accepted range overwrite all map to entries above. The defined
`copyValueLHStoRHS` snippet has no generated call site on this snapshot, so it
is not cataloged as an operation the current normalizer actually performs.
