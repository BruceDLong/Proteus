# Normalization framework audit and consolidation plan

Proteus4 should add semantics through graph relationships and use proved relationships to shorten normalization. The current implementation has useful semantic components, but several overlapping systems decide what work remains valid, what to revisit, and when a result is complete. The next change should consolidate those responsibilities and remove their duplicate implementations.

The recommended replacement is **one shared lifecycle for deductions and their premises, with request expansion expressed as additional obligations**. Keep the existing definition compiler, correspondence algorithms, and checked writes where they implement valid semantic rules. Replace the machinery that repeatedly reconstructs the same work around them. Do not add another cache or faster global rescan as the architectural solution.

This audit describes the working tree inspected on 2026-10-10, including its pre-existing changes. It changes no normalization semantics. The [source manifest](../../../graph/framework-audit-2026-10-10/source-manifest.json) identifies that tree; the [small traces](../../../graph/framework-audit-2026-10-10/results.json) make the worked example reproducible.

## What the measurements establish

The earlier isolated solved-state run took 23.703 seconds without the GUI, state2, streaming input, or an overlapping history request. It created 20,806 tasks and 462,272 trace events. Profiling the same behavior measured 2,194 reachability reconciliations, 32,875,928 cumulative task-table entries, and 8.417 seconds in that reconciliation. All 26 poses, 54 stickers, and 156 side colors passed the independent model comparison. These are normalization measurements, not GUI frame timings.

Only one publication in the profiled process was broad; 2,054 were scoped. Precise invalidation therefore already works at one level. The expensive full task-set reconciliation survives underneath it. Whole-state and ordinary member traversal also produce overlapping value requests: 2,444 node/context/coordinate combinations received both lazy and fill-immediate demands.

The detailed baseline is [state1-normalization-cost/findings.txt](/home/bruce/devl/Slipstream/LocalBuild/State2ColorsPoses/state1-normalization-cost/findings.txt). Its timings are observations, not a promised speedup from this proposal.

## The current path through the engine

| Stage | What supplies the semantics | What manages its execution today |
| --- | --- | --- |
| Request | `normalize`, bounded depth, or whole containment | Ordinary sessions and root-specific shared producers have separate task tables. |
| Definition application | Catalogue binding, declaration meaning, instance context, and compatibility with supplied facts | `ProteusTypeGoal` passes; compiled certificates; witness validation epochs; instance-check caches; a prepared application plan. |
| Part correspondence | Existing identities, unique nominal matches, or ordered span alignment | Definition plans and record goals rebuild or validate mappings; retained identities supply reusable relationships. |
| Value transfer | A compatible source supplies missing size/value/type facts; contextual expressions remain bound to their source instance | Goal traversal finds a route, prepares a Set, and publication invalidates dependent evidence. |
| Child traversal | Membership, count, and requested depth introduce child obligations | `PersistentTraversal` retains its own entries, support links, evidence, and ready queue inside an outer scheduler task. |
| Completion | Required constraints, child proofs, and relevant membership/closure evidence | `settle` rebuilds the required dependency DAG; `ProteusCompletion` records exact-query receipts; several goals maintain their own verified/done flags. |
| Temporal preservation | Corresponding earlier occurrence, compatible value, and exhausted supplied constraints without `%?` | A guarded goal waits for the session frontier to drain before the scheduler examines its prerequisite closure. |

The duplication is concrete. [GraphSearch.h](../../../graph/GraphSearch.h) owns `Task` activation, dependencies, demands, invalidation, and a queue. [PersistentTraversal.h](../../../graph/PersistentTraversal.h) owns another set of entries, activation/support relations, proofs, and a queue. A changed retained deduction wakes its enclosing task, which can trigger `reconcileTasks` over the session. The path implementation can additionally abandon retained traversal through `NonPersistentNavigation` and resume the restart lifecycle.

These structures do not all represent the same information. A proof dependency, a requested result, and an algorithm's cursor are different things. The consolidation must give each one a clear owner, rather than mechanically combining their structs or deleting required support information.

## A small example isolates the useful work

The probe defines a state containing `thing.location`, with `xloc:1`, `yloc:2`, and `zloc:3.5`. It uses the current compiled library, with no GUI or test-only engine changes.

| Input | Observed result | Events | Visits | Distinct visited queries |
| --- | --- | ---: | ---: | ---: |
| Named solved state | All three values supplied by one finite definition application | 194 | 37 | 35 |
| Two temporal states, later `zloc:_` | Later zloc becomes 3.5 | 1,271 | 259 | 103 |
| Same history, later `zloc:%?` | Later zloc remains unknown | 1,218 | 242 | 96 |
| Same history, later `zloc:7` | Later zloc remains 7; earlier zloc remains 3.5 | 1,197 | 236 | 96 |
| Same descriptions in a non-temporal list | Later zloc remains unknown | 804 | 139 | 77 |

All five leave the declarations unchanged. Unknown size slots remain symbolic, so these cases do not assert that `symbolicValues` is false. The tests check the specific acquired values and temporal boundaries.

This matters for the deletion plan: **the useful copy is already batched**. `ProteusDefinitionPlan` can fill the small state in one atomic application. Replacing it with another copying algorithm would miss the architectural problem. The intended simplification is to reuse its established correspondence and facts through the common deduction lifecycle.

## Graph relationships required by the example

The following notation describes semantic claims, not a proposed new parser syntax or a second model graph.

| Claim | Necessary premises | Consequence |
| --- | --- | --- |
| `Meaning(instance, declaration, binding)` | Applicable nominal declaration and the instance's original context binding | Declaration requirements are available for that instance. The declaration stays read-only. |
| `Corresponds(targetPart, sourcePart, relation)` | The enclosing relationship, compatible membership/layout, and unique permitted matching | Child constraints can be related in the specified direction and mode. |
| `Value(part, v)` | A literal, a proved expression result, or compatible transfer along an allowed relation | The value is available to every demand concerning that occurrence and binding. |
| `Represented(target, source)` | The source's applicable current facts are entailed by the target | Reapplying those facts is unnecessary. The relationship remains available for later constraints. |
| `Count(list, n)` | Membership and embedded-span widths establish n | Scalar payload changes do not invalidate this count. |
| `SuppliedRoutesSettled(part, premises)` | The relevant supplied expressions, containing constraints, and input dependencies have been examined and settled | A preservation guard may use this evidence, even if the value remains unknown. |
| `Preserve(later, earlier)` | Temporal correspondence, known compatible prior facts, settled supplied routes, and no applicable independence declaration | Missing later facts may be acquired without equating the two temporal occurrences. |
| `RequestSatisfied(request, evidence)` | The particular request's required obligations are proved | That request has proof of completion. This is stronger than having an empty work queue. |

`Corresponds` must retain its relationship kind. Instantiating a declaration, asserting equal descriptions, and relating successive temporal parts are not interchangeable. In particular, neither declaration application nor preservation grants permission to merge an instance with its template or identify successive states.

For state1, the useful derivation is: bind the solved declaration to this instance; establish its part correspondence; apply the compatible facts; retain evidence of those facts and remaining constraints. Named colors, side expressions, and face selectors add their own graph obligations. They do not require a different rule for retaining a fact.

For state2's zloc, the useful derivation is: establish prior-state correspondence; extend correspondence through thing and location; settle the relevant later supplied constraints; observe prior zloc 3.5; apply the guarded transfer. An unresolved unrelated color does not itself belong in this scalar guard. A containing expression that could change this part or introduce `%?` does belong in it. The existing preservation semantics determine the prerequisites, not queue emptiness.

## A common deduction contract

Use the existing POVs, infonViews, catalogues, and registry as the model. Virtual relationships may be generated on demand. Execution records retain derivations over that model; they must not duplicate all model adjacency or expand sparse domains.

Separate four responsibilities:

1. **Semantic claims and obligations.** Identify the occurrence, relationship endpoints, interpretation/binding context, qualifications, and relevant coordinates. A type name alone, an infon pointer alone, or a caller's session ID is insufficient. Different derivations of the same conclusion remain alternatives with their own premises.
2. **Derivations.** A rule states its conclusion, premises, and any checked publication it can perform. The common runtime owns scheduling, subscriptions, required versus exploratory dependencies, and support withdrawal. A rule must not hide recursive normalization inside a matcher.
3. **Requests.** A handle owns requested roots, expansion policy, cancellation, and result delivery. Multiple handles can depend on the same semantic derivation in one context. Releasing one handle does not cancel another's work or delete established model facts.
4. **Checked publication.** A Set validates its live premises and authority before writing, publishes only actual effects, and retains the evidence established by its own write. Locking, parser publication, and exactly-once external effects remain explicit boundaries.

The shared unit is a semantic derivation, not a whole `normalizeAll` session. A node's fact proof can be used by its history request, its own scene request, and a field request when their occurrences and bindings agree. Their overall completion claims remain distinct.

Factor request expansion from fact acquisition. Ordinary normalization must retain its current rule for filling the explicitly requested result's immediate definition parts. Whole normalization adds obligations for further contained parts. Bounded depth still limits requested containment, not dependency search. These policies should introduce additional premises into the same execution system, rather than wrap a second recursive value walk around it. Dropping the current extent argument from query keys without this factorization would be incorrect.

A native rule implementation may compile a declaration, run nominal matching, or calculate a compressed span layout. Those are useful algorithms. It should return relationships, prerequisites, evidence, and checked effects using the common contract, instead of owning another subscription or restart protocol.

A common scheduler alone would not meet the Proteus4 goal. The rule boundary must allow relationships in the graph to introduce deductions and allow proved equivalences or implications to supply shorter routes. Adding a semantic relationship should not require another scheduling lifecycle. A useful acceptance exercise is to add a rule and its justified shortcut by changing the graph while leaving the runtime unchanged. This audit has not demonstrated that capability; it is a requirement for the replacement, not a property conferred merely by consolidating C++ classes.

## Optimizations expressed as theorems

These are candidate rules whose premises must be explicit and tested. They are not claims that an unrestricted theorem prover already exists.

| Rule | Premises and justified shortcut | What it replaces |
| --- | --- | --- |
| Reuse an established fact | Same occurrence and interpretation; the proof's semantic premises remain valid | Reacquiring the same scalar merely because another request or unrelated fact appeared. |
| Reuse a conjunct | A request's proof includes the exact obligation another caller requires | Separate execution of common field/header work for lazy, full, and overlapping requests. It does not equate their completion policies. |
| Retain list count | Membership and embedded widths are unchanged | Recounting after payload updates. New adjacency or width information remains relevant. |
| Transfer corresponding known parts in a batch | Finite unambiguous mapping; all applicable constraints and contexts checked; all effects valid before publication | Repeated rediscovery of correspondence for each copied scalar. Existing atomic plans can implement this rule. |
| Skip represented work | The target entails the source's current applicable facts under the relationship's qualifications | Replaying a satisfied RHS. This does not erase the relationship, certify all future constraints, or imply whole normalization. |
| Publish a locally ready guarded result | The guard's complete required dependency closure is proved, with live premises checked again at commit | Waiting for unrelated session work before an eligible preservation Set. |

A theorem can expose a derived or virtual route with its proof attached. The runtime should be able to use that route without first repeating the traversal that proved it. Choosing the shortcut must preserve the same semantic winner: ordered first-match and unordered alternatives cannot change with scheduling or cost.

Proof reuse and an index are different. Compiled declaration trees, nominal indexes, span layouts, and candidate indexes may remain as efficient representations of semantic data. Their lifetime and validity should follow the common evidence contract. They should not introduce another normalization lifecycle. Ordinary implementation improvements to allocation or lookup do not need to masquerade as language theorems.

## Conditions that prevent an unsafe simplification

**Learning a value, invalidating a provisional selection, and completing a request are different events.** A known value does not prove all constraints; a quiescent request may remain symbolic. A new contradictory assertion must still be checked even when a previous value proof is reusable. A mutation of an actual proof premise must still invalidate affected derived conclusions.

**Negative evidence needs its boundary.** Missing optional declarations, absent matches, and exhausted supplied routes are claims about particular bindings or search domains. An implied open unordered tail is not proof of absence. Pending parser input, unresolved candidates, and containing transition expressions cannot be ignored to make a guard ready.

**Sharing needs semantic identity.** Preserve original contextual bindings, source owner stacks for selected aliases, world/topic dependencies, and distinct temporal occurrences. An epoch should disappear only when its actual semantic revalidation requirement has an explicit representation. Completed queries must retain the current contract for reconsidering later definitions on explicit normalization.

**Cycles need support analysis.** A cycle cannot prove its own conclusion. Removing the last external request/support must not leave an otherwise unreachable cycle alive. Simple incoming reference counts are insufficient. The common runtime needs cycle-aware analysis confined to the affected component when supports change; the audit does not establish a constant-time bound for this operation.

**Commands need current authority.** Cancellation, changed guards, and withdrawn alternatives must prevent stale writes. Preserve the atomic validation boundary, rejection propagation, source immutability, and effect-delivery semantics. A ready queue alone is not the proof of any of these conditions.

## Deletion and replacement map

These are conditional deletions, not instructions to remove the named code immediately. The proposal removes duplicate lifecycle ownership while retaining the semantic information that ownership currently protects.

| Current mechanism | Replacement and deletion condition | Semantic code to retain |
| --- | --- | --- |
| `GraphSearch.h`: restarting `Task` state and `resetTask`, alongside `PersistentTraversal` entries/ready/support state | One runtime owns retained derivations. Remove the restart/persistent protocol split, duplicate queue and support bookkeeping, `persistent()` branching, and synthetic continuation edges once all users have moved. | Required/exploratory support, rejection, cancellation, context locking, and bounded scheduling. |
| `GraphSearch.h`: whole-session `reconcileTasks` after local invalidations | Maintain changed support relations and repair affected components. Remove the full task-table sweep from the normal local-publication path after cycle/withdrawal tests pass. | Correct reachability when a root, alternative, or prerequisite disappears. |
| `GraphSearch.h`: global `finishPass` readiness scan and rebuilt `settle` DAG | Completion and publication eligibility derive from explicit local premise/closure transitions. Remove repeated whole-session scans once aggregate and guarded producers use those transitions. | Finite candidate exhaustion, partial stream handling, transitive guards, and no circular proofs. |
| `NormalizationDepth.h`: private traversal and overlapping recursive header/member requests | Expansion policy generates additional obligations in the common runtime. Remove the private traversal wrapper and redundant header walks. | Depth meaning, immediate omitted parts, selected-source context, sparse/open safety. |
| `GraphSearch.h`: separate ordinary sessions and per-root shared producer task storage | Handles reference common context-owned derivations. Remove the distinct sharing machinery after ordinary/full/multiple-root cancellation and lifetime cases pass. | Per-handle result, cancellation, completion, and delivery policy. |
| `FieldCandidates.h`: `Answer` validity/wake lifecycle and `PathSelection.h` candidate epochs | Candidate conclusions become common derivations with explicit interpretation dependencies. Remove duplicate ready/invalidation ownership and epoch-based duplication only after explicit revalidation behavior is preserved. | `FieldPredicate`, nominal matching, ordered precedence, alternatives, and useful lookup indexes. |
| `DefinitionApplications.h`: parallel positive-check validity and validation-epoch lifecycles | Express catalogue and instance-check premises through common proofs; remove epoch/accepted-check duplication when unpublished edit and explicit-reset behavior is covered. | Immutable compiled meaning, source witnesses where needed, contextual bindings, partial/recursive definitions, and `ProteusDefinitionPlan`. |
| `ProteusCompletion`: separate exact-query receipt validity machinery | Request completion references common obligation proofs; semantic proof keys carry all required binding/context distinctions. Remove duplicated proof lookup/validity handling. | `isNormed` compatibility behavior and exactly-once reply bookkeeping. |
| Goal-local `beginPass`/`verified_`/`done_` restart state across type, record, path, and preservation producers | Persistent rule results and premises express these conclusions. Remove state used solely to reconstruct a lost pass. | Algorithm-local scratch data, actual semantic alternatives, matching plans, and guard rules. |

Retain `POVDescription`, the model/registry, dependency publication at the mutation boundary, nominal correspondence, compressed list/span algorithms, numeric compatibility, and atomic Set application. They are not redundant merely because they contain code. No whole file should be deleted simply because one responsibility moves out of it.

## Implementation sequence with retirement gates

1. **Implement the common contract on the small definition and temporal examples.** Represent meaning, correspondence, known facts, supplied-route closure, and request completion separately. Use the existing checked application operations. Prototype outside the production execution path; do not ship a third permanent scheduler.
2. **Replace lifecycle ownership in the runtime.** Migrate the existing retained field/list/depth/concat consumers and restarting producers to the same contract. A temporary adapter must delegate scheduling and evidence ownership to that runtime, not keep a private restart engine. Finish by removing the restart/persistent split and nested traversal scheduling. The production cutover must cover current semantics, not silently send difficult cases back through an unmeasured alternate engine.
3. **Make all request forms share their common obligations.** Preserve distinct expansion policies and occurrence contexts. Remove the separate producer-sharing path and whole-depth traversal wrapper when the ordinary/full/overlap cases pass.
4. **Consolidate cache validity and completion.** Keep useful semantic indexes and compiled data; remove duplicate proof lifecycles. Make guarded publication depend on its own closure, and remove routine full-session reachability/completion scans.

Do not add new language semantics during this consolidation. Each implementation slice should name the old code it retires and report net production-source changes separately from tests and diagnostics. A replacement is unfinished if the old execution mechanism remains active beside the new one. This is a staged replacement plan, not an instruction to stack all four stages on the existing framework.

## Acceptance criteria

| Obligation | Existing evidence or coverage to preserve | Additional acceptance condition |
| --- | --- | --- |
| Definition and record behavior | Small traces in this audit; `graphDefinitionValidity`, `graphRecordIndex`, `graphDefinitionSharing`, and `graphInstanceChecks` groups | Shared declarations unchanged; distinct targets/scopes remain distinct; ambiguous correspondence stays pending; known values and conflicts preserved. |
| Temporal behavior | `graphPreservation` scalar/nested/ordinaryUnknown/blocked/pendingGuard/pendingParent/stateIsolation/sparseSafety cases | The one missing zloc is acquired only through its proved temporal route; `%?`, explicit changes, non-temporal lists, and transitive pending guards retain their behavior. |
| Request composition | `test_normalization_depth.cpp`, `test_shared_requests.cpp`, and `graphCompletion` groups | A deeper or overlapping caller reuses common deductions; cancellation and released handles do not revoke another caller's work; no context-blind sharing. |
| Support and completion | `test_graph_search.cpp` cycle, withdrawal, rejection, stale publication, and retained-proof cases | Local changes do not sweep unrelated task sets; unsupported cycles cannot keep themselves active or certify completion. |
| Real application | Current state1 and single-unknown state2 fixtures and geometry/color verifiers | Preserve all 26 poses, 54 stickers, and 156 side colors per solved state; state2 root zloc becomes 3.5; compare actual Slipstream startup and visible rendering as well as headless results. |

Measure total work: materialized nodes, semantic rule evaluations, distinct claims/derivations, queue entries including stale ones, dependency notifications, support-repair visits, completion work, elapsed time, and memory. Trace-event reductions alone can conceal work moved into helpers.

Use fresh contexts and the same loaded definitions for paired baselines. Include ordinary, whole, and overlapping requests; repeat an unchanged request; add an unrelated fact; add a relevant late fact; withdraw the last support; and exercise an ambiguous/open case. For finite records with a fixed number of fields, fixed definitions, and no changing/ambiguous relationships, compare increasing instance counts and require work to scale with introduced parts and premises rather than publications multiplied by all accumulated tasks. This is a bound for that workload, not a claim that arbitrary inference is linear.

The full audit suites were not rerun for this documentation-only change. The five new small probes passed against the existing library. Cube measurements and its complete geometry/color validation are the earlier recorded baseline; the new framework remains a proposal until implementation satisfies these gates.
