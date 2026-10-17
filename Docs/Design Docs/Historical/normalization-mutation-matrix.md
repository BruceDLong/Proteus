# Normalization mutation matrix

Source snapshot: `main` at `d10cad662bc0a2a7985ddf08c265f5a62ef5a8f6`

## Purpose

This is the call-site inventory for state changes reachable from
`WorldManager.processAgendaItem()`.  It is deliberately lower-level than
`Docs/Design Docs/Historical/infon-operation-catalog.md`: the catalog describes possible planner outputs;
this matrix records what the current normalizer actually mutates and under
which source branch.

A row is one **mutation unit**: one guarded branch or helper invocation with a
single purpose.  Adjacent field assignments that implement that purpose stay
in one row.  Repeated calls to the same helper under different guards have
separate rows.  Inline writes are included.  Helper implementations are listed
again in the primitive section so that a call-site row never hides whether it
rebinds a handle, copies state, edits structure, or only changes bookkeeping.

The root is `Proteus.Lib.dog:2580-3092`.  The included lower call tree is:

```text
processAgendaItem
  rsInfon
    word/definition, scalar, cursor, worklist, path, concat, cleanup
  rsMerge
    generated WorldManager rules, unordered merge, propagation
    ordered-span helpers and list traversal/materialization
  rsIntersect
    alternative normalization/selection and function return
  agenda/lifecycle and directly invoked external effects
```

Parser construction not invoked by the include-stream branch, debug-only
graphs/logging, cache/index reads, and public entry points that are not below
`processAgendaItem()` are out of scope.  `Agent.appendToWorld()`, for example,
is an operation in the broader catalog but is not reachable from this root on
the snapshot.  Defined but uncalled helpers are also excluded.  The notable
ordered-span examples are `materializedOrderedSpanChildAt()`,
`makeOrderedSpanContainerView()`, and `resolveTypedFieldPathInfon()`.

## Conventions

Targets are not limited to the object currently stored in
`aItem.LHS_item.pItem`.  They include descendants, POV/list structure, a
selected source object reached through an `aRefTo` view, agenda state that
controls the LHS, and externally visible stores changed as a consequence of
normalizing it.

Identity column:

| Code | Meaning |
| --- | --- |
| `-` | No infon-identity effect. |
| `C` | Copies content/state.  This is not identity evidence. |
| `R` | Rebinds a POV to the exact existing infon object. |
| `S` | Preserves an identity obtained by explicit selection/source traversal. |
| `P` | Records provenance/proxy linkage after an accepted merge. |
| `A?` | Current code treats the merge/assertion as authorization, but does not carry an explicit assertion-evidence reference. |

Verification column:

| Code | Required oracle |
| --- | --- |
| `V1` | Exact handle/POV identity before and after. |
| `V2` | Structural/value delta plus unchanged fields. |
| `V3` | List topology, size, tail, tentative, and parent links. |
| `V4` | Generated obligation, dependency, and agenda trace. |
| `V5` | Rejection/undefined/terminal status and absence of unintended writes. |
| `V6` | External-effect spy or store/index inspection. |
| `V7` | Replay: applying the recorded operation produces the observed post-state. |

`preserves` and `invalidates` in the effect cells describe state that a future
operation contract must make explicit.  Lifecycle-only rows are not proposed
planner opcodes; they are executor responsibilities.

## A. `rsInfon` mutation units

| ID | Source and trigger | Reads / precondition | Writes / effect, including preservation | Identity | Creates, removes, or follows up | Candidate operation / status | Verify |
| --- | --- | --- | --- | --- | --- | --- | --- |
| I-001 | `Proteus.Lib.dog:2613-2626`, include stream | `itmMode=mIncludeStream`, propagation flags, `includeSpec` | Changes `itmMode` to `mItem`, sets `hasPropagated`, then `streamState=parseStarted` | `-` | May call propagation before parsing | `begin_include_stream` plus executor flags | `V2,V4` |
| I-002 | `Proteus.Lib.dog:2620-2622`, include not yet propagated | Current include POV and no RHS | `propagate()` may materialize/traverse descendants; marks POV propagated | `-` | Queues returned AItem | `propagate_constraint` | `V3,V4` |
| I-003 | `Proteus.Lib.dog:2626`; `3313-3329`; `3265-3271` | Valid `file:` stream and parse result | Replaces `toFill.pItem` with parsed top infon, then normalizes it | `R` to newly parsed object, not RHS identity | Creates parsed graph and nested normalization | `normalize_include_stream` | `V1,V4,V6` |
| I-004 | `wordDefRules`, `1089-1098` | Unregistered `mWordDefn` | `setOrderMode()` may set definition and argument order; sets `defnRegistered` | `-` | Calls definition indexing | `derive_order_mode`; registration flag is executor-only | `V2,V6` |
| I-005 | `setOrderMode`, `300-319` | Function args supply or resolve an order mode | Writes owner `orderMode` and `funcArgs.orderMode` | `-` | Definition lookup only | `derive_order_mode` | `V2` |
| I-006 | `rangeRules`, `1102-1109` | Literal number | Normalizes the `FlexNum` representation in place | `-` | None | `normalize_number` | `V2` |
| I-007 | `rangeRules`, `1111-1117` | Inverted literal with literal size | Writes `value.num=size-value`, clears `value.inverted` | `-` | None | `resolve_inverted_offset` | `V2` |
| I-008 | `splitLangText`, `1121-1150` | `english-text`, literal string | Retypes to `english-phrase`; changes value kind to list; creates `word` list spec; appends typed word infons | `-` | Creates one child per token; current scalar string storage is not explicitly cleared | `tokenize_language_text` | `V2,V3` |
| I-009 | `completeItemsType`, `ModelManager.dog:281-296` | Untyped literal list with singular typed list spec | Creates plural `WordUse`, assigns `item.type`, sets `wordDerefed` | `-` | May short-circuit later dereference | `derive_plural_list_type` | `V2` |
| I-010 | `derefInfon`, `1154-1174`, concrete target and concrete model | Definition selected; target already has concrete state | Marks model abstract; target concrete; adds model as priority work constraint and marks it `trueByDefinition` | `C` for later constraint copy | Creates merge obligation; invalidates `isNormed` through worklist insertion | `add_template_constraint` | `V2,V4` |
| I-011 | `derefInfon`, `1175-1192`, unknown target | Definition model selected | Deep-copies model into existing target; restores target type, negation, item mode, path state; sets definition provenance and type-example flags | `C` | Replaces most target state without rebinding storage | `copy_from_template` | `V1,V2,V7` |
| I-012 | `derefInfon`, `1193` | Definition exposes a type not already known | Appends definition type to `addedTypes` | `-` | Adds a type fact | `add_type_fact` | `V2` |
| I-013 | `derefInfon`, `1201`; `derefWordRules`, `1205-1219` | Dereference attempted | Sets `wordDerefed`; when LHS changed, inserts it in parent or world type index | `-` | Index mutation; undefined word invokes error path | executor cache plus `index_infon_type` | `V2,V6` |
| I-014 | `funcArgRules`, `1221-1237` | `argPos` selects first/last and target exists | Creates POV to `funcArgs`, marks RHS internal identity, adds it to selected item worklist; clears owner `funcArgs` and `argPos` | `A?` authorization is the function-argument relation | Creates merge obligation and invalidates selected item normalization | `attach_function_arguments` | `V2,V4` |
| I-015 | `func_altRules`, `1243-1255` | `altRulesApplied=false`; intersection present | Sets `altRulesApplied`; schedules intersection processing | `-` | Creates `rsIntersect` obligation | `schedule_intersection`; flag executor-only | `V4` |
| I-016 | `lookUpCursorRules`, `1327-1345` | `%prev` or `%next` lacks a neighbor | Sets `value.undefined` on current infon | `-` | Terminal undefined candidate | `mark_undefined` | `V5` |
| I-017 | `lookUpCursorRules`, `1354` | `%world` selected | May inherit open-list definition shape into the world target | `C` metadata only | See G-002 | `copy_list_shape` | `V2` |
| I-018 | `lookUpCursorRules`, `1355-1358` | `%world` with no remaining path | Rebinds LHS POV to `agent.world`; sets `accessMode=aRefTo` | `R,S` | Selected world identity becomes result | `select_cursor_source` | `V1` |
| I-019 | `lookUpCursorRules`, `1359-1362` | `%prev` or `%next` found | Deep-copies neighbor, then overwrites current storage | `C` | Deliberately detaches cursor result | `copy_selected_cursor_result` | `V1,V2` |
| I-020 | `lookUpCursorRules`, `1363-1366` | Context/user/args/this or world with path | Struct-assigns selected source into current infon storage | `C` shallow/state overwrite, not a rebind | Referenced subobjects may be shared | `overwrite_infon_state` | `V1,V2` |
| I-021 | `lookUpCursorRules`, `1367-1373` | Cursor selection succeeded | Restores path, reply, origin, item mode; sets `propagateInMrg2`; clears target worklist | `-` | Preserves selector-local metadata; removes old obligations | `preserve_slot_metadata` plus `clear_constraints` | `V2,V4` |
| I-022 | `normSize`, `1379-1397` | Size is non-string concat and not already handled | Sets `sizeNormed`; wraps size expression as temporary infon | `C` into temporary value handle | Creates subscribed `rsInfon` size obligation | `normalize_size_expression` | `V4` |
| I-023 | `wrkLstRules`, `1406-1413` | List-spec inner/outer feed present | Moves `funcArgs` into owner worklist with loose/outer/internal flags; clears owner feed fields; marks list spec `asWrkLstOutr` | `A?` internal asserted relation | Creates merge obligation | `attach_outer_constraint` | `V2,V4` |
| I-024 | `wrkLstRules`, `1421-1432` | Worklist nonempty | Removes first work POV, appends it to `usedWrkList`, marks LHS propagated | `-` | Creates subscribed `rsMerge` AItem unless `dontProcess` | `dequeue_constraint` plus `schedule_merge` | `V3,V4` |
| I-025 | `wrkLstRules`, `1438-1444` | LHS has never propagated | Marks propagated and schedules single-sided merge | `-` | Creates merge/propagation obligation | `start_sequence_propagation` | `V4` |
| I-026 | `partsRules`, `1682-1687` | `partRef` present | Appends `partRef` to `parts`, then clears `partRef` | `-` | Transfers path ownership | `queue_path_segment` | `V2` |
| I-027 | `partsRules`, `1694` | Untransferred path segment | Sets `PartPath.wasTransfered` | `-` | Prevents replay | executor-only path flag | `V2` |
| I-028 | `partsRules`, `1701-1708`, direct dot/first/last hit | Explicit field or endpoint selection resolves | Later result replacement uses the exact selected infon | `R,S` | No synthetic selector | `select_path_result` | `V1` |
| I-029 | `partsRules`, `1709-1749`, unresolved dot | Direct lookup failed | Builds skip and match patterns, list skip, intersection, metadata and source constraint; moves nested path | `C` into new patterns | Creates synthetic selector graph and work obligation; temporarily clears reply on old target | `compile_path_selector` | `V2,V3,V4` |
| I-030 | `partsRules`, `1755-1780`, numeric index without spec | Numeric index selector | Builds open inner intersection sized from index; attaches current infon as work | `C` into selector | Creates selector and merge obligation | `compile_index_selector` | `V2,V3,V4` |
| I-031 | `partsRules`, `1781-1812`, numeric index with spec | Numeric index plus explicit spec | Builds skip plus marked spec intersection; records marked return POV and path metadata | `C` into selector | Creates marked selector and merge obligation | `compile_marked_index_selector` | `V2,V3,V4` |
| I-032 | `partsRules`, `1818` | New path constructed | Moves all path work to new path and clears source worklist | `C` constraints may be shared POVs | Invalidates new target normalization | `move_constraints` | `V3,V4` |
| I-033 | `partsRules`, `1819-1834` | Path segment has/no semicolon | Without semicolon rebinds active LHS POV to path result; with semicolon retains a POV and schedules it separately | `R` to selected/new path object | Creates nested normalization only for semicolon case | `select_path_result` or `schedule_path_side_effect` | `V1,V4` |
| I-034 | `joinOnRight`, `1558-1573` | Adjacent literal numbers, same kind | Adds contextual number or performs positional multiply/add; may update size | `-` | Consuming caller removes RHS child | `concatenate_number` | `V2` |
| I-035 | `joinOnRight`, `1574-1577` | Adjacent literal strings | Appends RHS string and recomputes LHS size | `-` | Consuming caller removes RHS child | `concatenate_string` | `V2` |
| I-036 | `joinOnRight`, `1579-1587` | Adjacent literal lists | Appends POVs referring to every RHS child into LHS | `S` for shared child objects | Creates new POVs; no deep child copy | `concatenate_list` | `V1,V3` |
| I-037 | `processConcat` / `JoinIfConcat`, `1651-1676` | Previous concat operand accepts RHS | Erases consumed POV; then collapses singleton concat if possible; records the AItem phase as processed | depends on collapse | Removes child; wrapper may be overwritten | `remove_child` plus `collapse_singleton_concat`; phase flag executor-only | `V2,V3` |
| I-038 | `collapseSingletonConcat`, `1596-1607` | Concat has one child | Copies only child value when size is known, otherwise struct-assigns the whole child into wrapper | `C` | Preserves wrapper storage; representation changes from concat | `collapse_singleton_concat` | `V1,V2` |
| I-039 | `cleanUpInfon`, `1898-1908` | Trailing exploratory tentative or null function intersection | Pops trailing tentative; may erase null intersection from parent | `-` | Removes POVs | `remove_exploratory_tail`; `remove_null_intersection` | `V3` |
| I-040 | `cleanUpInfon`, `1911-1953`; `validateItem`, `1850-1892` | List has candidate/implicit last item | Removes candidate record, clears tentative flags, closes alternatives/lists, trims exploratory tails, recounts size | `C` when closing an outer alternative in place | Removes suffixes; may insert child dots; invalidates structural caches | `close_list` / `select_unique_alternative` | `V1,V2,V3` |
| I-041 | `unwrapSingletonSubItems`, `1633-1648` | Ordinary untyped closed singleton sublist | Rebinds wrapper POV to sole child and copies `isSubItm`; recounts parent size | `R,S` to explicitly contained child | Removes semantic wrapper without unlinking POV | `flatten_singleton_sublist` | `V1,V3` |
| I-042 | `reFunctionize`, `666-670`; completion `2660-2665` | `cnvtToFunc` set | Struct-copies old infon into new inner, converts original storage to intersection, clears/reopens alt processing | `C` | Creates inner infon; re-runs normalization | `make_intersection` | `V1,V2,V4` |
| I-043 | completion `2648-2677` | No ready pending tasks | Clears `doReply`, sets `wasProcessed`; optionally records context and submits string result | `-` | External effects E-003/E-004 | `complete` plus external effects | `V2,V6` |
| I-044 | `constructAltModelFromWord`, `ModelManager.dog:354-509`, called by `derefInfon` | LHS/RHS `WordUse` needs definition resolution | Advances `WordUse.nextStage`; may split model suffix from `asGiven`, cache one selected `defToUse`, or build an intersection of copied definition alternatives; plural forms construct a plural list model | `C` | Creates definition/template candidates; mutates word-resolution cache fields | `resolve_word_definition` / `branch_definition_candidates` plus executor cache | `V2,V3` |

`localNullableContinuationRules()` at `Proteus.Lib.dog:1468-1472` is a
read-only dispatch gate.  When it matches, all mutation is performed by
`partsRules()` and is therefore covered by I-026 through I-033.

## B. `rsMerge` core mutation units

These rows describe mutations in the `rsMerge` dispatch outside the generated
decision tree.  Section C lists each implemented generated branch.

| ID | Source and trigger | Reads / precondition | Writes / effect, including preservation | Identity | Creates, removes, or follows up | Candidate operation / status | Verify |
| --- | --- | --- | --- | --- | --- | --- | --- |
| M-001 | `2706-2752`, merge step 0 | RHS presence and whether already usable | Initializes merge status; may dereference RHS word; schedules RHS normalization; advances step | `-` | Creates subscribed RHS `rsInfon` | `schedule_normalize` plus executor state | `V4` |
| M-002 | `2708-2728`, no RHS/skipped RHS | Single-sided merge | Initializes iterators when needed, advances to step 2, propagates without RHS | `-` | Traversal/agenda effects | `start_sequence_propagation` | `V4` |
| M-003 | `2760-2785`, compatibility check | Infon type/item compatibility and question mode | Sets `mergeStatus=reject` on definite or disallowed unknown incompatibility | `-` | Prevents generated merge writes | `reject` | `V5` |
| M-004 | `2788` | Non-rejected, non-unordered merge | Calls `mergeSizeRules`, which may copy list shape and size | `C` | See G-001/G-003 | `copy_size` / `copy_list_shape` | `V2` |
| M-005 | `2790-2792`, `RHS.forceID` | Force flag | Deep-copies RHS state into existing LHS storage | `C`, despite the flag name | Skips generated merge rules | `copy_infon`; semantic authorization needs definition | `V1,V2,V7` |
| M-006 | `processUnorderedInfons`, `2520-2536`, field exists | Both sides unordered; compatible field found | Adds RHS field infon to matched LHS field worklist, marks RHS field `notParsing`, and normalizes immediately if field already propagated | `A?` field assertion relation | Creates merge/normalize obligation | `add_constraint` | `V2,V4` |
| M-007 | `processUnorderedInfons`, `2540-2547`, field absent but from definition | Definition-provenance RHS field | Struct-copies RHS field into new storage, records definition provenance, appends field, increments LHS size | `C` | Creates child field | `create_field_from_definition` | `V1,V2,V3` |
| M-008 | `processUnorderedInfons`, `2563-2571` | Ordered/unordered mismatch with both sides resolved | Sets merge reject | `-` | None | `reject` | `V5` |
| M-009 | `2810-2818`, RHS unknown | RHS has type; LHS lacks type | Copies RHS word type; list case initializes iterators and leaves merge unknown | `C` for type handle | May start propagation | `copy_word_type` | `V2,V4` |
| M-010 | `2820` | Ordered, concrete generated merge case | Calls `mergeRules()` | varies | See section C | dispatch only | corresponding row |
| M-011 | `2825-2830` | LHS negation/inverted acceptance | Swaps accept/reject; writes LHS POV `rejected` | `-` | Influences branch selection/validation | `accept` / `reject` | `V5` |
| M-012 | `2840-2848` | Accepted type-example RHS | Sets matching `addedTypes[].typeInitialized` | `-` | Avoids repeat type initialization | `add_type_fact`; flag executor-only | `V2` |
| M-013 | `2850-2853` | Accepted non-loose-size merge with concrete RHS | Sets LHS concrete and `proxy=RHS` | `P,A?` | `dpItem()` subsequently resolves through RHS | `record_proxy_source` with assertion evidence | `V1,V2` |
| M-014 | `2854-2858` | Accepted terminal dot-created tentative | Clears LHS tentative | `-` | Allows list validation | `validate_tentative_member` | `V2` |
| M-015 | `2860-2861` | Accepted merge | Advances step and propagates LHS/RHS/remainder | `-` | Creates downstream constraints/AItems | `propagate_constraint` | `V4` |
| M-016 | `2862-2882` | Rejected merge | May accept negated parent, propagate nullable/last candidates, or report error | `-` | Branch/control and external error effects | `reject` plus conditional propagation | `V4,V5,V6` |
| M-017 | `2885-2903`, merge step 2 | Propagation finished | Resolves unknown merge status; recounts/verifies open literal size; resets iterators and may propagate again | `-` | Completes subscriptions | `recompute_size` plus executor completion | `V2,V4,V5` |
| M-018 | `getOrderMode`, `286-298`, called at `2503`, `2506`, `2788` | Infon order unknown and typed definition exists | Lazily writes `orderMode` from definition meaning on LHS or RHS | `-` | Changes unordered dispatch and size/propagation path | `derive_order_mode` | `V2` |

## C. Generated `WorldManager` merge call sites

The guard descriptions below use `strict` for `!RHS.looseType()` and `loose`
for `RHS.looseType()`.  Each implemented source call site is represented even
when several call the same helper.

| ID | Source and guard | Direct current action | Identity | Candidate operation / status | Verify |
| --- | --- | --- | --- | --- | --- |
| G-001 | `WorldManager.dog:2-28`, strict definition list shape | Copies missing list spec, order, size state, and open-tail state into LHS | `C` | `copy_list_shape` | `V2,V3` |
| G-002 | `39-74`, cursor/world target has an open-list definition | Copies list spec, order, size, size mode, and tail state from definition meaning | `C` | `copy_list_shape` | `V2,V3` |
| G-003 | `76-85`, strict LHS size unknown and RHS size literal | Runs G-001, then `DO_COPY` on `infSize` if guards permit | `C` | `copy_size` | `V2` |
| G-004 | `91-93` | Runs `orderedSpanMergeRules` before generated scalar/list table | varies | ordered-span dispatch; see S-011..S-014 | corresponding row |
| G-005 | `94-97`, RHS infon mode unknown | Copies RHS word type into LHS if present | `C` | `copy_word_type` | `V2` |
| G-006 | `98-101`, LHS infon unknown, strict scalar/list RHS | Calls `copyIdentity` | `R` or `C`, `A?` | `rebind_identity`, `materialize_identity_into_slot`, or `merge_open_list_shape` | `V1,V2,V7` |
| G-007 | `102-106`, LHS unknown, loose scalar/list RHS | Copies RHS value kind and infon mode, then value | `C` | `copy_value_kind`; `copy_value` | `V2` |
| G-008 | `107-110`, unknown LHS, strict RHS intersection | Branches LHS across RHS alternatives | `C` | `branch_rhs_intersection` | `V2,V3,V4` |
| G-009 | `111-114`, unknown LHS, loose RHS intersection | Same helper as G-008 under loose guard | `C` | `branch_rhs_intersection` | `V2,V3,V4` |
| G-010 | `115-118`, numeric LHS versus strict string/list | Rejects and marks LHS POV rejected | `-` | `reject` | `V5` |
| G-011 | `119-122`, string LHS versus strict number/list | Rejects and marks LHS POV rejected | `-` | `reject` | `V5` |
| G-012 | `123-126`, list LHS versus strict number/string | Rejects and marks LHS POV rejected | `-` | `reject` | `V5` |
| G-013 | `127-130`, strict number unknown/unknown | If LHS is `aRefTo`, calls `copyIdentity`; otherwise copies RHS worklist only when nonempty | `R` or `C`, `A?` | identity realization or `copy_constraints` | `V1,V4` |
| G-014 | `131-134`, strict number unknown/literal | Copies RHS numeric value | `C` | `copy_value` | `V2` |
| G-015 | `138-141`, strict number literal/literal | Rejects unequal numeric content | `-` | `check_value` / `reject` | `V5` |
| G-016 | `142-145`, strict string unknown/unknown | `aRefTo` identity realization or RHS worklist copy | `R` or `C`, `A?` | identity realization or `copy_constraints` | `V1,V4` |
| G-017 | `146-149`, strict string unknown/literal | Copies RHS string value | `C` | `copy_value` | `V2` |
| G-018 | `153-156`, strict string literal/literal | Rejects unequal string content | `-` | `check_value` / `reject` | `V5` |
| G-019 | `166-170`, strict literal list/list | Definition-shape-only is accepted without element propagation; otherwise source-backed LHS copies identity, ordinary LHS starts propagation | varies | `merge_open_list_shape`, identity realization, or `start_sequence_propagation` | `V1,V3,V4` |
| G-020 | `177-180`, loose list versus number/string | Starts list/scalar propagation | `-` | `start_sequence_propagation` | `V4` |
| G-021 | `184-192`, loose number unknown/literal | Range-checks, then copies value and negation; invalidates outer alternative rules | `C` | `check_numeric_range`; `copy_value`; `copy_negation` | `V2,V5` |
| G-022 | `203-206`, loose string unknown/literal | Calls `mergeLooseStrings` | `C` | `consume_string_prefix` | `V2,V4` |
| G-023 | `210-213`, loose string literal/literal | Calls `mergeLooseStrings` | `C` | `consume_string_prefix` | `V2,V4,V5` |
| G-024 | `223-226`, loose literal list/list | Starts propagation unless RHS is only definition shape | `-` | `start_sequence_propagation` or `merge_open_list_shape` | `V3,V4` |
| G-025 | `227-230`, concat LHS and any compatible RHS intersection | Branches LHS across RHS alternatives | `C` | `branch_rhs_intersection` | `V2,V3,V4` |
| G-026 | `231-234`, strict number unknown and numeric/unknown RHS intersection | Branches RHS intersection | `C` | `branch_rhs_intersection` | `V2,V3,V4` |
| G-027 | `235-238`, strict number literal and numeric/unknown RHS intersection | Branches RHS intersection | `C` | `branch_rhs_intersection` | `V2,V3,V4` |
| G-028 | `239-242`, strict string unknown and string/unknown RHS intersection | Branches RHS intersection | `C` | `branch_rhs_intersection` | `V2,V3,V4` |
| G-029 | `243-246`, strict string literal and string/unknown RHS intersection | Branches RHS intersection | `C` | `branch_rhs_intersection` | `V2,V3,V4` |
| G-030 | `247-250`, strict list unknown and list/unknown RHS intersection | Branches RHS intersection | `C` | `branch_rhs_intersection` | `V2,V3,V4` |
| G-031 | `251-254`, strict list literal and list/unknown RHS intersection | Branches RHS intersection | `C` | `branch_rhs_intersection` | `V2,V3,V4` |
| G-032 | `264-269`, strict concat-range LHS, literal numeric RHS | Range-checks; when accepted struct-overwrites LHS storage with RHS and reopens outer alternatives | `C`, not identity | `check_numeric_range`; `overwrite_infon_state` | `V1,V2,V5` |
| G-033 | `270-275`, strict literal numeric LHS, concat-range RHS | Same accepted overwrite as G-032 | `C`, not identity | `check_numeric_range`; `overwrite_infon_state` | `V1,V2,V5` |
| G-034 | `321-324`, strict list concat/list concat | Intersects numeric range representation by mutating LHS bound children | `-` | `intersect_numeric_ranges` | `V2` |
| G-035 | `343-346`, loose string unknown and RHS intersection | Branches RHS intersection | `C` | `branch_rhs_intersection` | `V2,V3,V4` |
| G-036 | `347-350`, loose string literal and RHS intersection | Branches RHS intersection | `C` | `branch_rhs_intersection` | `V2,V3,V4` |
| G-037 | `351-354`, loose list unknown and RHS intersection | Branches RHS intersection | `C` | `branch_rhs_intersection` | `V2,V3,V4` |
| G-038 | `355-358`, loose list literal and RHS intersection | Branches RHS intersection | `C` | `branch_rhs_intersection` | `V2,V3,V4` |
| G-039 | `362-367`, loose numeric unknown LHS, list concat range RHS | Range-checks and accepted struct-overwrites LHS; reopens outer alternatives | `C` | `check_numeric_range`; `overwrite_infon_state` | `V1,V2,V5` |
| G-040 | `377-382`, loose concat-range LHS, literal numeric RHS | Range-checks and accepted struct-overwrites LHS; reopens outer alternatives | `C` | `check_numeric_range`; `overwrite_infon_state` | `V1,V2,V5` |
| G-041 | `383-386`, loose literal number versus concat range | Range-checks only; rejects on failure | `-` | `check_numeric_range` / `reject` | `V5` |
| G-042 | `startPropRules`, `459-487` | List propagation guards, size compatibility, loose mode | Rejects, skips, or creates iterators and sets merge unknown | `-` | Starts traversal; `sizesAreCompatable()` is currently unconditional true | `start_sequence_propagation`; unsupported size proof gap | `V4,V5` |

### Generated branches with no implemented mutation

These are not successful planner operations.  They must become an explicit
`unsupported`/`blocked` result unless a general rule is supplied.

| Kind | Count | Exact `WorldManager.dog` lines |
| --- | ---: | --- |
| Explicit `Do Nothing` | 6 | `136, 151, 182, 195, 201, 208` |
| `TODO: unfinished` | 60 | `158, 161, 164, 172, 175, 198, 215, 218, 221, 256, 259, 262, 277, 280, 283, 286, 289, 292, 295, 298, 301, 304, 307, 310, 313, 316, 319, 326, 329, 332, 335, 338, 341, 360, 369, 372, 375, 388, 391, 394, 397, 400, 403, 406, 409, 412, 415, 418, 421, 424, 427, 430, 433, 436, 439, 442, 445, 448, 451, 454` |
| Missing final rule | 1 branch | `456`: logs and exits rather than returning a normal-form plan |

## D. Merge helper implementations

| ID | Source | Reads / precondition | Writes / effect | Identity | Creates/removes/follow-up | Candidate operation | Verify |
| --- | --- | --- | --- | --- | --- | --- | --- |
| H-001 | `copyIdentity`, `2109-2115`; `mergeOpenListShapeIntoConcreteList`, `2141-2147` | RHS is open shape; LHS has concrete list members | Copies only missing type/list spec/order and requested item mode | `C`; no rebind | Preserves concrete LHS members | `merge_open_list_shape` | `V1,V2,V3` |
| H-002 | `copyIdentity`, `2116-2119`, source-backed target | LHS POV is `aRefTo` back to existing source slot | Deep-copies RHS into the existing source object; restores item mode and clears tentative when authorized | `C,A?`; preserves target storage identity | Mutates the selected source object | `materialize_identity_into_slot` | `V1,V2,V7` |
| H-003 | `copyIdentity`, `2120-2123`, ordinary target | Strict merge reaches identity copy | Rebinds `LHS_item.pItem` to exact `RHS.pItem`; restores item mode | `R,A?` | Old target object and its pending work become unreachable through that POV; work is not carried here | `rebind_identity` | `V1,V4,V7` |
| H-004 | `preserveLHSSlotMetadata`, `2125`; `2149-2154` | Identity copy completed | Restores old LHS type and request list spec unless RHS has a concrete list spec | `-` | Preserves selector-local constraints | `preserve_slot_metadata` | `V2` |
| H-005 | `mergeRHSIntersect`, `2069-2102` | RHS is OR-like intersection | Unconditionally closes the RHS inner tail in current code; deep-copies LHS per RHS alternative, marks copies tentative, adds branch constraint, records outer/predecessors, converts LHS to intersection | `C` | Mutates RHS shape, creates alternative graph and obligations, and clears LHS `altRulesApplied` | `branch_rhs_intersection` | `V2,V3,V4` |
| H-006 | `mergeLooseStrings`, `1972-2018` | Loose compatible strings | Copies whole/prefix into LHS or rejects mismatch; records `sizeToCopy` | `C` | May create RHS remainder POV/infon | `consume_string_prefix` | `V2,V4,V5` |
| H-007 | `DO_COPY`, `2023-2037` | Same pure value kind | Copies format, size mode, and number/string/list state; list assignment shares POV-list head/tail | `C`; list children may be shared | No identity proof; may copy string prefix only | `copy_value` / `copy_size` | `V1,V2,V3` |
| H-008 | `resetNumRange`, `1515-1525`; `mergeANDRanges`, `1527-1540` | Two recognized concat-range forms | Writes LHS low-bound value and span size to intersection | `-` | Does not implement empty-range rejection | `intersect_numeric_ranges` | `V2,V5` |
| H-009 | `carryPassiveScalarSource`, `2156-2161` | Scalar RHS selected during propagation | Stores source POV on LHS POV without changing LHS item handle | `S` | Source is promoted only if result is later selected | `record_selected_source` | `V1` |
| H-010 | `selectReturnedInfonFromPOV`, `2162-2172` | Function returns a selected POV | Sets LHS `outerPOV`/`accessMode=aRefTo` when source can be proven by POV linkage | `S` | Returns selected infon for later rebind | `select_path_result` | `V1` |

## E. `rsIntersect` mutation units

| ID | Source and trigger | Writes / effect | Identity | Creates/removes/follow-up | Candidate operation | Verify |
| --- | --- | --- | --- | --- | --- | --- |
| X-001 | `2917-2922`, inner/outer feed | Adds inner `funcArgs` as internal work, marks loose top, clears feed fields | `A?` | Creates inner merge obligation | `attach_outer_constraint` | `V2,V4` |
| X-002 | `2923-2925`, marked return present | Copies outer marked-return POV handle to inner marker | `S` | Retains explicit selection location | `map_marked_return` | `V1` |
| X-003 | `2933-2945`, closed OR | Assigns missing `outerPOV`, copies outer work into each branch, sets `aRefTo` and question mode | `S` | Schedules each alternative normalization | `normalize_alternatives` | `V1,V4` |
| X-004 | `2947`, after OR fan-out | Clears original outer worklist | `-` | Removes duplicated outer obligations | `clear_constraints` | `V4` |
| X-005 | `2950-2959`, function-like intersection | Retains POV to inner and schedules its normalization | `S` within retained POV | Creates subscribed normalization | `normalize_intersection_inner` | `V1,V4` |
| X-006 | `2988-2995`, exactly one valid OR branch | Selects branch object, marks parent `dontProcess`, sets last/outer and predecessor metadata | `R,S` at final rebind | Prepares unique alternative result | `select_unique_alternative` | `V1,V2` |
| X-007 | `2996-3003`, selected branch tentative | Rewrites tentative state from outer and validates sole predecessor | `-` | May close source list/intersection | `validate_tentative_member` | `V2,V3` |
| X-008 | `3004-3013`, no valid OR branch | Clears inner list, makes size literal zero, marks result undefined, clears paths | `-` | Removes alternatives and path obligations | `mark_undefined` | `V3,V5` |
| X-009 | `3018-3021`, get-last/get-first | Selects endpoint infon; last uses source-aware helper | `S` when POV source is known | Returns result candidate | `return_last` / `return_first` | `V1` |
| X-010 | `3022-3039`, tentative marked slice | Creates literal list wrapper whose first POV is `aRefTo` the marked source; marks dots and counts size | `S` | Creates source-backed wrapper | `return_marked_source_view` | `V1,V3` |
| X-011 | `3040-3045`, direct marked return | Chooses sparse-exposed or original marked object by recorded POV | `S` | Returns existing selected object | `return_marked` | `V1` |
| X-012 | `3048-3060`, result work handling | Clears selector-local work or moves outer work onto selected result and clears source | `C` for constraints | Invalidates result normalization | `move_constraints` / `clear_constraints` | `V4` |
| X-013 | `3063`, no function result | Sets `value.undefined` | `-` | Terminal undefined | `mark_undefined` | `V5` |
| X-014 | `3066-3075`, result installation | Copies item/reply/path/origin metadata into selected result, propagates undefined, then rebinds LHS POV to it | `R,S` | Current code mutates selected source metadata, including origin | `preserve_slot_metadata`; `select_intersection_result` | `V1,V2,V7` |

## F. Propagation, traversal, and structural materialization

### Propagation call sites

| ID | Source and trigger | Writes / effect | Identity | Creates/removes/follow-up | Candidate operation | Verify |
| --- | --- | --- | --- | --- | --- | --- |
| P-001 | `findNextRHS`, `2190-2197`, scalar RHS repeated over list | Copies RHS POV state to a fresh POV and marks loose inherited | `S` to same `pItem` | Creates traversal view | `make_repeated_source_view` | `V1` |
| P-002 | `propagate`, `2264-2266`, accepted concrete match | Clears LHS tentative | `-` | Enables validation | `validate_tentative_member` | `V2` |
| P-003 | `propagate`, `2273`; traversal helpers below | Fetching next LHS may append tentative/sparse children and seed type specs | varies | Materializes target structure | `advance_or_materialize_lhs` | `V3,V4` |
| P-004 | `propagate`, `2287-2311` | Builds/alters ordered-span traversal windows and exposes sparse heads on either side | `S` for views | May split/insert span nodes | ordered-span operations S-001..S-014 | corresponding row |
| P-005 | `propagate`, `2278-2283` | Rejects negated parent match and marks next LHS POV rejected | `-` | Stops enqueue for this path | `reject` | `V5` |
| P-006 | `propagate`, `2314-2345`, marked selector match | Replaces `intersectMarkedRetPOV` with matched RHS/source POV; sets dots/keep-dots flags for tentative source | `S` | Preserves exact source selection | `map_marked_return_to_source` | `V1,V3` |
| P-007 | `propagate`, `2360-2384`, sequence endpoint | Adds candidate-for-last record or sets `keepDots` | `-` | Drives later list closure | `record_list_endpoint_candidate` | `V3` |
| P-008 | `propagate`, `2400-2404`, tentative/tentative pair | Marks LHS alternative to add sub-dots and parent to preserve dots | `-` | Defers structural closure | `preserve_tentative_tail` | `V3` |
| P-009 | `propagate`, `2406-2414`, cross-product/alternative line | Propagates loose flags and calls `pushAltIntoLineAlt` | `S` in alternative POVs | Creates or extends OR work item | `branch_constraint` | `V1,V3,V4` |
| P-010 | `propagate`, `2416-2419` | Adds predecessor link to LHS POV | `S` provenance of traversal path | Used for validation/closure | `record_predecessor` | `V1,V3` |
| P-011 | `propagate`, `2422-2428`, ordinary match | Propagates loose flags, records scalar source, adds RHS POV to LHS worklist | `S,A?` | Creates merge obligation and invalidates LHS normalization | `propagate_constraint` | `V1,V4` |
| P-012 | `propagate`, `2441-2457` | Schedules next LHS normalization; records candidate/debug metadata; clears transient alt links | `-` | Creates `rsInfon` AItem | `schedule_normalize` plus executor cleanup | `V4` |

### Traversal and list materialization helpers

| ID | Source | Writes / effect | Identity | Creates/removes/follow-up | Candidate operation | Verify |
| --- | --- | --- | --- | --- | --- | --- |
| L-001 | `seedTypeSpecs`, `infonList.dog:281-305`, blank member | Deep-copies first applicable list-spec template into new member; records list-spec provenance | `C` | Materializes member state | `copy_from_template` | `V1,V2` |
| L-002 | `seedTypeSpecs`, `306-313`, nonblank/additional specs | Deep-copies template into constraint and adds it to member worklist | `C` | Creates merge obligation | `add_template_constraint` | `V4` |
| L-003 | `makePotentialMemberInfon`, `318-333` | Creates blank/template-derived member; sets item and question modes | `C` if template used | New infon not yet linked | `instantiate_list_member` | `V2` |
| L-004 | `makeSparseSpanMemberTypeOnlyInfon`, `335-354` | Creates unknown type-only member with list-spec provenance and ready flags | `C` for type/provenance | New infon not yet linked | `instantiate_sparse_head_type_only` | `V2` |
| L-005 | `tryAddingListSpecSparseTail`, `443-462` | Creates sparse list tail carrying remaining count, list spec, order and open-tail shape; appends to parent | `C` metadata | Creates child POV/infon | `instantiate_sparse_tail` | `V2,V3` |
| L-006 | `tryAddingTentativeChild`, `464-488` | Closes full list, or appends potential child and marks it tentative/from-dots when size unknown | `-` | Creates child or closes tail | `instantiate_list_member` / `close_list` | `V3` |
| L-007 | `getFirst`, `491-524` | May call L-006, seed existing child types, and assign child `pParent` | `S` parent linkage | May block on stream | `advance_or_materialize_lhs` | `V3,V4` |
| L-008 | `getNext`, `550-573` | May call L-006, mark parent `forLastUseLastItem`, seed types, and repair `pParent` | `S` parent linkage | Advances traversal | `advance_or_materialize_lhs` | `V3,V4` |
| L-009 | `getFirstTight/getNextTight`, `575-642` | Normalizes nested transparent items, records marked outer provenance, sets alternative traversal link | `S` | May invoke nested normalization and discover alternative | `advance_tight_traversal` | `V1,V4` |
| L-010 | `splitSparseSpanPrefix`, `136-149` | Shrinks original span count, inserts copied remainder after it, assigns parent/subitem metadata | `C` for remainder | Creates remainder POV/infon | `split_sparse_span` | `V2,V3` |
| L-011 | `exposeSparseHeadWithItem`, `151-167` | Inserts materialized head before sparse span; decrements or erases original span | `C` for created head state | Creates head; may remove exhausted span | `expose_sparse_head` | `V2,V3` |
| L-012 | `makeConcreteSiblingSpanView`, `198-239` | Creates list view and `aRefTo` child POVs over exact siblings | `S` | Creates view only; underlying children are not copied | `make_span_view` | `V1,V3` |
| L-013 | `insertChildDotsBefore`, `241-250` | Inserts open sublist dots before position and closes parent tail | `-` | Creates child dots node | `insert_child_dots` | `V3` |
| L-014 | `closeItemsAtPOV`, `251-279` | Truncates after selected last item, removes exploratory tails, clears tentative flags, inserts dots when requested, closes/recounts list | `-` | Removes suffix nodes; may create dots | `close_list` | `V3` |
| L-015 | `noteMarkedOuterFrom`, `65-73`; `pushPredecessor`, `650-657`; `addCandidateForLastItem`, `83-88` | Records selection/traversal provenance and endpoint candidates | `S` for selection/predecessor | No modeled value change | executor trace needed by semantic selection/closure | `V1,V3` |

### Ordered-span helpers

| ID | Source | Writes / effect | Identity | Creates/removes/follow-up | Candidate operation | Verify |
| --- | --- | --- | --- | --- | --- | --- |
| S-001 | `makeSparseOrderedSpanCompositeHead`, `WorldManagerOrderedSpans.dog:37-71` | Builds typed container plus sparse child from definition shape and records definition provenance | `C` | Creates composite head graph | `instantiate_ordered_span_head` | `V2,V3` |
| S-002 | `exposeSparseOrderedSpanHeadForTraversal`, `26-35` | Chooses composite or ordinary sparse exposure | varies | Delegates to L-011 | `expose_sparse_head` | `V2,V3` |
| S-003 | `removeConsumedOrderedSpanSkipWork`, `82-96` | Replaces request worklist with all constraints except matching negative skip | `-` | Removes consumed constraint | `consume_ordered_span_skip` | `V4` |
| S-005 | `makeDirectOrderedSpanSourceView`, `233-243` | Creates `aRefTo` POV to exact source item, may set source item mode, marks source/view graph ready | `S` | Creates view; mutates readiness flags on source object | `make_span_view` plus executor flags | `V1,V2` |
| S-006 | `makeOrderedSpanUnitView`, `245-312` | Either creates flat sibling view or recursively builds typed composite view with source-backed children | `S` | Creates view graph | `make_span_view` | `V1,V3` |
| S-007 | `firstOrderedSpanChildForRequest`, `314-346` | Repairs parent links, may expose sparse head, then selects/builds compatible unit view | `S` | May materialize head/view | `select_ordered_span_child` | `V1,V3` |
| S-008 | `tryBuildOrderedSpanTraversalWindow`, `348-381` | Removes negative skip when paired; selects contained source; sets request `accessMode=aRefTo` | `S` | Returns traversal window | `consume_ordered_span_skip`; `make_span_view` | `V1,V4` |
| S-009 | `tryBuildOrderedSpanTraversalWindow`, `383-411` | Selects direct unit or creates prefix view; may set inferred type; links suffix through `next`; marks request `aRefTo` | `S` | Creates prefix and traversal continuation | `make_span_view` | `V1,V3` |
| S-010 | `markOrderedSpanViewReady`, `168-182` | Recursively sets word/type/normal/processed/size flags | `-` | Skips later executor phases | executor-only readiness | `V2` |
| S-011 | `orderedSpanMergeRules`, `435-447`, compatible spans equal/RHS longer | Records copy count; splits RHS when longer and returns remainder | `C` for split remainder | May create RHS remainder | `align_ordered_spans` | `V2,V3` |
| S-012 | `orderedSpanMergeRules`, `449-453`, compatible LHS longer | Splits LHS to RHS count and records copy count | `C` for new LHS remainder | Mutates target list topology | `align_ordered_spans` | `V2,V3` |
| S-013 | `orderedSpanMergeRules`, `456-477`, convertible spans equal/RHS longer | Compares base counts; may split RHS prefix and return remainder | `C` | May create RHS remainder | `align_ordered_spans` | `V2,V3` |
| S-014 | `orderedSpanMergeRules`, `479-485`, convertible LHS longer | Splits LHS at converted prefix and records size | `C` | Mutates target list topology | `align_ordered_spans` | `V2,V3` |
| S-015 | `orderedSpanMergeRules`, `490-493`, source-backed LHS | Calls `copyIdentity` unless RHS is internal identity | `C,A?` in this source-backed case | Mutates selected source slot | `materialize_identity_into_slot` | `V1,V2,V7` |

## G. Shared mutation primitives

These are executor mechanisms, not necessarily planner opcodes.  The same
primitive can realize different semantic operations depending on its caller.

| ID | Primitive source | Physical mutation | Identity consequence | Required contract concern |
| --- | --- | --- | --- | --- |
| F-001 | `pureInfon.copyAsTypeTo`, `Proteus.Lib.dog:77-95` | Struct-copies scalar/list state, clears destination containers/indexes, recursively deep-copies children | `C` | Must specify deep-copy graph policy and rebuilt indexes |
| F-002 | `infon._copyAsTypeTo`, `408-455` | Copies flags/provenance/origin/type/order; optionally path/work; recursively copies size, value, args, inner and marked POV mapping | `C` | Clears `doReply`; preserves/rewrites `thisPtr`; cycle handling matters |
| F-003 | Direct `infon! <- other!` | Struct assignment of handles and scalar fields into existing storage | `C`, usually shallow for referenced members | Must never be described as identity proof |
| F-004 | Direct `POV.pItem <- source` | Rebinds a view to an existing object | `R` | Requires assertion or explicit selection evidence according to caller |
| F-005 | `addPOVToWrkList`, `531-536` | Appends existing POV and clears target `isNormed` | `S` to RHS item | Duplicate-by-handle suppression affects obligations |
| F-006 | `addInfonToWrkList`, `537-545` | Creates POV to existing infon, appends it, clears `isNormed` | `S` | `internalIdent` argument is currently not applied in this function |
| F-007 | `addInfonToWorkListWithPriority`, `547-555` | Marks source internal identity/not-parsing, prepends POV, clears target `isNormed` | `A?` | Mutates source flags as well as target worklist |
| F-008 | `copyWrkListFrom`, `557-566` | Deep-copies each constraint infon into new POV and appends it | `C` | Does not itself clear `isNormed` |
| F-009 | `takeWrkListFrom`, `568-573` | Adds existing work POVs to target; optionally clears source list | `S` for constraint objects | Transfer versus sharing must be explicit |
| F-010 | `convertThisToIntersection`, `465-473` | Rewrites target modes/size and assigns inner object | `C` | Caller decides whether old target was preserved in inner |
| F-011 | `PovList.append/appendPov/prepend`, `infonList.dog:705-745` | Allocates or links POV, updates head/tail/prev/next | `S` if existing infon handle supplied | Does not update parent infon size |
| F-012 | `PovList.pushAsAlt`, `747-757` | Appends alternative POV, sets outer/loose/predecessor metadata | `S` | Alternative provenance is semantic |
| F-013 | `PovList.erase`, `759-769` | Unlinks one POV and repairs neighbor/head/tail links | `-` | Does not update parent size or clear removed links |
| F-014 | `PovList.insertBefore/insertAfter`, `771-799` | Allocates and links POV around position | `S` if existing infon supplied | Does not update parent size |
| F-015 | `PovList.popFirst/popLast`, `801-825` | Unlinks endpoint and repairs head/tail | `-` | Assumes endpoint exists before reading `pItem` |
| F-016 | `PovList.clear` / assignment, `698-702`, `879` | Drops head/tail or aliases another list's head/tail | assignment can share topology | Clear does not walk/detach nodes |
| F-017 | `infon.countSize`, `248-282` | Computes literal string/list size and may close `tailUnfinished` | `-` | `verify` can close a list without rewriting size |
| F-018 | `infon.clear/setToList`, `603-624` | Clears value/size/flags/provenance/args/inner/type then initializes open literal list | `-` | Destructive to target state; callers must authorize replacement |

## H. Agenda, lifecycle, and externally visible effects

### Executor-only lifecycle mutations

| ID | Source | Effect on LHS processing | Why it is not a planner opcode | Verify |
| --- | --- | --- | --- | --- |
| C-001 | `Agenda.addAItemToAgenda`, `933-943` | Marks rsInfon target `isOnAgenda` and queues AItem | Queue implementation detail | `V4` |
| C-002 | `processInfonViaAgenda`, `960-988` | Creates AItem, subscriber/parent links, priorities, cursor anchor, pending counts | Realizes `schedule_normalize` | `V4` |
| C-003 | `processMergeViaAgenda`, `990-1018` | Creates/queues merge AItem and dependencies | Realizes `schedule_merge` | `V4` |
| C-004 | `processIntersectViaAgenda`, `1020-1044` | Creates/queues intersection AItem and shares iterators | Realizes `schedule_intersection` | `V4` |
| C-005 | `wakeSubscriptions`, `1049-1058` | Holds pending count, marks subscriber woken, clears subscriber LHS `isNormed`, requeues | Streaming early-wake mechanism | `V4` |
| C-006 | `fulfillSubscriptions`, `1061-1077` | Pops subscribers, advances steps, clears subscriber LHS `isNormed`, repairs held counts, requeues | Realizes obligation fulfillment | `V4` |
| C-007 | `processAgendaItem`, `2586-2593`; completion `2681-2690` | Clears enqueue state, adjusts pending counts/steps, restarts parent | State-machine implementation | `V4` |
| C-008 | Flags at `1085-1087`, `1243-1247`, `1379-1381`, `1672-1675`, `2663`, `2670`, ordered span `168-182` | Sets init/alt/size/concat/processed/readiness flags | Cache and phase gating; operation effects must state invalidation instead | `V2,V4` |
| C-009 | `wrkLstRules`, traversal, propagation POV flags | Mutates `hasPropagated`, `keepDots`, `altPOV`, candidate and predecessor state | Executor trace needed to realize semantic propagation/closure | `V3,V4` |
| C-010 | `processUnorderedInfons`, `2561`, after distributing RHS fields | Sets `AItem.skipRHSUsageInPAI` so the aggregate RHS is not merged again | Unordered-dispatch state-machine detail | `V4` |

### External effects directly reachable from normalization

| ID | Source | External target and effect | Planner result field | Verify |
| --- | --- | --- | --- | --- |
| E-001 | `wordDefRules`, `1094-1097`; `ModelManager.indexDefinition`, `299-330` | Mutates definition metadata and word-to-model store | `external_effect: register_definition` | `V6` |
| E-002 | `derefWordRules`, `1212-1217` | Inserts dereferenced infon in parent/world type index | `external_effect: index_infon_type` or executor cache | `V6` |
| E-003 | completion `2667-2669`; `AgentContext.addContextItem`, `ModelManager.dog:120-143` | Removes duplicate context identity, prepends result, culls history | `external_effect: record_context` | `V1,V6` |
| E-004 | completion `2648-2655`, `2671-2676` | Clears `doReply`, stringifies result, submits by origin/position | `external_effect: emit_reply` | `V2,V6` |
| E-005 | include branch `2613-2627`; core `3313-3329` | Reads named stream, parses it, replaces target, starts nested normalization | `external_effect: read_include_stream` plus I-003 | `V1,V4,V6` |
| E-006 | undefined word `1195-1199`, merge rejection `2876-2879`, missing rule `WorldManager.dog:456` | Undefined word calls the currently empty `Agent.setErrorState`; rejected concrete merge calls the core error handler; missing generated rule logs and exits | `diagnostic` / `status=error`; never a successful no-op | `V5,V6` |
| E-007 | word-form resolution `ModelManager.dog:354-445` | Builds derived `WordUse` records and plural definition models; line `430` also changes a stored matched definition's POS to noun | `external_effect: derive_word_form`; the stored-definition mutation needs an explicit contract or removal | `V2,V6` |

## Completeness checks performed

The matrix was checked in both directions:

1. **Dispatch to helpers.** Every mutating branch in `rsInfon`, `rsMerge`, and
   `rsIntersect` at `Proteus.Lib.dog:2602-3077` has a row or is identified as
   executor-only control.
2. **Generated actions.** All 33 implemented calls in `WorldManager.dog` to
   `DO_COPY`, `copyIdentity`, `mergeRHSIntersect`, `mergeLooseStrings`,
   `mergeANDRanges`, `checkNumRange`, or `startPropRules` are represented by
   G-003 through G-042.  Its inline type/value/rejection/struct-copy writes are
   also represented.
3. **Generated gaps.** All 60 `TODO: unfinished` and six explicit `Do Nothing`
   branches are counted and line-listed above.
4. **Transitive structure changes.** Traversal materialization, sparse and
   ordered spans, list closure, alternatives, and every reachable `PovList`
   mutator are included in sections F and G.
5. **Identity audit.** Every pointer rebind, source-backed view, deep/state
   copy, and proxy/provenance link has an identity classification.  Content
   comparison branches G-015 and G-018 only reject mismatches; they never prove
   two infons identical.
6. **Non-model effects.** Agenda state and external stores are separated so a
   planner cannot claim normal form merely because an AItem completed.

This is a source inventory, not yet a proof that a proposed planner covers it.
The next artifact should map each non-control row to an operation schema and an
executor implementation, with no row marked covered until its preconditions,
identity evidence, preservation set, invalidations, and verification oracle
are explicit.
