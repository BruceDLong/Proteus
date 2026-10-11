# Proteus4 normalization design

Proteus4 represents semantics through graph relationships and justifies semantic optimizations through theorems. This specification consolidates settled choices for normalization. The [issues and decision log](/home/bruce/devl/Proteus4/graph/normalization-design-decisions.md) holds alternatives, reasons, evidence, and unresolved questions.

Status on 2026-10-11: the WorldManager and virtual graph boundaries, CodeDog policy, core D02 lifecycle, and D03 list principles below are settled. Goals acquire requested facts, stop when satisfied, and continue at waiting links without restart. List scope and shared propagation are specified; exact graph operations, matching records, and subsequent intersection/definition/rule composition remain open. Planning design stays deferred until intersections and definitions have been considered. The [architecture audit](/home/bruce/devl/Proteus4/graph/normalization-framework-audit.md) remains supporting evidence and a source of proposals.

## Design objective

Semantic relationships must compose through the reasoning framework. A justified optimization may provide a cheaper way to establish the same result, with its premises and validity explicit. Native algorithms may still be needed; their boundary with graph-expressed rules is to be settled. Merely consolidating implementation classes does not establish this capability. See decision G01 and issues D06–D09.

Streaming behavior and computational cost are part of every operation's contract. Complexity statements identify the workload, relevant graph size, virtual extent, candidate count, changes, and output. They distinguish initial work from incremental updates and memory retention. No general linear-time claim is made for arbitrary inference.

## WorldManager reasoning interface

The `WorldManager` class in `WorldManager.dog` owns the complete reasoning interface. Substituting a differently implemented `WorldManager.dog` must permit replacing the reasoning system without changing reasoning callers. Internal helpers can support the implementation, but callers must not depend on its scheduler or graph implementation details. The exact methods and lifecycle contract will be specified as the solving design develops. See G03.

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

In `A=(B=C)`, C belongs to B's wrkList. Rejection at B can block that route to C. Qualified relationships such as `==` can require resolving the B/C relationship first; C's type may not be directly applicable to A. The virtual graph must preserve this dependency rather than expose an unjustified shortcut from A to C. Executable examples must confirm the parser's actual relationship structure.

The interface removes fully merged pending work, not meaningful qualifications, contexts, or identities required by other uses. Stopping a goal and consolidating a work entry are separate operations. Neither requires deleting a source infon that other POVs still reference. Exact consolidation rules remain to be specified for each relationship.

### Streaming continuation

Ordinary progress reveals information about immutable infons. When an edge is promising but its information is being parsed or normalized, retain the local continuation and subscribe at that link. Other work or admissible alternative routes may proceed. A new edge or newly available fact continues affected work at that point.

The graph manager provides no clear-and-restart operation for a live goal. It retains progress across ordinary publications; a method named “continue” must not secretly discard the goal's work and replay it. A satisfied goal remains finished. Exceptional semantic changes explicitly create a new goal; their triggers belong to the relevant language rules.

### Cost and remaining interface work

For k constant-cost compatible links with the value known only at the end, search costs O(k) link inspections but need not publish k intermediate copies. If the first usable source already satisfies the goal without additional premises, further chain traversal is unnecessary. Include payload-copy and compatibility costs separately.

Measure local subscription delivery, newly exposed work, memory, and sparse allocations. The design must not turn each arrival into a scan from the goal's root. Arbitrary rule search has no general linear bound established here.

Still to specify: the exact goal requirements and `isNormed` scope within the numeric rule above, edge readiness and consolidation operations, local continuation/subscription delivery, shared ownership, and coherence-mode reporting. Planning remains deferred. These details must implement the accepted lifecycle rather than reintroduce restart behavior.

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
