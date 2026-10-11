# Proteus4 normalization design

Proteus4 represents semantics through graph relationships and justifies semantic optimizations through theorems. This specification consolidates settled choices for normalization. The [issues and decision log](normalization-design-decisions.md) holds alternatives, reasons, evidence, and unresolved questions.

Status on 2026-10-11: the WorldManager and virtual graph boundaries, CodeDog policy, core D02 lifecycle, and D03 list principles below are settled. Goals acquire requested facts, stop when satisfied, and continue at waiting links without restart. List scope and shared propagation are specified; exact graph operations, matching records, and subsequent intersection/definition/rule composition remain open. Planning design stays deferred until intersections and definitions have been considered. The [architecture audit](/home/bruce/devl/Proteus4/graph/normalization-framework-audit.md) remains supporting evidence and a source of proposals.

## Design objective

Semantic relationships must compose through the reasoning framework. A justified optimization may provide a cheaper way to establish the same result, with its premises and validity explicit. Native algorithms may still be needed; their boundary with graph-expressed rules is to be settled. Merely consolidating implementation classes does not establish this capability. See decision G01 and issues D06–D09.

Streaming behavior and computational cost are part of every operation's contract. Complexity statements identify the workload, relevant graph size, virtual extent, candidate count, changes, and output. They distinguish initial work from incremental updates and memory retention. No general linear-time claim is made for arbitrary inference.

## Replacement development

Develop the new solver on `codex/normalizer-redesign` in `/home/bruce/devl/Proteus4-redesign`, within the Proteus4 repository. The original checkout at `/home/bruce/devl/Proteus4` retains checkpoint `b52322a867d2be2c1fda186929b05dc0c70e8be4` on `codex/proteus4` as the implementation and diagnostic reference. See G06.

Build the replacement behind WorldManager in CodeDog. Reuse the parser, POV storage, printing, and semantic tests where their contracts fit; assess solver internals individually. The branch started from that checkpoint and now runs the replacement through the normal Proteus Engine and TestProteus path. Unsupported features are reported explicitly; the old solver is not linked as a fallback. Build outputs are separate; public-world resources remain an external shared input.

## WorldManager reasoning interface

The `WorldManager` class in `WorldManager.dog` owns the complete reasoning interface. Substituting a differently implemented `WorldManager.dog` must permit replacing the reasoning system without changing reasoning callers. Internal helpers can support the implementation, but callers must not depend on its scheduler or graph implementation details. The exact methods and lifecycle contract will be specified as the solving design develops. See G03.

Agent owns and tracks sessions, as reaffirmed on 2026-10-11 (G07). A session can retain several normalization requests; Agent creates and ends sessions and closes them at shutdown. WorldManager performs the reasoning and releases the affected request resources when Agent ends a session. Session ownership is not transferred to the solver.

Normalize returns a handle whose status and result the caller can inspect. The implementation returns `WorldRequest` with `status()`, `complete()`, `result()`, `failed()`, `error()` and `cancel()`. A status snapshot reports the outcome and detail under the model lock. Runnable work drains synchronously; parser completion can continue a waiting handle. Status distinguishes ready, completed, partial, waiting, rejected, unsupported, cancelled and error results without exposing solver frames. Sharing deductions across sessions is deferred until needed; Agent still owns their separate lifetimes. See D02H.

The interface must cover the chosen normalization and streaming behavior, including the necessary input publication, result, subscription, and request-lifetime operations. It does not dictate a planning architecture or restore the old engine's internal mechanisms. The implementation must demonstrate replaceability; it is not assumed from the class name alone.

## Virtual graph access

Inside WorldManager, the solver accesses POVs through a virtual graph interface for reads and writes. It exposes semantic relationships, their qualifications and contexts, and links waiting for information. It also owns semantic publication and removal of fully merged POV RHS entries from wrkLists. This separates traversal from storage and allows useful routes without copying every intermediate value. See G05 and D02D.

## CodeDog implementation policy

CodeDog is the maintained source for the reasoning implementation. CodeDog and its build workflow may be optimized to improve iteration speed. Alternatively, a concrete problem may be investigated by unpacking and modifying generated C++; once solved, bring the accepted changes back into CodeDog, regenerate, and validate the relevant behavior. Include generator changes if the solution requires them. See G04.

Track build time separately from normalization time and memory. Generated-code experiments retain the same semantic and streaming obligations as the maintained implementation. The temporary C++ workflow does not establish a separately maintained C++ reasoner.

## Existing semantic constraints

These constraints are carried forward from the current [repository instructions](/home/bruce/devl/Proteus4/AGENTS.md), including the recorded user clarifications. They establish the starting semantic contract. The newer D02 decisions govern default completion and continuation instead of inherited restart behavior, including D02F's explicit permission for numeric completion with unknown size; D03 specifies list expansion scope and shared propagation. Other changes require an explicit decision in the log.

### Occurrences and relationships

A value, a contextual occurrence, a list position, and a temporal support are distinct notions. Equal values do not identify occurrences. Applying a shared definition does not make its instances identical to the declaration. Preservation does not identify successive temporal states.

Contextual expressions retain their original source occurrence and owner context. Definition applications retain their original binding on re-normalization; rebinding is a separate unresolved facility. Qualification and relationship direction cannot be discarded merely to reuse a result.

### Partial knowledge and lists

Unknown information is different from a proved contradiction. A failed exploratory match does not reject unrelated routes or mutate its source into a global failure. A contradiction encountered on a required route must still be reported. Default acquisition is not an exhaustive search for contradictions on every unused route; coherence checking is a separate mode under D02B.

Sparse and repeated descriptions retain virtual structure. Access must not enumerate an unrequested tail to discover a header, boundary, or selected position. Already demanded occurrences must keep their identity when more structure arrives. A received prefix is not proof of final extent.

Unordered descriptions have an implied open remainder. An explicit total size bounds membership, not the states of those members. Omission alone does not establish absence or removal. Explicit open syntax, pending parser input, and implied openness retain their respective semantics.

List membership/count, values, and temporal duration are distinct. Member count does not establish duration. ListSpec templates are not interchangeable with the occurrences they describe.

### Definitions and selection

Fields are typed members with nominal compatibility. Equal structure alone does not make sibling types interchangeable. Ordered lookup selects the first proved compatible logical member; unresolved earlier candidates can block a later winner. Unordered lookup retains matching alternatives. Route cost must not change the semantic winner.

Definitions are optional sources of meaning. Compiling meaning does not prove instance validity. Supplied typed descriptions can be partial, including ordered descriptions. Explicit normalization requests the result's immediate omitted definition parts; incidental traversal remains lazy. D03 limits an unordered target to its main level rather than renewing this expansion policy recursively for every member. Dependency search may still reach deeper facts needed for that level. Route choice must not silently determine expansion.

Shared declarations remain read-only during application. An unchanged completed infon returns from ordinary normalization without creating goals. A semantic event requiring reconsideration, including a relevant meaning change, must explicitly request a new goal. D05 will specify the declaration/context triggers; later information does not restart every completed query.

### Temporal semantics

T-list adjacency supplies preservation of otherwise unchanged values unless the model implies a change. A later part reference targets the later occurrence and leaves the earlier occurrence intact. Ordinary non-temporal adjacency does not supply this preservation relationship.

`%?` withholds default preservation for the later value. It supplies no value, asserts no inequality, and does not prevent explicit constraints from resolving the value. Ordinary `_` and `?` do not themselves declare temporal independence.

Preservation requires the applicable supplied constraints and transitive prerequisites to be settled. Pending input, ambiguous correspondence, or unresolved containing constraints can withhold it. An unrelated unfinished calculation does not become a prerequisite merely because it shares a session. Commit must use current evidence.

The present contract does not automatically retract an already committed value when a later assertion arrives. The treatment of provisional results and late conflicting evidence remains a D07 question within the no-restart contract: any exceptional semantic recalculation creates a new goal. General causal closure, membership changes, and sparse temporal correspondence need distinct specification and implementation status; they cannot be assumed solved by scalar preservation.

### Publication and completion

Completion is relative to the requested facts. For default scalar acquisition, a value compatible with the required size/type and the prerequisites of the selected route completes that goal; unused strategies are cancelled. Ordinary numeric normalization can complete with size still unknown once the value is established and the given idents are exhausted (D02F). An unknown size is not itself a size requirement. Producing requested containment, exhaustive coherence checking, and temporary quiescence are different claims. Default completion does not wait for unrelated searches or require all intermediate unknowns to be filled.

A write must have valid premises and apply only to its intended occurrence. Ordinary equality does not replace a conflicting known value. Explicit replacement semantics and their implementation status remain separate. Cancellation, shared callers, subscriptions, and exactly-once effects must have stated ownership; their replacement mechanisms are still open.

## Normalize contract and planning

**Planning design deferred: D01.** Consider intersections and definitions before deciding whether planning belongs at normalize entry, subgoal initialization, or after the obvious strategy fails or proves inadequate. Choose from concrete cases where the baseline strategy is unsuccessful or suboptimal, accounting for both the cost of planning and the work it avoids.

The starting examples are direct transfer for `_ = 5`, with the required flag updates, and forward propagation through both or all compatible lists for ordinary merges. These are baseline strategies to specify, not a universal claim about every list or a decision on the scheduler. During D02–D05, develop the request contract and collect the cases where these strategies need an alternative.

Local preparation of an atomic write does not settle whether a normalize request needs strategic planning. No planner location, representation, or fallback trigger is chosen yet. A wait for relevant input must be distinguished from a failure of strategy.

## Solving loop and wrkList relationships

**Accepted core contract: D02A–D02F.** The following specifies behavior; the precise methods, flag layout, and scheduler data structures remain open.

### Requested facts and intermediate results

A goal states what facts its target needs. `_ = 5` can acquire the supplied value directly. A chain may be searched for a usable source without publishing copies at every intermediate infon. Preserve all size, type, qualification, and contextual requirements governing the chosen route.

Useful partial answers can remain in infons as oneOfs, ranges, or compound alternatives. Search cursors and subscriptions can remain in the live solver's memory. Print/reload resumption is not required. This permits sparse spans to remain uncluttered by occurrences created solely to record traversal progress.

### Default completion and coherence mode

Once the requested facts are established, complete the goal and cancel further searching and subscriptions for that goal. Work still required by another goal retains its own ownership. An assertion is not automatically an instruction to evaluate every route as a separate consistency test.

The interface may mark the completed infon `isNormed`, so normalization of the same settled result creates no goals. The parser can establish completion for an already complete literal with given type, size, and value; this is a sufficient example, not a requirement that every normalized numeric infon have known size. Exact flag scope remains to be specified: literal completion must not accidentally stand for completion of an unfinished containing expression or a stronger request.

A separate coherence/debug mode tries multiple strategies and checks agreement. It is explicit additional work. Default normalization still checks the compatibility and prerequisites of the route it actually uses and does not accept an encountered contradiction as an answer.

### Numeric completion with unknown size

An ordinary numeric infon is eligible for `isNormed` once its value is established, the given idents are exhausted, and the applicable acceptance conditions are satisfied. Size may remain unknown, including when explicitly unspecified. Named range types such as `rot` follow this same rule. Preserve known constraints without inventing a size or treating range bounds as size.

Use retained ident cursors and attempt outcomes to establish exhaustion. Required pending relationships continue at their waiting links; an unknown size alone starts no search or subscription. Exhausting the given idents does not mean searching every possible proof. An operation explicitly requiring size must establish that fact separately; ordinary numeric completion cannot certify a stronger request or an unfinished containing expression.

### Flat and nested work relationships

In the schematic flat asserted `A=B=C` case, B may supply everything needed by A. Then A is complete; the interface can remove remaining RHS entries when they carry no further useful information for A. It need not fill or inspect C solely because C is reachable.

Ordinary `A=B=C` and `A=(B=C)` usually have the same meaning; parentheses alone do not require a separate strategy. Type-changing qualifications are the important exception: in `A==(B=C)`, C can have a different type from A. Preserve the transformation and its required premises rather than applying C's type directly to A. The existing `unit/graphText/identityInSource` fixture exercises this with a list matched against a nested string identity; carry its behavior through the engine test migration.

The interface removes fully merged pending work, not meaningful qualifications, contexts, or identities required by other uses. Stopping a goal and consolidating a work entry are separate operations. Neither requires deleting a source infon that other POVs still reference. Exact consolidation rules remain to be specified for each relationship.

### Streaming continuation

Ordinary progress reveals information about immutable infons. When an edge is promising but its information is being parsed or normalized, retain the local continuation and subscribe at that link. Other work or admissible alternative routes may proceed. A new edge or newly available fact continues affected work at that point.

Once the relevant infon has finished parsing and every current wrkList member has been tried, its given work is exhausted. Some entries may remain stored. Any further items during ordinary normalization come from requested graph links; track those pending requests explicitly. Required pending replies prevent false exhaustion. If the requested facts remain unknown and no link request is pending, report partial information. Do not wait on unspecified future additions. An arriving result continues its local attempt without revisiting the tried prefix. See D02G.

The graph manager provides no clear-and-restart operation for a live goal. It retains progress across ordinary publications; a method named “continue” must not secretly discard the goal's work and replay it. A satisfied goal remains finished. Exceptional semantic changes explicitly create a new goal; their triggers belong to the relevant language rules.

### Cost and remaining interface work

For k constant-cost compatible links with the value known only at the end, search costs O(k) link inspections but need not publish k intermediate copies. If the first usable source already satisfies the goal without additional premises, further chain traversal is unnecessary. Include payload-copy and compatibility costs separately.

Measure local subscription delivery, newly exposed work, memory, and sparse allocations. The design must not turn each arrival into a scan from the goal's root. Arbitrary rule search has no general linear bound established here.

Still to specify: the exact goal requirements and `isNormed` scope within the numeric rule above, edge readiness and consolidation operations, local continuation/subscription delivery, ownership of shared work within a session, and coherence-mode reporting. Sharing across sessions is deferred. Planning remains deferred. These details must implement the accepted lifecycle rather than reintroduce restart behavior.

## Lists and sparse structure

**Accepted principles: D03A–D03D.** The specific edge API and chart representation remain proposals in the decision log.

### Member relationship sources

A member can use its own wrkList, a corresponding member reached through a parent or ancestor ident, its type, and its parent's or an applicable ancestor's listSpec. Projection preserves the original scope, qualifiers, and placement or nominal correspondence. An ancestor template is not automatically applied directly to every descendant.

The virtual graph therefore exposes more than identities. Structural and parameterized navigation helps discover applicable relationships; semantic operations can then derive new correspondences or facts. Navigation itself grants no identity or write permission. Joins, numeric forming, and other transformations must expose their premises and effects through the same reasoning interface. Temporal correspondence preserves distinct occurrences and its own guards.

### Main-level normalization

When an unordered list is the target, fill its main level as far as reasonably possible. Resolve the requested facts about its members without automatically descending through every contained level to materials or atoms. Member obligations inherit that scope rather than restarting ordinary top-level normalization at each child.

Dependency search can reach deeper information needed to resolve a main-level member. Sparse virtual members and open tails are not enumerated merely to fill the target's level. The remaining meaning of “reasonably possible” requires examples; no arbitrary work limit is implied.

### Targeted lookup and span propagation

For an individual requested member, find its correspondence using existing relationships, known-span skips, or an index. A known extent can avoid traversal into an embedded list when the layout and relationship justify it. Matching still respects nominal types, qualification, context, and any acceptance conditions.

For a whole list or span, propagate over the corresponding spans together and reuse the established member correspondences. Do not independently locate every member from its parent again. A propagation that already accounts for a parent ident or listSpec carries that contribution into member work, avoiding rediscovery and reapplication in the same scope.

Reuse must identify the particular relationship or template application and its binding; a single global handled flag on a node would suppress distinct useful contributions. The exact representation of that retained progress remains open.

### Streaming alignment and AND OR records

General alignment of two or more lists with unknowns or uncertain member extents uses a retained, streaming Earley-like correspondence process. Each distinct semantic attempt is registered once; missing information suspends that attempt at its prerequisite. Later information continues it and joins newly enabled states without reconstructing the chart or revisiting the entire prefix.

WrkList constraints provide an AND representation and oneOf alternatives an OR representation. Each alternative retains its own correlated bindings and simultaneous constraints. Tentative correspondence must not publish unconditional identities or mix facts from incompatible branches. The request's semantics determine which alternatives are needed and when the goal can finish.

An execution index can deduplicate attempts without becoming a second copy of the model graph. The exact key is not yet chosen; it must distinguish semantically different positions, offsets, qualifications, contexts, and bindings. A node-pair-only visited flag is insufficient for the general case.

### Complexity and template inspection

For fixed-width unambiguous correspondence and constant-cost checks, shared propagation should scale with inspected members/spans and output. Repeated per-member searches can multiply that work. Account for index construction, sparse allocation, retained alternatives, wakeups, and per-match costs.

Deduplication bounds repetition of a particular attempt; it does not bound the number of distinct ambiguous attempts. General matching has no linear guarantee here. Template inspection may eliminate impossible alignments; it is a candidate for the deferred planning discussion after definitions and intersections.

## Intersections

**Unresolved: D04.** Specify pattern, input, alternatives, result, and result-constraint relationships using the common solver. Include list and sparse composition, partial publication, and matching costs.

## Definitions

**Unresolved: D05.** Specify binding, compiled or virtual meaning, instance obligations, part correspondence, and materialization. Explain composition with leaves, lists, and intersections, and separate shared declaration costs from per-instance costs. Specify semantic triggers for new goals when later declarations or context changes require reconsideration, consistent with D02C.

## Rules and temporal normal forms

**Unresolved: D06.** Specify premises, conclusions, bindings, guards, equivalence, and direction for theorem/transformation rules. Give temporal situations explicit graph forms using the same framework, with source examples and unresolved cases. Distinguish rule normal form from the normal form of a requested result.

## Rule search

**Unresolved: D07.** Specify discovery, indexes, forward/backward/inverse routes, matching, temporal guards, recursive support, termination/resource policy, and local continuation. State when a result can be published during streaming and distinguish later knowledge from a semantic change requiring a new goal. No whole-goal restart is available.

## Other language constructs

**Unresolved: D08.** Express `%rules`, concatenation, and the remaining constructs as graph contributions to the chosen core. Identify any new semantic primitive explicitly, with its streaming contract and cost.

## Optimization contract

**Unresolved: D09.** State the proof obligation for a shortcut, the conditions under which it is usable, and how pending premises continue its application or an exceptional semantic change requests a new goal. Include the exercise of adding one semantic rule and one justified shortcut through graph changes without changing the generic solving loop.

## Integrated behavior and implementation boundary

**Unresolved: D10.** Show the complete path for a fully expressed state, a definition-derived state, and a second state with one missing value. Include overlapping requests and streaming updates. Record semantic validation, scaling measurements, and the old execution machinery retired by each implementation stage. Demonstrate replacement behind the WorldManager interface and validate any generated-C++ experiment after its accepted changes have been returned to CodeDog and regenerated.

As each issue is settled, replace its unresolved section with the chosen contract. Keep alternatives and the history of revisions in the decision log.


## Proposed implementation roadmap

**Begun on 2026-10-11; updated by P02 to make the normal Proteus Engine/test cutover the next milestone.** This is a sequence of reviewable working increments, not a decision on the open semantics above. See P01 in the [decision log](normalization-design-decisions.md#p01-implementation-sequence). Before implementing an unresolved construct, work through its examples and record the chosen meaning in both documents. The implementation plan does not lift the deferral of a strategic planner.

The replacement runs in `/home/bruce/devl/Proteus4-redesign`. The checkpoint remains the comparison implementation in `/home/bruce/devl/Proteus4`. A replacement test must identify its engine and run entirely through WorldManager; unsupported features are reported explicitly, never supplied by an old-engine fallback. Keep the original language fixtures intact. Port useful newer behavior assertions through the new interface, without reproducing their old cache or scheduler APIs.

### 0. Preserve the reference and test inventory — done

The checkpoint and test-pruning manifest already preserve the reference. The pruned existing engine reproduced the same outcomes on 332 language cases and 75 selected compiled cases: 40 existing failures in total. These results validate test preservation, not the replacement. See the [test-selection record](README.md#test-selection-for-the-normalizer-redesign) and [retirement manifest](retired-normalizer-tests.json).

As coverage moves, record which feature a case exercises, whether it has been ported, and its replacement result. Separate an unimplemented feature, an existing implementation failure, an ambiguous expectation, and a deliberate semantic change. An unsupported case with clear semantics remains a failure, rather than being relabeled as ambiguity or silently excluded from totals.

### 1. Establish WorldManager and a replacement test target

Specify the smallest external contract needed for the first working examples: target and requested facts/scope; result and meaningful partial progress; completion, waiting, rejection and error; per-caller cancellation and lifetime; world/context binding; and parser publication and safe model access. Distinguish waiting for a useful result from waiting for an input stream to close. These are responsibilities to settle, not a commitment to the current GraphNormalizer method list.

Implement WorldManager in CodeDog and route the replacement harness through it. Reuse parsing, model storage and printing. Extract only the storage or semantic helpers needed to compile that path. The parser currently uses GraphPublication/InputPause, Agent exposes GraphNormalizer, and host code accesses graph read/update operations. Move those dependencies behind the new contract as each caller is migrated; do not move old scheduling ownership into a renamed wrapper.

**Gate:** parse and print a literal through the replacement target, submit a request, and report its status through WorldManager. Generated code and the exercised call path must show that no old solver runs. A build identifies its source checkout and engine so baseline results cannot be confused with replacement results. This stage retires the direct old-runtime dependencies of the migrated harness and input boundary; later constructs need not work yet.

### 2. Make the smallest scalar normalization work

Add the virtual POV access needed for supplied leaf relationships, a goal frame, retained source cursors and attempts, a ready queue, and checked publication. Choose only the fields demonstrated necessary by these examples. The same queue-processing step will later be driven by the worker; a synchronous test runner must not become a separate solver.

Implement an already expressed value, `_ = 5`, compatible and conflicting supplied constraints, and short flat/nested chains. Preserve qualification, type/range and known-size requirements. Decide the precise completion flag scope before setting it. Ordinary numeric completion follows D02F: known compatible value, exhausted given idents, satisfied required conditions, with size permitted to remain unknown. A stronger size request is separate.

**Gate:** executable parser examples distinguish flat assertions from nested or qualified prerequisites; unknown-size numbers complete without a size subscription; another ordinary normalize of an unchanged completed result creates no work. Chain traversal does not require publishing every intermediate value. Apply only a proved-safe initial RHS consolidation case, with a shared-source check; other relationships remain available until their consolidation conditions are specified. This replaces leaf begin-pass/restart and completion-receipt machinery for the covered cases.

### 2a. Cut over the Proteus Engine and real tests

Route the public Agent normalization methods and normal TestProteus language path through WorldManager now that the first scalar slice works. Keep the returned handle and the synchronous convenience method. Migrate initialization, parser publication and stream ownership dependencies as part of the engine boundary; no migrated path may invoke the old solver. Stage 3 supplies local continuation for live input. Lists, definitions and the remaining language features need not be complete before the real tests run.

Keep `.tst` fixtures unchanged and report the engine's status alongside their semantic output. Unsupported constructs must be visible failures, including those whose unchanged printed form happens to match an expectation. Port retained compiled behavioral assertions through the same public interface. Fold the useful scalar harness coverage into the normal suite and remove obsolete engine dependencies as their callers move. Use the other Proteus checkouts for historical comparisons.

**Gate:** fresh CodeDog build of TestProteus, supported historical scalar checks, a reported language-suite baseline, and verification that the replacement test path never calls the old solver. Record unsupported constructs separately from supported behavior failures. Report the production code and dependencies retired. Do not retain the old implementation solely for tests coupled to its internal APIs. See P02.

### 3. Retain progress across streaming and overlapping requests

Implement local edge subscriptions and continuation delivery before adding complex language constructs. Make checking readiness and registering a wait safe against a publication between those actions. Apply D02G: after parsing is done and every current wrkList member has been tried, further work comes from requested links. Track their outstanding results, including required pending replies. New facts and new edges continue the relevant retained attempt, without clearing its cursors or completed work.

Within a session, define shared-attempt identity and per-handle ownership using target occurrence, requested facts and semantic context. Share only equivalent work. Sharing across sessions is deferred and is not a gate for this stage. Cancelling one caller must leave another caller's needs intact. Release no-longer-needed waits and frames, including cyclic waiting dependencies, without treating a circular dependency as a proof. Finish the worker/input/close lifecycle using the same solving step as the scalar tests.

**Gate:** delayed values, an appended ident, both publication/subscription arrival orders, duplicate notifications, shared requests, cancellation and shutdown. Unrelated publications do not revisit finished prefixes; a required pending ident prevents false exhaustion. Measure affected subscriptions and newly enabled work, not the whole world. This replaces the old split between restarting tasks, persistent traversal, and separately shared requests for migrated features.

### 4. Add ordinary list propagation and member access

Specify and implement fixed-width unambiguous list correspondence first, including two or more simultaneously constrained lists. Carry the particular parent correspondence and applied listSpec contribution into member frames. Expose the four sources from D03: own work, ancestor correspondence, type meaning, and applicable listSpec; declaration-dependent meaning is connected in stage 7.

Use one retained propagation for a whole list/span. For a single member, allow justified known-length skips and existing indexes. Preserve nominal correspondence for unordered members. Inherit the requested expansion scope so normalizing immediate members does not recursively request their internal levels. This walk is the deterministic case of the correspondence system extended in stage 5, not a second scheduling system.

**Gate:** direct fully expressed state data, finite merges, nested parent correspondence, listSpec reuse, an isolated member request and a whole-span request. With constant-cost matching, measured work grows with inspected members/spans and output; doubling a finite list must not expose repeated searches from its start. This replaces per-member parent rescans and the private depth traversal for covered requests.

### 5. Extend correspondence to sparse, ambiguous and streamed lists

Settle the D03 attempt equivalence and chart representation using unknown extents, alternate bindings and three-list examples. Extend the same correspondence operations with retained Earley-like states. Keep AND constraints and OR alternatives correlated; tentative branches must not publish unconditional identities. Index execution attempts without copying the semantic graph.

Retain compressed spans and demanded occurrence identities. Resume states when their missing extent, member, template or constraint arrives, in either arrival order. A known prefix does not prove an end. Define the boundary between completing a partial request and retaining an open result before treating either as normalized.

**Gate:** a large virtual extent with bounded demanded members; late prefixes; two bindings for the same node pair; ambiguous widths; and correlated alternatives across several lists. Count unique attempts, transitions, subscriptions and materialized occurrences. Sparse work follows inspected spans and demanded output; ambiguous matching has no blanket linear bound. This retires duplicate list-matching continuation and retry machinery as its consumers migrate.

### 6. Specify and implement intersections

Resolve D04 before choosing its concrete records: pattern, source, candidate, selected/alternative result, and constraints attached to that result. Walk a scalar predicate, a list pattern, ordered precedence, unordered alternatives and an intersection with a later result constraint. Preserve source context and distinguish rejection from unavailable information.

Implement intersections using the existing frames and correspondence operations. Matching must expose prerequisites to the common loop, rather than recursively normalize its operands through another engine. Record a concrete case if the obvious strategy does unnecessary search; this is evidence for stage 8.

**Gate:** partial and streamed candidates, a blocked earlier ordered candidate, branch-correlated outputs, and no reinterpretation of result constraints as pattern input. Charge candidates, matching and alternatives explicitly. This replaces FieldCandidates/PathSelection lifecycle ownership where covered, while preserving useful nominal matching and navigation algorithms.

### 7. Specify and implement definitions

Resolve D05 using direct and typed versions of the same object, partial definitions, distinct contexts sharing one declaration, a definition containing an intersection, and recursive/late meanings. Decide how immutable meaning is exposed or compiled and when instance members actually need allocation. Keep declaration work separate from instance checks and publication.

Connect type meaning and applicable listSpecs to the ordinary graph operations. Define exceptional declaration/context events requiring a new goal; do not interpret ordinary normalize as permission to restart a finished result or rebind its context.

**Gate:** the direct and definition-derived state1 produce the same requested facts; instances remain distinct; declarations remain unchanged; requested main-level expansion does not enumerate sparse tails or deeper containment. Report one-time declaration work, per-instance checks and output allocation separately. This retires duplicated definition application, validation-epoch and represented-work lifecycles as their semantic responsibilities move.

### 8. Revisit the need for planning

Review the concrete intersection and definition counterexamples from stages 6–7 under D01. Compare the obvious strategy, a cheap local dispatch rule, and an actual planning step. Choose a planner's location and representation only if an example justifies its cost. It is a valid outcome to need no separate planner yet.

**Gate:** document what work is avoided, the detection/preparation cost, first useful result during streaming, and any retained progress when an alternative strategy is tried. Waiting alone is not strategy failure; trying another route must not reset the goal. Add no permanent planning layer solely to complete this checklist.

### 9. Represent and apply theorem and temporal rules

Resolve D06–D07: premises, conclusions, bindings, direction, guards, discovery and cycle behavior. Use the established graph/attempt machinery for rule applications. Then add temporal correspondence and preservation with distinct occurrences and locally proved supplied-constraint closure. Settle the handling of later conflicting assertions or exceptional new-goal triggers before claiming those cases work.

**Gate:** a theorem with a pending premise, a justified reverse route, a cycle without an independent fact, and temporal unchanged/changed/unknown/`%?` cases. For state2, start fully supplied, then leave exactly one zloc unknown; obtain that fact from state1 without requesting unrelated color or pose work solely for this goal. Pending relevant constraints block preservation, but unrelated session work does not. Report candidate-rule and firing costs. This replaces separate temporal guard/pass lifecycles; sparse temporal cases reuse stage 5 rather than enumerating history.

### 10. Complete composition and demonstrate graph-based extension

Work through D08 with `%rules`, relative references, selectors, concatenation, numeric forming, functions and reply/effect boundaries required by the retained coverage. Each contributes relationships or uses a justified primitive with explicit prerequisites. Reuse list/navigation/scalar operations; a difficult combination does not justify its own scheduler.

Complete the D09 architectural exercise: introduce a semantic rule and a proved shortcut by changing graph contributions while leaving the generic solving loop unchanged. Add the explicit coherence/debug mode to compare applicable strategies where it is useful. It must not make ordinary requests exhaust unused routes.

**Gate:** composition fixtures, unfinished operands and exactly-once reply/effect behavior. Compare normal and coherence results, shortcut applicability and work saved, including cases where the shortcut cannot apply. Port accepted generated-C++ experiments back to CodeDog and regenerate before acceptance. Retire construct-specific restart/cache ownership, retaining only algorithms and indexes justified by the resulting semantics.

### 11. Integrate Slipstream and finish retirement

Replace remaining host dependencies on GraphNormalizer/runtime details with the WorldManager contract, including model reads/writes, context and asynchronous startup. Exercise fully expressed state1, definition-derived state1, solved state2 and one-missing-value state2. Check ordinary, whole, overlapping, cancelled and streamed requests.

**Gate:** run retained language coverage and migrated behavior tests through the replacement and report every remaining failure or unsupported feature. Verify actual cube colors and poses, first useful display, stable already-known colors, and final completion in Slipstream. A clean build or a headless normalization result is insufficient evidence of correct rendering.

Remove old execution code and temporary migration adapters once their callers are gone. Confirm that the replacement build has no hidden fallback; substitution of the WorldManager implementation must leave reasoning callers unchanged. Report net production-source changes separately from tests/docs, including code retained only by the old engine. The reference checkout supplies historical comparisons, so dead runtime code need not remain in the replacement branch.

### Checks applied to every increment

Each increment records its chosen semantics, one trace, focused behavior tests, streaming behavior, cost assumptions and code retired. Count relationship inspections, distinct attempts, queue entries, local wakeups and materialized output; include helper work so a shorter visible trace cannot hide repeated scans. Compare growing finite inputs, sparse logical extents and relevant versus unrelated arrivals. Charge payload processing, matching, indexes and memory explicitly.

Do not require the entire old suite to pass before the scalar increment is useful, or treat existing failures as acceptable semantics. Preserve the fixtures and report coverage honestly as it grows. Start with stages 1–2 as the first reviewable implementation milestone, then cut over the engine/test path in stage 2a and establish stage 3 before proceeding to complex constructs. The roadmap remains the order of work; the implementation status below states what has actually been built.


## Implementation status: Proteus Engine cutover

On 2026-10-11, the normal Proteus Engine and TestProteus moved to [WorldManager.dog](../WorldManager.dog). Agent's `normalize`, `normalizeAll`, `normalizeAndWait`, explicit sessions, parser publication and input-stream lifetime use this path. There is no old-solver fallback. The initial standalone TestWorldManager harness has moved into TestProteus and its source has been removed. Slipstream remains a separate migration; no rendering result is claimed.

### Interface and ownership

`Agent.startSession()` registers a session; `normalizeInSession(session, target, requireSize)` submits its request; `endSession()` cancels unfinished requests and removes the session. Ordinary `normalize(target)` uses an Agent-owned default session. Agent closure ends all of its sessions. A foreign or closed session is rejected. Completed handles retain their result and status after engine closure. Request-local frames and waits are released when the request settles or is cancelled. Sharing deductions across sessions remains deferred.

Runnable work drains before normalize returns. `WorldRequest.waitForCompletion()` acquires the model lock to wait for any running publication, then returns even if the request is waiting on live input. Callers inspect `status()`; waiting is not completion. Input streams own parser and normalizer thread lifetime separately and join their parser even when a normalization request is already settled. `normalizeAll` currently has the same supported scope as normalize, including closed ordered-list traversal and composition. Broader depth and streaming policies remain separate work.

### Implemented scalar and closed-list fragment

The fragment is context-free numeric and string leaves, strict supplied relationships including direct exclusion proofs, supported scalar `==`/`=:`/`==:` matching, single-operand positive parenthesized expressions, numeric additive/radix expressions, closed positive string expressions, computed scalar size expressions, direct numeric interval constraints, and closed ordered-list member normalization, concatenation, adoption, strict positional correspondence and finite context-free listSpec application. Each request retains frame cursors and a hash index of encountered context-free source views. Derived member-pair frames hold occurrence-specific combinations separately; they are not written into that source cache. General bindings, qualified applications and shared target constraints require the richer attempt equivalence from D03. Completion and literal consolidation live in virtual POV access. The iterative ready stack processes prerequisites without recursive normalize calls or writes to intermediate infons. Scalar expression frames expose and fold the existing size sequence, then the value sequence, before processing their supplied relationships, retaining the current slot and next operand across parser waits. Closed literal lists expose their applicable listSpec before enumerating the members; template projection and positional correspondence share derived-pair construction. Publication follows the requested target and its supported ordered-list containment scope after their supplied constraints have been checked; identity and expression prerequisites remain read-only. An unchanged partial interval already expressed in the model needs no replacement body. Conflicts and unsupported constructs publish nothing.

Ordinary numeric completion permits unknown size under D02F. A stronger size request stays partial if size is unknown. Completion is tied to its infonView owner, so copying a view cannot copy its completed status to another view. An unchanged completed request creates no new frames. Publication isolates scalar state borrowed from another view. Fully checked closed scalar leaf RHSs leave the pending work list, including redundant unknowns and successful qualified matches. Nested relationships remain until their consolidation is specified; fully checked numeric/string expressions can be discharged when their facts are represented. An ordinary unknown-size string still needs endpoint evidence; a closed string expression counts its represented text. An exclusion template is not a positive value; a negative target stays partial under the current value-oriented completion request.

The printer now shows actual retained expressions and constraints rather than consulting retired solver receipts. The initial cutover exposed consolidation and scalar-representation failures; the scalar follow-up below resolves them against unchanged expectations. A numeric comparison such as `-1 = 9` remains partial while extent-dependent equality is unresolved; a lone `-1` can finish without size.

### Local input continuation and limits

A request waiting for a particular infon to finish parsing subscribes at that infon. Readiness checks, subscription registration, input publication and solving use one recursive model lock, closing the check/subscribe race. Parser reads release it while input may block. Completion wakes only that infon's registered requests, resumes their exact waiting frames, and preserves the earlier relationship cursors and facts. Duplicate or unrelated completion notifications do not replay the prefix. Cancelling a request removes only its own wait; another request waiting on the same input can still finish. Agent owns the session lifetimes throughout.

The input lifecycle tests also exposed a pre-existing parser loop for EOF after `_ = `. A failed RHS parse returned -1 and moved the identity-loop cursor backwards onto `=`. The parser now propagates that failure instead of adding the negative length. The reference executable reproduced the hang under a three-second timeout; the new error and shutdown checks finish. The parser's existing EOF-column convention is unchanged.

This implements parser-completion continuation, not all of roadmap stage 3. General requested-link arrivals, independent alternate routes during a wait, shared in-progress deductions, and cyclic dependencies remain unimplemented. General list correspondence, definitions, general intersections, replacement relationships, qualified negative templates, contextual member access, temporal rules and effects report unsupported. Closed ordered-list members and strict equal-shape correspondence use the traversal and composition described below. Direct scalar prefixes are supported; general list prefixes are not. Streaming list-prefix normalization is not claimed. Planning remains deferred.

### Initial cutover validation (before the scalar follow-up)

Build and run from the replacement checkout:

```sh
codeDog Proteus.Lib.dog
cd LocalBuild
./TestProteus -t unit/worldManager/
./TestProteus -t defaultTests.tst: -T 3
```

The startup banner identifies CodeDog WorldManager. The inherited harness can exit zero on failed tests; inspect its printed failure count. `UNSUPPORTED` and `NOT_PORTED` count as failures, not skips or unchanged-output successes.

Fresh CodeDog generation and compilation of TestProteus succeeded in **127.48 seconds**. The three focused cases pass: **335/335 scalar/session checks**, **14/14 input-lifecycle checks**, and the incomplete-RHS parser regression. The 64- and 128-link cases inspect exactly 64 and 128 relationships, with one root publication each and unchanged intermediate values. Expected negative parse errors appear in the focused output.

The full default run (`./TestProteus -T 3`) executes 1,865 cases with per-case protection:

| Suite | Passed | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| Unchanged default language fixtures | 85 | 228 | 0 | 19 | 332 |
| Compiled cases, including three new cases | 53 | 2 | 1,467 | 11 | 1,533 |
| Total | 138 | 230 | 1,467 | 30 | 1,865 |

The 11 compiled output/assertion failures comprise four `unit/graphText` string-consolidation cases and seven `unit/referenceRegistry` cases. All seven registry failures reproduce in the reference executable. The retained typed-list index test reports unsupported world loading explicitly; its original assertions are preserved for when that prerequisite works. The reference default language run still reports 32 failures out of 332; none of the replacement's 19 scalar-output mismatches occur there.

The separate shared-library build (`codeDog BuildProteusGraph.dog`) also succeeded, in **88.52 seconds**, producing `SharedBuild/libproteusGraph.so`. Its generated source/interface and exported binary contain no old solver symbols. Generated TestProteus source and binary contain no old GraphNormalizer, GraphManager or proteus4 runtime symbols. These results establish the engine cutover and the supported fragment; they do not establish full language, stream-reply or application parity.

All 15 protected `.tst` files keep their original hashes. All 1,530 previously retained compiled registrations remain selectable and are indexed with their original assertion locations in [engine-test-migration.json](engine-test-migration.json). Of these, 63 use the engine/parser/model path and 1,467 report `NOT_PORTED` until their useful assertions are adapted from the reference source. The three new WorldManager cases add scalar/session, input-lifecycle and malformed-RHS coverage. The inventory also records 14 standalone native test drivers awaiting migration; they are not compiled into this engine. The separate streamReplyTest suite has API changes but has not been validated for reply behavior; effects remain unsupported.

The initial default language suite's 19 non-unsupported failures were all new relative to the reference binary. They cover these existing fixture names (under `defaultTests.tst:`):

- `merge/num/literalToUnknown`, `merge/num/typed6`.
- `merge/str/literalToUnknown`, `merge/str/typed2`, `merge/str/typed4`, `merge/str/typed5`, `merge/str/loose`.
- `merge/directLooseStr/exactWidthCopy`, `merge/directLooseStr/exactLiteralAccept`.
- `merge/directUnknown/numNoConstraints`, `merge/directUnknown/strNoConstraints`.
- `merge/directRange/unknownAccept`, `merge/directRange/insideAccept`, `merge/unknown/loose`.
- `tUnknown/simple`, `tUnknown/simple2`, `tUnknown/unknownIsNum`, `tUnknown/unknownIsStr`, `temp/t1`.

These failures were compatibility gaps, not accepted changes to language semantics. The scalar follow-up resolves them with local consolidation and publication rules rather than restoring old solver receipts.

### Scalar compatibility follow-up, 2026-10-11

The historical test fixtures and the Proteus3/Proteus4 implementations supply detailed semantics beyond this architecture document. See D02I for the reference locations and rationale. The scalar follow-up changes virtual scalar merge/publication; it does not change the printer or language expectations.

A checked closed scalar leaf contributes only its type, size and value constraints. After those constraints are represented in the target, remove its work entry even if its value is unknown. Thus `10 = _` yields `10`, `_ = _` yields `_`, and `? = $` yields `$`. The latter two remain partial and do not gain `isNormed`; discharging a redundant constraint is distinct from acquiring a value. A known successful `==` test can also leave the pending work list without adding identity to its candidate. Sources remain unchanged. Nested prerequisites, expressions and unimplemented semantic declarations are not pruned by this leaf rule; `%?` explicitly reports unsupported.

An unresolved `?` retains its any view until type information is acquired. A parenthesized unknown retains its expression body until a value is available; publishing type evidence alone must not turn `(_)` into `0` or `($)` into an empty string.

Strict string value acquisition copies the source's counted/given representation: `*5+$ = 'Hello'` yields `'Hello'`, while `*5+'Hello' = *5+'Hello'` retains its explicit size.

A numeric range match uses the accepted candidate's representation but keeps the target's checked size: `*4+_ == 3` prints `3` and retains internal size 4. Qualified string acquisition uses counted width when known and keeps an unknown endpoint explicit. Known size, displayed size and known value are separate facts. Numeric value acquisition preserves an explicitly unknown source size (`_ = *_+9`), and learning a size preserves an existing literal's given form (`*_+'Hello' = 'Hello'` becomes `*5+'Hello'`). A qualified unknown numeric source with positive known size can supply that size to an unknown target without supplying a value; the result remains partial.

Fresh TestProteus generation/build succeeded in **124.10 seconds**. All **19 previously failing language scalar cases** and **four compiled string cases** now pass without changing their expectations. The three WorldManager cases pass **367/367 scalar/session checks**, **14/14 lifecycle checks**, and the incomplete-RHS parser regression. The chain checks still inspect 64 and 128 relationships once each and publish only their requested root.

At the end of the first scalar follow-up, the retained suite reported:

| Suite | Passed | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| Unchanged default language fixtures | 104 | 228 | 0 | 0 | 332 |
| Compiled cases | 57 | 2 | 1,467 | 7 | 1,533 |
| Total | 161 | 230 | 1,467 | 7 | 1,865 |

The seven remaining assertion failures are the known `unit/referenceRegistry` cases. The separate numeric-literal fixture has 8 passes and 8 unsupported cases; the numeric-range fixture has 6 passes and 14 unsupported cases, with no other failures. Unsupported cases require expression, definition, list, prefix or other semantics outside this slice. All 15 protected fixture hashes remain unchanged.

A 196-case positive-scalar reference audit agrees on 185 printed results. The remaining differences are nine already-known numeric literal `==` tests that Proteus4 leaves pending, and two unknown-string cases where the replacement prints the retained known width explicitly. Proteus3 independently confirms that `9 == 9` resolves to `9`. This comparison exposes reference behavior; it is not a demand to restore the old engine's limitations or erase known size evidence. No protected expectation was adjusted to hide a failure.

The separate shared-library build also succeeds from CodeDog (**88.80 seconds**). Generated executable and library sources/binaries contain no old GraphNormalizer, GraphManager or proteus4 runtime symbols. The parser/printer source is unchanged in this follow-up.

This follow-up adds fourteen production lines to WorldManager, with no additional traversal, graph copy, proof cache or solver fallback. Consolidation uses the prerequisite result already computed by the existing frame. The earlier O(V + E) scalar traversal bound and root-only publication remain unchanged. At that point, prefix relations and complemented constraints remained unsupported, along with arithmetic/concat expressions, nominal types, lists and their compositions. The next follow-up adds the direct scalar prefix/exclusion slice; neither follow-up claims all scalar-related constructs are complete.

### Scalar prefix and exclusion follow-up, 2026-10-11

D02J records the recovered endpoint and exclusion semantics. `*3+$ =: 'Hello'` now acquires `'Hel'` while `$ =: 'Hello'` acquires `*_+'Hello'` and stays partial: the source did not prove a target endpoint. The same rule applies to `==:`. Existing literal prefixes are compared over their common length without overwriting either literal. Numeric prefixes keep the target's own size; `*4+_ ==:*10+3` yields `*4+3`. Whole qualified matching still checks endpoints and available text. Insufficient text, noninteger/negative widths and empty unknown numeric sources do not establish a match.

Direct strict negative constraints now prove, reject or retain the relationship using known scalar facts. `!*_+8 = *16+9` discharges the accepted constraint while remaining `!*_+8`; it does not acquire size 16 or value 9. Equal excluded values and scalar wildcards reject their candidates. Unknown candidate facts remain unresolved. Equivalent exclusions are redundant; different exclusions remain conjoined. Signed numeric comparisons can use local extent evidence without changing either operand. A negative target stays partial because its excluded literal is not a requested positive value.

Six existing compiled tests have been ported: `graphQualifiedText/endModes`, `unknownSize`, `mismatchReuse`, `shortInput`, `invalidWidth`, and `graphNumericRange/endModes`. The text endpoint matrix covers both known/unknown source endpoints and both colon operators plus whole matching. The ports preserve semantic result, size, source and repeat-completion assertions. Old global-subscription and identity-cache assertions are replaced with checks appropriate to Agent/WorldManager. Proven qualified mismatches have the replacement's established rejected status; unknown evidence stays partial. The original case registrations and assertion-source locations remain in the migration inventory. Future link-arrival, cost-policy and predicate/list tests remain visibly NOT_PORTED.

A new scalar-constraint case covers direct exclusion in both operand directions, wildcard/any types, equivalent and unequal exclusions, missing sizes/values, relative negative numbers, nested prerequisites, nested borrowed text prefixes, zero/fractional/oversized widths, and parser arrival/cancellation at the existing relationship cursor. The parser rejects a negative-width surface literal, so that invalid-state boundary check constructs the size through the model API. No parser or printer change was needed.

The final CodeDog TestProteus build succeeds (**131.82 seconds**); the shared-library build succeeds (**89.60 seconds**). Focused checks pass **364/364 scalar/session**, **229/229 scalar-constraint**, **14/14 parser/stream lifecycle**, and **461/461 endpoint** assertions, plus the existing incomplete-RHS parser regression. The production change was unchanged after its first successful build; subsequent edits corrected only new test setup (canonical printed spacing, the parser's negative-width syntax, and setting FlexNum's rational payload instead of invoking its native formatting constructor).

The final full retained suite reports:

| Suite | Passed | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| Unchanged default language fixtures | 112 | 220 | 0 | 0 | 332 |
| Compiled cases | 64 | 2 | 1,461 | 7 | 1,534 |
| Total | 176 | 222 | 1,461 | 7 | 1,866 |

Eight additional protected language fixtures now pass: the two loose-size string cases and six inverted direct-atomic cases. The seven other failures remain the same known reference-registry cases. The numeric-range fixture improves to 7 passes / 13 unsupported; numeric literals remain 8 passes / 8 unsupported, with no other failures. All 15 protected fixture hashes and the reference assertion source hash are unchanged. No original registration was removed. The reference executable independently passes all seven `merge/directAtomic` cases and eight of nine `graphQualifiedText` cases; its remaining failure is the pre-existing cost-policy case, which has not been ported.

Generated executable/library source and binaries contain no old GraphNormalizer, GraphManager or proteus4 runtime symbols. Parser/printer sources are unchanged by this follow-up. Validation logs are in `/tmp/proteus-prefix-negation/`.

Only WorldManager changes in production: 78 added net lines. No new scheduler or attempt table is introduced. Frames add a polarity bit and a represented text length alongside the existing borrowed source. Comparison is linear in the common text prefix (exact comparisons reject unequal lengths first); acquisition does not allocate intermediate strings. Existing chain tests continue to inspect 64/128 relationships once each and publish only the requested root. Parser readiness continues at the saved edge without restarting. The original graph bound remains O(V + E), plus scalar comparison and final payload-copy costs.

At the end of the prefix/exclusion follow-up, qualified negative matching, negative expression templates, arbitrary exclusion conjunctions, general arithmetic/concat expressions, definitions, intersections and lists remained outside that slice. The numeric follow-up below adds finite forward arithmetic and direct interval narrowing. The broader continuation limitations above still apply. There is no old-solver fallback.

### Numeric expression and range follow-up, 2026-10-11

D02K records the operand and range rules. `(2 +3)` now yields `5`, `(*8+4 *4+3)` yields `*32+19`, and nested numeric concatenation reaches the unchanged `*151200+79005` fixture. A finite range plus known offsets supplies exact half-open value bounds. Strict overlap narrows the model: `*10+(*5+_ +2) = *10+(*4+_ +5)` yields `*10+(*2+_ +5)`. The result remains partial and can constrain a later request. Empty and touching ranges reject; qualified partial overlap remains unresolved. A range's width and its outer size remain separate, including under prefix matching.

Numeric operands use the existing frame queue, relationship counter and parser continuation. Local previous/next consistency detects malformed operand cycles. Contextual occurrence checks happen before cached facts are reused. Admitted operands can supply their own checked wrkList constraints; no helper recursively calls normalize or rewrites the operands. A changed interval publishes two canonical model terms at the requested target. A partial body already expressing the same interval/endpoint is retained without allocation, preserving nested grouping and avoiding a gratuitous rewrite. Fully represented numeric expression RHSs are discharged.

Nine retained compiled cases now run through the Engine: `graphNumericRange/direct`, `upperEndpoint`, `intersection`, `touching`, `outerSizeConflict`, `replay`; and `graphFoundation/expressions/numeric`, `nestedNumeric`, `sizeConflict`. The ports preserve numeric values, sizes, interval results, source immutability and repeated-body stability. Proven conflicts use the replacement's established rejected status; old provenance/cache/subscriber assertions are not restored. The original tests and locations remain indexed in the migration inventory.

A new numeric-expression case covers reference probes, exact rational offsets, lower/upper endpoints, empty/touching/disjoint intervals, unknown operands/radices, nested prerequisites, shared sources, prefix endpoint preservation, further narrowing on a new request, atomic failures, malformed operand links, contextual cache reuse, and parser arrival/cancellation. The 33- and 65-operand checks inspect each operand once and publish only their requested numeric result; completion of the last operand preserves the existing frames and edge count. The existing 64/128 identity-chain checks retain their previous bounds.

The final CodeDog TestProteus build succeeds (**132.30 seconds**); the separate shared-library build succeeds (**88.49 seconds**). Focused checks pass **364/364 scalar/session**, **229/229 scalar-constraint**, **186/186 numeric-expression**, and **14/14 parser/stream lifecycle** assertions, plus the incomplete-RHS parser regression. All nine newly ported compiled cases pass.

The full retained suite reports:

| Suite | Passed | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| Unchanged default language fixtures | 115 | 217 | 0 | 0 | 332 |
| Compiled cases | 74 | 2 | 1,452 | 7 | 1,535 |
| Total | 189 | 219 | 1,452 | 7 | 1,867 |

The three newly passing default language cases are `concat/numCat`, `range/join`, and `range/concat1`. The seven other failures remain the same known `unit/referenceRegistry` cases. The separate numeric-range fixture improves to **12 passes / 8 unsupported**; numeric literals remain **8 passes / 8 unsupported**, with no other failures. All 15 protected fixture hashes, the reference assertion-source hash, and all 1,534 previous compiled registrations are unchanged; one numeric-expression case was added. The migration inventory marks the nine semantic ports without removing their original assertion-source locations.

Only WorldManager changes in production, growing from 549 to 747 lines. It adds finite numeric bounds and operand folding at the existing virtual graph boundary, with no new scheduler, reset path, secondary normalizer or old-solver fallback. Generated executable/library sources and binaries contain no old GraphNormalizer, GraphManager or proteus4 runtime symbols. Parser/printer sources are unchanged by this follow-up. Reference probes, build logs and validation results are in `/tmp/proteus-numeric-expressions/`.

At the end of the numeric follow-up, string/list concatenation, computed size expressions, inverse arithmetic, non-contiguous numeric possibilities, qualified negative templates, general definitions/intersections/lists, and general requested-link arrivals remained later work. The string follow-up below adds closed positive text concatenation. Arithmetic involving an unknown offset/radix stays partial. This does not claim the complete scalar-related language or general stream propagation is finished. Planning and cross-session sharing remain deferred.


### String expression follow-up, 2026-10-11

D02L records the recovered string rules and representation choice. `('Hello' ' world')` and nested text expressions now normalize through the existing WorldManager operand frames. Prefixes can cross operand boundaries, and a prefix used as a later operand contributes only its represented text. Source operands and constraints remain unchanged. Conflicting outer sizes or supplied values reject atomically; unknown operands remain partial. A closed expression counts the available bytes, including an operand with an explicitly unknown size, while a bare unknown-size string still requires endpoint evidence.

String facts now share borrowed leaves and concatenation descriptors. Each join stores references and lengths; no intermediate string is assembled. The same iterative cursor compares represented text and emits the final requested payload. Whole comparisons check lengths first; prefixes bound traversal to the needed text. The new publication counters record emitted bytes and visited descriptors. Empty text, repeated/shared operands, different text-piece boundaries, exact matches, exclusions, UTF-8 byte widths, nested prefixes, and retained streaming continuation are covered.

Three retained compiled cases have been ported: `graphFoundation/expressions/strings`, `graphFoundation/expressions/rhs`, and `graphText/concatScalarSource`. Their value, endpoint, completion and repeated-result assertions use the Engine; source immutability and publication checks replace dependence on the old expression provenance. Original registrations and assertion locations remain indexed. A new WorldManager string-expression case exercises the semantic and cost boundaries; the earlier temporary unsupported-string probe has moved from the numeric case into that supported string coverage.

The initial deeply nested stress input omitted the optional `|`, causing repeated speculative parsing of its first member as a possible listSpec. The user clarified that a leading `|` explicitly denotes no listSpec: `(| first second)`. Using it at each nested level makes the 32/64/128-leaf fixtures parse and normalize in **0.01 seconds total** in the replacement executable (**0.07 seconds** in the reference executable). These are observed fixture-run times, not a general performance bound. The stress tests now parse those inputs through the real parser instead of constructing the model directly. Unmarked deep inputs still exercise the ambiguous syntax, but no parser change is needed for this normalization test. The fixtures and timing logs are in `/tmp/proteus-explicit-empty-listspec/`.

The final CodeDog TestProteus build succeeds (**138.08 seconds**), as does the separate shared-library build (**95.80 seconds**). Focused checks pass **364/364 scalar/session**, **229/229 scalar-constraint**, **181/181 numeric-expression**, **235/235 string-expression**, and **14/14 parser/stream lifecycle** assertions, plus the incomplete-RHS regression. All three newly ported compiled cases pass. The string tests include exact length overflow from a shared input with only 65 distinct infonViews, which reports an error without publishing or allocating the expanded payload.

The full retained run has no timeouts and reports:

| Suite | Passed | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| Unchanged default language fixtures | 117 | 215 | 0 | 0 | 332 |
| Compiled cases | 78 | 2 | 1,449 | 7 | 1,536 |
| Total | 195 | 217 | 1,449 | 7 | 1,868 |

`concat/simpleStr` and `concat/strCat` are the two additional passing language cases. The seven other failures remain the same known reference-registry cases. All 15 protected fixture hashes, the reference assertion-source hash, and all 1,535 previous compiled registrations are preserved; one new string-expression case is registered. No expected language output was changed.

For the nested strings parsed with explicit empty listSpecs, 32/64/128 nonempty leaves take 94/190/382 facts frames and 93/189/381 relationship inspections, including the deliberately inserted empty operands. They emit only 32/64/128 bytes at the requested target. The 33- and 65-operand parser-continuation cases inspect each operand once, retain the existing frames while waiting, and publish once; cancelled requests emit nothing. Whole-string, differently segmented, and prefix comparisons share the same bounded text cursor.

Production changes are confined to WorldManager, growing from 747 to 822 lines. No second solver, new scheduler, restart path or intermediate text publication is introduced. Generated executable/library sources and binaries contain no old GraphNormalizer, GraphManager or proteus4 runtime symbols. Validation logs and the parser investigation are in `/tmp/proteus-string-expressions/`.

The subsequent leading-`|` correction retains all solver work bounds and adds three deep-parse assertions. The rebuilt TestProteus completed successfully in 131.78 seconds. The string case passes 238/238 assertions and also passes with the three-second crash-protection limit. These correction-run logs are in `/tmp/proteus-explicit-empty-listspec/`.

Computed size expressions, list concatenation, inverse solving, text/list matching, definitions, intersections and general link arrivals remain unfinished. No parser/printer, CLI selection or protected fixture changes are part of this work.

### Computed size follow-up, 2026-10-11

D02M adds computed scalar sizes through the existing operand traversal. `*(10)+5` yields `*10+5`; nested arithmetic, numeric radix formation, computed-size operands, strings, signed displacement, numeric ranges and qualified supplied relationships compose with the new endpoint. Unknown or unsupported endpoint values remain expressions. A known numeric value does not bypass an unresolved supplied size expression. General lists and inverse solving remain outside this slice.

A frame advances through size operands, value operands and wrkList entries. It shares checked operand facts between the slots when the source view and admitted context permit it. No extra model node or separate evaluator represents the size slot. Publication creates a numeric size state only at the requested target and retains source operands unchanged. Conflicting prerequisites or values publish nothing. A computed endpoint accompanying an already expressed partial range is still materialized; an unchanged partial value body alone must not suppress that new endpoint.

The retained `graphFoundation/expressions/computedSize` and `negativeSize` cases now use Agent.normalize, with their original semantic expectations. The former sizeExpression provenance assertion is replaced by source immutability and completed-request checks. `computedSubscription` remains NOT_PORTED because it depends on general later identity arrivals, not parser completion. The new `worldManager/computedSizes` case tests both slots, atomic failure, source preservation, nested sizes, shared operand facts and parser continuation.

The TestProteus CodeDog build succeeds in **131.20 seconds**. The separate shared-library build succeeds in **92.06 seconds**. Focused checks pass **188/188 computed-size**, **364/364 scalar/session**, **229/229 scalar-constraint**, **181/181 numeric-expression**, **238/238 string-expression**, and **14/14 parser/stream lifecycle** assertions, plus the incomplete-RHS regression. All 19 reference probes passed on the preserved Proteus4 implementation; their cases also pass in the replacement's focused coverage.

The 35/67/131-operand continuation cases inspect exactly 35/67/131 relationships and publish once, despite waiting first at a size operand and then at a value operand. Deep 32/64/128-level computed-size chains use 33/65/129 frames and 32/64/128 relationship inspections, with one publication and no intermediate writes. These parser-to-solver inputs explicitly use empty listSpecs (`|`) at each nested level. Cancelled or failed size input publishes nothing.

The full retained run has no new failures or timeouts:

| Suite | Passed | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| Unchanged default language fixtures | 119 | 213 | 0 | 0 | 332 |
| Compiled cases | 81 | 2 | 1,447 | 7 | 1,537 |
| Total | 200 | 215 | 1,447 | 7 | 1,869 |

The newly passing prior cases are `merge/calcSize`, `merge/calcNegativeSize`, and the two ported computed-size cases. The seven other failures are the same known reference-registry cases. All previous compiled registrations, all 15 protected fixture hashes and the reference assertion-source hash are preserved; the new computed-size case adds one registration.

Production changes are confined to WorldManager, from 822 to 841 lines. They replace two operand-stage booleans with the current expression slot, share expression-shape validation, and route both slots through the same folding operation. Parser/printer, CLI behavior and protected language fixtures are unchanged. Logs and reference probes are in `/tmp/proteus-computed-size/`. List concatenation, inverse solving, text/list matching, definitions, intersections and general requested-link arrivals remain unfinished.

### Closed ordered-list follow-up, 2026-10-11

D03E adds ordinary closed ordered-list members through the same retained cursor and frame stack as expressions. `{_ = 3 $ = 'hi'}` yields `{3 'hi'}`, and `{{1 2} {} {_=3} _=4}` yields `{{1 2} {} {3} 4}`. Existing physical list states, member POVs, parent links and infonView identities remain intact. Known scalar facts are published to the requested members; their RHSs, size operands and value-expression operands remain unchanged.

An unresolved member does not block independent siblings. `{(2 +_) _=5}` becomes `{(2 +_) 5}` and remains partial. The partial status returns through the current frame stack, without restarting any goal or treating unchecked partial facts as established. A parser wait retains its cursor, earlier member frames and prerequisite results. Actual contradictions and unsupported required routes still publish nothing for the request.

Completion combines the immediate member count with each member's completion. An absent count-derived size can be counted; `*_+{1 2}` retains its explicit unknown size. A supplied size that differs from the stored count remains unchanged and partial, even when all stored members complete. Nested ordered lists count as single members at their parent's level. Unordered lists, listSpecs, embedded/sparse members and list-to-list correspondence remain unsupported; no unordered depth policy has changed. List concatenation remains a following increment, after this member-normalization foundation.

Four retained compiled cases now use Agent.normalize: `graphFoundation/closedList`, `givenUnknownSize`, `givenSizeMismatch` and `innerOuterSize`. They preserve the original semantic assertions, including member completion and the actual given/count size state. The temporary unsupported probes for `{1 2}` and `{(2 +3)}` move into supported list coverage. The new `worldManager/listMembers` case checks identity preservation, partial siblings, source immutability, atomic failure, size distinctions, parser continuation, cancellation, shared views and malformed member links.

The final regenerated TestProteus build succeeds in **141.82 seconds**. The separate shared-library build succeeds in **98.09 seconds**. Focused checks pass **194/194 list-member**, **361/361 scalar/session**, **229/229 scalar-constraint**, **176/176 numeric-expression**, **238/238 string-expression**, **188/188 computed-size**, and **14/14 parser/stream lifecycle** assertions, plus the incomplete-RHS regression. The two formerly unsupported probes account for the reduced scalar/numeric assertion totals and now have supported list assertions. All 18 corrected reference probes pass. The initial test build found an incorrect native access to sizeMode; the next test run exposed a duplicate identity in the arrival fixture and the printer's symbolic representation of repeated views. Those test setup/assertion corrections required no production changes.

For 32/64/128 scalar members each with one supplied identity, the request inspects exactly 64/128/256 relationships, creates 65/129/257 frames and publishes the 33/65/129 requested views. Waiting at the last member preserves the earlier frames; arrival adds only that member's newly parsed identity. For 32/64/128 nested ordered lists around `_ = 5`, the request creates 34/66/130 frames, inspects 33/65/129 relationships and publishes 33/65/129 requested views. Source operands are not filled. Both deep fixture families use explicit empty listSpecs (`|`).

The full retained suite reports no new failures or timeouts:

| Suite | Passed | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| Unchanged default language fixtures | 122 | 210 | 0 | 0 | 332 |
| Compiled cases | 86 | 2 | 1,443 | 7 | 1,538 |
| Total | 208 | 212 | 1,443 | 7 | 1,870 |

Seven previous cases now pass: `concat/twoNums`, `parse/innerID`, `trav/list/withEmpty`, and the four ported foundation cases. The seven other failures remain the same known reference-registry cases. All prior registrations, all 15 protected fixture hashes and the reference assertion-source hash are unchanged; one new list-member case is registered. Generated executable and library sources contain no old GraphNormalizer, GraphManager or proteus4 runtime calls.

The production change is confined to WorldManager, from 841 to 899 lines. It reuses the existing cursor and ready stack for solving and publication, with one shared scalar publication routine. No parser, printer, CLI or protected fixture changes are included. Logs and reference probes are in `/tmp/proteus-list-members/`.

### List composition follow-up, 2026-10-11

D03F adds closed ordered-list concatenation, adoption and strict positional correspondence through the same checked member sequences. `({1 2} {} {3 4})` becomes `{1 2 3 4}`; nested concatenations join without copying intermediate prefixes. `{_ _ _} = {_ 2 _} = {1 _ _} = {_ _ 3}` becomes `{1 2 3}`. Source/target concatenations, nested list members, computed scalar members, transitive sources, and size/value information supplied by separate lists compose through the existing merge operation.

A closed shape is checked before member pairing. Each pair gets a result frame separate from the cached source facts; its scalar fields use the existing scalar merge and its nested lists use retained correspondence cursors. The target's previously checked member wrkList is not reread. Known numeric members can finish with unknown numeric size under D02F. Member completion and the parent's endpoint are separate: `*1+(*_+{1})` completes with the target's endpoint, while `? = *_+{1}` preserves the unknown endpoint and stays partial. A conflicting operand endpoint cannot be hidden by concatenating another unsized operand.

The publication pass prepares its destinations before writing. Existing literal targets keep their physical list body, infonViews, POVs and parent/previous/next links. Adoption and concatenation allocate independent destination occurrences, including nested members and repeated uses of one source. Borrowed source lists, expression operands and transitive intermediate views remain untouched. Partial ranges, exclusions and unknown members retain their actual facts; unresolved member expressions remain expressed rather than becoming empty output. Incompatible projections into a shared target infonView are unsupported before any publication; joint constraints for that case remain later work.

The request keeps source cursors and checked frames while a member finishes parsing. Arrival continues at that member, then proceeds through the necessary pairs; it does not replay the earlier list prefix. Cancelled waits publish nothing. General link arrivals, progressive open-prefix publication, sparse/embedded spans, listSpecs, nominal or unordered correspondence, qualified list matching, definitions, intersections and inverse solving remain unfinished.

Twenty retained compiled cases now run through Agent.normalize: twelve correspondence cases, two list-expression cases, and six adoption cases. They retain the value, size, shape, occurrence, completion and source-immutability assertions. Old identity-registry receipts and scheduler metrics are replaced by request/pair checks. The retained bare-number case keeps its payload and unknown-size assertions and follows accepted D02F for completion instead of the reference engine's old size requirement. All inherited registrations and protected language expectations remain unchanged. Three formerly unsupported list probes move from the member suite into the new composition suite.

The fifteen closed-composition language probes all pass on the reference engine and the replacement. Four additional exploratory partial-adoption probes are not reference parity claims: the old engine prints open remainders plus RHSs for range/exclusion adoption, prints an empty list for an unconstrained member, and counts an explicitly unknown adopted endpoint. The replacement checks preservation of those explicit partial facts and the existing D03E endpoint policy. These are new checks, not changed protected expected outputs.

The final regenerated TestProteus build succeeds in **141.82 seconds**; the separate shared-library build succeeds in **99.90 seconds**. Focused checks pass **222/222 list-composition**, **176/176 list-member**, **361/361 scalar/session**, **229/229 scalar-constraint**, **176/176 numeric-expression**, **238/238 string-expression**, **188/188 computed-size**, and **14/14 parser/stream lifecycle** assertions, plus the incomplete-RHS regression. The member-suite total decreases only because its three former unsupported probes now have supported composition coverage.

The full retained run has no new failures or timeouts:

| Suite | Passed | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| Unchanged default language fixtures | 131 | 201 | 0 | 0 | 332 |
| Compiled cases | 107 | 2 | 1,423 | 7 | 1,539 |
| Total | 238 | 203 | 1,423 | 7 | 1,871 |

Twenty-nine previous cases now pass: the twenty ports and nine unchanged language cases, including `concat/simpleList`, `concat/listCat`, `merge/list/nums`, `merge/list/multiID` and list adoption/type rejection. The seven remaining other failures are the same known reference-registry cases. All 1,538 prior compiled registrations remain, with one new composition case. All 15 protected fixture hashes and the reference assertion-source hash are unchanged. Generated executable/library sources contain no old GraphNormalizer, GraphManager or proteus4 runtime calls.

For 32/64/128 members, whole-list correspondence inspects 65/129/257 structural/supplied relationships and makes exactly 32/64/128 member pairs. Its two source cursors and publication cursor each visit one flat sequence node. Nested concatenation fixtures with 32/64/128 output members use 126/254/510 frames, 125/253/509 relationship inspections and 63/127/255 sequence-node visits; they publish only the 33/65/129 requested result occurrences. Deep ordered nesting, source reuse, source immutability, local parser waits and cancellation also pass. The D03F bound separately accounts for copying any retained constraint vectors; these exhausted-leaf measurements have none.

A shared input with 66 distinct views reaches the signed-64-bit sequence-length guard without allocating the exponential output. This test exposed a real narrowing bug: the native FlexNum equality overload took a 32-bit integer, converting a 2,147,483,648-member sequence count to -2,147,483,648. List extent comparisons now explicitly construct an exact FlexNum from the 64-bit count. The debugger also confirmed that combining an explicit target endpoint with an unknown source endpoint must not inherit source incompleteness. The resulting fixes remain in CodeDog, without changing the external CodeDog runtime. Test setup corrections preserve the parser's actual parent links and invalidate a counted header when the synthetic stress fixture replaces its parsed operands.

Production changes are confined to WorldManager, from 899 to 1,088 lines. The change reuses the scalar merge, frame stack, parser continuation and publication machinery, adding borrowed list sequences and pair cursors. No parser, printer, CLI or protected fixture change is included. Both design documents and the compiled-test migration inventory are updated. Validation and debugger evidence are in `/tmp/proteus-list-composition/`.

### Retired code and cost

Removed GraphFoundation.Lib.dog and 44 graph runtime headers: **45 files, 12,569 lines**, verified against the reference checkpoint before deletion. The remaining production changes in this cutover add a net 72 lines, including WorldManager's parser continuation and locks, for a net production reduction of **12,497 lines** relative to the initial scalar/session slice. Tests and documentation are excluded. Generated test and library sources must contain no GraphNormalizer, GraphManager or proteus4 runtime calls. Test assertion recovery locations and retired-file hashes are in the migration inventory.

For a closed scalar graph with V distinct views and E supplied relationships, retained progress and expected hash/queue work are O(V + E), plus numeric and payload costs. String evidence keeps a source reference until publication rather than copying each intermediate string. A parser completion performs an expected constant-time waiter lookup plus work for the affected requests and their newly enabled steps. Cancelling one wait filters the subscribers at that input; Agent session ownership checks scan the small session list. These bounds do not extend to future ambiguous list or rule matching.

### Finite listSpec follow-up (2026-10-11)

D03G applies context-free listSpecs to the existing immediate members of closed ordered lists. `{*10+7| *10+_ *10+_}` becomes `{*10+7 *10+7}`; `{*10+_| _ _} = {2 3}` becomes `{*10+2 *10+3}`. The template is evaluated once, and each member receives a derived strict relationship using the same pair construction and merge operations as correspondence. Nested templates, concatenated sources, numeric/string expressions and independent member values compose through the existing frame stack. Member-source and template infons remain unchanged; each requested target occurrence retains its identity.

The structural cursor exposes a listSpec before the list's first member, retaining that prerequisite through a parser wait. A later member wait keeps all earlier projections. Parent correspondence reuses projected member facts. A closed empty list has no template applications. Publication removes a listSpec once every finite contribution is represented and the known endpoint agrees with the member count; explicitly unknown endpoints retain it. Sparse expansion, ancestor inheritance across embedded spans, contextual/nominal templates and expression-level listSpecs remain unsupported. General partial-template constraint solving is not claimed: an unresolved template prerequisite remains partial.

The regenerated TestProteus build passes in **137.95 seconds**. The separate shared-library build passes in **92.07 seconds**. The new suite passes **201/201 assertions**, and all ten WorldManager suites pass. The retained `graphTemplates/closedLiteral` case is ported through Agent.normalize with its original member count, values, endpoint and completion assertions, plus source and occurrence checks (**7/7**). Nine finite-template language probes pass on both the preserved Proteus4 executable and the replacement. Computed-template folding is additional composition coverage: the reference leaves `{(2 +3)| _ _}` unresolved while the replacement produces `{5 5}`; no protected expectation is changed.

For 32/64/128 members sharing a computed template, relationship reads are **35/67/131** and template applications are exactly **32/64/128**. There is one template evaluation, one derived merge per member, and publication only into the requested list and its members. Existing scalar/list suites retain their prior passing assertion counts.

The full suite reports **1632 failures / 1872 tests**, with **no new failures** relative to the list-composition baseline:

| Suite | Passed | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| defaultTests.tst | 131 | 201 | 0 | 0 | 332 |
| Compiled cases | 109 | 2 | 1422 | 7 | 1540 |
| Total | 240 | 203 | 1422 | 7 | 1872 |

One retained case newly passes, and the new suite adds one registration. The seven other failures remain the existing reference-registry cases. All prior 1539 compiled registrations, all 15 protected fixture hashes and the reference assertion-source hash are preserved. The implementation adds **47 production lines and removes 9 (net 38)** in WorldManager; no parser, printer or old solver is involved. Logs and reference fixtures are in `/tmp/proteus-list-spec/`.
