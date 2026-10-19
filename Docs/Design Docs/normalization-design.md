# Proteus4 normalization design

Proteus4 represents semantics through graph relationships and justifies semantic optimizations through theorems. This specification consolidates settled choices for normalization. The [issues and decision log](normalization-design-decisions.md) holds alternatives, reasons, evidence, and unresolved questions.

Status on 2026-10-12: the WorldManager and virtual graph boundaries, CodeDog policy, core D02 lifecycle, and D03 list principles below are settled. Goals acquire requested facts, stop when satisfied, and continue at waiting links without restart. Member, span, whole-list, intersection and dot/index consumers now share correspondence requests, logical layouts and predicate comparison. Selected text parsing, nominal record-field meaning application and known-endpoint selection are supported as specified below. General definition, sparse-pattern and rule composition remain open. Planning design stays deferred until intersections and definitions have been considered. The architecture audit (`Historical/normalization-framework-audit.md` in Git history) remains supporting evidence and a source of proposals.

## Design objective

Semantic relationships must compose through the reasoning framework. A justified optimization may provide a cheaper way to establish the same result, with its premises and validity explicit. Native algorithms may still be needed; their boundary with graph-expressed rules is to be settled. Merely consolidating implementation classes does not establish this capability. See decision G01 and issues D06–D09.

Streaming behavior and computational cost are part of every operation's contract. Complexity statements identify the workload, relevant graph size, virtual extent, candidate count, changes, and output. They distinguish initial work from incremental updates and memory retention. No general linear-time claim is made for arbitrary inference.

## Replacement development

Develop the solver on `main` in `/home/bruce/devl/Proteus`. D084 promotes the cleaned redesign with its history intact; `proteus3b` retains the previous engine and `codex/normalizer-redesign` retains the imported redesign checkpoint. The failed Proteus4 checkout and branch were removed under D072. Checkpoint `b52322a867d2be2c1fda186929b05dc0c70e8be4` remains an ancestor for implementation and diagnostic reference through `git show`. See G06 for the original setup and D072 for its cleanup.

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

These constraints are carried forward from the historical repository instructions (`git show b52322a:AGENTS.md`), including the recorded user clarifications. They establish the starting semantic contract. The newer D02 decisions govern default completion and continuation instead of inherited restart behavior, including D02F's explicit permission for numeric completion with unknown size; D03 specifies list expansion scope and shared propagation. Other changes require an explicit decision in the log.

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

Definition applications must not escape their own scope and modify outside
occurrences (D098). This is distinct from preserving the shared declaration.
D099 establishes explicit external references and embedded parent context. Whether
passed or global references may also be modification targets inside a definition
remains open; the assertion scope of an agent interaction does not automatically
establish identical modification permissions for a word definition.

### Participant, time-slice and session context (D099)

The following is an accepted design baseline, not an implementation-status claim.
Definitions have class/function-style scope: no implicit access to their
data-structure parent, while parameters and explicit supplied global/context
references provide outside connections. Parameter names cannot be reserved words.
Arguments resolve in the caller's scope; the definition body uses its own local
parameter bindings. A stable referent does not imply a state frozen at call entry.

In `{ A {b c} E }`, b has no local predecessor; an outer listSpec may relate the
nested member's b to A's b, or the nested definition may use an explicit argument
or supplied global reference. Embedded lists inherit surrounding list context:
in `{ A &{ B C D } E }`, B's predecessor is A and E's predecessor is D. The same
logical-neighbour rule applies to named embedded words. An embedded listSpec can
refer to its parent without extending its own coverage to the parent's other
members. `%prev`/`%next` remain unavailable in unordered scope.

Ordinary references, including `%`, `%self`, `%prev`, `%next` and parameter
referents, describe states at the common S-list time-slice unless another time
or span is explicit. T-list L-values address the next state under the existing
relative assignment convention. Obtaining intermediate or parallel histories
at a different level requires converting the S-list to a T-span at that level;
the conversion notation and detailed correspondence remain open.

`%ctx` is initially a session-scoped, recency-ordered collection of model references
established by explicit agent interactions. It works in ordinary assertion lists
as well as T-lists. Assertions resolve references in source order and contribute
their resolved referents for subsequent assertions; this order does not advance
modeled time. Selection uses ordinary model matching. Definition-internal reads
do not populate the session context. Resolved references stay bound when context
changes, without freezing their state values. Another list does not clear the
session; unordered assertions cannot derive recency from evaluator scheduling.

The `%`-as-current-member versus `%self`-as-model-scope distinction remains the
working proposal to revisit with substantial temporal models. Concrete examples,
the bike/planet assertion list and remaining boundaries are maintained in
[word-application-and-time-cases.md](word-application-and-time-cases.md).

### Finite scalar alternative domains

Under D04A, `*_+[...]` denotes the remaining scalar alternatives, not a request to choose the last list member. Borrow each finite candidate and normalize its own conjunctive work through an ordinary frame. An unpatterned open body supplied by `<~` enumerates the finite input domain. Reuse derived relationship pairs to filter candidates; an RHS OR preserves the original operator, while an outer constraint on a target OR uses member qualification. Only a proved contradiction removes a branch. A contradiction in an ordinary required dependency still rejects that dependent goal.

Retain unknown candidates and distinct viable conjunctions. Collapse one survivor, or several provably equal complete scalar answers, to ordinary facts. Store multiple answers as a partial `*_+[...]` through staged publication with independent output occurrences. The outer value remains `emIntersection`, the inner list remains literal with unknown size and `fromGiven`. Borrowed candidates remain immutable, and no result publishes if required branch evidence would be lost. A later goal may constrain a published partial domain further. A completed scalar answer retains the usual no-new-goals fast path.

Cursor, candidate and pair state survive local parser waits. Domain headers must be ready before classification; a symbolic open tail does not establish exhaustion. Scalar filtering is O(N) pairs plus candidate work and payload comparisons; two ORs can require O(NM) pairs and survivor records. Nested OR flattening has additional branch-reference costs. General branching is not claimed linear. The first implementation evaluates a finite domain before filtering an external constraint; demand-pushed enumeration and early existential success remain optimization work. Structured alternatives, general patterned input, OR-dependent expression/template/chart evaluation and unresolved layout obligations stay explicit boundaries. Planning, path costs and `align4b` remain deferred.

### Open results and bounded selected spans

Under D04B, a plain unbounded `[...]` with no body members, listSpec, input, given extent or body relationships contributes an unconstrained result. Process its outer RHSs through ordinary merging. Qualified list adoption is admissible when the target has no type, extent or contents to test; retain top-only size qualification, so `[...] ==: {1 2 3}` publishes `*_+{1 2 3}` as partial knowledge. A bare open selector supplies no answer and remains expressed. This does not alter finite one-of classification.

A marked embedded occurrence denotes the span itself. The virtual structural cursor can expose that wrapper instead of flattening it away. For a span with an established nonnegative extent, acquire its corresponding source window as one borrowed list RHS. Walk the prefix and window once, skip justified whole widths, normalize only the requested source members, and reuse ordinary list correspondence and staged publication. Preserve occurrence paths, source listSpecs, source endpoint limits and relationship qualification. Source layout relationships that have not been established prevent use of their coordinates.

Assertions need structural correspondence; `<~` additionally needs pattern acceptance. The bounded path checks explicit prefix/suffix predicates through the common matcher, including finite ordered structures. Members are width-one captures. At a variable-width event it continues the existing chart with the remaining cursors and any established capture. Counted repeated predicates, unresolved selected widths and enclosing target-template projection remain explicit boundaries. It does not guess an offset or accept an unchecked suffix. Existing scalar-pattern charts continue to handle their established cases. The selected span's own listSpec and member constraints use ordinary list merging.

Retain both cursors and the current candidate through parser waits. A numeric width is not an instruction to allocate or enumerate that many virtual positions. For a fixed nesting depth, bounded correspondence costs O(V + K), where V is inspected represented layout and K is demanded source-member work, plus ordinary merging and output costs. Occurrence-path comparisons and ancestor checks add depth-dependent work; arbitrary nested shapes are not claimed globally linear. No persistent parent-progress index or planning policy is introduced.

### Path reads and writes as correspondence consumers

`WorldPOVGraph.correspondence` constructs the common request/result contract. A proved finite positional pairing uses the linear `correspond` path; variable patterns use the chart. Both belong to the same operation and ordinary member merging. The separate positional member walker has been removed. Logical access exposes literal, embedded, concatenated and bare named layouts without normalizing unrelated values. A list supplied by a path resumes its requesting layout cursor at the selected occurrence.

Dot lookup lowers to the existing first-match pattern: zero or more proved nonmatches followed by one positive capture. Bracketed positions and spans lower to the existing positional/marked selectors. Item indices are one-based; slice indices are zero-based boundary counts. Negative coordinates use the proved source endpoint. These consumers preserve qualifiers and occurrence ancestry; they do not implement a second name search or matcher. Finite structured predicates recurse through the same fact comparator. Distinct nominal types are not made equal by matching structure. Negative predicates require a proof; an unresolved earlier candidate cannot be skipped as a mismatch.

Intermediate path steps can request only a reference. A final read acquires the selected facts; it need not fully normalize every containing list. A retained parent returned after a write has its own ordinary normalization obligations, including definitions. `WorldRequest.selectedSource()` exposes a unique selected POV after publication; span provenance retains the original member positions. Refining a captured value preserves that source mapping.

Writes add constraints to those proved occurrences. The request stages checked facts; subsequent path steps see them, but model publication waits until all requested work and destinations are checked. Conflict, unsupported work or cancellation publishes none of the staged edits. Returned lists get their own result containers; retained literal parents preserve their existing POVs. Shared definition bodies are borrowed read-only until instance creation is implemented. This is assertion-based updating; force-replacement is not implemented.

The demand and all cursors survive local parser waits. Named-layout borrowing, field chains and fixed spans reuse the same source positions. Work is proportional to the inspected structure, matching pairs and requested output, with depth and ambiguity costs still explicit. Indexed traversal and finite captures share the same known-list-width proof; scalar lengths and unchecked predicates cannot justify a skip. These skips do not enumerate their virtual extent. A reference-only layout demand also avoids value-normalizing unused suffixes. No planning, path costs or persistent parent index are added.

Remaining boundaries include unordered structured matching, fields requiring inheritance or more general definition composition, virtual positions inside sparse spans, counted repeated pattern predicates, unresolved layout identities, boundary and other path providers, non-list slice specifications, and definition-instance writes. An empty match cannot prove an unknown expression's result type; such a source remains pending until its list type is established. Negative indices require a proved endpoint. These boundaries are reported, never replaced by old-engine output.

### Temporal semantics

**Reference status (D063).** The preservation rules below are proposed semantics, not implemented behavior. The user now gives historical main-branch behavior precedence. The literal Proteus `main` snapshot (`86321ea`) does not infer values from T-list adjacency and does not parse `%?` as a single independence form. Resolve the historical reference before adding implicit preservation; explicit temporal relationships and contextual templates retain their validated behavior.

T-list adjacency supplies preservation of otherwise unchanged values unless the model establishes an applicable change relationship. An ordinary later value assertion does not itself establish that exception. With earlier `{x:1, y:2}` and later `{x:9, y:_}`, preservation of x conflicts with x:9 and the whole state constraint fails; the engine must not accept x:9 as an override while preserving only y. A later part reference targets the later occurrence and leaves the earlier occurrence intact. Ordinary non-temporal adjacency does not supply this preservation relationship. See D062.

`%?` withholds default preservation for the whole value at its scope. On a compound value, this includes its components: descending to a component must not bypass the containing value's independence. Scoped to x alone, it does not remove preservation for sibling y; scoped to the containing record, it withholds preservation for both. It supplies no value, asserts no inequality, and does not prevent explicit constraints from resolving the value or its components. Ordinary `_` and `?` do not themselves declare temporal independence.

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

### Member goals request enclosing correspondence

A member goal first tries the available entries in its own wrkList. If it still needs facts after trying the current supplied work, it requests additional RHSs from the virtual graph (D03Q). Checking the parent for a corresponding list is one source of those RHSs. Given `A R B`, with `R` one of `=`, `==`, `=:` or `==:`, establish the corresponding part of B for `A.m` and return it as a qualified RHS. The usual case requires finding this correspondence from scratch; an already established mapping is optional reuse. If the member's supplied work satisfies its goal, no parent search is needed.

Retain the member's tried-work cursor and the continuation state needed by active discovery/correspondence attempts. Exhausting current work does not require removing its meaningful remaining entries. A derived RHS or arriving premise continues the same member goal without replaying its earlier work. Ancestor routes follow the same discovery principle without requesting whole-parent normalization. Inspect only the structure and constraints needed to establish and accept the correspondence. The relative scheduling of other RHS sources is not settled by this ordering.

Do not initially build a separate persistent record of how far each parent has merged. Reuse an already available propagation cursor or derived relationship when inexpensive; otherwise locate the requested correspondence. Broader reuse bookkeeping must justify its lookup, maintenance and memory costs through measured work saved. This is independent of retaining a live attempt across a wait. Whole-list/span requests still use one shared traversal rather than independent searches for every member.

Finding correspondence and normalizing the enclosing relationship have distinct completion conditions. The correspondence request stops when the demanded mapping and its required premises are established; `A R B` can remain partially normalized. It may inspect or solve facts needed for placement, but it does not require all source members or the parent relationship to finish first. The member goal then obtains the requested facts from the projected RHS. Establishing a mapping neither marks the parent normalized nor discharges its remaining work.

A projected result carries the source POV or span, occurrence context, applicable member relation and required premises. Preserve qualification and direction without assuming every parent relation projects to plain equality or the same textual child operator. Feed the justified RHS into ordinary member solving, with its own wrkList, type and scoped listSpec contributions. Whole-list propagation uses the same operation and records contributions already covered; do not rediscover a parent relationship independently for each member.

Ambiguous projection retains correlated candidates. Missing input or placement evidence retains a local continuation; exhaustion and contradiction remain distinct. A selected result must satisfy its branch's required pattern conditions. Known lengths can skip irrelevant embedded spans, but navigation cannot skip semantic prerequisites. A request on one member does not require normalizing or publishing all of either containing list. `align4b`'s exact alternative representation remains deferred.

The first implemented slice uses borrowed occurrence paths and local structural cursors to expose a finite member, then feeds its derived RHS through ordinary solving. It supports immediate parent relations and embedded ancestor coordinates, applicable listSpecs, last/marked/positive-index selection, and repeated scalar predicates in `<~` patterns, including overlapping exits (D03R). The child relation preserves inherited qualifications while dropping the parent's top-only prefix qualifier. Parent parse completion and candidate parse completion resume their exact waiting points. Known numeric values finish without size unless size was explicitly requested.

The existing intersection fixtures in `defaultTests.tst` remain the primary acceptance path: `getLastItem/*`, `query/first`, `query/getNth`, `query/firstStrB/C` and `marked/getNotLast`. Whole-list propagation keeps its shared cursor instead of calling this targeted lookup for each member. Internal scope, immutability, continuation and work-count assertions supplement the language fixtures. No primary expected result is changed.

The current targeted cursor walks represented positions; it does not yet skip a known-length embedded span or consult an index. It does not infer arbitrary span alignment, cross ordinary ancestor-list boundaries to find a remote RHS, or project a contextual RHS. Repeated range predicates, structured list predicates, negative/span selection and general oneOf semantics remain explicit unsupported boundaries. Multiple viable selected occurrences retain the original expression as partial. D03S below adds whole-list top-only prefix matching; other whole-list qualifications and `align4b` remain separate work.

### Overlapping scalar-pattern correspondence

A `<~` member lookup uses a streaming chart over borrowed pattern and source positions (D03R). Each source column holds one state per viable pattern position. Repeated predicates admit both a zero-width exit and a consuming transition; a failed lookahead drops only that path. Take and combine zero-width exits before requesting source facts. Existing scalar operations check each live pattern/source pair once. Normalize each pattern predicate once, and share any needed candidate normalization across competing paths. Type-only checks can still avoid evaluating ordinary nested-list contents.

Each state retains the selected occurrence and its shared evidence cell. Paths at the same pattern position have the same future predicates in the supported fragment, so their captures can be packed while recording any ambiguity. This keeps correspondence and selection correlated without enumerating repetition-length combinations. If a complete match and all live continuations select the same occurrence, return its RHS immediately; optional trailing patterns need no more source reads. Multiple accepted occurrences stay partial. In particular, equal values at different occurrences are not treated as proof of a unique correspondence.

The chart is local to the request. Parser waits preserve its closure/consumption phase, candidate and cursors; arrival continues at that pair. No source or intermediate pattern is published. D03S extends this chart with complete mappings for whole-list prefix goals, followed by shared list propagation. It does not settle `align4b`'s representation. General bindings, structured predicates and unresolved comparison obligations remain later work.

For P represented pattern positions, N visited source positions and C live pairs, chart work is O(P + N + C), plus semantic predicate/value/context costs. C can be O(PN); a fixed pattern gives linear input work. O(P) live chart/capture references and cursor depth are separate from the ordinary solver's retained normalized facts. Sorted transitions merge adjacent equal destinations without an extra index. The counters and finite oracle test both ambiguity and unique selection; no general inference complexity or path-cost policy is implied.

### Whole-list prefix mappings

A represented ordered-list `=:` goal uses the same chart to establish a whole mapping (D03S). Its fixed predicates borrow already-checked target member frames; repeated predicates check the applicable tail requirements. Each consuming transition adds a shared predecessor record tying a source occurrence to a pattern slot. Joins retain both predecessor alternatives. Member selection still needs only its selected occurrence, but whole-list completion requires unique assignments for every span and member, including the source prefix endpoint. Alternative width splits remain partial with no publication, even if they would yield the same selected scalar value.

A unique mapping supplies each repeated tail's width. Compute enclosing widths from derived embedded frames, then feed the bounded target and matched source prefix into the existing list merge/correspondence path. This preserves physical target wrappers, ordinary member semantics, inherited requirements and staged publication. Normalize only source members needed for the match or accepted output; reuse their shared evidence cells. No input model, intermediate pattern, or irrelevant suffix is published. A source cursor carries enclosing known endpoints in flattened coordinates, preventing a too-long embedded span or a too-short represented span from supplying an unjustified sibling correspondence.

Parser completion resumes the exact chart comparison or accepted-member read, retaining its records and cursors. A newly completed source may supply both a body and relationships; inspect those obligations after parse completion, before accepting its layout. Shared mapping records cost O(P + N + C) for P pattern positions/wrappers, N visited source positions and C live pairs; processing and applying the accepted mapping adds linear walks over its represented positions, plus ordinary semantic costs. Multiple inherited predicates add their actual comparison work. This is the extra storage needed to retain full assignments instead of only a capture, not a cost-based planning policy or a persistent parent-progress cache.

The unchanged `merge/lookahead` fixture is the primary whole-list acceptance test. The supported increment is top-only prefix matching of represented ordered layouts with scalar predicates and repeated tail requirements. Constrained open target extents, unrepresented source positions, source-layout relationships that remain unresolved, structured/range repetition, dependent bindings and unresolved comparison obligations remain explicit boundaries. Other whole-list qualifiers and `align4b` remain deferred. Ordinary strict list merging continues through its existing shared propagation path.

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

**Basis accepted: D03Q; remaining details unresolved in D04.** An intersection exposes its selected body occurrence as the normalization target. That member finds RHSs by requesting correspondence through the containing list relationship; the ordinary solver then applies them. Selection, qualification, matching premises and correlated alternatives retain their semantics. This shares the mechanism used for direct member access and whole-list propagation. Specify the remaining pattern, input, result-constraint, sparse, partial-publication and cost contracts; D03Q/R implement the finite member and scalar-pattern slice; broader contextual/binding cases remain pending.

## Definitions

**First increment: D05A; broader D05 remains open.** WorldManager stages declaration registration at normalization and commits it at the ordinary publication boundary. Meaning bodies remain shared and unnormalized until demanded. Fresh named uses bind a unique language-compatible declaration, optionally qualified by model ID; ambiguous meanings stay unsupported. A bound use retains its binding. A named value requests the body as an ordinary RHS after its supplied work, reusing the existing scalar, expression, intersection and list operations. Publication records the binding and applied meaning on the requested result.

Pure nominal query predicates can compare supplied type headers without evaluating skipped meanings. Bound declaration identity, rather than equal structure/value, establishes nominal agreement. A bare unconstrained named list can supply a virtual layout to the structural cursor: it visits positions and normalizes only the selected member. The original declarations, body payloads and sibling occurrences remain unchanged. Staged declarations are private to their request and are not committed on failure/cancellation; declaration operations cannot be adopted as ordinary values.

The catalogue is local to WorldManager and shared by its Agent's sessions. Locale filtering reuses the existing vocabulary rules. Lookup costs the word's candidate bucket; a positional query costs visited layout positions and the selected member's work. Shared context-free bodies are evaluated once per request, with separate per-occurrence merges and output. Meaning/parser waits retain local progress. General definition-dependent record correspondence, nominal inheritance/conversions, nominal arithmetic, overload/context selection, late declaration subscriptions, semantic-change triggers and cross-Agent sharing remain open. Planning/path costs remain deferred. See D05A in the decision log for rationale and boundaries.

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

The replacement runs on `main` in `/home/bruce/devl/Proteus`. Read the comparison implementation from checkpoint `b52322a` through Git; its former checkout was removed. A replacement test must identify its engine and run entirely through WorldManager; unsupported features are reported explicitly, never supplied by an old-engine fallback. Keep the original language fixtures intact. Port useful newer behavior assertions through the new interface, without reproducing their old cache or scheduler APIs.

### 0. Preserve the reference and test inventory — done

The checkpoint and test-pruning manifest already preserve the reference. The pruned existing engine reproduced the same outcomes on 332 language cases and 75 selected compiled cases: 40 existing failures in total. These results validate test preservation, not the replacement. See the [test-selection record](../README.md#test-selection-for-the-normalizer-redesign) and retirement manifest (`graph/retired-normalizer-tests.json` in Git history).

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

On 2026-10-11, the normal Proteus Engine and TestProteus moved to [WorldManager.dog](../../WorldManager.dog). Agent's `normalize`, `normalizeAll`, `normalizeAndWait`, explicit sessions, parser publication and input-stream lifetime use this path. There is no old-solver fallback. The initial standalone TestWorldManager harness has moved into TestProteus and its source has been removed. Slipstream remains a separate migration; no rendering result is claimed.

### Interface and ownership

`Agent.startSession()` registers a session; `normalizeInSession(session, target, requireSize)` submits its request; `endSession()` cancels unfinished requests and removes the session. Ordinary `normalize(target)` uses an Agent-owned default session. Agent closure ends all of its sessions. A foreign or closed session is rejected. Completed handles retain their result and status after engine closure. Request-local frames and waits are released when the request settles or is cancelled. Sharing deductions across sessions remains deferred.

Runnable work drains before normalize returns. `WorldRequest.waitForCompletion()` acquires the model lock to wait for any running publication, then returns even if the request is waiting on live input. Callers inspect `status()`; waiting is not completion. Input streams own parser and normalizer thread lifetime separately and join their parser even when a normalization request is already settled. `normalizeAll` currently has the same supported scope as normalize, including closed ordered-list traversal and composition. Broader depth and streaming policies remain separate work.

### Implemented scalar and ordered-list fragment

The fragment is context-free numeric and string leaves, strict supplied relationships including direct exclusion proofs, supported scalar `==`/`=:`/`==:` matching, single-operand positive parenthesized expressions, numeric additive/radix expressions, closed positive string expressions, computed scalar size expressions, direct numeric interval constraints, and closed ordered-list member normalization, concatenation, adoption, strict positional correspondence, finite context-free listSpec application, closed embedded spans with ancestor template projection, open-prefix normalization, finite trailing-tail closure, uniquely determined embedded span extents, retained ancestor listSpecs across open embedded spans, adoption of represented open structure into an unconstrained target, strict open-source correspondence bounded by a represented closed target, and single-gap open correspondence with proved non-overlapping anchors. Single-operand list expressions carry the same future-member requirements. Each request retains frame cursors and a hash index of encountered context-free source views. Derived member-pair frames hold occurrence-specific combinations separately; they are not written into that source cache. Pending ancestor listSpec projections can pass through structural containment; member values are checked before a relationship, expression, template or publication consumes them. A cached frame with pending descendants is not a completion proof. General bindings, qualified applications and shared target constraints require the richer attempt equivalence from D03. Completion and literal consolidation live in virtual POV access. The iterative ready stack processes prerequisites without recursive normalize calls or writes to intermediate infons. Scalar expression frames expose and fold the existing size sequence, then the value sequence, before processing their supplied relationships, retaining the current slot and next operand across parser waits. Closed literal lists expose their applicable listSpec before enumerating the members; template projection and positional correspondence share derived-pair construction. Publication follows the requested target and its supported ordered-list containment scope after their supplied constraints have been checked; identity and expression prerequisites remain read-only. An unchanged partial interval already expressed in the model needs no replacement body. Conflicts and unsupported constructs publish nothing.

A closed target with a known extent equal to its logical member count can bound an open strict-identity source (D03O). The existing correspondence operation projects the source against checked target facts, then merges that result into the original target structure. Borrowed sources remain unchanged and different endpoint bindings use independent derived frames. A checked zero endpoint and nonnegative width summaries prove all source spans empty without unfolding shared occurrence paths. Ambiguous positive splits, two open layouts, source comparison barriers and qualified list relationships retain their existing limits. New ordinary open members retain their applicable templates for later requests. No path cost or strategic planner is involved.

Both `tUnknown` and `tUnset` contribute no type evidence for unknown payloads; their interpretation can be acquired through the ordinary merge (D02O). An untyped literal remains unsupported. Ordinary numeric completion permits unknown size under D02F. A stronger size request stays partial if size is unknown. Completion is tied to its infonView owner, so copying a view cannot copy its completed status to another view. An unchanged completed request creates no new frames. Publication isolates scalar state borrowed from another view. Fully checked closed scalar leaf RHSs leave the pending work list, including redundant unknowns and successful qualified matches. Nested relationships remain until their consolidation is specified; fully checked numeric/string expressions can be discharged when their facts are represented. An ordinary unknown-size string still needs endpoint evidence; a closed string expression counts its represented text. An exclusion template is not a positive value; a negative target stays partial under the current value-oriented completion request.

The printer now shows actual retained expressions and constraints rather than consulting retired solver receipts. The initial cutover exposed consolidation and scalar-representation failures; the scalar follow-up below resolves them against unchanged expectations. A numeric comparison such as `-1 = 9` remains partial while extent-dependent equality is unresolved; a lone `-1` can finish without size.

For two unbounded single-gap lists, checked prefixes align from the start and checked suffixes align from the end (D03P). Before combining their longest boundaries, reject every possible shorter overlap through the ordinary member merge. If an overlap remains possible, no unconditional alignment is published. Proven alignments reuse physical target traversal and derived member pairs; incoming boundaries occupy the target's existing gap while its remaining width stays symbolic. Future-member requirements stay in the live continuation and, where necessary, the original stored relationship. Required writes are staged, source frames are read-only, and independent target occurrences have separate anchor cursors. A finite incoming prefix never proves a closed endpoint.

### Next correspondence work

The next concrete examples are forced first/last members around multiple ambiguous gaps; bounded two-open layouts; correlated shorter-overlap alternatives; and later target evidence needed by an earlier partial source. D03P now handles a parsed incoming open prefix and the proved single-gap alignments described above. These are distinct remaining cases, recorded under “D03 next evidence” in the decision log. Current code stops before pairing ambiguous spans and at an unresolved source prerequisite. A further increment should preserve those local attempts and expose only justified member facts, using the common correspondence machinery. It must not select an arbitrary width split, enumerate a virtual extent, treat a prefix as an endpoint or restart the request. D03R and D03S use the shared chart for scalar-pattern member lookup and represented whole-list prefix mappings; no general open-span chart or strategic planning policy is chosen by this investigation.

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

All 15 protected `.tst` files keep their original hashes. All 1,530 previously retained compiled registrations remain selectable and are indexed with their original assertion locations in `graph/engine-test-migration.json` (Git history). Of these, 63 use the engine/parser/model path and 1,467 report `NOT_PORTED` until their useful assertions are adapted from the reference source. The three new WorldManager cases add scalar/session, input-lifecycle and malformed-RHS coverage. The inventory also records 14 standalone native test drivers awaiting migration; they are not compiled into this engine. The separate streamReplyTest suite has API changes but has not been validated for reply behavior; effects remain unsupported.

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


### Closed embedded-list follow-up, 2026-10-11

D03H extends the shared sequence and correspondence traversal to finite embedded lists. `{_ &{_ _} _} = {1 2 3 4}` retains the target wrapper as `{1 &{2 3} 4}`. Right-hand embedded spans supply logical members in order; empty spans contribute zero members and ordinary nested lists contribute one. Nested wrappers share a single right cursor through retained projected frames. Literal traversal, correspondence, listSpec projection, concatenation and publication use the same occurrence-aware sequences. No parser, printer, planner, path-cost calculation or old-solver fallback is added.

A parent listSpec applies through embedded spans without treating their wrappers as scalar members. The embedded list's own constraints are checked first and the ancestor requirement is projected into its logical members with the existing merge operations. `{*10+_| &{7| _ _}}` becomes `{&{*10+7 *10+7}}`. Ordinary nested list requirements continue to merge as lists. Publication preserves physical target occurrence identities and keeps source/template infons unchanged. Closed embedded endpoints must agree with their represented length; unknown or inconsistent embedded endpoints stay partial before any correspondence writes. Source DAG length overflow is checked before output allocation. Open/sparse spans, embedded strings, nominal/contextual templates and general shared-target unification remain unsupported.

The preserved engine flattens wrappers when adopting `? = {&{1 2} 3}` into an unknown target. The replacement's existing independent-copy publication preserves the borrowed sequence's wrapper structure, producing `{&{1 2} 3}` with distinct destination views. Both have three logical members; this additional adoption check is a documented representation choice, not a reference text-parity claim. Existing literal target wrappers must be preserved. Protected fixture expectations remain unchanged.

The final CodeDog TestProteus regeneration succeeds in **141.44 seconds**. The separate shared-library regeneration succeeds in **94.45 seconds**. Both generated targets contain no GraphNormalizer, GraphManager or proteus4 runtime references. All **11 WorldManager suites pass**, including **356/356 embedded-list assertions**, **361/361 scalar/session**, **229/229 scalar constraints**, **176/176 numeric expressions**, **238/238 string expressions**, **188/188 computed sizes**, **170/170 list members**, **216/216 list composition**, **195/195 listSpecs**, and **14/14 input lifecycle** assertions, plus the incomplete-RHS regression. The three older suites each lose six assertions only because their former unsupported embedded probes move into supported embedded coverage. Nine exact-output language probes pass on both the preserved engine and the replacement.

For **32/64/128** one-member embedded spans, requests create **162/322/642** frames, inspect **97/193/385** relationships, perform exactly **32/64/128** scalar pairs, and visit **67/131/259** sequence nodes. For **32/64/128** nested wrappers around one member on each side, requests create **101/197/389** frames, inspect **67/131/259** relationships, perform one scalar pair, and visit **99/195/387** sequence nodes. One ancestor template crosses those deep spans with one leaf application. A 32-layer shared empty source representing over four billion empty paths needs at most four sequence visits and forty frames; its checked zero extent permits skipping the entire span. Shared-length overflow, including a scalar appended after the maximum embedded extent, stops before any publication. Parser arrival and cancellation preserve the earlier template work and existing frames.

Seven retained cases now run their semantic assertions through Agent.normalize: `listRHS/embedded` and the `spans/embeddedLeft`, `embeddedRight`, `emptyEmbedded`, `nestedIsOne`, `embeddedSizeConflict`, and `sharedEmbeddedContext` cases under `unit/graphFoundation`. Obsolete registry receipts are replaced by request/publication checks. Six unchanged default fixtures also become supported: `EmbeddedLists/LHS`, `RHS`, `eLHS`, `eRHS`, `e_e`, and `f_f`.

The full retained run reports **1619 failures / 1873 tests**, with **no new failures or outcome-category regressions** against the finite-listSpec baseline:

| Suite | Passed | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| defaultTests.tst | 137 | 195 | 0 | 0 | 332 |
| Compiled cases | 117 | 2 | 1415 | 7 | 1541 |
| Total | 254 | 197 | 1415 | 7 | 1873 |

The seven other failures remain the same reference-registry cases. All prior 1540 compiled registrations, all 15 protected fixture hashes, the original assertion-source hash and the unrelated `myTests.tst` are preserved. One embedded suite is added. WorldManager adds 112 lines and removes 40, a net 72 lines; all reasoning remains CodeDog. Logs and probes are in `/tmp/proteus-embedded/`.


### Finite open-tail follow-up, 2026-10-11

D03I adds the unambiguous trailing-tail case to the shared correspondence cursor. `{1 ...} = {1 2 3}` becomes `{1 2 3}`; `{*10+_| _ ...} = {2 3}` becomes `{*10+2 *10+3}`. Existing prefix members keep their occurrence identities and receive ordinary positional pairs. Remaining finite source members pass through the same target template operation and are published into independent new occurrences. Nested normal lists and closed embedded prefixes/sources compose with this operation. No planner, path pricing, general alternative matcher or source mutation is introduced.

An open list without finite endpoint evidence keeps its tail and any listSpec while normalizing its actual prefix. A billion-sized symbolic tail creates no members. A supplied extent equal to the checked prefix count proves an empty remainder, including zero-size empty lists. A source with an explicitly unknown endpoint does not supply closure evidence. Middle gaps, embedded open spans, open-source correspondence, open concatenation and unresolved open-list copying remain unsupported. Nominal/definition semantics and general later-link continuation also remain outside this increment.

This milestone also fixes a reproduced parser constraint-loss bug. A terminal explicit `&{9 ...}` or `&{5| ...}` was being removed by the same cleanup as a bare ellipsis. The preserved engine reproduces both losses. `parseListItem` now reports actual ellipsis-token provenance to its containing parse; only that token is folded into `tailUnfinished`. Explicit embedded prefixes, templates and supplied work survive parsing. A locally supplied finite identity can then close an embedded child through ordinary normalization; unresolved embedded alignment remains unsupported. No printer change is made.

New suffixes are prepared off-model before publication checks complete. A late conflict cannot leave appended nodes, modified next/previous links, cleared tail flags or discharged templates behind. Parser waits retain the checked prefix and source frames; cancellation attaches nothing. This continues the existing parser-completion contract, without claiming streaming prefix publication or a general sparse matcher.

The final TestProteus CodeDog regeneration succeeds in **141.87 seconds**; the separate shared-library regeneration succeeds in **91.91 seconds**. Both generated targets contain no GraphNormalizer, GraphManager or proteus4 runtime calls. All **12 WorldManager suites pass**, including **354/354 open-tail assertions**, **356/356 embedded-list assertions**, **361/361 scalar/session**, **229/229 scalar constraints**, **176/176 numeric expressions**, **238/238 string expressions**, **183/183 computed sizes**, **164/164 list members**, **216/216 list composition**, **189/189 listSpecs**, and **14/14 input lifecycle** assertions, plus the incomplete-RHS regression. Three former unsupported probes move into supported tail coverage; their old suites lose only those assertions.

Eleven exact-output tail language probes pass on both the preserved engine and the replacement. Two additional composition probes pass on the replacement while the reference leaves them unresolved: a typed template across an existing prefix and new tail members, and an explicitly zero-sized empty tail. These are new checks, not changed protected expectations. Three direct parser probes now preserve the explicit embedded prefix, template and supplied work; all three reproduce constraint loss on the preserved engine. Native assertions also check the retained graph structure, including an explicit empty embedded wrapper. The protected open-span spelling probes pass **2/2**.

For **32/64/128** members supplied to a tail with a shared computed template, requests create **71/135/263** frames, inspect **37/69/133** relationships and consume **31/63/127** new tail members. There is one existing-prefix pair, exactly **32/64/128** template applications, three sequence-node visits and **33/65/129** publications. Deep ordinary-list fixtures inspect **2n + 2** relationships for n wrappers and add one supplied tail member. Billion-sized and larger symbolic extents create no members: the bounded fixtures use at most four frames and preserve any future template. Wait/cancellation checks retain prior work and attach nothing before readiness; the late shared-view conflict check leaves a previously prepared suffix detached.

Four retained compiled cases now run through Agent.normalize: `spans/closeTail`, `givenTail`, `newNestedView` and `zeroTail` under `unit/graphFoundation`. Their value, endpoint, shape, prefix-identity and source-immutability checks are preserved. Six unchanged default cases also become supported: `merge/list/N_dots3`, `dots`, `misc`, and `noRHS/dots/nestedIn`, `nestedOut`, `simple`.

The full retained run reports **1609 failures / 1874 tests**, with **no new failures or outcome-category regressions** against the embedded-list baseline:

| Suite | Passed | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| defaultTests.tst | 143 | 189 | 0 | 0 | 332 |
| Compiled cases | 122 | 2 | 1411 | 7 | 1542 |
| Total | 265 | 191 | 1411 | 7 | 1874 |

The seven other failures remain the same reference-registry cases. All prior 1541 compiled registrations, the 58 preserved parser/model registrations, all 15 protected fixture hashes and the reference assertion-source hash remain intact. One tail suite is added. WorldManager adds 71 lines and removes 10 (net 61); the parser adds 8 and removes 4 (net 4). Reasoning remains in CodeDog, and the unrelated `myTests.tst` is untouched. Logs and probes are in `/tmp/proteus-open-tail/`.


### Unique embedded-span extent follow-up, 2026-10-11

D03J extends the existing tail operation to leading, middle and nested embedded spans when extent equations prove their widths. `{1 ... 4} = {1 2 3 4}` becomes `{1 &{2 3} 4}`, preserving the existing wrapper and endpoints. Explicitly sized gaps can coexist, and several unsized gaps all close when the finite source leaves a zero remainder. A given size alone does not expand a span. Summaries distinguish physical member count, minimum logical width and unresolved widths; exact arithmetic preserves very large declared extents without narrowing or enumeration.

Every embedded projection shares the enclosing right cursor and stops at its proven endpoint. Its existing prefix is paired normally, and its supplied tail members use the prior listSpec and detached-publication operations. Ordinary nested lists still contribute one member to their parent. Sources and templates remain unchanged. No copied source slices, repeated member lookup, planning stage or path costs are added.

An ambiguous division remains partial with the original work intact. In particular, an unresolved embedded projection cannot advance into sibling matching: its enclosing correspondence stops at that prerequisite. General alternatives, open-source correspondence, ancestor templates across unresolved embedded spans and publication of anchored partial facts around ambiguous gaps remain later work. Parser completion continues existing frames; cancellation and late conflicts publish nothing.

The final TestProteus CodeDog regeneration succeeds in **144.61 seconds**; the separate shared-library regeneration succeeds in **92.44 seconds**. Generated executable and library sources contain the new extent operation and no GraphNormalizer, GraphManager or proteus4 runtime references. All **13 WorldManager suites pass**, including **269/269 span-extent assertions**, **350/350 closed-embedded assertions**, **342/342 open-tail assertions**, **361/361 scalar/session**, **229/229 scalar constraints**, **176/176 numeric expressions**, **238/238 string expressions**, **183/183 computed sizes**, **164/164 list members**, **216/216 list composition**, **189/189 listSpecs**, and **14/14 input lifecycle** assertions, plus the incomplete-RHS regression. Three formerly unsupported embedded-gap probes move to supported span coverage. The preserved-prefix parser regression now checks rejection of the conflicting value rather than rejection of the formerly unsupported construct. No protected fixture expectation changes.

All twelve focused language probes pass on the replacement. Eleven also match the preserved engine's output exactly. For the local typed-template example, both compute `{1 &{*10+2 *10+3} 4}`; the reference additionally retains a RHS constraint. The replacement's checked member projections fully represent that constraint and its source/template immutability checks pass. This twelfth case is not an exact reference-output parity claim. A new negative-size fixture initially used an invalid spelling; it now uses the existing `*(-1)+` syntax and verifies partial preservation.

The first build exposed a continuation error on `{&*2+{... ... 4} 5} = {1 4 5}`. Its inner division was partial, but the outer cursor then compared the following `5` to the unconsumed source `1`. A debugger check confirmed `worldRejected` with `Incompatible scalar values`. Propagating the unresolved embedded prerequisite to its containing correspondence now yields `worldPartial` with zero publications. This guards source positioning, without rescanning the prefix or trying speculative partitions.

For **32/64/128** explicitly sized one-member gaps, requests create **98/194/386** frames, inspect **65/129/257** relationships and visit **35/67/131** sequence nodes. For **32/64/128** nested embedded wrappers with following siblings, they create **163/323/643** frames, inspect **98/194/386** relationships and visit **66/130/258** sequence nodes, consuming one new tail member. Billion-sized and larger embedded extents stay compressed with fewer than ten frames and no created tail members. Parser-wait tests reuse all previously inspected relationships; cancelled waits and later sibling conflicts publish nothing.

Four retained compiled cases now use Agent.normalize: `spans/middleGap`, `zeroGaps`, `nestedOpenSpan` and `sizeConflict` under `unit/graphFoundation`. Original value, shape, width and conflict assertions remain, with source and occurrence preservation checks. Three unchanged default fixtures also become supported: `merge/list/N_dots1`, `N_dots2` and `merge/dots/align1`.

The full retained suite reports **1602 failures / 1875 tests**, with **no new failures or outcome-category regressions** against the finite-tail baseline:

| Suite | Passed | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| defaultTests.tst | 146 | 186 | 0 | 0 | 332 |
| Compiled cases | 127 | 2 | 1407 | 7 | 1543 |
| Total | 273 | 188 | 1407 | 7 | 1875 |

The seven other failures remain the same reference-registry cases. All prior 1542 compiled registrations, all 58 preserved parser/model registrations, all 15 protected fixture hashes and the original assertion-source hash remain intact. One new suite is added. Production changes stay in WorldManager: 95 lines added, 26 removed, **net 69**. No parser, printer or legacy runtime change is involved. The unrelated `myTests.tst` is untouched. Logs, probes and debugger evidence are in `/tmp/proteus-span-extent/`.

### Retained sparse-member requirements follow-up, 2026-10-11

D03K lets inherited listSpecs cross unresolved embedded spans. `{*10+_| &{7| ...}} = {_ _}` becomes `{&{*10+7 *10+7}}`: both scoped requirements apply to each supplied member, while source/template infons stay unchanged. Existing prefix members already carry their local and ancestor projections and are not projected again when correspondence fills the remainder. The leading `|` spelling still ends the parser's listSpec search; it does not remove an applicable ancestor requirement from an embedded logical member.

A persistent chain retains checked requirement references only for future tail members. Appending a requirement does not copy earlier requirements. New members use a retained cursor over those references and the existing derived-pair merge; no second template evaluator, planning phase or path pricing is introduced. Empty spans do not compare unused template values, so `{5| &{6| ...}} = {}` closes as `{&{}}`. Open results preserve both original listSpec scopes for unmaterialized members. The shared correspondence and detached publication paths preserve existing occurrences and prevent partial suffix writes on failure.

A requirement that needs more scalar evidence can still block later requirements under the current partial-conjunction boundary. Ambiguous matching, open-source correspondence, contextual/nominal templates and general later link arrivals remain unfinished. Counts separate requirement-chain visits, member applications and wrapper projections. One ancestor across many wrappers is linear; many ancestors across the same wrappers can still cause quadratic traversal. This is explicit remaining work, not hidden inside a claim about constant-time chain insertion.

The concatenated-source probe `{*10+_| &{7| ...}} = ({_} {_})` completes its target members but retains the unresolved source expression in the printed RHS. This is the existing conservative consolidation rule for incomplete expression sources, not a new member-normalization failure. The new regression records both the completed member values and the retained relationship; improving that discharge rule remains a separate task. No protected language expectation changes.


With 32/64/128 supplied members and two requirements, the new checks measure 107/203/395 frames, 39/71/135 relationship inspections and 62/126/254 retained-requirement visits. Each member gets two applications; the existing prefix member is not reapplied during correspondence. One ancestor over 32/64/128 embedded wrappers uses 108/204/396 frames and 101/197/389 sequence visits, with only two member applications. With a template at every one of 8/16/32 ancestors and three supplied members, member applications remain exactly 24/48/96, but frames rise to 89/237/725 and sequence visits to 54/170/594. These separate measurements expose the remaining repeated wrapper traversal.

Billion-sized and greater-than-int64 symbolic spans generate no members or retained-requirement visits and use fewer than ten frames. Tests cover local/ancestor template waits, a later source-member wait, cancellation, source/template immutability, prefix/POV preservation, partial open results and a separate new caller request using their retained scopes. The new suite also checks empty spans with differing requirements, ordinary nested-list members, computed templates, multiple RHS relationships, and unsupported nominal/open-source boundaries.


The regenerated TestProteus build passes in **146.22 seconds**; the separate shared-library build passes in **94.10 seconds**. Both generated targets contain the same WorldManager requirement operations and no legacy GraphManager/GraphNormalizer references. All **14 WorldManager suites** pass, including **252/252 assertions** in the new span-requirement suite. Fifteen focused language probes pass. Five corresponding probes also pass on the preserved Proteus4 engine when expressed with a known extent or a nonterminal embedded wrapper. Those spellings avoid the reference parser's already documented terminal-wrapper loss. Of the ten direct-spelling probes, the reference passes only one: the differences include lost wrappers/templates and an unresolved nested template. The replacement parser was fixed in the earlier tail milestone and is unchanged here.

The full retained run reports **1602 failures / 1876 tests**, with **no new failures or outcome-category regressions** against the span-extent baseline:

| Scope | Pass | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| Default language fixtures | 146 | 186 | 0 | 0 | 332 |
| Compiled registrations | 128 | 2 | 1407 | 7 | 1544 |
| Total | 274 | 188 | 1407 | 7 | 1876 |

No historical case is newly ported in this increment; the additional passing registration is the new suite. The seven other failures remain the same reference-registry cases. All prior 1543 compiled registrations, all 58 original parser/model registrations, all 15 protected fixture hashes and the reference assertion-source hash are preserved. WorldManager adds 53 production lines and removes 21 (**net 32**). No parser, printer or old-solver change is involved, and no path costs are computed. The unrelated `myTests.tst` is untouched. Logs and probes are in `/tmp/proteus-span-specs/`.


### Batched ancestor projection follow-up, 2026-10-11

D03L removes the repeated wrapper walk measured in D03K. An embedded occurrence can carry pending listSpec projections, preserving the checked template and originating edge for each scope. A persistent ordered join shares those requirements without copying the accumulated collection. One correspondence walk carries them through the wrappers and applies them to actual members in local-to-ancestor order. It also retains requirements for symbolic tails without generating members.

A list's own wrkList is a boundary: its member requirements settle before its supplied identities. Ordinary nested members, source lists, structured templates and expression operands likewise receive checked member facts. The requested root settles its pending projections before publication. The active list frame continues this work directly; a borrowed source receives a separate projected frame. The usual member merge, prerequisite stack, parser continuation and detached publication are reused. No goal restarts, planning phase, path costs, parser changes or old solver are involved.

For three supplied members and a template at every embedded ancestor:

| Ancestors | Previous frames | Batched frames | Previous sequence visits | Batched sequence visits |
| ---: | ---: | ---: | ---: | ---: |
| 8 | 89 | 69 | 54 | 27 |
| 16 | 237 | 133 | 170 | 51 |
| 32 | 725 | 261 | 594 | 99 |

The new checks extend to 64 and 128 ancestors. Open spans use 517/1029 frames and 195/387 sequence visits; closed lists use 452/900 frames and 130/258 sequence visits. Each case applies every ancestor exactly once per actual member. These checks enforce linear bounds in wrappers plus required applications. Independent ordinary-list, concatenation, embedded-correspondence and finite-span scaling counts are unchanged. A simple inherited span pays one additional frame for carrying a pending projection; the benefit is avoiding a new descendant walk for each added ancestor.

`requirementVisits` now counts the ordered requirement cursor's joins and leaves, including existing-prefix requirements. Previously it counted only backward tail-chain links. With two requirements and 32/64/128 supplied members, counts therefore change from 62/126/254 to 94/190/382; the number of semantic applications remains exactly 64/128/256, and correspondence does not reapply the prefix. The existing tests retain their value, completion, source-preservation and application assertions while updating these work-accounting expectations.

This is a bound for one batch, plus required nested member correspondence, payload operations and retained-constraint copies. Intervening identities and separate source uses can require separate operations; arbitrary inference and ambiguous matching have no general linear guarantee. Partial conjunctions, general streamed-prefix publication and the previously documented conservative RHS consolidation remain unchanged.


The final regenerated TestProteus build passes in **148.75 seconds** and the separate shared-library build in **99.94 seconds**. Both generated targets contain the new requirement traversal, with no legacy GraphManager/GraphNormalizer references. All **15 WorldManager suites** pass, including **97/97 assertions** in the new traversal suite and all 252 span-requirement assertions. The new cases cover both open and closed chains through 128 ancestors, independent sibling scopes, source reuse, structured templates, expression operands, conflicts before publication, a delayed deep member, cancellation, and an inner identity arriving before the outer constraint is applied. Tests preserve the actual parsed parent pointers and existing prefix occurrences. A separate 485-case capture comparison against the pre-change executable has identical captured output, including reported errors and unsupported results; these deliberate capture failures are separate from the ordinary pass/fail suite.

The full retained run reports **1602 failures / 1877 tests**, with **no new failures or outcome-category changes** against the D03K baseline:

| Scope | Pass | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| Default language fixtures | 146 | 186 | 0 | 0 | 332 |
| Compiled registrations | 129 | 2 | 1407 | 7 | 1545 |
| Total | 275 | 188 | 1407 | 7 | 1877 |

The seven other failures remain the same reference-registry cases. All 1544 prior compiled registrations, 58 original parser/model registrations, 15 protected fixture hashes and the original assertion-source hash are unchanged. The added registration is the traversal regression; no historical case is newly ported. WorldManager adds 82 lines and removes 30 (**net 52**). The unrelated `myTests.tst` is untouched. Logs, comparison fixtures and debugger evidence are in `/tmp/proteus-batched-specs/`.


### Pending scalar comparison follow-up, 2026-10-11

D02N allows a member or scalar target to keep an undecided comparison while continuing its remaining supplied evidence. The existing merge marks the two admitted cases: an exclusion against a positive scalar whose facts are incomplete, and a signed numeric equality whose extent is missing. A pending entry borrows the checked source frame and qualified edge through the existing persistent requirement structure. Unknown-to-known type, size or value transitions wake these entries; other edges do not. Resumption calls the same merge and retains only still-undecided checks. No tried wrkList prefix, operand fold or ancestor projection restarts.

For example, `_ = !5 = 7` now becomes `7`, `_ = !5 = 5` rejects, and `9 = -1 = *10+_` becomes `*10+9`. The same mechanism lets `{7| &{!5| ...}} = {_}` become `{&{7}}`, including existing prefixes and later finite correspondence. It does not require a list-specific exclusion rule. Checked sources/templates remain unchanged, and separate occurrences share their evidence without sharing mutable pending state.

Completion requires an empty pending-comparison chain. An unresolved scalar keeps its original body and relationships; containing lists preserve applicable templates and unresolved correspondence work. Detached publication refuses a new member whose checks are unresolved. A contradiction publishes nothing. Grouped RHS prerequisites and expression operands with pending checks remain partial rather than lending unchecked facts to their consumer. Negative targets, negative ranges, inverse expressions, ambiguous alignment and general later-link delivery retain their existing boundaries. Numeric size remains optional when no supplied comparison requires it. The conservative printing of nested RHS work also remains unchanged.

Within an occurrence, each retained comparison is resumed at most once per newly known type, size and value: at most three resumptions in this fragment. Different occurrences pay for their own checks even when their sources are shared. Tests distinguish these attempts from original relationship reads, template applications, frame allocations and sequence visits. Both the initial pending insertion and occurrence inheritance use existing persistent joins. Payload comparison costs and other member/list work are additional. Parser arrival uses the same waiting request and cursor, with no replay of prior relationships; cancelled requests publish nothing.

With 32/64/128 pending exclusions, then the same number of unrelated unknown edges, followed by separate type/size/value arrivals, tests measure **96/192/384 comparison resumptions**, **67/131/259 relationship reads** and **68/132/260 frames**. Without the three fact arrivals there are zero resumptions and no publication. With one exclusion at each of 32/64/128 embedded ancestors, three members and one later positive ancestor value, there are **96/192/384 resumptions**, **269/525/1037 frames** and **102/198/390 sequence visits**. Each scoped requirement still applies exactly once per member; comparison continuation does not repeat the wrapper walk.

The final CodeDog regeneration builds TestProteus in **155.19 seconds**, and the separate shared library builds in **100.47 seconds**. Both generated targets include the same continuation and no GraphManager/GraphNormalizer or legacy namespace dependencies. All **16 WorldManager suites** pass, including **373/373 assertions** in the new pending-comparison suite and all existing span/traversal assertions. Tests cover accepted and excluded values, type and extent arrivals, multiple pending exclusions, every ordering of four supplied scalar assertions, existing/new members, partial tails, independent contexts sharing evidence, parser waits, cancellation, and grouped-source boundaries. The previously partial ancestor-exclusion regression now requires its completed `{&{7}}` result.

All seven focused language examples fail on the saved baseline executable and pass after this change. Of 485 existing projection output captures, 483 are identical. The two changed captures now reject a delayed contradiction without first discharging an inner relationship; tests explicitly require rejection, unchanged target text and zero publications. These deliberately mismatched capture fixtures expose outputs and are not ordinary passing tests.

The full retained run reports **1602 failures / 1878 tests**, with **no new failures or outcome-category regressions**:

| Scope | Pass | Unsupported | Not ported | Other failures | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| Default language fixtures | 146 | 186 | 0 | 0 | 332 |
| Compiled registrations | 130 | 2 | 1407 | 7 | 1546 |
| Total | 276 | 188 | 1407 | 7 | 1878 |

The seven other failures remain the same reference-registry cases. All 1545 prior compiled registrations, all 58 original parser/model registrations, all 15 protected fixture hashes and the original assertion-source hash are preserved. No historical case is newly ported in this increment; the additional passing registration is the new continuation suite. WorldManager adds 52 production lines and removes 11 (**net 41**), reusing the requirement representation, scalar comparison and staged publication. Parser, printer and path-cost policy are unchanged. The unrelated `myTests.tst` is untouched. Logs, probes, baseline executable and comparison scripts are in `/tmp/proteus-pending-comparisons/`.


### Represented list source discharge follow-up, 2026-10-11

D03M distinguishes a fully represented contribution from a fully known value. A closed, known-extent source with representable member facts can be discharged after its facts merge into the target, even when some member values remain unknown. The existing correspondence path retains the relationship whenever unresolved target member work still needs it. No second traversal or printer special case is added.

`{7 8} = ({_} {_})` becomes `{7 8}`; `{_ _} = ({_} {2})` becomes `{_ 2}` and stays partial. Explicit range and negative-member facts can likewise be transferred or checked without retaining the original expression syntax. The previously documented ancestor-template example `{*10+_| &{7| ...}} = ({_} {_})` now prints `{&{*10+7 *10+7}}`. Source expressions, member states and target occurrence identities are preserved. Unresolved comparisons, inverse expressions, mismatched extents, qualified list matches and open/unknown-extent source boundaries keep their existing conservative behavior.

The existing cached representability, closed-layout and endpoint facts supply the discharge decision. Correspondence owns the final decision to retain an RHS, combining the source's remaining meaning with the result of member work. This also removes duplicate retained entries, including unresolved unknown-extent sources, without a deduplication scan. Together these changes add **nine net production lines**. A partial repeated request performs its requested member work without another walk over the discharged RHS. Input waits and cancellation use the existing retained frames and staged publication.

The final TestProteus regeneration passes in **158.47 seconds**, and the separate shared-library build passes in **101.64 seconds**. All **17 WorldManager suites** pass, including **169/169 assertions** in the new discharge suite. All six targeted language probes fail on the saved baseline and pass after the change. The wider 140-case output capture has 37 intended changes: redundant expression relationships or duplicate retained relationships disappear; unsupported/error results are unchanged. Capture fixtures are diagnostic output comparisons, not ordinary passing tests.

For 32/64/128 unknown members merged with a two-part list expression, first-pass work uses **100/196/388 frames**, exactly one member correspondence per position and five sequence-node visits. Re-normalizing the partial stored result uses **33/65/129 frames**, zero member correspondence and no source-expression reads. Unresolved unknown-extent and member-comparison cases preserve exactly one original RHS across repeated requests. Parser waits preserve the edge until arrival; cancellation leaves it intact.

The full retained run reports **1602 failures / 1879 tests**, with no new failures or category changes: default fixtures remain 146 pass / 186 unsupported; compiled registrations are 131 pass / 2 unsupported / 1407 not ported / 7 existing reference-registry failures. All prior 1546 registrations, all 58 original parser/model cases, all 15 protected fixture hashes and the reference source hash are preserved. The additional passing registration is the new discharge suite; no historical test is newly ported. The change adds 16 production lines and removes seven (**net nine**). Parser, printer, planning and path-cost behavior are unchanged. Logs, probes and the saved baseline executable are in `/tmp/proteus-list-discharge/`; unrelated `myTests.tst` remains untouched.


### Open structure adoption follow-up, 2026-10-11

D03N permits strict adoption of a represented open list when the target has neither a value nor a size. `? = {1 ...}` becomes `{1 ... }`, and `? = *1000000000+{...}` preserves its given extent with zero actual members. It never enumerates that extent. An adopted open layout remains partial. A size-constrained or already valued target still needs open-source correspondence, which remains unsupported.

The existing list merge copies its symbolic-tail and template-discharge facts. The same small metadata operation carries checked local listSpecs and retained future-member requirements through adoption and single-operand list expressions. Thus `? = (({5| ...})) = {_ _}` becomes `{5 5}`, and `? = {*10+_| &{7| ...}} = {_ _}` becomes `{&{*10+7 *10+7}}`. No new member evaluator or scheduler is involved. Multi-operand open concatenation remains unsupported.

Detached publication copies listSpec headers when their obligations remain, while creating independent destination list bodies and member POVs. This preserves requirements after transient request frames are released. A fresh request can close one stored adoption without changing another adoption or the shared source. The previous handle remains partial; it is never restarted. Nested open ordinary members can also be published through the same operation. Checked closed templates still discharge. A fully represented adoption with no remaining source work discharges its original edge even when the copied endpoint is open or explicitly unknown, so `? = (*_+{_})` becomes `*_+{_}` without losing that unknown endpoint.

The final regenerated TestProteus build succeeds in **156.59 seconds**, and the separately regenerated shared library in **102.75 seconds**. All **18 WorldManager suites** pass, including **277/277 assertions** in the new adoption suite. The cases cover nested and middle spans, explicit unknown and enormous extents, preserved scoped templates, grouped sources, independent occurrences, closure in the same and later requests, parser arrival, cancellation, rejected constraints and retained unsupported boundaries. All five original probes and three supplementary language checks pass. The three retained historical cases `adoption/open`, `adoption/billion` and `expressions/computedListSize` now run their substantive assertions through WorldManager.

For **32/64/128** represented members under one listSpec, adoption uses **67/131/259 frames**, **34/66/130 relationships**, exactly one template application per member, and one sequence-node visit. No unrepresented tail member is allocated. The original billion-member test additionally bounds frame creation below 100; a larger-than-64-bit given extent is preserved in the new suite.

A **378-case** direct/grouped adoption comparison has **182 identical outputs** and **196 transitions from the previous unsupported boundary to ordinary results, partial constraints or rejection**; no previously supported output changes. The separate 140-case discharge corpus has two intended changes: open adoption now succeeds, and a redundant source edge disappears after adoption with an explicitly unknown endpoint. These capture fixtures deliberately use mismatched expected text to expose output; they are not ordinary passing test totals.

The full retained run reports **1599 failures / 1880 tests**, with no new failures or outcome-category regressions: default fixtures are 146 pass / 186 unsupported; compiled registrations are 135 pass / 2 unsupported / 1404 not ported / 7 existing reference-registry failures. All prior 1547 registrations, 58 original parser/model cases, 15 protected fixture hashes and the historical assertion-source hash remain intact. This increment adds one behavioral suite and ports three original registrations. WorldManager adds 26 lines and removes 14 (**net 12**, including clarified comments). Both generated targets use the same implementation without old GraphManager/GraphNormalizer dependencies. Parser, printer, planning and path costs are unchanged; Slipstream integration is not claimed. Evidence is in `/tmp/proteus-open-adoption/`; unrelated `myTests.tst` remains untouched.


### Unknown overlay compatibility follow-up, 2026-10-11

D02O restores the original representation contract for `tUnset`: like `tUnknown`, it contributes no known type. The previous reader reported its enum value as a concrete type, so an otherwise compatible numeric, string or list source could be rejected. The fix changes one condition in virtual POV access. Merge, comparison, publication, scheduler and parser behavior are unchanged.

A small C++ client linked against the saved generated shared library reproduced `? = *10+7` completing with `tUnknown` and rejecting with `tUnset` (`Incompatible scalar types`, zero publications). The unchanged client linked against the regenerated library now completes both cases, each with one publication. The compiled regression additionally covers type-only and range evidence, list adoption and member correspondence, pending exclusions, immutable intermediate sources, parser waits and cancellation. A literal payload with no type still reports unsupported; absence of evidence does not invent a type.

The new suite passes **140/140 assertions**, and all **19 WorldManager suites** pass. Seven retained overlay registrations now execute their original semantic assertions: `number`, `string`, `parsedAny`, `sizeWithoutType`, `knownTypeRetained`, `listMember` and `unsetOverlay`. The old Set-trace assertion is one target publication; rejection is checked by unchanged target/work and zero publication. Original tests requiring general later-link subscriptions, registry inference or mutation of intermediate sources remain pending. Existing nested scalar relationships still remain expressed; the new transitive check preserves that contract.

TestProteus regenerates and builds in **153.11 seconds**, and the separate shared library in **100.50 seconds**. The 378-case adoption corpus is entirely unchanged. The full retained run reports **1592 failures / 1881 tests**, with no new failures or category regressions: default fixtures are 146 pass / 186 unsupported; compiled registrations are 143 pass / 2 unsupported / 1397 not ported / 7 existing reference-registry failures. All prior 1548 registrations, 58 original parser/model cases, 15 protected fixture hashes and the reference assertion-source hash are preserved. Both generated targets contain the same correction without legacy solver dependencies. Evidence and before/after clients are in `/tmp/proteus-unset-overlay/`; unrelated `myTests.tst` remains untouched.


### Retained list and signed-number test ports, 2026-10-11

Ten more original registrations now run through WorldManager: `graphFoundation/nestedAndEmbedded`, `countedNested`, `countedEmbedded`, `sparseStillLazy`, `childConflict`, `listTemplate`, `emptyList`, and `graphSignedNumber/parser`, `replayAndCopy`, `sizeModes`. No production code changes in this increment. The historical assertion source and registration names remain intact in the migration inventory.

The list ports distinguish an ordinary nested list's one outer position from an embedded list's members; remove parser-provided counts to verify the inferred sizes; preserve child completion and occurrence identities; leave a billion-member tail unmaterialized; reject a conflicting child before publication; and normalize finite templates and the empty list. Signed tests preserve rational values, sign flags and size modes through parser round trips, normalize negative offsets only once, and copy the normalized values without reapplying the offset. Repeated completed requests create zero frames and publications while keeping payload pointers unchanged. General late-subscription and definition-dependent cases remain pending.

Independent shared-library clients first passed all seven list cases and all eight signed numeric examples. The regenerated TestProteus build then succeeded in **150.88 seconds**. All **19 WorldManager suites** still pass, and the full run reports **1582 failures / 1881 tests**: ten former NOT_PORTED failures now pass, with no new failures or category changes. Default fixtures remain 146 pass / 186 unsupported; compiled registrations are 153 pass / 2 unsupported / 1387 not ported / 7 existing reference-registry failures. All 1549 registration names, 58 original parser/model cases, 15 protected fixture hashes and the historical source hash are unchanged. The shared library's production source is unchanged from the preceding validated build. Evidence is in `/tmp/proteus-historical-lists/`; unrelated `myTests.tst` remains untouched.


### Closed targets bounding open sources, 2026-10-11

D03O composes the existing finite correspondence in the other direction. `{_ _} = {5| ...}` now becomes `{5 5}`, `{_ _ _} = {1 ... 3}` becomes `{1 _ 3}` and stays partial, and `{_ _} = {*10+_| &{7| ...}}` becomes `{*10+7 *10+7}`. Source templates, prefixes and uniquely determined spans use the same member merges. Publication preserves target occurrences and borrowed source structure. Shared sources can serve independent target endpoints, and newly created ordinary open members retain their templates through a later explicit request.

A measured shared-source case exposed unnecessary expansion of empty occurrence paths. Nonnegative span widths summing to zero must all be zero; the checked extent and representability summaries therefore suffice. An unchanged client linked against the regenerated shared library now uses **7/9/11/13 frames** at source depths **4/6/8/10**, instead of **38/136/522/2060**, and one sequence-node visit at each depth. Regression checks extend through **128 levels**, using **131 frames, 257 relationship reads and one sequence visit**. Every borrowed source endpoint remains symbolic. Invalid minimum widths and given nonzero endpoints cannot use this proof.

For **32/64/128** represented target members under two source requirements, work uses **141/269/525 frames**, **37/69/133 relationship reads** and nine sequence visits. Each requirement applies once per member. With **32/64/128** nested source wrappers, work uses **72/136/264 frames** and **68/132/260 sequence visits**. These bounds concern the measured unambiguous projections; arbitrary shared-target constraints or alternative bindings have no general linear claim. Parser waits continue at the exact prerequisite, cancellation publishes nothing, and late contradictions leave target members unchanged.

The regenerated TestProteus build succeeds in **150.65 seconds**, and the separate shared-library build in **95.84 seconds**. All **20 WorldManager suites** pass, including **508/508 assertions** in the new bounded-source suite. All six original language probes change from unsupported to their expected results. A 378-case capture comparison has **340 identical outputs** and **38 transitions from unsupported to results or preserved partial constraints**; no previously supported output changes. These diagnostic capture fixtures are not ordinary passing tests.

The full retained run reports **1582 failures / 1882 tests**, with no new failures or outcome-category regressions: default fixtures remain **146 pass / 186 unsupported**; compiled registrations are **154 pass / 2 unsupported / 1387 not ported / 7 existing reference-registry failures**. All 1549 previous registrations, 58 original parser/model cases, 15 protected fixture hashes and the reference assertion-source hash are preserved. The additional registration is the bounded-source regression. The historical `spans/reverseTail` test remains pending because it also requires source mutation; that assertion has not been removed merely to mark the test ported.

The implementation adds **23 production lines**, reusing correspondence, projected facts and extent checks. Both generated targets contain the same implementation without GraphManager/GraphNormalizer or legacy namespace dependencies. Ambiguous positive widths, general two-open-list alignment, grouped pending source comparisons, inverse expressions, definitions, intersections and planning remain later work. The printer still uses its existing repeated-view notation for shared templates; their semantics and later closure are tested directly in the stored model. Slipstream rendering is not covered. Evidence is in `/tmp/proteus-bounded-source/`; unrelated `myTests.tst` remains untouched.


### Retained boundary assertions and next-case audit, 2026-10-11

Three additional original registrations now execute their substantive assertions: `graphFoundation/string`, `copyPrefix` and `fractionalSpanExtent`. They cover string value/size acquisition, direct prefix copying with size-mode/inversion metadata preservation, and refusing to infer an integral parent extent from a fractional embedded span. The independent generated-library probe passes all three. The sibling inverted-span test remains pending: its original partial-result expectation is not weakened to accept the current unsupported status.

TestProteus regenerates successfully in **154.44 seconds**. All **20 WorldManager suites** pass. The full run reports **1579 failures / 1882 tests**, with exactly the three former NOT_PORTED cases now passing and no other category changes: default fixtures remain **146 pass / 186 unsupported**; compiled registrations are **157 pass / 2 unsupported / 1384 not ported / 7 existing reference-registry failures**. All 1550 compiled registration names, 58 original parser/model cases, 15 protected fixture hashes and the historical source hash are preserved. This is a test/documentation-only increment; the shared library remains the separately regenerated D03O build.

The seven generic reference-registry assertion failures correspond to six distinct expressions: unordered and ordered `.b` selection, a positive index, `.first`, `.last`, and a positive span. A shared-library client confirms that each normalize request stops at the unsupported-semantics reader check after one frame, with zero publications and selection references. The ordered `.b` expression serves two registrations. Their tests still fail with their original assertions; this audit does not silently reclassify them as passing or remove their registration.

The same audit records the next partial-correspondence examples in the current design and decision log: forced endpoints around ambiguous gaps, bounded two-open layouts, incoming prefixes and later evidence for a pending source. No new chart, planning or subscription contract is chosen. Probe sources, logs and the integrity comparison are in `/tmp/proteus-retained-boundaries/`; unrelated `myTests.tst` remains untouched.


### Single-gap open-source correspondence follow-up, 2026-10-11

D03P aligns represented prefixes from the start and suffixes from the end while keeping the middle width symbolic. Before combining those boundaries, the ordinary member merge must disprove every shorter overlap. Existing correspondence then handles member pairs, scoped requirements and occurrence-preserving publication. No parser, printer, planner, path-cost evaluator or alternative scalar matcher is added. Unsettled overlap alternatives retain the unsupported boundary without publication.

Five original default fixtures now pass unchanged: `merge/list/dotsC`, `merge/dots/align2a`, `align2b`, `align3` and `align4`. The sixth, `align4b`, remains unsupported. Its input admits both `{1 4 5 6}` and `{1 4 4 5 6}`, which its historical expected output excludes; the old Proteus4 executable also fails that fixture. The user subsequently confirmed that the expected text was wrong. The interim fixture expectation is `{1 4 ... 5 6} = {... 4 5 6}`, recording the forced final `6` and retaining the unresolved source constraint; the user subsequently deferred choosing its final representation. The current engine still fails it as unsupported. Regression cases verify both valid concrete models and rejection of `{1 4 7 5 6}`. The user proposed a form such as `{1 *_+[4 &{4 ... 4}] 5 6}` and then postponed this case. Its normalization contract remains pending alongside the general representation of ambiguous correspondence.

The regenerated TestProteus build succeeds in **157.69 seconds**, and the separately regenerated shared library in **103.23 seconds**. All **21 WorldManager suites** pass. The new suite includes **226/226 behavioral assertions** covering nested and embedded occurrences, shared immutable sources, retained future templates, subsequent finite closure, contradiction, enormous unexpanded endpoints, parser arrival and cancellation. An independent finite-word recognizer checks **512 pattern pairs** against all binary words of lengths zero through six: **331 accepted / 117 rejected / 64 unsupported**, with **56,896 comparisons and zero counterexamples**. This finite oracle is supporting evidence, not a proof for all lengths or arbitrary member semantics.

For **32/64/128** incoming members on each side, alignment uses **76/140/268 frames** and **six sequence-node visits**, retaining exactly one symbolic remainder. For **32/64/128** nested source wrappers it uses **43/75/139 frames** and **37/69/133 visits**. The overlap proof has its own comparison counter and may require quadratic work in represented boundary lengths; it never enumerates the unknown gap. A **378-case** diagnostic capture comparison has **351 identical outputs** and **27 transitions from unsupported to ordinary results or preserved constraints**, with no change to a previously supported output. Capture fixtures deliberately use mismatched expected text and are not passing-test totals.

The full retained run reports **1574 failures / 1883 tests**: default fixtures are **151 pass / 181 unsupported**; compiled registrations are **158 pass / 1384 not ported / 2 unsupported / 7 existing reference-registry failures**. Exactly the five named default cases newly pass, plus the new regression registration; no remaining failure changes text or category. All **1550 previous registrations**, **58 original parser/model cases**, **15 protected fixture hashes** and the historical assertion-source hash remain intact. Existing regression expectations for newly supported open correspondence now assert its semantic result instead of the former unsupported boundary.

The maintained WorldManager source adds 168 lines and removes 17 (**net 151**). Both generated targets use this implementation without legacy solver dependencies. Multiple gaps, given endpoints on an open layout, viable shorter overlaps, unresolved source work and qualified list matching remain explicit limits. Slipstream rendering is not covered. Sources, logs, reference comparisons and independent library probes are in `/tmp/proteus-open-source-alignment/`; unrelated `myTests.tst` remains untouched.


### Confirmed alignment fixture correction, 2026-10-11

The user confirmed that `defaultTests.tst:merge/dots/align4b` had an incorrect expected result. Only that expected field changes, from `{1 4 &{4 5 6 ... } 5 6}` to `{1 4 ... 5 6} = {... 4 5 6}`. The final value `6` follows from the shared endpoint. Retaining the complete RHS keeps its suffix constraint, including the condition on the member preceding `5`, without choosing a gap width. This was an interim partial-result target; the subsequent decision below postpones choosing the final normalized representation. It does not accept the replacement engine's current unsupported result. The fixture hash manifest records both the former and corrected hashes and the reason for this user-authorized exception; all other fixtures and registrations remain unchanged. No engine code changes in this correction.

The reference Proteus4 executable passes the corrected fixture (**0/1 failures**). The replacement still reports the existing open-source alignment boundary (**1/1 failures**), preserving this as outstanding work. Verification confirms exactly one expected field changed, with the input, test name and all other fixture contents preserved; all 15 fixture hashes match the updated manifest. Logs are in `/tmp/proteus-align4b-correction/`.


### Ambiguous alignment representation deferred, 2026-10-11

At the user's request, postpone `merge/dots/align4b`. The proposed `{1 *_+[4 &{4 ... 4}] 5 6}` expresses the intended distinction between a single `4` shared by the two anchors and distinct boundary `4`s around a symbolic span. The current parser/printer round-trips that spelling, but this does not validate normalization or choose it as the canonical form. Other representations remain possible. The existing retained-RHS fixture expectation is provisional; keep the input and registration visible as a pending failure. No new fixture or engine change accompanies this decision. Continue with settled functionality in `defaultTests.tst`; D03Q establishes targeted member correspondence as the prerequisite for the `getLastItem` group and other intersections, independently of this alternative representation choice.


### Member-directed correspondence and intersection selection, 2026-10-11

D03Q now supplies a member RHS without normalizing either containing list. A fresh occurrence can exhaust its own work, use scoped listSpecs, then locate a corresponding RHS through an immediate parent relation or embedded ancestor coordinates. The four parent operators preserve their derived member qualifications. Intersection selection supplies the occurrence and, for `<~`, a required parent matching relation. Scalar solving, constraint comparison and publication remain shared with the existing engine. Repeated scalar predicates require a proved disjoint exit and proof for each consumed candidate; mere compatibility cannot select a repetition count.

**Original fixtures:** all nine `getLastItem` cases, `query/first`, `query/getNth`, `query/firstStrB/C`, `marked/getNotLast`, and the three `notTemplate` cases now pass unchanged: **17 newly passing original fixtures**. The selected `$` in the first-string fixtures has no own RHS; its answer comes through enclosing correspondence. A nested list can be excluded by type without evaluating its contents, including a deliberately unsupported `rot` member. Two additional reproductions exposed lost selection publication metadata under listSpec/correspondence projection; preserving the marker fixes both, and the regression also verifies that the published member can participate in a later list goal.

The new `unit/worldManager/memberCorrespondence` suite passes **845/845 assertions**. It covers member-only entry, all four parent qualifications, scoped templates, explicit size demand, own-work completion before parent lookup, unchanged siblings and borrowed bodies, pending parent/candidate parsing, continuation without replay, cancellation, suffix contradiction, circular prerequisites, malformed states, independent contexts, and projection/publication composition. An independent finite oracle checks **363 sequences** of numbers, ordinary lists and strings against their first-string positions. Unsupported ambiguity or an uncertain candidate produces no guessed result.

For **32/64/128** ordinary nested-list candidates, an independent shared-library client measures **42/74/138 structural visits**, **35/67/131 predicate comparisons**, and **five value frames** at every size. It passes all **12 checks**, including positive and negative repeated-value patterns. Whole-list normalization retains its existing shared traversal. These bounds concern the supported represented routes and scalar predicates; general alignment and nested shared constraints have no blanket linear claim.

The final CodeDog test build succeeds in **185.52 seconds**, and the separately regenerated shared library in **120.39 seconds**. Sixteen changed graph/solver method bodies match between the generated targets. All **22 WorldManager suites** pass. The full retained run reports **1557 failures / 1884 tests**, with no new failures or category regressions: default fixtures are **168 pass / 164 unsupported**; compiled registrations are **159 pass / 1384 not ported / 2 unsupported / 7 existing reference-registry failures**. All **1551 previous registrations**, **58 original parser/model cases**, **15 protected fixture hashes**, and the historical assertion-source hash are preserved. The inventory includes the one new suite and repairs nine previously omitted entries for already-existing suites; that repair adds no tests.

Ordinary ancestor-list projection, contextual RHS projection, known-extent skipping/indexes, structured predicates, general qualified whole-list matching, overlapping pattern alternatives and negative/span selection remain later work. In particular, `marked/lookahead` still needs overlapping-pattern correspondence, and `align4b` remains deferred. Planning/path-cost policy is unchanged. Slipstream rendering is not covered. Evidence is in `/tmp/proteus-member-implementation/`; unrelated `myTests.tst` remains untouched.


### Overlapping scalar-pattern correspondence, 2026-10-11

D03R replaces the disjoint-only matcher with a streaming chart. A repeated predicate and its following pattern remain viable together; each pattern/source pair is checked once, and matching later predicates reject only the incompatible paths. Captures retain occurrence identity and share candidate evidence. Equal remaining predicates pack their possible captures without enumerating repetition lengths. The old scalar comparator, borrowed-position traversal and final RHS/publication path remain in use. The maintained production source has **198 added / 103 removed lines (net 95)**.

The unchanged primary fixture `defaultTests.tst:marked/lookahead` now returns **11**. Longer lookahead, consecutive overlapping repetitions, multiple repeated regions, expression candidates and embedded input use the same mechanism. Multiple selected occurrences remain partial with no publication, even if their values agree. Zero-width exits are processed before source reads: `[<1> &{_|...}]<~{1 rot}` now returns `1` without evaluating the optional `rot`, and the same capture can finish before a symbolic input tail. Required suffix contradictions still prevent publication.

The new `unit/worldManager/overlappingPatterns` suite passes **9,556/9,556 assertions**, including an independent enumerator over **4,732 pattern/input cases**. It considers every allowed repetition length and input prefix, covering both unique and ambiguous captures. No-match cases must be partial or rejected; unsupported/error results do not count as successful oracle outcomes. Candidate and suffix-predicate parser arrivals retain the same comparison counts as synchronous input; cancellation and borrowed-source immutability are checked. The former unsupported lookahead assertion in the member-correspondence suite is now a success case in the new suite; all other **843/843 member-correspondence assertions** still pass.

For **32/64/128** numeric source members and one selected member before a string, structural visits are **43/75/139**, comparisons **98/194/386**, and value frames **six** in every case. With four-member lookahead, comparisons are **188/380/764** and value frames nine; with twelve-member lookahead, comparisons are **384/832/1728** and value frames seventeen. The source is walked once. These measurements support the fixed-pattern linear bound; varying both pattern and source permits O(PN) pairs. The earlier disjoint-only fast path needed fewer comparisons per input member; the shared matcher accepts that constant-factor tradeoff to handle overlapping paths without duplicating semantics.

Both targets regenerate from CodeDog. An independent client linked against the shared library passes **13/13 checks**, including primary/longer/consecutive overlap, ambiguity, optional suffix pruning, direct asserted member access, parser continuation and the three scaling samples. All six changed solver methods match between generated targets, with no legacy GraphNormalizer/GraphManager definitions. All **23 WorldManager suites** pass. The ordinary full retained run reports **1556 failures / 1885 tests**, down from 1557 / 1884: one original fixture now passes and one new suite is added, with no new failures or changed remaining failure messages. Default fixtures are **169 pass / 163 unsupported**; compiled registrations are **160 pass / 1384 not ported / 2 unsupported / 7 existing reference-registry failures**.

All **1552 previous registrations**, **58 original parser/model cases**, **15 protected fixture hashes**, the historical assertion-source hash and unrelated `myTests.tst` are preserved. Both design documents and the test inventory reflect this increment. Whole-list `merge/lookahead`, general dependent/structured predicates, unresolved comparison obligations, and `align4b`'s alternative representation remain pending. Planning and path-cost policy are unchanged. Evidence is in `/tmp/proteus-overlapping-patterns/`; this increment does not cover Slipstream rendering.

### Whole-list prefix mapping and publication, 2026-10-11

D03S extends the existing overlap chart to whole-list `=:` goals. The unchanged primary fixture `merge/lookahead` now produces **`{&{33 22} 11 'hi'}`**. Shared predecessor/choice records retain member and span assignments; a unique accepted mapping supplies widths to ordinary list correspondence and staged publication. Ambiguous span assignments publish nothing. Source models remain unchanged, target occurrences are retained, and an irrelevant source suffix is not normalized.

The new `unit/worldManager/listPatternMappings` suite passes **9,618/9,618 assertions**, including **4,732 exhaustive pattern/input cases**. Its independent oracle enumerates all complete assignments and accepted prefix endpoints. Additional checks cover inherited requirements, nested/zero-width spans, scalar expression candidates, source and target parser waits, cancellation, live packed alternatives, declared source endpoints, source immutability and repeated-normalize completion. Two prior native unsupported-boundary assertions now assert the new behavior described in D03S; no protected language expectation changes.

Two concrete edge cases were reproduced and fixed during verification. Known endpoints are carried through the source structural cursor, so a too-long embedded span cannot supply extra members and a too-short span cannot move a later sibling into a missing position. The check is local to demanded traversal, preserving a valid earlier prefix. Also, a source arriving from the parser can bring relationships as well as members. Checking its wrkList before parsing completes incorrectly accepted an unproved source layout. The shared-library reproduction changed from five publications and an incorrect completed result to **zero publications**, with the arriving relationship retained and the unresolved-layout boundary reported. The source check now follows parse completion without restarting the chart.

For **32/64/128** numeric source members followed by a string, the simple whole-list pattern uses **34/66/130 structural visits**, **98/194/386 comparisons**, **65/129/257 mapping records**, and **75/139/267 value frames**. Four-member lookahead uses **188/380/764 comparisons** and **155/315/635 records**; twelve-member lookahead uses **384/832/1728 comparisons** and **351/767/1599 records**. These are one shared source traversal with bounded records per successful pair, not per-member searches from the start. General pattern growth still permits O(PN) pairs; full mappings use more storage than a single selected capture.

Both targets are regenerated from CodeDog: the final test build succeeds in **196.34 seconds** and the separate shared-library build in **122.17 seconds**. A standalone client linked against the generated library also passes all **9,618 checks**. Six changed graph/solver methods match between generated targets; the library contains no legacy GraphNormalizer/GraphManager definitions. All **24 WorldManager suites** pass. The full retained run improves from **1556 failures / 1885 tests** to **1555 / 1886**, with `defaultTests.tst:merge/lookahead` the only resolved old failure and no new failures or changed remaining failure messages. Default fixtures are **170 pass / 162 unsupported**; compiled registrations are **161 pass / 1384 not ported / 2 unsupported / 7 existing reference-registry failures**.

All **1553 previous registrations**, **58 original parser/model cases**, **15 protected fixture hashes**, the historical assertion-source hash and unrelated `myTests.tst` are preserved. Both design documents and the test inventory include this milestone. Constrained open target patterns, unresolved source-layout relationships, general structured/dependent predicates and non-parser comparison obligations remain later work. **`align4b`, planning and path costs remain deferred.** Evidence is in `/tmp/proteus-whole-list-lookahead/`; this increment does not validate Slipstream rendering.


### Finite scalar one-of domains, 2026-10-11

D04A implements the finite scalar alternative forms that previously stopped at “Intersection needs additional selection semantics.” The five primary fixtures are `merge/num/literalSelect`, `merge/str/literalSelect`, `merge/intersect/onRHS`, `altSelect/simple` and `innr/oneOf`. The engine borrows candidate frames and uses ordinary derived merges to filter them. Contradictions remove only the affected OR branch; unknown values and ranges remain possible answers. Several answers publish as a partial one-of, and a later goal can narrow that result. Scalar equality consolidation preserves the distinction between value alternatives and occurrence-specific pattern captures.

The new `unit/worldManager/alternatives` suite contains **672 assertions**, including **225 finite-set conjunction cases** checked against an independent bit-set oracle. It covers both operator directions, source immutability, nested scalar alternatives, listSpec filtering, expression candidates, unknowns, ranges, branch-local and required-route rejection, independent partial output, local domain/candidate/constraint parser waits, cancellation and no-new-goals completion. A standalone client linked against the regenerated shared library passes **672/672**. With **16/64/256** candidates and one scalar constraint, the request records exactly **16/64/256 merge pairs** and **35/131/515 frames**. Two OR domains may still require their Cartesian product; nested flattening and payload comparison costs are separate.

Verification found and corrected three integration hazards: a narrowed scalar RHS must discharge its original one-of relationship; parser completion must precede domain classification and layout-work inspection; and direct graph comparisons must not treat an undistributed OR as an unconstrained unknown. An additional reproduction showed `*_+[...] <~ *2+{1}` incorrectly collapsing to `1`. Enabling the existing structural cursor's endpoint checks also fixes too-short and too-long embedded domains. All four endpoint regressions now remain partial without publication. No new span-alignment algorithm was added.

Twelve changed graph/solver methods match between the generated test and shared-library targets; the library contains no legacy GraphNormalizer/GraphManager definitions. The maintained production change adds **192 lines and removes 12**. Structured alternatives, general patterned inputs, branch-dependent expression/template/chart evaluation, source comparisons still needing evidence, and demand-pushed enumeration remain future work. **`align4b`, planning and path costs remain deferred.** This increment does not validate Slipstream rendering. Evidence is in `/tmp/proteus-oneof/`.

Both targets regenerate successfully from CodeDog: **196.49 seconds** for TestProteus and **131.54 seconds** for the separate shared library. All **25 WorldManager suites** pass. The ordinary full retained run improves from **1555 failures / 1886 tests** to **1550 / 1887**, with exactly the five original fixtures above resolved, no new failures, and unchanged remaining failure messages. Default fixtures are **175 pass / 157 unsupported**; compiled registrations are **162 pass / 1384 not ported / 2 unsupported / 7 existing reference-registry failures**. The previous member-correspondence, overlapping-pattern and whole-list mapping suites retain **843/843**, **9556/9556** and **9618/9618** passing assertions.

All **1554 previous compiled registrations**, **58 original parser/model cases**, **15 protected fixture hashes**, the historical assertion-source hash and unrelated `myTests.tst` are preserved. Both design documents and the migration inventory include this milestone.

### Open results and bounded selected spans, 2026-10-11

D04B resolves the seven primary fixtures previously reporting “Selected position needs open-span selection”: `merge/func/loose`, `merge/func/looseSize`, `merge/func/typed`, `func/simpleList1`, `func/simpleList2`, `func/simpleList3` and `slice/select`. Open result facts use ordinary scalar/list merging; marked embedded spans supply a bounded source window to ordinary list correspondence. Top-only size qualifications remain partial where appropriate. Borrowed bodies, sources and unrelated siblings are unchanged.

The new `unit/worldManager/openSelection` suite and the standalone shared-library client cover **425 assertions**, including **70 independently calculated offset/length windows**. Checks include direct asserted span goals, selected listSpecs, source ancestor templates, source endpoints, neutral prefix/suffix members, cyclic layouts, exact parser waits, late source relationships, cancellation, atomic rejection, partial open-result publication and completion without new goals. The direct-member probe reproduced the `FlexNum(int)` formatting-constructor trap; unit widths now use arithmetic increment. One former internal unsupported-boundary assertion, `? == {1 ...}`, now verifies supported partial adoption. All protected language expectations remain unchanged.

A **10-position** and a **1,000,000,000-position** symbolic prefix each require **five frames and eleven layout visits** to return `{8 9}`. Known widths avoid expanding irrelevant virtual positions. Bounded lookup uses one retained source cursor for the whole requested span, not a fresh parent search for each output member. At fixed depth the cost is proportional to inspected represented layout and demanded members, plus ordinary value/merge/output work; occurrence-path comparisons add depth-dependent costs.

The maintained production change adds **156 lines and removes eight**. Eight changed methods match between the generated test and library targets, and the library contains no legacy GraphNormalizer/GraphManager definitions. Constrained bounded prefix/suffix matching, enclosing target-template projection, unknown-width span selection and general structured patterns remain explicit boundaries. **Planning, path costs and `align4b` remain deferred.** Evidence is in `/tmp/proteus-open-selection/`; this increment does not validate Slipstream rendering.

Both targets regenerate successfully from maintained CodeDog source: **202.41 seconds** for TestProteus and **130.14 seconds** for the shared library. All **26 WorldManager suites** pass, including **425/425** open-selection assertions; the independent library client also passes **425/425**. Member correspondence, overlapping patterns, whole-list mappings and alternatives retain **843/843**, **9556/9556**, **9618/9618** and **672/672** passing assertions.

The full retained run improves from **1550 failures / 1887 tests** to **1543 / 1888**: exactly the seven original fixtures above resolve, with no new failures or changed remaining failure messages. Default fixtures are **182 pass / 150 unsupported**; compiled registrations are **163 pass / 1384 not ported / 2 unsupported / 7 existing reference-registry failures**. All **1555 previous compiled registrations**, **58 original parser/model cases**, **15 protected fixture hashes**, the historical assertion-source hash and unrelated `myTests.tst` are preserved. Both design documents and the migration inventory include this increment.

### Named meanings, indexed queries and nominal search

The D05A increment makes `query/getNthSub`, `query/firstType`, `word/deref` and `word/useTags` pass with their original expectations. Declarations supply shared meaning bodies through ordinary RHS requests. Indexed selection borrows a named list's layout; nominal search reuses the existing correspondence matcher. No dot-name shortcut, copied definition instance, old-solver fallback or path-cost planner is introduced.

`unit/worldManager/namedQueries` passes **299/299 checks** in both the test executable and an independent client linked against the shared library. Coverage includes named list/scalar values, inline declarations, expression and intersection meanings, equal-valued distinct types, negative type predicates, stable bindings, explicit model IDs, ambiguous meanings, borrowed nested/repeated layouts, cycles, private declaration staging, failed/cancelled publication, repeated partial requests, parser continuation and unsupported record/inheritance boundaries. In particular, a possible subtype must not be skipped merely because its written name differs. Copying a declaration operation as a list value remains unsupported and publishes nothing.

Measured work:

| Request | Cases | Observed work |
| --- | --- | --- |
| Normalize repeated uses of one scalar meaning | 1, 16, 128 uses | 3, 18, 130 frames; one body evaluation plus one relationship per use |
| Select a position from a named list | Positions 8, 64, 512 | Two frames, one virtual named layout and no whole-meaning normalization; 11, 67, 515 inspected positions/wrappers |
| Search past nominal nonmatches | 4, 32, 256 skipped members | Six frames, two demanded meanings and 10, 66, 514 chart checks |

The final CodeDog builds succeed separately: TestProteus in **213.87 seconds**, and the shared library in **134.12 seconds**. All **93** graph, manager, request and Agent-entry method bodies match between generated targets. No legacy GraphNormalizer/GraphManager definitions are present. All **27 WorldManager suites** pass. The full retained run changes from **1543 failures / 1888 tests** to **1539 / 1889**, with four resolved original failures and no newly failing test names. Default fixtures are **186 pass / 145 unsupported / 1 output mismatch**; compiled registrations are **164 pass / 1384 not ported / 2 unsupported / 7 existing reference-registry assertion failures**.

**Exposed compatibility gap:** `lang/splitText` was already failing, formerly at the generic nominal-type guard. It now preserves its supplied `english-text` string, while the unchanged expectation requires an `english-phrase` word list. Proteus3 implements that language-specific transformation in `splitLangText`; the checkpoint implementation owns it in `graph/WordForms.h`. That intrinsic transformation has not been ported. This is an explicit remaining semantic gap, not a changed expected result or a claim of complete word-form support. The other 87 changed remaining failure descriptions still report unsupported operations, generally after successful declaration registration or more world-loading progress.

All **15 protected fixture hashes**, **58 original parser/model registrations**, the historical assertion-source hash and unrelated `myTests.tst` are unchanged. The inventory contains every previous compiled registration plus the new named-query suite. Evidence is in `/tmp/proteus-query-search/`. That milestone preceded generic dot/part-path selection, nominal record-field correspondence and chained reads/writes; their current contracts appear below. Adjective-search composition remains later work. General inheritance, word forms, late declaration delivery and contextual overload search remain open. **Planning, path costs and `align4b` remain deferred.**

## Shared correspondence consolidation (2026-10-12)

The user approved the four-step consolidation following the cross-route code review: define a common correspondence request/result, expose logical list layouts consistently, route member/span/whole-list operations through that contract, and make dot/index reads and writes consumers of the same selected occurrences. Validation must compare these routes across equivalent representations as well as their local correctness. Planning and path-cost estimation remain deferred.

`WorldCorrespondenceQuery` carries the target occurrence, qualified relation, pattern-testing obligation, and requested whole-list coverage. `WorldCorrespondenceResult` carries the selected occurrence/facts and correlated bindings. `WorldCorrespondence` owns retained algorithm progress. An asserted lookup may acquire facts without proving every unrelated sibling; a tested pattern must establish its required predicates. These are semantic obligations, independent of whether a deterministic cursor or a branching chart discharges them.

The first implementation stage separates the existing request/result data from its cursors and chart and gives parent, intersection, and whole-prefix requests one constructor. This stage deliberately preserves behavior; logical-layout consolidation and path consumers follow it.

Logical layout and finite capture stage: one `enterLayout` operation exposes literal, embedded, concatenated and directly named list layouts to all structural consumers. A logical embedding belongs to its occurrence; concatenation does not mutate stored POV flags. Named layouts retain their use ancestry. Scalar members and fixed spans now use the same coordinate traversal, explicit prefix/suffix predicates and symbolic-width skipping. On reaching a variable-width event, the existing chart continues the remaining cursors with any already established capture; it does not replay the prefix. Nominal predicates can use explicit names without demanding a declaration. Source layouts with unresolved additional relationships remain a guarded boundary.

Validation: all 28 WorldManager suites pass, including 86 new cross-route/cancellation checks. All 12 temporary language probes pass; 11 also pass on the preserved reference engine, whose billion-item scalar skip remains unresolved. The replacement uses fewer than 20 structural visits for both member and span skips at extents 10 and one billion. The full retained inventory is 1539 failures / 1890 tests, with exactly the prior failure names. Protected fixtures and historical registrations are unchanged. One native boundary assertion moved into the existing rejection group: the shared member route can now prove that selected literal 3 contradicts both alternatives 1 and 2.


Final consumer validation: all **29 WorldManager suites** pass, reporting **26,418 native checks**. The two new suites contribute **250 checks** (86 correspondence-composition and 164 path-consumer checks); an independent client linked against the regenerated shared library passes all 250 as well. All **47 focused language probes** pass. Both targets were regenerated from maintained CodeDog: TestProteus in **183.88 seconds**, the library in **121.93 seconds**. All **103** graph/manager/request/Agent-entry method bodies match between generated targets, and neither target contains the old graph solver.

The full retained inventory improves from **1539 failures / 1889 tests** to **1519 / 1891**, with **20 recovered default cases and no newly failing names**. The default file is now **206 pass / 124 unsupported / 2 output mismatches**. Compiled registrations are **166 pass / 1384 not ported / 2 unsupported / 7 existing reference-registry failures**. Recovered cases cover dot reads/writes, positive and negative indexing, first/last updates, field/index chains, nominal record reads, a path-supplied indexed layout, and the protected slice-sugar case. All 1557 previous compiled registrations, 58 historical parser/model tests, 15 protected fixture hashes, the original assertion-source hash, and unrelated `myTests.tst` are preserved.

Remaining output mismatches are the previously exposed `lang/splitText` word-form transformation and `write/openNegativeLast`: the latter now retains its unresolved end-relative update expression because no endpoint is proved, whereas the fixture expects only the original open list. It does not claim completion or publish the write. The other 37 changed remaining failure diagnostics still identify unsupported work. `query/chain3` and `query/chain4` were blocked at that milestone by field correspondence; D05B below supplies it while preserving the returned parent's definition obligations. General sparse interiors, bounded repeated predicates, inherited record fields, scalar slice specifications, boundary/temporal providers, and definition-instance writes remain future semantics. Evidence is in `/tmp/p4-final-*` and `/tmp/p4-acceptance-test-build.log`.

Cost checks compare extents 10 and one billion with fewer than 20 structural visits for member/span skips and indexed path access. Increasing an unused path-returned suffix from 8 to 512 members stays below 30 value frames and 40 structural visits. The existing overlap and whole-list mapping oracles, each covering 4732 pattern/input cases, still pass. Parser waits retain the demanded cursor; cancelled/conflicting multi-field and span writes publish nothing. No protected expectation was edited to obtain these results.

## Structured parsing through correspondence (2026-10-12)

D03T extends whole-list correspondence to type-changing `==` and `==:` patterns. It does not change parsing syntax. `WorldPatternChart` belongs to the existing `WorldCorrespondence` operation. Each `WorldPatternGoal` identifies a checked template and an input boundary and returns possible end boundaries with projected result frames and shared proofs.

- A scalar terminal uses the existing scalar matcher against one source member or an established-width text window.
- A list sequences child goals and retains its nested or embedded output shape. Its listSpec supplies repeated child goals; any given size constrains resulting member count.
- A finite OR requests each existing alternative template at the same boundary.
- The root accepts an endpoint match for `==`, or a prefix match for `==:`. Multiple valid outputs or unbounded zero-width repetition remain partial and unpublished.

Equivalent continuation states share their future work and retain ambiguity in shared proof nodes. The scalar matcher, requirement projection, result sequences and publication are unchanged consumers of these results. Sources remain immutable and existing target member occurrences survive publication. Completion includes member completeness; a resolved mapping with unknown member facts cannot mark the containing list complete.

The virtual list cursor visits only demanded source structure and shares acquired candidate facts across branches. Text windows use a lazy index of borrowed source leaves, retaining a leaf cursor for sequential access and binary lookup for a nonsequential branch. This avoids repeatedly traversing a concatenation tree from its root. Fixed-width repetitions have linear semantic work and output size; map lookup costs and ambiguous boundary combinations are accounted for separately. Exact parser dependency waits retain all earlier progress, and cancellation releases it without publication.

Named templates acquire their available meanings before a type-changing test. An unconstrained named alternative domain contributes its member alternatives, preserving their nominal types. General nominal inheritance is unchanged. Tests now exercise these semantics through the original `parse/lang` fixture rather than requiring a special grammar-name handler.

Remaining boundaries include general selected list regrouping, recursive grammars, open-source span completion, unknown-width text terminals, negative structured patterns and unresolved template relationships. Existing assertions, strict list merging, deterministic correspondence, path consumers, planning and path costs retain their previous scope. No Slipstream rendering claim follows from these engine tests.

**Acceptance evidence.** All **19 `parse/` tests** pass (14 original default fixtures and five compiled syntax-error tests). All **30 WorldManager suites** pass, including **350/350 structured-parsing assertions**; the same 350 assertions pass in an independent client linked against the regenerated shared library. All **56** combined correspondence/path/representation probes pass. Checks cover fixed and nested groups, text fragments, repetitions with suffix overlap, finite empty tokens, scoped ancestor templates, named alternatives, rejected and ambiguous mappings, parser waits, cancellation, source immutability, completion reuse and traversal growth.

The full retained inventory improves from **1519 failures / 1891 tests** to **1510 / 1892**. Exactly the eight requested default failures and `unit/graphText/identityInSource` recover. No new failing names or changed remaining diagnostics occur. Defaults are **214 pass / 116 unsupported / 2 existing output mismatches**. Compiled registrations are **168 pass / 1384 not ported / 1 unsupported / 7 existing reference-registry failures**. All **1559 prior registrations**, **58 historical parser/model tests**, **15 protected fixture hashes**, the reference assertion hash and unrelated `myTests.tst` remain unchanged.

Both targets regenerate from maintained CodeDog: TestProteus in **200.61 seconds** and the library in **132.06 seconds**. All **113** graph/manager/request/text-cursor/Agent-entry method bodies match across the generated targets, with no legacy GraphManager or GraphNormalizer implementation linked. The new definition-ordering check initially dereferenced a deliberately empty worklist edge; the existing malformed-input regression and a debugger trace caught it, and the normal empty-edge error path is preserved before acceptance. Evidence is in `/tmp/p4-parse-*`. The remaining semantic boundaries listed above are unchanged by successful compilation or the reduced failure count.


## Selected structured correspondence (2026-10-12)

D03U connects member-only requests and intersections over text to the structured span matcher. `concat/parseSrc`, fixed-width token selection, nested text groups and overlapping repetition use the same token-window and proof machinery as whole-list parsing. A capture is carried through chart continuations. An asserted deterministic prefix can finish when its requested capture is established; an intersection checks its required surrounding pattern.

Borrowed literal pattern layouts create lazy template frames. No sibling result is written, and an irrelevant suffix need not even supply its header. Ordinary list sources continue through their existing correspondence cursors. Named source classification examines the meaning's header without normalizing the record or losing its occurrence path. Parser waits retain the local chart and cursor; cancellation publishes nothing.

Cost is proportional to the visited pattern/source structure and chart states, plus the selected output. Fixed-width cases retain one forward scan and borrowed text windows. There is no new global cache, planning stage, goal restart, or repeated per-member parent normalization. Captured text facts do not yet identify a writable source occurrence; synthesized-group navigation and general selected list regrouping remain explicit boundaries.

**Acceptance evidence.** All 31 WorldManager suites pass, including 499 selected-parsing assertions; those 499 also pass in an independent regenerated-library client. The full inventory is 1509 failures / 1893 tests, recovering `concat/parseSrc` with no new failing names. All 115 generated graph/manager/request/text-cursor/Agent-entry methods agree between targets. The final test regression executable used an unoptimized build and the named-layout method regenerated by the library target; the next regular regeneration restores the usual optimized test build.


## Nominal record fields (2026-10-12)

D05B extends meaning application to supplied field records. Unique nominal keys locate candidate declaration fields, the nominal comparator proves their identity, and ordinary derived merges combine their facts. A retained field plan controls sequential evaluation. Omitted immediate fields are inserted around the original supplied order; existing POVs and infons survive staged publication. Closed meanings, given extents, pending relationships and repeated listSpec constraints are not discarded.

List-valued supplied children retain acquisition scope, so normalizing a returned record expands its own immediate omissions without recursively expanding the records it contains. Explicitly typed unknown scalar fields remain legitimate partial results on subsequent normalization. Bare names without a primitive type, value or available meaning retain their unsupported boundary.

The ordered index avoids independent whole-record searches for each field. It does not replace nominal proof with text-name equality or infer inheritance. Duplicate/unresolved field identities remain explicit boundaries; counted templates continue through list correspondence. All work is request-local and parser continuation preserves prior progress.

**Acceptance evidence.** `word/types`, `query/chain3` and `query/chain4` now pass. All 32 WorldManager suites and all 62 record-field assertions pass; the same 62 assertions pass through an independent regenerated-library client. The full inventory is 1506 failures / 1894 tests, with exactly those three recovered fixtures and no new failing names. Both regular optimized targets regenerate from CodeDog, and all 118 relevant generated method bodies agree. The two earlier compiled assertions marking supplied records as unsupported now assert their supported semantic results; the archived original assertions and protected language fixtures remain unchanged.


## End-relative selection (2026-10-12)

D03V treats a selector coordinate as a requested scalar fact. Literal coordinates retain their direct path; expression coordinates use the normal scalar solver and keep that goal across parser waits. A negative intersection coordinate requires a proved source endpoint. Convert it to a nonnegative boundary and use the same symbolic prefix/span construction as an indexed path slice. Unknown endpoints and coordinates outside the known source remain partial without publication.

`offsetDomain` borrows the pattern and marked occurrence, adds a fixed symbolic prefix and preserves the source relationship context. Ordinary correspondence then handles nested lists, selected items and bounded spans. No reverse value traversal or enumeration of an unrequested sparse prefix is needed. Source coordinates, occurrence ancestry and write eligibility survive adapter construction; an input wrapper cannot turn a borrowed declaration into a writable instance.

The end-relative fixtures now share their matching and publication machinery with positive slices and ordinary member requests. Scalar slice specifications, unknown-endpoint completion, general sparse interior materialization and writes requiring definition instances remain outside this milestone.

A supplied named list must establish its declared logical layout before indexing. An unapplied listSpec can describe different units or grouping from the stored members; traversal reports unsupported template correspondence instead of returning a completed physical index. This was reproduced with hours stored as seconds, and checked with independently named templates for positive items, negative items and negative spans. General unit conversion remains deferred.

**Acceptance evidence.** All five `slice/endRelative…` fixtures pass, completing all nine requested original fixtures across the three groups. All 33 WorldManager suites pass. The end-relative suite passes 884 checks covering independent finite expected results, nested spans, source ownership, coordinate/endpoint/member waits, cancellation, symbolic prefix skipping and unproved named layouts. An independent client linked to the regenerated library passes the same 884 checks plus 499 selected-parsing and 62 record-field checks (1,445 total). Both optimized targets regenerate from CodeDog; all 119 relevant generated method bodies agree.

The complete inventory reports 1501 failures / 1895 tests: exactly nine prior failures recovered and no new failing names. Of the remaining failures, 1384 are explicitly `NOT_PORTED` and 109 are language fixtures. Forty-two remaining diagnostics changed as work reached different semantic boundaries; unit-layout requests now report the template-correspondence limitation rather than completing at physical indices. All 15 protected fixture hashes, 58 historical parser/model registrations, 1559 original compiled registrations and the archived assertion-source hash are preserved. Unrelated `myTests.tst` is unchanged.


## Numeric predicate search (2026-10-12)

D03W composes computed and negated range predicates with the existing correspondence matcher. The scalar frame evaluates the positive expression before applying outer negation. Membership uses exact value-domain containment/disjointness; partial overlap stays unresolved, and representation endpoints remain distinct from this comparison. A selected positive range keeps its checked endpoint and the supplied displayed form; a selected exclusion yields the proved candidate. The source occurrence is retained and never rewritten. Strict scalar identity keeps its existing endpoint checks.

**Acceptance evidence.** `range/dot` and all five previously unsupported focused field-search cases now pass. Default ranges are 3/6 and the focused numeric-range file is 17/20; their remaining failures are the three filtering cases addressed next. All 34 WorldManager suites pass, including 148 new checks. The independent regenerated library passes the same 148 checks. The full inventory is 1500 failures / 1896 tests, with exactly `range/dot` recovered, no new failing names and no changed remaining diagnostics. All 120 relevant generated methods agree; protected fixtures, original assertion archive, historical cases and existing registrations are preserved.


## Projected repetition and symbolic ranges (2026-10-12)

D03X extends the existing span chart with a projection: match an intersection body, keep its consumed endpoint, and return the selected scalar or span. Repeating this operation supplies range filters, string filters, grouped captures and every-other-member selection through the same matcher. Templates are prepared once and matches are memoized by template and source boundary. Direct `<~` input and a retained projected-output `==` relationship use the same source path.

Supplied list members constrain projected outputs. They are evaluated once, matched against each corresponding output, and retained across parser waits. They do not replace the projection template or advance the raw input cursor themselves. A partial result can therefore be normalized again without duplicating its prefix or interpreting output values as source positions.

An unbounded projection that cannot finish its next match retains its proved prefix, open remainder, original template and input relationship. Given output extents remain constraints. Uncertain overlap, ambiguity and pending input do not authorize a speculative prefix. Contained symbolic ranges remain partial values; disjoint ranges are skipped; partial overlaps stay unresolved. Published output occurrences keep their own endpoints and can receive new constraints without changing the source.

Parser continuation retains the same chart, predicate frames, source cursor and supplied-output state. It waits on original parser-owned nodes, including before borrowing a projection body. The scope covers local parser completion; arbitrary later semantic link arrivals remain deferred. Complexity is linear in visited input and output for the simple disjoint filters, plus ordered-table lookup costs, with the general chart's existing ambiguity bounds otherwise.

**Acceptance evidence.** All six default range fixtures and all 20 focused numeric-range fixtures pass. All 36 WorldManager suites pass, including 148 predicate, 289 repeated-filter and 123 symbolic-filter assertions (560 total); an independent client linked to the rebuilt shared library passes the same 560 assertions. The full inventory is **1495 failures / 1898 tests**, recovering `range/dot`, `range/select1`, `range/select2`, `range/select3`, `outr/everyOther1` and `outr/everyOther2` from the starting baseline, with no new failures and no changed remaining diagnostics. Of the remaining failures, 1384 are explicitly `NOT_PORTED` and 103 are default language fixtures. Both regular optimized targets regenerate from CodeDog; all 122 relevant generated methods agree. All 15 protected fixture hashes, 58 historical registrations, prior compiled registrations and the original assertion archive are preserved. Unrelated `myTests.tst` is unchanged.

## Agent world roots and source references (2026-10-12)

Under D03Y, Agent passes its world view into WorldManager when creating a request. `%W` supplies that borrowed occurrence to ordinary path selection. A query can select a known field without normalizing unrelated members; source paths used by intersections are resolved once before their layouts are consumed. Definition declarations remain declarations and are excluded from value matches. Existing occurrence provenance, staged writes, rejection and parser continuations apply unchanged.

The request retains its original world across waits. The initial focused suite verifies missing roots, declaration-bearing worlds, exact selected POVs, writes to unknown members, conflicting-write atomicity, intersections over world paths, unused pending suffixes, exact candidate waits and cancellation. General temporal and other environment providers are not supplied by this change.

**Acceptance evidence.** All 37 WorldManager suites pass, including 31 world-source checks also passing in an independent regenerated-library client. Default fixtures remain 103 failures / 332 tests: 32 cases now reach later semantic boundaries, with no new failing names. The full inventory remains 1495 failures / 1899 tests. Both targets regenerate from CodeDog and all 122 relevant generated methods agree. Protected fixture hashes, historical and prior compiled registrations, the archived assertion source and unrelated `myTests.tst` are unchanged.

## Literal list layout and named record values (2026-10-12)

D03Z carries list ordering and T-list status in checked facts and through publication. T-list identity propagates its context. Unordered records retain implicit openness independently of their physical contents; only an explicit size can bound total membership. Too many known members for that size reject atomically.

Named record definitions reuse field correspondence to check supplied parts and fill missing ones. The resulting layout retains unordered status even when the supplied instance had only one field. Partial named fields remain represented, and a named definition can acquire another named body's checked value while retaining its own primary name. General inheritance comparison, unordered alternative search and temporal preservation remain distinct operations.

**Acceptance evidence.** All 38 WorldManager suites pass; the 55 new literal-layout assertions also pass against the separately regenerated shared library. Default fixtures improve from 103 to **98 failures / 332 tests**, recovering `temporal/identityCarriesT`, `temporal/openShapeCarriesT`, `temporal/definitionCarriesT`, `name/value/direct` and `unordered/seq1`. The full inventory is **1490 failures / 1900 tests**, with no new failing names. Four replacement-milestone assertions now expect the newly supported partial/literal results; the unordered positional-identity check remains unsupported, preventing arbitrary positional pairing of unordered members. All 122 relevant generated methods agree. Protected fixture hashes, archived assertions, historical/prior compiled registrations and unrelated `myTests.tst` remain unchanged.

## Definition-derived unit windows (2026-10-12)

Under D040, finite homogeneous definitions provide exact widths in a shared numeric leaf unit. Path selection and finite homogeneous intersection selection use one arithmetic window operation. It retains structural traversal state, treats sparse gaps as measured segments, and creates only the demanded grouped value. Unit conversions do not identify differently sized containers. Supplied source extent determines negative coordinates; declaration counts describe unit shape.

Projected leaves retain their original source occurrences. Reads preserve the source; writes reuse ordinary checked merges and staged publication, recursively following the projected view's origins. A conflicting write rejects atomically. Positive requests ignore unneeded suffixes; endpoint requests retain their progress through parser waits. Unit definitions remain borrowed, and their T metadata is preserved in composite views. Sparse-gap materialization, arbitrary heterogeneous conversion and general pattern alignment remain separate requirements.

## Inherited type qualifiers (2026-10-12)

D041 preserves the distinction between a member's primary name and a type-only qualifier inherited from its listSpec. A previously unnamed member stores the latter in `addedTypes`; subsequent reads expose it as checked nominal evidence. Explicit primary names and value-producing named templates remain primary. Thus `{second|1 2}` can publish `{1 2}` while retaining each member's second-type evidence for later queries and writes. The printer is unchanged. The current supported representation has one inherited nominal without a primary type; general nominal conjunction remains separate work.

### Demanded sparse windows (D042)

Homogeneous fixed prefixes, numeric captures and suffixes can share the unit cursor across compressed runs. A selected base-unit position borrows its recipe and retains its original coordinate. A fully unknown selected window remains a counted listSpec; later selections compose through its stored source offset. Returning a partial path or unit result also transfers its list requirements.

Writes first pass ordinary comparisons and then stage original source positions. Publication validates the destinations and model integer bounds, coalesces duplicate constraints, orders the demanded positions, and uses existing sparse split/expose operations with explicit virtual parent context. It materializes only demanded values. Rejection, waiting and cancellation leave the source intact; existing repetition identities survive materialization. Traversal is independent of virtual extent, with O(K log K) ordering and O(K) staged storage for K actual writes. Unproved sparse alignment and native composite-gap edits remain unsupported.

## Boundary arguments (2026-10-13)

D043 lowers first/last argument syntax into one virtual list identity with an unconstrained remainder. List normalization and member-only discovery share this edge and ordinary correspondence. Named layouts precede application; member requirements remain checked, source arguments remain borrowed, and successful publication removes the consumed syntax. Fixed-coordinate cursors retain partial skips of proved sparse prefixes, avoiding virtual member enumeration. Streaming waits and cancellation retain their existing local/atomic behavior.

**D043 acceptance.** Both regular CodeDog targets regenerate successfully and agree across 153 solver-related method bodies. All 42 WorldManager suites pass. The argument client passes 58 checks; argument, path and end-relative checks together pass 1,106 assertions. Fixed-width lists of 32/64/128 members use 72/136/264 frames. Default failures drop from 45 to 40 out of 332; the full inventory is 1432/1904, with no new failing names. Protected fixtures, archived assertions and prior registrations remain unchanged. Validation caught and corrected partial skipping of represented lists: only empty unconstrained spans admit a retained arithmetic offset; represented lists use structural traversal.

## Finite temporal occurrences (2026-10-13)

D044 adds virtual begin/end boundaries and represented previous/next T-occurrence lookup. Boundary null identities are canonical in the Agent registry and created atomically. Temporal navigation crosses nested closed T-spans, borrows named layouts and retains the instance context of a shared definition. An S-list supplies no temporal adjacency; an unrepresented open side remains unresolved.

Navigation context is separate from parent-RHS discovery. The ordinary cache still shares context-free facts, while contextual results require the same occurrence path. Parser waits retain their local cursor and subscription. Broader T-rule preservation, cuts and symbolic sparse temporal adjacency remain separate semantics.

**D044 acceptance.** All 43 WorldManager suites pass in the regular executable. The independent regenerated-library client passes 29,807 native checks, including the 75 temporal assertions. Predecessor chains of 32/64/128 members use 33/65/129 frames. All 155 relevant generated methods agree between the two regular targets. Default failures drop from 40 to 31 out of 332; the full inventory is 1423/1905, with exactly the nine temporal fixtures recovered and no new failing names. Protected language expectations, historical registrations and archived assertions remain unchanged. Two earlier internal assertions that expected bare boundary access to be unsupported now check canonical identified nulls.

### Checked nominal evidence and record values (D045)

Applied positive meanings retain their bound nominal qualifications in facts and in `addedTypes` at publication. Primary names remain stable. Type tests use directional ancestry evidence. Under D061, asserted identities may also combine distinct checked definitions when ordinary value/list correspondence proves their constraints compatible; this does not identify their declarations for unasserted type searches. A named identity lacking proof retains its resolved RHS while acquiring the missing target meaning, so record constraints use ordinary correspondence. Other pending comparisons retain the existing fact-arrival continuation.

Unique exact record field names share the existing indexed alignment, including inherited field types. Unordered record comparison uses the corresponding field identities. Missing/duplicate identities and unmatched inherited fields remain unresolved. A stronger value demand continues retained predicate-frame cursors. A tried meaning with no available value does not requeue itself. General nominal conjunctions and overloaded/ambiguous field inheritance are not yet implemented.

### Selection providers and instance context (D046)

Positive type selection preserves an existing primary name and pending list requirements. An anonymously qualified member still takes the explicit descriptor as its requested view name. A selected result never inherits its old wrapper's size when its own size is unknown. An unbound field name cannot match a closed anonymous numeric/string literal; unknown candidates remain unresolved. Bracketed literal scalar predicates use ordinary search after a lowered boundary, while literal list specs describe span windows. Negative boundaries require an endpoint.

`%` uses the owner of the retained containing definition occurrence. Given instance fields and borrowed named layouts use existing selection. Ordinary RHSs and scalar operands inherit occurrence context; record merges retain contextual dependence. A self-named given constructor does not recursively reapply itself. Unknown alias cycles and values available only through an unfinished enclosing merge remain separate proof/layout boundaries. Explicit size demands propagate through path selection.

### Explicit scalar replacement (D047)

A positive numeric/string `<<=` supplies new value and size evidence. Anonymous RHSs keep the declared destination name; named RHSs supply their own. One fresh value goal checks the retained meaning and parent listSpec at the destination occurrence, preserving completed RHS cursors and exact parser waits. This is a new semantic state, not a restarted old goal. Staged publication preserves occurrence identity, replaces discarded extents, and leaves sources intact on rejection or cancellation. List and qualified replacement semantics remain separate work.

**D047 acceptance.** Both regular CodeDog builds succeed and agree across 160 relevant generated methods. All 46 WorldManager suites pass; the independent regenerated library passes 29,998 native checks, including 69 replacement assertions. Default failures fall from 28 to 27 out of 332, recovering `merge/ident/force`; the full inventory is 1419/1908 with no new failing names. Protected fixtures, historical registrations, archived assertions and unrelated `myTests.tst` remain unchanged.

### External properties and resolution (D048)

An external association connects an exact bearer occurrence, descriptor and independent value. Asserted values pass through ordinary normalization and staged publication. The existing IdentityRegistry persists the relationship and explicit alias proofs, including later POV wrappers of a stored view. Shared definitions retain each actual instance as the anchor.

`~` uses direct member correspondence first. Only a no-match proof requests the external tier; uncertainty, waiting and selected-value contradictions block fallback. A positive typed RHS request can use the same provider to select a normally normalized bearer's associated value. Dot navigation primitives remain distinct from word resolution. Missing/multiple external candidates and unexpanded asserted-reference presentation remain separate work.

**D048 acceptance.** Both regular CodeDog builds succeed; 164 relevant generated methods, including registry association lookup, agree between targets. All 47 WorldManager suites pass; the independent library passes 30,078 native checks including 80 new association assertions. Defaults improve from 27 to 22 failures out of 332; the full inventory is 1414/1909 with no new failing names. Direct resolution, external fallback, direct precedence, name-value isolation and typed RHS association projection fixtures are recovered. The asserted-name-reference fixture now returns a checked expanded value and remains a presentation/selection-demand difference. Protected fixtures, historical tests, archived assertions and unrelated work remain unchanged.

### English lexical text (D049)

The existing `english-text` primitive maps a settled finite string to an ordered `english-phrase` of space-delimited lexical word occurrences. It preserves spelling, attaches checked word-class evidence and keeps token spellings separate from dictionary evaluation. The normal text cursor reads borrowed leaves once; ordinary publication writes the requested phrase. Unknown string endpoints are not closed by this operation, and partial lexical tokens remain partial. Parser waits and source ownership follow the normal frame lifecycle.


### Deferred future member templates (D050)

An empty unbounded named list can acquire its declared temporal/member shape without evaluating a nonexistent member. Keep its future listSpec as a deferred frame with header evidence; demand the body through the existing prerequisite continuation when a member is requested. A bare sole-tail list can adopt another sole-tail list's shape and future requirements directly. Publication preserves that template on the target, so later finite goals still enforce it without retaining a redundant identity RHS. Existing bounds, members and constraints keep ordinary correspondence. The cube shape now resolves; its historical printer spelling remains a separate difference.

### Parser-only stream insertion (D051)

An insertion node caches one parser-owned child. WorldManager requests it through the existing loader and continues its child frame at parser readiness; Agent retains session ownership. Imported declarations and effects share the outer transaction, while source opening occurs on a loader-owned worker outside the graph lock. Publication attaches declaration identities and recomputes only inferred closed-list extents. Given extents and ordinary declaration-copy restrictions remain. General symbolic size expressions, streamed-prefix publication and reply effects are separate requirements.

### Inferred plural list descriptions (D052)

A positive simple listSpec supplies its anonymous list's implied plural name through a read-only ModelManager helper. Ordinary nominal correspondence consumes that evidence. A bare named member without a definition can acquire a positive parent type qualification while retaining its name; bound meanings and primitive values still require compatibility. Closed ordered and unordered lists share the type-only coverage proof for compact template printing (D054). The template remains stored; open tails and explicitly unknown extents keep it visible. Published primary names do not also occur as redundant added qualifications.

A merge may retain an already applied meaning from a context-free RHS with the same nominal identity. Contextual meanings still require proof at their own occurrence.


### Initial conditions and template applicability (D053)

This preserves existing main-branch semantics, confirmed by the user. The remaining work is to port that behavior into the replacement WorldManager.

A template requiring a relative link such as `%prev` or `%next` is inapplicable at an occurrence where its required neighbor is absent. Record the relative-context requirement on the template and applicability at each occurrence. Thus a predecessor-dependent template can skip the first state, and a successor-dependent template can skip the last. This is neither contradiction nor rejection. Other applicable assertions retain their normal checks; a given value alone does not exempt an occurrence from its template, and a constant template has no initial-state exception.

Keep proven absence separate from information pending at a stream or normalization link. Pending context retains the local continuation; it does not permanently discharge the application. Shared templates retain occurrence-independent metadata, while contextual values and applicability are scoped to their occurrences. No goal restart or repeated prefix traversal is introduced.

The user removed `defaultTests.tst:time/t2` because it predates these semantics. The retirement manifest preserves the removed fixture and authorized hash change. This is a compatibility gap in the replacement: current temporal lookup can still turn absent context into `UNDEFINED` and then reject the dependent merge. D053 specifies template application; direct standalone cursor output and finite-tail materialization are separate concerns.


### Retained listSpecs and compact printing (D054)

Applying a listSpec to all represented members does not remove it from the model. Keep the original template reference through finite normalization and adoption, so later reads and writes still have its constraints. Shared template bodies remain unchanged.

Publication records the existing coverage proof as an occurrence-local `listSpecApplied` marker. `InfonViewPrinter.isNiceList` uses that marker to omit redundant listSpec text from an ordinary closed literal list. Debug printing reveals the retained template. Open and unproved applications remain visible; new supplied work clears the display proof through the normal invalidation hook. A same-occurrence refinement reuses the already applied template proof; a replacement value still requests its parent template. Deferred projection work does not invalidate type-only coverage when embedded children retain the same positive future template. Parent-template requests retain their containing context. Earlier references to listSpec discharge mean completed application work, not template deletion.


## Remaining default-test classification (2026-10-13)

**Historical nineteen-case snapshot.** The D056 follow-up below resolves three display expectations and replaces the external world dependencies with synthetic fixtures; the current default result is 16/332.

This is an audit of the replacement at `de34172`, not a change to language semantics or test expectations. The fresh `cd LocalBuild && ./TestProteus -t defaultTests.tst:` run reports **19/331**, with exactly the D054 failure names and messages. The classification inventory (`graph/default-test-classification.json` in Git history) records every fixture, current failure, main-branch source occurrence, saved reference result, supporting probe and next action.

Thirteen of these test names occur in main at `86321ea`; six were added later. Name continuity does not imply unchanged syntax or expectations: for example, the nested ordinary-write fixture originally expected replacement, which the later explicit forceID decision corrected. Fourteen of the current nineteen also failed in the saved original-Proteus4 comparison at `b52322a`. That is historical baseline evidence, not proof that their current behavior is correct. Main was inspected as source, not rebuilt or rerun for this audit.

| Classification | Count | Interpretation |
| --- | ---: | --- |
| Formatting / printed representation | 4 | The reported mismatch does not demonstrate a wrong computed value. |
| Result representation / normalization demand | 7 | Reference expansion, child normalization or pending/no-match treatment needs review; these are not automatically cosmetic. |
| World-loading blockers | 6 | The named query has not run; there are three shared prerequisite families. |
| Fixture issue plus an implementation gap | 1 | Correcting the capture in a temporary probe still exposes unsupported work. |
| Explicitly deferred | 1 | Ambiguous correspondence and its result representation remain deferred. |

Every row below remains registered and failing. “Previous” means the saved original-Proteus4 comparison, not main execution.

| Test | Classification and finding | Previous | Next action |
| --- | --- | --- | --- |
| `merge/dots/align4b` | **Deferred.** Unsupported ambiguous open-span alignment; the current retained-RHS expectation is provisional. | Pass | Revisit with general alternative correspondence, as already agreed. |
| `write/openNegativeLast` | **Pending-result contract.** Unknown endpoint; current handle is partial and retains the update expression rather than printing only its source. | Pass | Specify the returned view while preserving pending work; do not drop the update to match text. |
| `name/reference/externalProperty` | **Reference/expansion contract.** Lookup returns the expanded name record instead of the asserted `Morgan` reference. | Fail | Check reference identity and demand; this fixture deliberately distinguishes reference lookup from standalone expansion. |
| `cube/getLstSpec` | **Display.** Correct temporal list/template shape, with an extra `*_+` unknown-size prefix on the member template. | Fail | Review inferred unknown-size printing; this is not a rendered-cube test. |
| `rubix/sparseNestedContinuation` | **Load blocker: Rubix.** Missing available named definition before selection. The resource uses undeclared `reel`; supplying it in an isolated copy exposes unsupported unordered correspondence next. | Fail | Resolve the intended declaration/context, then reduce the definition-composition gap before assessing continuation. |
| `user/getUser3` | **Load blocker: user model.** Shared `user_3.pr` scalar contradiction before title lookup. | Fail | Trace the first conflicting template/member pair and review fixture constraints and demand. |
| `user/getNth` | **Load blocker: user model.** Same prerequisite; nth-member selection has not run. | Fail | Resolve the shared load failure first. |
| `user/setNth` | **Load blocker: user model.** Same prerequisite; the force-write has not run. | Fail | Resolve the shared load failure first; do not infer that force-write itself is broken. |
| `this/lvl1` | **Whitespace.** One extra space after a comma; the red/pink values and nested structure agree. | Fail | Reconcile exact printer spacing; no solver correction indicated. |
| `search/adj1` | **Reference/expansion contract.** Selects Bob; expands `blue` into hue 4, saturation 5 and unknown brightness. | Fail | Review symbolic-reference preservation versus normalized returned values. |
| `sparse/implicit/readAfterSparsePrefixInSizedCompositeList` | **Fixture plus gap.** Numeric `<_>` expects a list-valued minute. `<?>` in a temporary probe exposes unsupported counted-template propagation. | Fail | Settle the intended capture, then address general sparse composite demand; preserve minute size 60. |
| `sparse/guard/nonAligningMinuteDoesNotSkipHalfMinute` | **No-match/result contract.** Does not incorrectly skip the half-minute or return 303; retains a partial out-of-range selection instead of `_`. | Pass | Review proof-of-no-match and result handling for the closed source; keep the guard. |
| `unit/day/fromMinutes/readFirstHour` | **Normalization depth.** Correct first six seconds, with both minute children closed rather than one residual template/tail. | Fail | Check D03 demand, retained constraints and work before changing the expected representation. |
| `unit/day/fromMinutes/readSecondHour` | **Normalization depth.** Correct seconds 7–12; expected children retain template/tail syntax. | Fail | Same finite-closure/demand review. |
| `unit/day/fromMinutes/readLastHourNegative` | **Normalization depth.** Correct last hour; same child-expansion difference. | Fail | Same review; no evidence here of a wrong negative endpoint. |
| `unit/world/dayFromSeconds/writeNestedSecond` | **Display after correct rejection.** Write rejects and seconds 1–12 remain; normal output hides `second|`. | Pass | Preserve rejection/retention checks; reconcile ordinary printing with D054 when authorized. |
| `unit/world/dayFromSeconds/writeNestedLastSecondNegative` | **Display after correct rejection.** Same behavior for negative indices. | Pass | Same D054 printer review. |
| `world/load/kernel` | **Load blocker: symbolic extent.** `world` cannot acquire its list shape through unresolved `*(20000+_)`. | Fail | Preserve symbolic size requirements while acquiring known list shape; retain the initialization check. |
| `world/load/publicKB` | **Load blocker: kernel prerequisite.** Public loading calls `loadKernel()` first and stops at the same failure. | Fail | Fix/validate that prerequisite, then inspect any later public-model failures. |

### Evidence and limits

The current file-loader run is `/tmp/p4-default-classification-current.log`. The inventory embeds its failure bodies and hashes, so the classification does not depend on temporary logs surviving. Historical results come from `/tmp/p4-checkpoint-remaining.log`; all nineteen names occur in that saved comparison. The isolated current-library diagnostic inputs and outputs are also embedded in the inventory.

The two rejected-write probes report rejection followed by the original twelve values. D054's retained-template checks establish that compact printing does not authorize deleting the listSpec. The three hour probes complete with the same finite values; a subsequent selected second returns 6. Those observations establish value agreement, not that the depth or amount of work is optimal.

The nonaligning sparse probe reports `Selected unit window is outside the source`; an aligned 60-second-prefix control returns 303. The composite-capacity fixture reports `Unit view needs a demanded sparse occurrence`, and its unknown-overlay variant reports `Counted template needs bounded predicate propagation`. Neither case should be fixed by discarding its unresolved expression without a semantic proof.

The Rubix load probe changes from unavailable named definition to `Unordered correspondence needs member matching` when an in-memory copy supplies `#reel=*_+_`. No fixture was edited. The user model contains constant template values 40/90 against supplied 20/100 and empty member names against nonempty names; a reduced screen-position example rejects. The first contradictory pair in the full loader is **not yet traced**, so this observation identifies a constraint/demand question rather than declaring those exact source fields the first cause. Ordinary constants must not silently become overridable defaults.

D051's kernel trace and a fresh minimal `*(20000+_)+{T _|...}` probe identify the symbolic-expression boundary. The latter is partial with `Expression operand value is unknown`. The shared load failure prevents these two fixtures from assessing later PublicKB behavior.

No engine, printer, fixture, registration or harness policy changed. All protected fixture hashes were checked; unrelated `myTests.tst` was preserved. The next implementation work should follow the semantic families above, not a target failure count. D053 relative-context applicability remains a known compatibility gap outside these nineteen; this audit does not claim that their eventual success would establish all normalization semantics.


## Agreed output corrections and independent world fixtures (D056)

The user approved the three straightforward expectation corrections: comma spacing in `this/lvl1`, and compact retained-listSpec printing in the two nested rejected-write tests. Their inputs, rejection assertions and preserved numeric values are unchanged. `cube/getLstSpec`, symbolic-reference output, child normalization depth, sparse-result policy and deferred alignment are not included in this approval. Each fixture edit and hash transition is recorded in the retirement manifest.

Default regressions must not depend on mutable application user/world models. The user chose small synthetic fixtures for the two world-loading cases. `Resources/TestFiles/world-load/symbolic/` retains the unresolved `*(20000+_)` shape-acquisition regression, using a small imported unit declaration. The original `world/load/kernel` and `world/load/publicKB` names remain, and both still fail at world initialization. `finite/` adds one positive control, `world/load/finitePublic`, checking `%W@[2]` and a public declaration dependent on an imported kernel declaration. Both kernel and public imports are resolved relative to their fixture origins.

`TestProteusCore.loadKernel` now accepts the requested filename. Public-fixture initialization resolves `KernelKB.pr` beside that fixture instead of opening the application's `public/KernelKB.pr`. No application loader, WorldManager, printer or shared-library interface changed. Committed `user_3.pr`, `userKB.pr` and Rubix files remain regression data; tests do not refresh them from live user models. Actual application-world compatibility is separate integration work.

### Shared loading diagnoses

The first contradictory merge for `user_3.pr` is now traced through the generated test core's real file-loader path: the first CodeDog TaskList supplies `screenPos.xPos:20` while its containing data listSpec requires `xPos:40` (`Resources/user_3.pr:14,16`). The result propagates to world loading before any of the three named queries runs. This is concrete constraint conflict, not evidence of three independently broken query/write operations. The fixture's intended template constraints still need review; ordinary constants must not become implicit defaults. No user-model resource was edited.

The same file-loader trace identifies the Rubix unsupported node as `reel`. This standalone fixture declares no such numeric meaning. The earlier isolated probe supplying it then reaches unsupported unordered correspondence, so declaring it would expose rather than solve the next composition gap. The synthetic symbolic kernel independently reproduces `Expression operand value is unknown`, without reading any external model.

### Validation

`codeDog Proteus.Lib.dog` regenerated the executable from the maintained test harness. Verification ran from an isolated directory with only tracked/proposed test assets and no `assets/public` tree. All 11 distinct default world roots and 15 transitive fixture documents resolve within those assets.

- Defaults: **16/332**. Exactly the three approved display failures recover; all sixteen remaining failure names/messages match the prior run. The extra case is the passing finite-world control.
- World-loading fixtures: **2/3** fail, both preserving the known symbolic-size limitation; the finite bootstrap/import control passes.
- WorldManager suites: **0/52** failures.
- Full retained inventory: **1408/1914** failures, comprising 1,384 unported assertions, eight historical compiled failures and sixteen defaults.

Protected fixture hashes, archived assertions and unrelated `myTests.tst` were verified. The original nineteen-case inventory remains a historical snapshot with this follow-up attached. No result here establishes live application-world or Slipstream rendering behavior.

**D056 fixture correction follow-up (2026-10-13).** The corrected committed user fixture now has `xPos:40` in the first CodeDog TaskList. Its first load contradiction advances to `yPos:100` versus the template's `yPos:90`. Rubix uses locally defined `rot = *_+_` in its nine numeric aliases; loading advances from an unavailable numeric definition to unsupported unordered `edgeCube` correspondence. These user-approved corrections preserve constant-listSpec semantics, unknown numeric size and all query expectations. Defaults remain at 16/332 with no new failing names. The classification inventory's `fixture_correction_follow_up` records current evidence; the earlier snapshots above remain historical.


## Active regression suite after D057 (2026-10-13)

The user chose fresh behavioral tests as each semantic area is implemented. The old 1,384 `NOT_PORTED` registrations are removed from the active runner; their original locations and registrations remain as `archived_reference` entries in `graph/engine-test-migration.json` (Git history). No old assertion body is lost: the inventory preserves the reference checkpoint and source hash. All 198 active compiled registrations, including 52 WorldManager suites, and all 332 defaults remain. The sixteen default failures are deferred. Earlier migration totals and requirements above are historical and are superseded by this policy.

The review grouped all 1,384 retired registrations across 73 original name groups and 79 registration macros. These are registrations, many containing several assertions. The primary-family counts below are a checklist of areas to consider as work proceeds, not a promise to recreate this number of tests:

| Semantic family | Archived registrations |
| --- | ---: |
| Scalars, numeric ranges and compatibility | 121 |
| List correspondence, spans and concatenation | 89 |
| ListSpecs, grouping, units and normalization depth | 148 |
| Definitions, scope, projection and word forms | 217 |
| Record matching, choices and descriptive queries | 163 |
| Paths, selection, slices and boundary arguments | 220 |
| Text and grammar recognition | 85 |
| Writes through selected occurrences | 37 |
| Context, world and temporal relationships | 171 |
| Stream insertion, engine entry and replies | 49 |
| Completion, publication and dependency isolation | 84 |
| **Total** | **1,384** |

Several families share the same core mechanisms. A member correspondence should serve merging, intersections, selection and writes; a definition or listSpec supplies scoped requirements to that same operation. Test those relationships together when implementing a capability, including their negative and partial cases. Keep tests for requested normalization depth and sparse structure so correctness does not depend on fully expanding an enclosing list or copying every intermediate value.

Representative archived bodies and 21 isolated input-level probes were reviewed. Existing outputs already include scalar/list definitions, pair/triple grouping, fixed-width text parsing, range filtering, selected writes, basic temporal navigation, permuted record fields and an open-span result. These probes do not establish all of the old cases' source-preservation, repeated-request, identity or continuation assertions. In particular, `graphText/firstAmbiguousMatch` and `graphGrammarChoice/ambiguousFirst` choose a first parse in the archive; current probes remain partial with multiple mappings, consistent with the current ambiguity contract. `graphSignedNumber/lateSize` also expects an initial subscription for `-1`; ordinary numeric completion now permits unknown size. Do not reproduce those assertions merely to lower a failure count.

For each implemented capability, add small fixed-fixture checks through the current engine for:

- Its result, applicable constraints and rejection/partial distinctions.
- The source occurrence and context, including preserving unrelated sources and atomic failure of writes.
- Its relevant parser or requested-link continuation, cancellation and session ownership, without restarting completed work.
- Justified work growth and sparse allocation bounds when an optimization is being introduced.

The archive is consulted for overlooked examples. It is not a porting queue, and retired implementation-specific assertions are not runtime requirements. No new cost planner, default coherence pass, cross-session sharing policy or test-generated semantic exception follows from this reset. Test models must remain independent of changing real user/world files; the archived full Rubix palette and transition tests need stable local fixtures before any future regeneration.

The pre-reset full run is **1408/1914**: 1,384 placeholders, sixteen default failures and eight existing compiled failures. That log is `/tmp/p4-step2-full.log`; the input-level review probes are `/tmp/p4-step2-probes.json`. Acceptance of the reset requires the regenerated runner to preserve exactly the latter 24 failure names and messages, while no longer registering the placeholders. The retired-test manifest records the resulting verification.

**Validated result:** the regenerated TestProteus build succeeds in 187.43 seconds. The full isolated run is **24 failures / 530 tests**: defaults **16/332**, compiled cases **8/198**, including **0/52** WorldManager suite failures. Every retained failure name and diagnostic is identical to the pre-reset run. No `NOT_PORTED` placeholders remain active. The protected fixtures, historical registrations, original source and unrelated user work are unchanged. This reduction reports retired placeholders, not newly fixed engine behavior.

## Anonymous described fields (D058, 2026-10-13)

Record merging and comparison share an effective field identity: the primary name, or a sole positive nominal qualification when no primary name exists. Thus an anonymous typed list can correspond by its inferred plural description alongside named fields. Duplicate/ambiguous identities remain unresolved; bound identity checks and source preservation still apply. No positional pairing is inferred for unordered fields.

The fresh piece-composition suite uses only synthetic definitions and explicitly finite unordered records. It checks position/orientation, colored sides, numeric values of unknown size, source and occurrence preservation, ambiguity, parser continuation/cancellation, repeated completion and linear work over many fixed-width pieces. Implicit unordered extents retain their partial status.

**D058 validation.** Both CodeDog targets regenerate successfully. All 53 WorldManager suites pass; the new suite passes 87 checks in both the executable and an independent shared-library client. Full validation is 24/531 failures with no new failing names. The Rubix fixture now loads past the former unordered-field blocker and stops later at an unsupported request. Protected test inputs and unrelated work are unchanged.

## Nested member correspondence (D059, 2026-10-13)

After exhausting local evidence, an incomplete member can request a containing or higher ancestor identity. Retained projections locate corresponding intermediate occurrences without normalizing their values. Ordered spans use the existing coordinate path; unordered records use unique field descriptions and nominal proof. Both deliver an ordinary RHS to the same merge used by whole-list normalization and selected writes.

Strict identities on a fixed layout of uniquely described non-embedded members do not prevent selecting an existing named occurrence. Its own frame checks relevant parent correspondence; unrelated values remain undemanded. The existing guard remains for layouts whose occurrence addressing can change. Header waits retain local progress. Nested projections consume a shared route built once, giving O(depth) route work and storage plus the field-header scans; they do not retrace or copy the remaining path at each level. Whole-list requests use shared traversal rather than one member lookup per element. This increment projects ancestor identities; it does not introduce general ancestor-template rewriting or ambiguous record search.

## Contextual template application (D060, 2026-10-13)

ListSpecs retain ordinary eager evaluation for context-free requirements. A neighbour request in an unbound recipe records a need for an occurrence. Each application evaluates contextual work in that member's navigation context, sharing only context-free facts or proved results for the same occurrence. A proved absent neighbour makes the entire dependent template inapplicable at that occurrence. This is separate from contradiction, unknown/open context and pending parser input. Templates remain stored, including initial templates that did not apply.

Ordinary previous/next links use adjacent ordered members; temporal links retain T-list boundary traversal. Both return ordinary source occurrences. Correspondence over already normalized facts consumes those facts directly without resolving the source expression again. Ahead-resolved members reuse the request's completed-occurrence proof when the containing list reaches them. These rules support two synthetic piece states with one missing coordinate, direct state requests, and a leaf request through an explicit state identity, with no special piece/cube logic.

Fresh checks cover nested recipes, first/last applicability, arithmetic, real conflicts, source/occurrence preservation, header continuation/cancellation, unknown future boundaries, linear flat-chain work and zero-work repeated completion. Arbitrary descendant requests through ancestor templates and more general temporal search remain later work.

**D060 validation.** The regenerated executable and independent shared-library client pass all new temporal checks (110/110), including both chain directions through 128 states. The full run is 24/533 with no new failing names; both CodeDog builds succeed. Existing default failures remain deferred. The protected language fixtures and unrelated user test file are unchanged.


## Compatible named assertions (D061, 2026-10-13)

An explicit identity combines the checked definitions of its named endpoints through ordinary constraint merging. Distinct primary names are not a contradiction when those meanings are compatible: `front-color=red` preserves the field name and obtains red's hue. Conflicting values or sizes still reject; unknown values stay partial. This supersedes the D045 restriction against sibling assertions. Nominal query predicates still require declaration/inheritance evidence; equal shape or a common parent alone does not make a search match.

If a named identity lacks nominal proof, its frame retains the resolved RHS while requesting the target's missing definition. Once that prerequisite is checked, the same frame resumes the saved identity through ordinary scalar/field correspondence. The ordinary relationship order stays intact. This avoids passing unresolved record constraints through a scalar-only pending comparison. Local waits retain this progress, cancellation publishes nothing, and source definitions remain unchanged. There is no planner, restart operation or separate compatibility solver.

The fixed-width assertion scaling checks are linear through 128 occurrences. General qualification accumulation and ambiguous correspondence retain their separate costs. Actual Slipstream integration remains incomplete: the cube's face/field selections and implicit T-state preservation are not established by the synthetic temporal-template tests.

**D061 validation.** Both maintained CodeDog targets regenerate successfully (library 136.75 s; executable 193.69 s). The independent library client passes the five focused suites, including all 111 nominal-evidence checks and the unchanged replacement and temporal suites. The full executable reports 24 failures / 533 registrations, with the same failing names and diagnostics as the baseline (apart from the crash-protection engine banner). Assertions formerly rejecting compatible named scalars and listSpec siblings now follow the user-approved semantics. All protected `.tst` inputs and unrelated `myTests.tst` remain unchanged. The real-model probe still reports unresolved inherited layouts and implicit temporal preservation; no Slipstream GUI claim follows from these results.


## Ancestor definitions and structured templates (D063, 2026-10-13)

A demanded descendant can use an unchecked ancestor definition or an ancestor member's structured listSpec as an ordinary RHS. The frame retains the member at each containing level; a shared correspondence route projects through the intervening lists. Named definition fields use the existing record identity rules, including ordered representations with omitted fields. Whole-list requests continue to propagate their template once across the list. This extends D059/D060's earlier boundary for descendant projection of ancestor templates.

Borrowed sources retain occurrence/navigation context but stop discovering enclosing requirements at their own constraint boundary. An unspecified corresponding container contributes no descendant facts, so its route exhausts without blocking other evidence. The caller continues its remaining sources after consuming the result. `%` binds to the represented containing object; named layout borrowing records its definition boundary. This prevents false cycles and query re-entry without normalizing unrelated sibling values or mutating shared definitions.

Parser waits retain the exact source and correspondence cursor; unrelated input does no work. Cancellation and conflicting selected writes publish nothing. Unknown exhausted evidence remains partial without a subscription. Fixed-layout depth/width probes and whole-list propagation are linear through 128 levels/members. Ambiguous layouts, source expressions requiring layout proof, implicit temporal preservation and definition-instance creation remain separate work. See `Docs/Design Docs/Historical/member-requirements-2026-10-13.md` for traces and validation.

**D063 validation.** Both maintained CodeDog targets regenerate successfully (library 139.12 s; executable 199.16 s). The independent regenerated-library client passes all 470 ancestor-requirement checks. The full executable passes all 56 WorldManager suites and reports 24 failures / 534 registrations: the same sixteen default and eight existing compiled failures, with identical names and diagnostics. The known cube coordinate still returns zLoc:1.1; state 2 and the hue/layout probes retain the documented incomplete boundaries. All 15 protected language fixtures and unrelated `myTests.tst` are unchanged. No Slipstream source or live world resource was modified.


## TCursor and supplied temporal work (D064–D071, updated 2026-10-17)

`TCursor` starts from a POV. `current` denotes the real list occurrence when a
materialized value is available, including a cursor cache entry. A reusable
`working` infon remains the fallback for read-only traversal. Callers must read
`current` after movement rather than assuming a stable returned address. Constructing
a cursor needs no Agent; applying queued work requires `attach()` with a
WorldManager and an Agent-owned session. `refresh()` initializes or settles pending
work; `goNext()` and `goPrev()` move between represented temporal neighbours;
`status()` exposes the result. `request` is null on a cache hit. `writeBack()` applies
supplied work and makes the selected item permanent, even when no value changed.

The regular solver computes the new values. The cursor adds retention of unaffected previous values. After the next item's given work drains, an internal retention relationship combines its result with the stable previous value using existing field/span correspondence. Known new values remain new; unknown corresponding values can borrow the previous description. This relationship is neither an identity assertion that old equals new nor a new authorization rule for transitions. Explicit contradictory new constraints still reject through ordinary solving. `:` retains its existing partial-match-from-the-start meaning; it does not authorize replacement. `%?` withholds retention for its whole scope, including descendants of a compound; field independence leaves sibling retention intact.

Work includes the item's wrkList, its definitions and the prerequisites needed to execute them. This first milestone stops additional ancestor-source discovery after that supplied work is exhausted. Applying ancestor listSpecs to temporal steps is the next integration boundary. There is no fixed syntactic depth limit: given work can reach nested values, while unrelated retained subtrees remain borrowed descriptions. A parsed literal initial list without its own type, work or listSpec is borrowed as a seed; unresolved work inside its children remains unresolved until demanded. This is not a claim that initialization fully normalizes the primary level of every model.

WorldManager's shared `solve()` entry point accepts source, destination and occurrence context separately. Ordinary `normalize()` uses the same source and destination; cursor execution uses a detached staging destination and the existing queue, dependency continuations, correspondence and transaction. Retention is enqueued once after the given work, in that same loop. The cursor never calls public normalize or starts a second solver. Ordinary relative-reference lookup shares the cursor's adjacent-position operation. A `%prev` read of the previous cursor occurrence uses its frozen value, so permutations and multiple rules all see the same inputs without rewiring model parent/prev/next links.

A waiting step retains its handle and exact waiting link. Calling movement again
while it is pending settles that step rather than advancing twice. Unrelated input
does no work; rejection and cancellation leave the visible cursor unchanged. A
successful step publishes into its real occurrence after ordinary checked solving.
Its saved original description permits later clearing. Immediate list wrappers are
owned by the new occurrence; immutable descendant descriptions remain shared.
Ordinary assertions into a cached descendant copy the selected shared branch before
publication, preserving earlier states and marking the containing cached value
permanent. Conflicting writes attach no branch. Publication uses the real occurrence
for parent links and identity witnesses.

A caller may register canonical represented field paths with `requireField()` before
initialization. These are value requirements for movement, not a request for an
entire recursively normalized state or for numeric size. The default remains
shallow. Required paths share their common prefixes, use ordinary WorldManager
requests, and retain the exact parser wait instead of retrying completed work.
Missing, ambiguous or exhausted unknown fields report partial; embedded field
layouts remain unsupported. A missing field alone does not create a subscription.

The cursor keeps its previous visible occurrence and prepared facts until every
required path succeeds. Shared prefixes are acquired once, with sibling requirements
visited in registration order and staged with private headers/member wrappers. Frozen input contexts remain separate
from that staged output. Checked facts are then stored in the selected occurrence
through the virtual graph, preserving the existing cache/write-back policy and
read-only working fallback. `preparedView(source)` identifies acquired fields in
that current stored value; it does not return a separate cache of field values.
Slipstream uses this to consume location/orientation immediately while optional
colors are still acquired progressively. Priority here is demand order before
optional display work; no general loader-priority API is introduced.

`clearCache()` explicitly clears unused disposable entries in the containing
represented list by restoring their exact original descriptions. It preserves
supplied seeds, written values, entries with new normalization work, and the current
entry held by any live cursor. Destroying a cursor releases that hold. There is no
automatic eviction budget. Keeping a returned POV outside a cursor does not itself
pin its cached value; use `writeBack()` when it must persist.

Backward movement returns an existing cached occurrence without solving. When that
entry was cleared, the cursor finds the nearest retained predecessor and replays
forward once to the requested destination. Successful intermediate results are
cached; the visible current changes only when the requested destination is ready.
A later wait or rejection may therefore leave a successful disposable replay prefix.
This reconstructs supplied history without inventing inverse transitions. Missing
neighbours distinguish a proved boundary from an unavailable future; creation of an
entirely new neighbour still has no subscription. Parser waits on existing supplied
headers/RHSs subscribe normally. Partial results can still contain unknowns and
deferred descriptions.

Successive steps retire the previous internal handle when it is last in the dedicated cursor session, so pure movement retains one internal request. Explicit write-back and other session requests remain Agent-tracked; callers may retain handles explicitly. Agent still owns session lifetime and cancellation. A completed, occurrence-owned scalar proof can be reused when reading the frozen previous value, avoiding replay of its transition recipe. List-backed values can be reused by another cursor on the same occurrence. There is no separate cross-session deduction cache or goal reset.

Fixed-width stepping is linear in the number of steps in the measured families. A changed record level is scanned once through existing field correspondence and immediate output construction. Unchanged nested descriptions are not recursively traversed; there is no general constant-time claim for wide output, definition solving, ambiguous alignment or changing list shape. Ordinary named-value arithmetic, general sparse correspondence, ancestor listSpec integration retain their existing boundaries. D068 adds the Slipstream cube materialization path. TSpan, SCursor and detailed materialization controls remain future work.

Original stepping measurements are recorded with D067 and in `Docs/Design Docs/Historical/tcursor-work-2026-10-14.md`. D071 records the direct-occurrence cache and backward replay validation.

### Deferred definition views and Slipstream (D068, 2026-10-14)

Composite definition layout acquisition retains literal composite fields as deferred descriptions. Field correspondence evaluates them when actual constraints require their interiors. A bare unbound field identity does not impose that demand. Deferred descriptions preserve list requirements and definition scope, and a demanded field binds `%self` to its materialized occurrence. Inherited constraints are still acquired and contradictions still reject atomically.

Explicit selection input is supplied work even when additional ancestor discovery is disabled. The same retained correspondence and solver handle execute it. Ordered members with repeated type names are not record keys. `materialize` supports the supplied-work option without a second solver.

Slipstream's TimeView now uses TCursor, with separate scene-owned materializations for renderer demands. Cursor stepping retains unknown unchanged values. D071 now stores disposable materializations in the temporal list while preserving their source descriptions; shared definitions remain unchanged. Scene sessions and request handles provide pending continuation and once-per-state display work. Ancestor temporal listSpec transitions and general inverse movement remain deferred. See `Docs/Design Docs/Historical/tcursor-viewer-2026-10-14.md` for the exact verified boundary.

### Fixed state-change verbs (D069, 2026-10-14)

A fixed verb can be an ordinary definition of the changed fields of the next temporal item. A closed preceding record supplies missing facts without prohibiting new fields; the new state's own explicit extent still constrains the result. TCursor evaluates its given definitions and prerequisites in the shared solver, then retains unaffected previous values. References to the preceding occurrence use the frozen cursor snapshot. `%tprev`/`%tnext` climb structural records to the nearest temporal level; ordinary `%prev`/`%next` remain local. Computed index expressions keep the caller's occurrence and template context.

Published `addedTypes` are checked nominal evidence and remain readable independently of whether this occurrence's own meaning has been demanded. Reading binds that evidence without prematurely marking the own meaning applied.

The public `rubixVerbs.pr` model implements both directions for all six faces, with clockwise viewed from outside the named center color. Its 26 named positions carry color descriptions and exact orientation catalogue indices. Nine positions change per turn; remaining positions and the overall pose are retained. `solved-rubix-turn-state` supplies the initial inventory. The ordinary renderer-facing parts and faces are deferred views of that inventory. The engine contains no cube-specific dispatch or geometry. This milestone covers fixed explicit verbs, not ancestor temporal listSpec execution or general inverse navigation.
