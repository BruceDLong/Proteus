# Proteus4 normalization issues and design decisions

The design discussion proceeds from the meaning of normalize and its solving loop through lists, intersections, definitions, and rules. Each choice must explain its streaming behavior, computational cost, alternatives, and justification. Semantics should be expressed through graph relationships, with optimizations justified by theorems.

This is the issue agenda and decision history. The companion [consolidated design](normalization-design.md) contains the resulting contract. The framework audit (`Historical/normalization-framework-audit.md` in Git history) supplies implementation evidence and proposals; its recommendations are not automatically accepted decisions.

Created 2026-10-10; updated 2026-10-11. Current issue: **D04, intersections and finite alternative domains**. D02 establishes directed acquisition, completion, and continuation without restart. Exact leaf and list interfaces remain under discussion, followed by intersections and definitions. D01 planning design is deferred until intersections and definitions have been considered. Develop the request contract alongside those examples. “T-rules” covers both theorem/transformation rules and temporal/T-list rules, as clarified in this discussion.

## Discussion order

| Issue | Question to resolve | Concrete result of the discussion |
| --- | --- | --- |
| D02 | What is one step of the solving loop, including leaf wrkList relationships? | Execution unit, dependency and publication rules, and a worked scalar trace. |
| D03 | How do lists, listSpecs, and sparse lists participate? | Shared list relationships, correspondence rules, and expansion limits. |
| D04 | How do intersections compose with leaf and list solving? | Pattern/input/result relationships and an account of alternatives and partial results. |
| D05 | How do definitions participate in all the above? | Declaration binding, instantiation, correspondence, and proof reuse. |
| D01 | After intersections and definitions, when is planning useful and where should it occur? | Counterexamples to the obvious strategies, request contract, and a justified planning boundary. |
| D06 | How are rules and temporal situations represented in normal form? | A matrix of situations and a rule representation with explicit premises and conclusions. |
| D07 | How are rules found, matched, and applied? | Search, indexing, direction, guard, cycle, and cost policies. |
| D08 | Do %rules, concatenation, and other constructs follow from the design? | Composition examples and a list of any genuinely missing primitives. |
| D09 | What counts as a justified optimization? | Equivalence obligations and a graph-only extension exercise. |
| D10 | Does the complete design work, and what old machinery can be removed? | End-to-end traces, complexity checks, and a replacement/deletion sequence. |

Issue IDs remain stable even though the discussion order has changed. Streaming and Big-O are part of every issue, not topics deferred until D10. During D02–D05, collect cases where the obvious strategy is inapplicable, fails to make progress, or does unnecessary work. Those cases are the input to D01; they do not require designing a planner in advance. A later example may expose a gap in an earlier choice; record the revision rather than introducing an undocumented exception.

## How each decision is recorded

An issue is **open** until a choice is made. A **deferred** issue is open but waits for specified prerequisite discussions or evidence. A **proposed** choice has an argument but is not settled. An **accepted** choice states the contract and its reason. A **superseded** choice retains its history and points to its replacement. Implementation and validation status are separate from decision status.

Each completed entry records:

- The question, chosen behavior, date, and the discussion or evidence that settled it.
- The graph representation and one worked solve, including the normal form before and after.
- The reason for the choice, alternatives considered, and why they were rejected or deferred.
- Streaming behavior: partial input, relevant late information, closure, invalidation, and result delivery.
- Complexity: workload variables, assumptions, worst-case and incremental work, memory, and unavoidable output cost.
- Justification: semantic premises, proof argument or counterexample, and tests or measurements. A passing example is not a proof of general equivalence or complexity.
- Consequences for the consolidated specification, existing behavior, and code that becomes unnecessary.

When a decision changes, update the specification and this history together. Distinguish a changed semantic choice from an implementation that has not yet met the choice. Existing unresolved topics in [open-questions.md](open-questions.md) remain open unless explicitly resolved here; do not create contradictory resolutions in the two documents.

## Common evaluation questions

For every issue, establish what is known, unknown, contradicted, provisional, or waiting for relevant input. Distinguish an unfinished parser input, explicit open syntax, implicit open membership, and a closed structure with unknown values. They need not have the same completion or absence rules.

Ask what can proceed before the rest of a stream arrives and which link or newly available edge continues a blocked deduction. Ordinary normalization learns more about mathematically immutable infons; it does not treat each publication as a mutation requiring a restarted goal. Exceptional semantic changes trigger a new goal explicitly. Specify those triggers where relevant, including what consumers observe. Do not make a global stream-end wait the default answer to a local question.

Use these complexity variables where applicable:

| Variable | Meaning |
| --- | --- |
| N, E | Materialized nodes and relevant relationships inspected for the request. |
| L | Logical list extent, potentially enormous or unknown. |
| S, A | Compressed spans/templates inspected and distinct occurrences demanded. |
| C | Candidate matches considered. |
| F, P | Rule firings and queued operations, including repeated or stale work. |
| U, K | Input publications and dependencies/support records affected by a publication. |
| O | Output facts or occurrences that must actually be produced. |

State additional parameters for branching, recursive rules, and numeric bit length when they matter. Separate the cost of finding a route from matching, applying it, repairing support, and proving completion. A priority queue may add logarithmic queue cost; a fast queue does not remove repeated semantic work. State worst-case bounds separately from amortized claims, and justify the latter over a specified sequence of updates.

Compare finite fixed-schema records, overlapping requests, repeated unchanged requests, sparse spans, growing streams, ambiguous matches, and recursive rules. A scan over finite records may be linear under explicit assumptions; that does not establish linear complexity for arbitrary inference. Materializing O output items necessarily costs at least proportional to that output. Merely asking about a sparse list header should not incur work proportional to L.

## D01 Normalize contract and planning

**Status: planning design deferred until D04 and D05 have been considered, by user decision on 2026-10-10.** The earlier agenda put this first; this decision supersedes that order. Develop the meaning of normal form and request scope alongside the solving examples, without requiring an advance planning architecture.

**Reason:** many tasks have an obvious strategy. For `_ = 5`, copy the supplied value into the unknown and update the required flags. For ordinary compatible list merges, propagating through both or all lists from start to end is the baseline strategy to examine. These examples motivate a cheap common path; they do not prove that every list operation should scan all members.

Before choosing a planner, record concrete definition and intersection cases in which the baseline strategy fails or is suboptimal. For each case state the input and requested result, what the baseline does, why it is insufficient or expensive, the alternative, and whether the need for that alternative is detectable cheaply before starting. Distinguish a contradiction, a wait for relevant streaming input, a lack of an applicable strategy, and excessive work.

**Options reserved for that discussion:** planning at the start of normalize; planning when a subgoal is initialized; or planning after the obvious strategy fails or proves inadequate. Reusable compilation and local preparation of an atomic write are separate mechanisms whose costs also need accounting. No location, trigger, or planner representation is chosen now.

**Reference implementation observation (before the replacement):** `GraphNormalizer.normalize` binds the target and submits a root goal. The runtime calls `beginSearch`, creates a session, and the worker enters `advance`/`follow`; there is no separate complete-request planning pass. Definition application can prepare a local `ProteusDefinitionPlan` within solving. Sources: [entry points](/home/bruce/devl/Proteus4/GraphFoundation.Lib.dog:103), [runtime submission](/home/bruce/devl/Proteus4/graph/GraphSearch.h:927), [search initialization](/home/bruce/devl/Proteus4/graph/ProteusGraph.h:985), and [definition plans](/home/bruce/devl/Proteus4/graph/DefinitionApplications.h:788). This observation does not choose the replacement design.

**Streaming and cost:** a complete advance plan must define its boundary on open input. Compare the cost of detecting an exceptional case and planning against the work saved, including work already spent on a failed baseline strategy. Charge revisions and discarded plans. Compare first-result latency, repeated or overlapping requests, and updates to premises. A streaming wait alone is not evidence that a different reasoning strategy is needed.

**Worked cases:** a literal; a fully expressed state; the same state obtained from a definition; a single requested field; whole state; an unfinished stream; and the counterexamples collected during D02–D05. Show when each returns useful information and what it still owes.

## D02 Solving loop and leaf work

**Status: partly accepted, 2026-10-10; numeric completion clarified 2026-10-11.** The decisions below settle progress, default stopping, subscriptions, and the role of the virtual graph. Exact interface operations, the complete matching rules, and list/intersection composition remain to be specified. Planning remains deferred under D01.

### D02A Acquire the requested facts without mandatory intermediate copies

**Accepted.** The goal identifies the facts needed by its target. For a scalar or string, these can include a value compatible with its requested size and type. The solver may follow virtual relationships to obtain those facts without filling every intermediate infon. It need not solve every unknown reachable from the target.

For `_ = 5`, direct transfer remains the obvious operation. For a chain such as `_ = (*10+_ = 5)`, examine the actual relationships and qualifications: do not require a publication at every intermediate node merely to advance the traversal. Preserve the intermediate size and any restrictions that govern what can reach the outer target. This is not a declaration that every chain can be flattened.

**Reason and alternatives:** filling each intermediate infon retains partial results directly in the model, but can repeat definition evaluation and clutter sparse spans. Always filling every stage is not the default. Always discarding intermediate results is also not required: useful partial answers such as oneOfs, ranges, or lists of ranges can remain in the model. Decide which facts must be published from the requested result and semantic relationships, not from a requirement to store the traversal cursor in infons.

### D02B Finish the goal when its requested facts are established

**Accepted.** In the default mode, once A has the requested compatible value and other facts needed for that goal, stop searching for A and cancel its remaining routes and subscriptions. A completed result is not reopened merely to explore additional possible proofs or to check global coherence. Completion of A does not cancel work still required by other goals.

An asserted identity supplies a route; it need not be converted into an exhaustive test of all alternative sources. Compatibility and prerequisites required to use the selected relationship still matter, and an encountered contradiction is not a valid result. A separate coherence/debug mode may try multiple strategies and compare their results. Its extra work is explicitly requested, not part of every ordinary normalization.

A completed infon can retain `isNormed` to make repeated normalization return without creating goals. The parser may establish this for an already complete literal with given type, size, and value; known size is sufficient in this example, not universally necessary. D02F specifies numeric completion when size remains unknown. The precise flag test must reflect the requested completion scope and the parsed relationship structure; a token's known payload alone must not accidentally certify an enclosing unfinished expression. Work out the exact flags with the leaf cases rather than inventing them here.

**Reason:** a known answer to the requested question does not benefit from restarting or repeatedly searching for the same answer. This narrows the earlier agenda's broad completion discussion: default acquisition and exhaustive coherence checking are distinct operations.

### D02C Continue at the waiting link and never reset a live goal

**Accepted.** Infons are mathematically immutable. Parsing and normalization reveal information about them. If a promising link cannot yet supply what is needed, subscribe at that link and retain the local continuation. Other useful work or semantically permitted alternative routes may proceed. An arrival resumes that continuation or exposes a new edge; it does not clear visited work, conclusions, or the goal's strategy and begin again.

The graph manager has no semantic operation for clearing and restarting a goal. Renaming a whole-goal replay to “continue” would not meet this decision. A goal may finish, reject a route, wait, continue, or be cancelled. Exceptional changes of meaning or state are handled by an explicit semantic trigger creating a new goal, not by resetting the old one. The precise event and ownership API remains open.

**Reason and alternatives:** retain the benefit of monotonic progress without forcing every search step to become a model copy. Restarting passes or whole goals after ordinary publications are rejected for the replacement design. This differs from the current `resetTask` path in [GraphSearch.h](/home/bruce/devl/Proteus4/graph/GraphSearch.h:520); it is not an assertion that the implementation already satisfies the decision.

### D02D Preserve relationship structure and prune completed work

**Accepted.** The POV interface exposes the semantic virtual graph and controls removal of fully merged RHS work. In the user's flat asserted `A=B=C` case, if B supplies everything A needs and the interface establishes that the remaining RHSs add nothing useful to A, A can complete and those redundant work entries can be removed. No intermediate copy is required just to justify taking the route.

**Clarified 2026-10-11:** parentheses usually do not change the result of ordinary `A=B=C` versus `A=(B=C)`. Do not impose a separate solving strategy merely because the parser represents a group. The important distinction is a type-changing relationship such as `A==(B=C)`: C may have a different type from A, so its type cannot be applied directly to A by flattening the chain. Preserve the qualifications and prerequisites that govern the transformation. The existing `unit/graphText/identityInSource` case in [ProteusTests.dog](../../ProteusTests.dog) exercises `{*3+$ *4+$} == ($ = 'CatDogs')`, expecting `{'Cat' 'Dogs'}`. It currently uses the checkpoint engine and must be ported with the real tests; it is not evidence that the scalar replacement already supports this case.

Stopping A's search and proving a work entry removable are distinct decisions. A completed scalar goal does not by itself erase a nested or qualified relationship that still carries meaning. The interface may discard fully represented pending work while retaining any identity or context still needed by other uses. Removal from one wrkList is not deletion of a source infon used elsewhere. Exact consolidation criteria for each relationship belong with the leaf, list, intersection, and definition rules. Current RHS consolidation (`Historical/rhs-consolidation-results.md` in Git history) is implementation evidence, not the chosen limit of the new design.

### D02E Partial answers and print reload

**Accepted scope clarification, 2026-10-10.** Print/reload resumption is not a requirement. A live goal may keep its traversal and subscription state in memory; no serialized continuation or infon at every intermediate step is required. Useful partial answers may still be represented by infons, including oneOfs and ranges. Semantic output and transient execution state have different purposes.

### D02F Numeric completion with unknown size

**Accepted, 2026-10-11, from the user's clarification.** For ordinary numeric normalization, once the value is established and the given idents are exhausted, the infon can be marked `isNormed` even though its size remains unknown. Applicable type constraints and the prerequisites of the used relationships still govern acceptance. Explicit unknown size does not create an obligation to discover a size. Named range types such as `rot` illustrate this rule; the implementation must not special-case that name.

Exhaustion is established by retained source cursors and attempt outcomes, not by rescanning the wrkList. A required ident still being parsed or normalized is pending, not exhausted; retain its local continuation. Relationships already fully accounted for need no further search. This does not require an exhaustive coherence pass or exploration of every reachable definition. Do not create a subscription merely because the size slot is unknown.

Keep any known size or range constraints. Do not fabricate a size or equate range bounds with infon size. An operation that explicitly needs size has a stronger obligation: ordinary numeric `isNormed` alone cannot satisfy it. This decision does not certify an unfinished list or containing expression.

**Reason and alternative:** low-level numeric infons often have no discoverable size, and some intentionally leave it unknown. Requiring both size and value would prevent normal completion and invite useless search. Completion describes the settled work for the request, not full knowledge of every attribute.

**Effect on earlier guidance:** this explicit decision supersedes the instruction in [AGENTS.md](/home/bruce/devl/Proteus4/AGENTS.md:178) not to relax `isNormed` for unknown sizes, for the numeric case specified here. The engine has not been changed by this design decision.

### D02G Exhaust parsed work and continue requested links

**Accepted, 2026-10-11, from the user's clarification.** Once parsing of the relevant infon is done and every current wrkList member has been tried, the given work is exhausted. The list need not be empty: meaningful or unresolved relationships may remain stored. Further items in ordinary normalization come from requests for other graph links. Track those requests and their pending continuations; a required pending result is not exhausted just because the current wrkList cursor has reached its end.

Do not subscribe to unspecified future additions after parsed work is exhausted. A requested link delivering facts or another relationship continues the affected attempt, without resetting the goal or rescanning the tried prefix. An unknown result with no pending requested links can be reported as partial; a required requested result still in flight is waiting. Ordinary numeric completion still follows D02F.

**Reason and cost:** parser completion, retained traversal position and the outstanding link requests supply the needed evidence. An empty global queue or an empty wrkList does not. Maintain these records as work proceeds so checking exhaustion does not introduce another traversal. Charge arrival work to the affected continuations and newly exposed relationships.

### D02H Return a handle and defer sharing across sessions

**Accepted direction, 2026-10-11.** Normalize continues to return a handle. The caller can inspect its status or mode and its result without depending on solver frames. The replacement returns `WorldRequest`, with `status()`, `complete()` and `result()`; runnable work drains synchronously and parser completion can continue a waiting request. Status distinguishes completion, partial information, waiting, rejection, unsupported work, cancellation and errors. This does not freeze the present field names or prescribe a new status layer.

Agent retains session ownership under G07. Sharing deductions among sessions is deferred until a demonstrated need arises; it is not a prerequisite for streaming or the engine cutover. Within a session, cancellation must still respect other callers that use shared work. Closing one session must not close another.

**Reason:** the handle already provides the caller's point of access to a normalization request. Local continuation can develop behind that boundary without adding cross-session ownership and context machinery now.

### D02I Recover the detailed scalar contract from fixtures and reference code

**Implementation clarification, 2026-10-11, following the user's instruction.** This architecture document is not a complete language-semantics specification. Preserve established fixture expectations and consult the reference implementations for details. Their execution architecture, caches and restart protocols are not requirements on the replacement.

For the admitted closed, context-free positive scalar fragment, all of a leaf RHS's type/size/value facts can be checked and represented without keeping that work entry. An unknown value supplies no extra value constraint. This is why `10 = _`, `_ = _`, and `? = $` print as `10`, `_`, and `$`; the last two still have unknown values and stay partial. A successful qualified scalar match likewise discharges the test without asserting identity to its source. Retain nested expressions/constraints until their own consolidation conditions are established. Pending parser input, nominal/contextual facts and temporal declarations must not be mistaken for empty scalar evidence.

The reference evidence is Proteus3 [planConsolidation](/home/bruce/devl/Proteus/Reasoner.dog:973), particularly `dcScalarUnknownWithoutConstraints` and `dcLiteralAlreadyConstrainsUnknown`, plus the unchanged `merge/directUnknown`, `merge/num`, `merge/str`, and `tUnknown` fixtures. The initial replacement harness's assertion that every unknown RHS must remain stored was an implementation restriction inconsistent with these fixture expectations; its replacement still asserts partial status and absence of `isNormed`.

Value acquisition also carries representation information. Proteus3 [DO_COPY](/home/bruce/devl/Proteus/WorldManager.dog:1420) copies the value's size mode; Proteus4 [scalar copy publication](/home/bruce/devl/Proteus4/GraphFoundation.Lib.dog:1090) preserves separately known numeric size, and [qualified scalar matching](/home/bruce/devl/Proteus4/graph/ScalarPredicate.h:97) uses the matched candidate's representation. Strict string acquisition copies counted/given form. Qualified numeric matching can print `3` after matching `*4+_ == 3` while retaining checked size 4. This is not permission to erase the size itself. Strict numeric value copying preserves given-size evidence from either operand, including an explicitly unknown source size. Learning size must also preserve the given form of an already known literal. A positive bounded unknown numeric RHS can supply size to an unknown target under `==`; it does not supply a value or make the result complete. The unresolved any view must remain any until type evidence is acquired; normalization alone must not turn `?` into an empty overlay. Likewise, keep an unresolved parenthesized expression as the partial result. Direct probes confirmed that the reference preserves `(_)` and `($)`; the replacement had incorrectly reinterpreted their bodies as `0` and an empty string. Type evidence alone is not a scalar value.

**Reference differences:** a 196-case positive scalar matrix is an audit, not an oracle for reproducing old defects. Proteus4 leaves equal numeric literal `==` tests pending; Proteus3 resolves `9 == 9`, consistent with the replacement's checked-match completion. The replacement also retains an explicitly learned width when printing an unknown string, where older code can print bare `$` despite having size evidence. These are representation/completion differences; no protected fixture expectation was changed.

**Reason and cost:** the first scalar implementation retained too much leaf work and copied too little representation information. Correct these at virtual POV access using already checked prerequisite facts. Do not add printer suppression, old representation receipts, intermediate writes or rescans to make text expectations pass. Source-preservation and internal-size assertions supplement the historical output fixtures. The [scalar follow-up](normalization-design.md#scalar-compatibility-follow-up-2026-10-11) records validation and limitations.

### D02J Separate scalar prefix evidence and exclusion proofs

**Implementation clarification, 2026-10-11.** A colon relates available prefixes; it does not equate endpoints. Preserve the target's known size and never acquire its missing size from the colon RHS. For strings, an unknown target with known integer width takes exactly that many available bytes. Insufficient text, fractional widths and negative widths remain partial without a write. With unknown target width, available text can be acquired but its endpoint remains explicitly unknown. Existing literal prefixes are compatible when their common prefix agrees, without truncating either literal. Whole qualified string matching still requires the whole available text, so it cannot silently truncate an unknown-end source.

Reference evidence: Proteus3 `mergeLooseStrings`, `looseStringPrefixesAreCompatible` and `shouldCopySize` in [Reasoner.dog](/home/bruce/devl/Proteus/Reasoner.dog:815); Proteus4 [matchText and matchNumeric](/home/bruce/devl/Proteus4/graph/ScalarPredicate.h:52); and its `graphQualifiedText` and `graphNumericRange/endModes` behavioral tests. The latter tests' old identity registry, subscription and cache assertions are not the new contract. The port preserves result, endpoint and source assertions; proved qualified mismatches use the replacement's existing `worldRejected` status, while missing proof remains `worldPartial`. A checked leaf can be discharged without asserting identity.

Strict scalar `!` constraints complement only proved comparisons. Distinct known type, size or value facts can prove a mismatch; missing required facts yield an unresolved comparison. A matching positive candidate is excluded; a proven nonmatch accepts the constraint without copying the candidate into the negative template. Equivalent negative templates are redundant. Non-equivalent exclusions remain conjoined; neither supplies a positive value. This follows Proteus4 [scalarConstraint](/home/bruce/devl/Proteus4/graph/ScalarPredicate.h:21) and the unchanged `merge/directAtomic` fixtures, with Proteus3's direct consolidation cases as a second reference. Numeric signed displacement comparison uses local extent evidence without writing either operand.

A negative template's literal describes what it excludes, not an acquired positive value. The current value-oriented completion request therefore remains partial for a negative target, even after an exclusion has been proved and redundant leaf work discharged. Qualified negative matching, negative expression templates and conjunction reasoning beyond equivalent direct exclusions remain later work. No claim is made that strict `!` proves all template or range semantics.

**Reason and cost:** express these finite relations at the virtual scalar access boundary. String frames carry a source reference and represented length, allocating a substring only at target publication. Prefix comparison costs the common length; whole comparison first checks lengths. Exclusion comparison is constant graph work plus scalar comparison cost. All new cases reuse the existing frame cursor and parser subscription; no additional solver, copied graph, cache, restart or intermediate publication is introduced. The [prefix and exclusion follow-up](normalization-design.md#scalar-prefix-and-exclusion-follow-up-2026-10-11) records validation.

### D02K Fold numeric expression operands through the retained frame

**Implementation clarification, 2026-10-11.** Numeric expressions use the same request stack and attempts table as supplied scalar relationships. The virtual graph exposes the expression's existing first/next operands before its wrkList. A frame holds the next operand and accumulated facts. Once an operand's prerequisites have been checked, its facts contribute to the expression; they are not copied back into that operand. Parser completion resumes at the pending operand with the prefix intact. Operand link consistency is checked locally, so a malformed cycle cannot make the scan run forever. Contextual aliases are checked before reusing an attempted view; the current item-keyed facts cache is valid only for admitted context-free scalar views.

Two finite arithmetic rules are recovered from Proteus4 [planConcat](/home/bruce/devl/Proteus4/GraphFoundation.Lib.dog:1338). A subsequent numeric operand with `fromContext` adds its value. Otherwise its known nonnegative size is the radix: `value = value * radix + nextValue`; known sizes multiply. Thus `(*8+4 *4+3)` gives `*32+19`. Missing operand values or radices stay partial, and mixed/string/list concatenation remains outside this slice. A conflicting outer size rejects the publication. Existing scalar displacement rules still distinguish an expression with an inferred known extent from one that learns its extent from an outer header; do not add unconditional modulo reduction.

Numeric value bounds are exact half-open intervals, with a point distinct from an empty interval and an unconstrained value distinct from either. A finite unknown numeric input followed by known additive offsets yields an interval. Interval width is not the expression's outer size: `*10+(*3+_ +2)` has size 10 and possible values in `[2,5)`. The outer size does not itself clip this explicit interval. Strict supplied constraints narrow by intersection; a qualified match of an explicit range requires containment, with partial overlap left unresolved. Missing RHS values are not fabricated from their sizes. Prefix relationships keep their independent endpoints as in D02J. Evidence comes from Proteus4 [NumericRangeAccess](/home/bruce/devl/Proteus4/graph/NumericRangeAccess.h:15), [NumericRange](/home/bruce/devl/Proteus4/graph/NumericRange.h:8), [matchNumeric](/home/bruce/devl/Proteus4/graph/ScalarPredicate.h:97), Proteus3 [evalForRange and mergeANDRanges](/home/bruce/devl/Proteus/Reasoner.dog:724), and retained language fixtures.

A narrowed partial range is published as the existing two-term Proteus expression `(*width+_ +offset)` with the target's independently known size. Sources and their operands stay unchanged. Fully checked numeric expression RHSs can be removed once their result is represented. The partial expression itself stores the remaining knowledge; no old valueExpression provenance or representation-receipt cache is needed for this monotonic slice. A body that already expresses the same interval and endpoint is kept intact, including grouped partial expressions. Repeated normalization may inspect a partial body in a new request, but it does not replace it with an equivalent allocation. Live requests still never restart.

**Reason and cost:** operand folding and exact interval arithmetic supply the required semantics without a recursive secondary normalizer or a planning phase. Each admitted operand/relationship is inspected once per request; each distinct scalar view gets at most one facts frame, after occurrence validation. Cost is O(V + E) graph work plus rational arithmetic/comparison cost. Only a changed requested interval needs two new model operands. Already represented partial ranges need no publication except discharging redundant work. Pending input preserves the fold and resumes locally. This is not a claim of linear general inference, inverse arithmetic, arbitrary numeric set operations or general intersection/list support.

See the [numeric expression follow-up](normalization-design.md#numeric-expression-and-range-follow-up-2026-10-11) for validation and remaining limits.

### D02L Borrow text pieces while folding string expressions

**Implementation clarification, 2026-10-11.** Closed positive string expressions use the operand frames from D02K. Each operand supplies its checked represented text, including a prefix obtained through `=:` or `==:`. Concatenation joins those texts in order. The expression counts the resulting bytes independently of the operands' declared or unknown sizes; a contradictory outer size rejects the target publication. Even a one-operand string expression can establish this counted size. Unknown text stays partial, unfinished parsing waits at the existing operand, and mixed or negative operand expressions remain unsupported.

This follows the string branch of Proteus4 [planConcat](/home/bruce/devl/Proteus4/GraphFoundation.Lib.dog:1362), its `graphFoundation/expressions/strings` and `rhs` cases, its qualified `graphText/concatScalarSource` case, and the unchanged `concat/simpleStr` and `concat/strCat` language fixtures. Reference probes also establish empty text, explicitly unknown operand sizes, declared operand widths that differ from their available payload, nested prefix operands, and the counted single-operand expression. String indexing remains the existing byte indexing; this work does not redefine Unicode width. For deeply nested parser-to-solver tests, use a leading `|` at each nested list/expression to specify an empty listSpec (`(| first second)`). This avoids repeatedly trying the first nested member as a listSpec candidate and lets the stress fixture exercise the real parser without changing its grammar.

A facts frame refers to borrowed text through a small shared tree. A leaf retains its source state; a concatenation retains its two source references and the represented length of its left branch. The containing facts retain total represented length, so a prefix is a bounded view without copying text. An iterative cursor visits the pieces in order for comparison or final publication, including prefixes ending inside a nested branch. Empty branches are skipped. No intermediate infon or growing string payload is written. Fully checked string-expression RHSs can be discharged when their text and endpoint are represented in the requested target, using the same consolidation rule as numeric expressions.

**Reason and cost:** repeatedly copying an accumulated string or flattening nested operand vectors would make simple nested concatenation quadratic. Each operand join instead allocates one small descriptor and reuses its children; scalar-fact copies share that descriptor. Each distinct admitted scalar view still uses one facts frame. Final assembly is linear in the represented tree occurrences visited plus output bytes, with amortized string append cost. Comparison visits only the required common prefix and can reject unequal whole lengths first. Shared text used twice must appear twice in the output. Exact length addition is checked before a join: a small shared graph can describe more output bytes than the length type can represent, which reports an error before publication rather than wrapping. This is not a global linear bound on arbitrarily many comparisons. The final-publication counters distinguish visited text descriptors and emitted bytes; streaming tests verify zero emitted bytes while waiting and unchanged operand/frame counts on continuation.

The engine retains no additional scheduler, planner, global text cache or restart path. Computed size expressions, list concatenation, text/list matching and general requested-link arrivals remain separate work. See the [string expression follow-up](normalization-design.md#string-expression-follow-up-2026-10-11) for validation.

### D02M Evaluate a supplied size expression through the same operand cursor

**Implementation clarification, 2026-10-11.** A scalar's computed size is a prerequisite on its endpoint. The virtual graph visits its existing size operands before its value operands and supplied identities. The frame records which slot it is visiting and retains the same operand cursor, accumulated facts, attempts table and parser wait. A slot transition is forward progress to a different stored expression; it never repeats a visited prefix. No synthetic infon, recursive normalize call or second evaluator is needed.

The checked **value** of the size expression supplies the owner's endpoint; the operand's own size, type and range width do not transfer to the owner's value. `*(10)+5` gives `*10+5`, and `*(*8+4 *4+3)+5` gives `*19+5` because the computed value is 19 although that operand expression has size 32. The established numeric operations and nested supplied constraints apply unchanged. The endpoint is available before signed displacement, value-expression size checking and outer relationships. Conflicts publish nothing.

For this slice, only nonnegative whole numeric results collapse to a size literal. Unknown, negative, fractional or nonnumeric size results remain expressed and partial, following Proteus4 [planConcat slot 1](/home/bruce/devl/Proteus4/GraphFoundation.Lib.dog:1379), `graphFoundation/expressions/computedSize` and `negativeSize`, and unchanged `merge/calcSize` / `merge/calcNegativeSize` fixtures. This does not impose a new rule on direct literal sizes or reinterpret negative sizes used by later selection semantics. An explicit unresolved size expression remains an obligation, unlike the bare unknown size allowed by D02F. Inverse solving from an owner endpoint back into its size expression is not implemented.

Publication replaces only the requested target's size state with a fresh numeric literal, removing its former operand links from that new state. Borrowed size states and operands stay unchanged. A computed endpoint can also be published with a partial value range, even when the value body already expresses the range. No old sizeExpression provenance cache or intermediate publication is restored. General link-arrival subscriptions, including the original computedSubscription test's later addRHS event, remain unported; parser completion uses the already supported continuation path.

**Reason and cost:** the existing fold already computes the required scalar value, so sharing it avoids duplicated semantics and traversal machinery. For the admitted acyclic context-free fragment, graph work remains O(V + E), including both slots and supplied relationships, plus exact arithmetic and text costs. Each distinct admitted view gets one facts frame, even when both slots refer to it; occurrence checks still precede reuse. A parser wait resumes at that operand with all earlier facts and cursors retained. Only the requested target is published, once after all its required work succeeds. See the [computed size follow-up](normalization-design.md#computed-size-follow-up-2026-10-11) for validation.

### D02N Retain undecided scalar comparisons while acquiring later facts

**Implementation clarification, 2026-10-11.** An undecided comparison is an obligation on the target, not an instruction to terminate its remaining supplied work. A positive target may retain a strict scalar exclusion awaiting type, size or value; a signed numeric equality may retain its comparison awaiting an extent. `_ = !5 = 7` resolves to `7`, while `_ = !5 = 5` rejects. `9 = -1 = *10+_` resolves to `*10+9`. This uses the existing exclusion and extent semantics, including negative targets remaining negative; it does not turn a negative literal into positive evidence.

The merge reports that more evidence is needed. Its caller keeps the checked source frame and original qualified edge in the existing persistent requirement structure, with the target's accepted facts unchanged. Later supplied relationships and member requirements continue normally. A transition from unknown to known type, size or value revisits pending comparisons using the same merge. Successful checks disappear from the pending chain; contradictory checks reject before publication. Relationships that supply no new fact cause no revisit. A derived member inherits pending checks by reference, so `{7| &{!5| ...}} = {_}` can resolve to `{&{7}}` without repeating ancestor propagation.

**Publication and boundaries:** a scalar with unresolved comparisons remains partial and keeps its original body and relationships; newly materialized members must have representable checked facts. Existing containing lists keep the relevant listSpecs and unresolved correspondence work. Other independent members may publish their results through the existing staged publication. A complete value does not override an unresolved supplied obligation. Grouped RHS work and expression operands must still settle before supplying their facts to a consumer; this increment does not distribute a consumer's facts into an unresolved RHS, solve inverse expressions, compare negative ranges, or align ambiguous lists. Ordinary unknown-size numeric completion is unchanged when there is no pending size-dependent comparison.

**Reason and alternatives:** immediately stopping at a partial comparison made later evidence unreachable. Ignoring that comparison would lose a semantic constraint. Restarting a frame or rescanning all its relationships would repeat completed work. Retaining only the undecided relationship allows directed continuation through the existing virtual graph, with no additional evaluator, planning phase or path costs. The reference engine's retained negative-template obligations in [planConsolidation](/home/bruce/devl/Proteus/Reasoner.dog:1079) confirm the need to preserve unresolved checks; its later-pass scheduling is not copied.

**Streaming and cost:** parser waits retain the same request, relationship cursor and pending checks; arrival resumes that source, and cancellation discards transient work without publishing. General later-link delivery remains deferred. For one occurrence in this monotone scalar fragment each of the three fact-presence fields changes at most once, so C retained comparisons cause at most 3C resumptions, plus their initial applications, requirement-node visits and payload comparison costs. Unrelated unknown edges add no resumptions. Persistent joins add constant storage per newly pending relationship and share checked sources across occurrence frames; only a fact arrival walks/rebuilds the pending chain. Separate occurrences pay for their own checks even when the source frame is shared. This bound does not claim linearity for arbitrary inference, changing ranges or backtracking.

### Streaming and complexity checks

For a simple acyclic chain of k constant-cost compatible links, locating a value available only at the far end requires O(k) link inspection. It need not require k target publications: the requested A may need only one, plus any semantic publication actually required by a relationship. If B already supplies the answer and no further premise is needed, inspect that route and stop without scanning the rest of the chain. Account separately for payload-copy cost and type/size compatibility; these are not always constant.

For incremental input, measure newly exposed work and the continuations actually woken. Reject an implementation that revisits the entire goal after each publication. This is a workload bound and a design requirement, not a proof that arbitrary matching or inference is linear. Include pending subscription storage, sparse occurrence allocation, and the extra cost of coherence mode.

Validate direct copying; a longer chain with only its end known; a flat versus nested RHS; a rejected B that blocks its route to C; a qualified relation requiring B to resolve first; an arrival at one pending link while another route proceeds; completed A creating no goals on another normalize; no sparse intermediate clutter; and useful partial oneOf/range results. Include parser completion metadata for literals versus unfinished containing expressions. Check a numeric value with unknown size and exhausted idents completing without size search; a named range type doing the same; a required pending ident continuing locally; and a stronger size request remaining unsatisfied by numeric `isNormed` alone. Preserve and enforce supplied constraints. With maintained cursor/status bookkeeping, checking numeric completion must not require another ident traversal. Print/reload checkpoint tests are out of scope.

### Remaining D02 questions

Specify exact goal requirements and `isNormed` scope within D02F's numeric completion rule, how the interface reports a usable or fully merged edge, and how local subscriptions retain and deliver their continuation without missed arrivals. Establish ownership when goals share useful deductions within a session; sharing among sessions is deferred under D02H. Define the coherence mode's comparison and reporting behavior, without making it a new default validation pass. Carry special meaning-change triggers into D05 and D07. None of these questions reopens the no-restart decision.

## D03 Lists and listSpecs

**Status: partly accepted, 2026-10-11, from the user's list discussion.** Identity sources, main-level scope, reuse during co-propagation, and deduplication of matching attempts are the design direction below. The proposed graph operations and chart layout remain to be reviewed. Planning remains deferred under D01.

### D02O Both absent overlay markers contribute no type constraint

**Compatibility correction, 2026-10-11.** `tUnset` and `tUnknown` both mean that a nonliteral payload has no known interpretation. Virtual POV access maps either marker to absent type evidence. The ordinary merge can then acquire numeric, string or list facts; neither marker may contradict an otherwise compatible known type merely because its enum value differs. A payload marked literal without a known interpretation remains unsupported.

**Reason and evidence:** the original overlay contract and retained `overlay/unsetOverlay` test explicitly cover both markers. A small client linked to the generated replacement shared library reproduces a mismatch: the same unknown target with a numeric RHS completes with `tUnknown` but rejects with `tUnset`. The reader had excluded only the former enum value when reporting known type evidence. Extending that one condition repairs the representation boundary; merge, comparison, publication and scheduling stay unchanged. No speculative type inference or new semantic rule is introduced.

**Streaming and cost:** parser readiness is checked before facts are read, so neither marker bypasses a parser wait. The existing pending comparison and publication rules still apply. Classification is constant work and does not add a traversal, cache, subscription or restart. Original tests requiring general link subscriptions, registry inference or mutation of intermediate sources remain pending; only assertions supported by the current contract are ported.

### D03A Sources of member relationships

**Accepted.** A member can acquire applicable relationships from its own wrkList; correspondence through an ident of its parent or a higher ancestor; its type; and its parent's listSpec or an applicable ancestor listSpec. Ancestor projection is central to intersections as well as to locating a given member in a corresponding list.

Keep the originating relationship, interpretation context, and correspondence. An ancestor listSpec is applied through its proper projection into the member; it is not automatically a direct constraint on every descendant. Already applied information is reused when following another route to the same member. Distinct templates, contexts, or relationships can still add information.

**Reason:** these sources contribute through the same virtual graph rather than requiring independent recursive normalization algorithms. A member's graph is richer than its short explicit wrkList chain.

### D03B Main-level scope for an unordered target

**Accepted.** Normalizing an unordered list fills its main level as far as reasonably possible: resolve its members, without automatically expanding all their contained levels down to materials or atoms. This does not prevent dependency search from reaching deeper information needed for a member's requested result.

The member obligations must inherit the parent's expansion scope. Calling ordinary normalize recursively on each member as a fresh top-level request would renew that scope at every level and defeat the decision. Sparse tails and unspecified open membership remain symbolic rather than being enumerated. The exact policy for what is reasonably available remains to be stated using examples; this decision does not invent a time or member-count cutoff.

### D03C Targeted access and shared span propagation

**Accepted.** When only a particular member matters, use its enclosing correspondence to find the matching member. Skip an embedded span when its extent and interpretation justify the skip; use an existing index when appropriate. A navigation shortcut locates a candidate and must not skip semantic conditions required to accept it. Unordered matching still follows nominal or explicit correspondence, not ordinal equality.

For a whole list or a requested span, co-propagate the corresponding spans and carry the resulting member relationships forward. Do not repeat a parent-to-member lookup from the beginning for every member. When propagation has already accounted for a parent ident or applied a listSpec, do not seek or apply that same contribution again in the same context.

**Reason and alternatives:** repeated targeted lookup can multiply traversal cost; eager flattening destroys sparse representation. Use targeted access for a target and sequential shared propagation for a span. Preserve which semantic contribution was covered, rather than attaching a context-blind “parent handled” or “listSpec handled” flag to a node.

### D03D Streaming correspondence and alternatives

**Accepted direction.** Use a streaming Earley-like correspondence process as the general way to align two or more list starts when unknowns and member extents permit several matches. Register each distinct semantic connection attempt once. A pending attempt is retained with its missing premises; later information continues it under D02C. “Already tried” must not mean “permanently failed because input was absent.”

A wrkList provides an AND representation for simultaneous constraints; a oneOf provides an OR representation for alternative correspondences. Keep each alternative's bindings and simultaneous constraints together. Do not combine a field from one possible correspondence with a field from another or publish an unproved branch's identities as unconditional facts. The execution index for finding existing attempts is separate from what the AND/OR structure means.

The default acquisition goal still stops once its semantic request is satisfied. A matcher must represent the alternatives required by that request; it need not perform additional coherence checking after completion. The exact relationship to first-complete grammar correspondence and unordered alternative selection belongs in D04.

### D03E Reuse the frame traversal for closed ordered members

**Implementation increment, 2026-10-11.** Ordinary closed ordered lists without listSpecs, sparse/embedded members, temporal markers or contextual/nominal semantics expose their existing first/next members through the operand cursor. Nested ordered lists use the same frame stack. A containment edge records the checked child frame instead of combining its value arithmetically. Expressions, size operands and supplied identities retain their existing roles. Each requested physical list keeps its original value state and member POVs, including their order, parent links and infonView identities.

List shape/size and member completion are separate. A list completes only when all requested members complete and its known extent agrees with the immediate physical count. Counting may fill an absent count-derived size, but never an explicitly unknown size. A supplied size mismatch stays partial and preserves that size while members can still normalize. This follows the retained `graphFoundation/closedList`, `givenUnknownSize`, `givenSizeMismatch` and `innerOuterSize` cases; inspect their size-state assertions, since the ordinary printer hides some supplied sizes on closed lists. A nested list counts as one immediate member. This increment does not implement embedded-span counting, and does not generalize ordered recursion to unordered descriptions; D03B's main-level scope remains unchanged.

An exhausted partial prerequisite returns its status through the existing pending frames. A required scalar relationship/expression stays partial; an independent list member can stay expressed while its siblings continue. A partial frame's unchecked facts are not published. Required parser waits keep the current cursor and accumulated member frames; cancellation and actual contradictions/errors retain their existing request-wide handling. No goal is cleared or restarted, and unknown values do not create speculative subscriptions.

After checking the request, publication follows only containment edges from its target, using the existing scalar publication routine for each requested member. RHSs, size operands and value-expression operands remain read-only. Repeated references to one admitted context-free infonView are checked and published once without combining their distinct POV positions. The model lock covers publication of the requested tree; a failed or unsupported route publishes none of it. The ordinary completed-list marker permits subsequent unchanged normalization to create no frames. No new scheduler, recursive normalize call, copied graph or reconstructed list body is introduced.

**Reason and cost:** list traversal and expression folding share structural enumeration, prerequisites and local continuation, while their edge meaning determines how results are used. For the admitted acyclic context-free physical graph, work and temporary frame/member storage are O(V + E), plus scalar arithmetic, comparison and output costs. Publication visits only the distinct views within the requested containment scope, not all prerequisite frames. The scope explicitly excludes correspondence, concatenation, unordered descriptions, listSpecs, sparse spans, contextual projection and general link arrivals; those need their own justified operations. See the [closed ordered-list follow-up](normalization-design.md#closed-ordered-list-follow-up-2026-10-11) for validation.

### D03F Closed list composition shares checked member sequences

**Implementation increment, 2026-10-11.** Closed ordered concatenation, adoption and strict positional correspondence use the same member-sequence description. A literal contributes its checked immediate member frames. A concatenation joins sequence descriptions without copying the represented members; a nested ordinary list remains one member. Equal closed shapes expose paired member relationships through two retained cursors. Each derived pair uses the existing scalar merge or, for nested lists, the same correspondence operation. Multiple supplied lists combine into successive checked results without rereading previously inspected member work.

**Reason and alternatives:** copying every intermediate concatenation would repeat growing prefixes. Looking up each corresponding member from its parent's beginning would repeat list traversal. Mutating a cached source frame would mix different target occurrences and make sibling projections interfere. Instead, read frames remain reusable source evidence; derived pair frames hold the target's combined facts and do not reread its wrkList. This is an explicit operation over proved placement, not a claim that parent/next navigation is itself identity. No planner, second evaluator or persistent copied adjacency graph is introduced.

The shape premise is limited to ordinary closed ordered lists without sparse/embedded members, listSpecs, nominal/contextual interpretation, temporal structure or qualified list matching. Supplied endpoint or physical-length mismatches remain partial before any member write, preserving the retained correspondence tests. A scalar disagreement at an established corresponding position rejects the request. Unknown numeric size still permits a known numeric member to complete under D02F. A parent endpoint can be supplied by a matching source; merely adopting an explicitly unknown endpoint does not manufacture one. Member completion is independent of the list header: `*1+(*_+{1})` completes using the target endpoint while its source remains explicitly unsized. A conflicting operand endpoint still blocks concatenation; it cannot disappear when another operand has unknown size. General sparse alignment and `==`, `=:`, `==:` list semantics remain separate operations.

Publication first prepares destination occurrences and checks the complete write scope. Existing literal target lists retain their value state, member POVs and links. Concatenation and adoption allocate independent destination members, including nested members, then use the common scalar/list publication routine. Source expressions, source lists and transitive prerequisites stay unchanged. Exhausted checked facts can replace the supplied constraint; unresolved source/member work remains expressed. An unresolved member expression is not materialized into a blank adopted result. Two different correspondence results targeting the same shared infonView need a joint-constraint operation; this increment reports that case unsupported before any writes instead of arbitrarily choosing one result. Repeated context-free reads and independent outputs from a shared source remain supported.

**Continuation and cost:** pending parser input retains the source cursor and earlier checked frames. Arrival resumes that exact frame, then enables the remaining pairs; cancellation publishes nothing. General subsequent edge arrivals and open-list prefix publication remain unfinished. For admitted input views V, inspected structural/supplied edges E, positional pairs P, materialized output occurrences O and copied retained-constraint entries R, processing/storage are O(V + E + P + O + R), apart from scalar arithmetic and payload costs and assuming the existing hash indexes have expected constant-time access. R is zero for the ordinary exhausted-leaf cases measured here; a pair currently copies its left member's retained-work vector when one remains. Sharing that residual storage is a possible later optimization. A concat join is constant work; traversing its result costs its sequence-description nodes plus emitted members, without recopying prefixes. Pairing two closed spans is linear in their represented members; nested members introduce only their own required pairs. Bounds do not extend to ambiguous sparse, unordered or nominal matching.

See the [list composition follow-up](normalization-design.md#list-composition-follow-up-2026-10-11) for retained semantic tests, scope and measured work.

### D03G Finite listSpecs derive ordinary member relationships

**Implementation increment, 2026-10-11.** A context-free listSpec on a closed ordered literal list supplies a strict requirement to each physical immediate member. The retained structural cursor exposes the template once before those members. Its checked facts are shared read-only; each member gets its own derived relationship through the same operation used for list correspondence. Numeric, string, range, exclusion and nested closed-list requirements use the existing merge operations. Later parent correspondence consumes the already constrained member facts without applying the template again. Ordinary nested lists remain single members; ancestor templates across embedded spans are not part of this increment.

**Reason and alternatives:** copying a template and normalizing the copy per member repeats template evaluation and creates model work solely to record progress. A request-local checked template plus occurrence-specific derived facts avoids both, without changing source facts or adding another evaluator. Speculative pair results are not stored in the context-free source cache. A closed empty list needs no template evaluation: there are no members to constrain.

Publication preserves existing member POVs and links. A finite listSpec is discharged only when its contribution is representable in the checked members and the known list endpoint equals their count. Explicit unknown or mismatching endpoints keep their template. An unresolved member/template retains its actual expression or requirement; rejection and unsupported required work publish nothing. No new global applied-template flags, planner or path costs are introduced.

**Continuation and cost:** the template and members use the existing prerequisite stack and parser wait registration. A template wait resumes that template; a later member wait retains the earlier projections. Cancellation publishes nothing. For context-free source views V, relationships E, template/member applications A, correspondence pairs P, output O and retained-constraint copies R, work and storage are O(V + E + A + P + O + R), apart from scalar/payload work and under expected constant-time source lookup. Applying a structured template still charges its required member pairs per occurrence; sharing its evaluation does not eliminate those semantic checks. Sparse/open or embedded spans, expression-level listSpecs, nominal/contextual meaning, qualified list matching and general later-link arrivals remain outside this slice.

### D03H Closed embedded spans preserve occurrence structure

**Implementation increment, 2026-10-11.** A finite `&{...}` contributes its logical members to its enclosing ordered list; an ordinary nested `{...}` contributes one member. Checked sequences retain both the physical member frame and the occurrence's embedding flag. The flag cannot live on the cached frame: one infonView may occur both as a span and as an ordinary member. Counted length sums embedded lengths, with exact overflow checks before materialization. Unknown or mismatching embedded endpoints remain partial; open/sparse and non-list embeddings remain outside this increment.

For correspondence, the left cursor retains physical wrappers and the right cursor emits logical members. A left embedded wrapper gets a projected frame whose children continue the same right cursor. Completion returns that wrapper to its parent's sequence. Empty wrappers consume no right member. The logical source cursor skips a checked zero-length sequence as a unit, including shared trees of empty wrappers; physical target traversal still retains those wrappers. This uses the existing ready stack and merge operation; ordinary nested lists create their own correspondence as before. Parent listSpecs use the same traversal, with one checked template in place of the consuming right cursor. A wrapper's own requirements have already been checked before an ancestor template is projected through it. Repeated parent correspondence does not reapply either template.

**Reason and alternatives:** flattening both sides would lose target occurrence structure and require another mapping pass to recover it. Materializing a slice for each embedded wrapper would copy deep prefixes repeatedly. Retaining physical left structure and sharing the right continuation requires neither. Publication keeps the existing target wrappers, POVs, views and links. Adopted/concatenated results receive independent physical occurrences using the same sequence representation; their source frames remain immutable. This is a finite deterministic placement rule, not a planner or a rule for ambiguous spans.

**Continuation and cost:** parser waits retain the same structural and template frames; cancellation publishes nothing. For the prior D03G quantities, add C embedded wrapper projections and S visited sequence-description nodes: work/storage remain O(V + E + A + P + C + S + O + R), excluding scalar/payload costs and assuming expected constant-time source lookup. A deep span does not multiply right-side scans by depth. Each applicable ancestor template still incurs its actual logical-member applications and wrapper visits; distinct constraints are not silently skipped. General sparse alignment, later requested-link arrivals, progressive prefix publication and shared target constraint unification remain later work.

### D03I Finite endpoint evidence closes a trailing open span

**Implementation increment, 2026-10-11.** An ordered literal with one implicit trailing open span exposes only its existing physical prefix. Its checked facts keep an explicit open-tail bit separate from value, size, represented prefix length and member completion. No absent count-derived size is inferred from an open prefix. Existing members can normalize while the containing list remains partial, without inventing a future-link subscription. A given endpoint equal to the checked prefix length proves the tail empty; this also permits an explicitly zero-size empty list without evaluating an unused template.

A strict correspondence from such a target to a closed source with a known consistent endpoint has a unique trailing alignment. First apply the existing positional pairs; after the left cursor ends, continue the same right cursor through the remaining actual members. Those checked members supply independent destination occurrences. Apply the target's checked listSpec to each new member through the ordinary derived relationship; the prefix's prior applications are reused. Empty remainders, closed embedded prefixes/sources, nested ordinary target lists, computed members and multiple constraints all use the same operations. A mismatching endpoint or overlong target prefix stays partial before correspondence writes. An explicitly unsized source remains partial for this operation rather than treating its physical prefix length as supplied endpoint evidence.

**Reason and alternatives:** the right endpoint and fixed prefix remove alignment ambiguity, so this is a deterministic extension of the existing cursor, not a new planner or an alternative-matching engine. Enumerating a declared extent would create speculative members and make large virtual spans expensive. Tail members are visited only when the finite source provides them. A known finite size without a source does not request expansion. Middle/embedded open spans and open-source correspondence still require the later chart/alternative design; their unsupported boundary is explicit. Open concatenation and copying an unresolved open list also remain unsupported rather than losing future member requirements.

Publication prepares new suffix POVs in a detached list, checks the entire destination scope, and only then attaches them. Existing prefix bodies, POVs and views are retained; the previous last member gains its new next link at commit. Failed or unsupported requests and unrepresentable new members attach nothing, including when a different target child fails a late shared-view check. Source/template infons remain unchanged. A closed result can discharge a fully represented template; an open tail retains it for unmaterialized members.

**Required parser correction:** the old end-of-list cleanup treated any final unsized embedded open list as if it were the bare `...` token. Both the preserved engine and the first replacement build parse `{&{9 ...}}` and `{&{5| ...}}` as `{ ... }`, losing the prefix or template before reasoning. The parser now returns whether the actual member token was ellipsis and folds only that token. Explicit embedded wrappers and their work remain in the model; unsupported embedded alignment cannot silently become an unconstrained successful tail match. This is token provenance local to parsing, not a new persistent solver flag. Marked-selector behavior and bare ellipsis folding retain their existing conditions.

**Continuation and cost:** the existing parser-completion wait retains template and source-member frames; no prefix or goal restarts. General later link arrivals and progressive stream-prefix publication are not implemented here. Add T checked source members used to extend tails to D03H's work quantities; each is consumed once per correspondence, plus its necessary template applications and output allocation. No term depends on a numeric declared extent alone. The `tailMembers` counter measures supplied member operations, not path prices. Finite ordered alignment remains linear in inspected structure and produced output, apart from scalar/payload costs and copied residual constraints. Ambiguous sparse alignment has no such general bound.

### D03J Extent equations bound uniquely determined embedded correspondence

**Implementation increment, 2026-10-11.** The D03I trailing remainder becomes a bounded operation usable inside embedded spans. A checked sequence retains its represented member count separately from its minimum required width, whether it contains unresolved spans, and a count of free widths saturated at two. An ordinary member contributes one. An embedded list with a supplied endpoint contributes that exact width; without one it contributes its checked minimum and free widths. An implicit tail contributes one free nonnegative width. These summaries compose as members and concatenation branches are already visited; there is no extra planning traversal or path pricing.

For a supplied finite source endpoint N and target minimum M, the remainder is N - M. With no free widths it must be zero. One free width takes the remainder. Several free widths are uniquely determined only when the remainder is zero, making each zero. Negative remainders or inconsistent given endpoints stay partial before writes. Otherwise multiple partitions remain partial with their original constraints intact. Known embedded endpoints bound their own internal equations: a containing width can be unique while that child's internal division remains unresolved. The enclosing correspondence then stops at that child, because subsequent source positions have not been consumed. Value-based disambiguation and correlated OR alternatives remain the later chart stage.

Each derived embedded frame uses the same source cursor, its proven width and its remainder; it does not construct a source slice or search from the beginning. Existing members consume their positions, and the D03I tail operation consumes only its assigned remainder, stopping before the following sibling. Nested embedded frames preserve their wrappers and occurrence identities. Local listSpecs apply to newly supplied members through the existing derived relationship. A source with an explicitly unknown endpoint still cannot close a sparse target. Unresolved source layouts, ancestor templates across unresolved embedded spans, open concatenation and copying unresolved layouts remain explicitly unsupported; those constraints cannot be silently discarded.

**Reason and alternatives:** nonnegative extent addition proves these correspondences without choosing among alternatives. This is an admission premise for the same propagation operation, not a replacement for the agreed Earley-like general alignment process. Trying partitions here would require branch-correlated scalar facts and publication; selecting the first successful partition or treating equal node pairs as equivalent would be unsound. A full copied layout/plan and repeated equation scans are unnecessary for this fragment. A numeric endpoint by itself never materializes members, including exact extents larger than signed 64-bit counts.

**Continuation and cost:** parser waits retain checked frames, summaries and relationship cursors; arrival continues locally and cancellation publishes nothing. Existing detached suffix preparation validates all destinations before writes. With the D03H/D03I work quantities, each inspected sequence contribution updates a constant-size summary and each projected wrapper adds constant scheduling work. Overall structural work stays linear in inspected descriptions, projections and supplied output, plus exact-number bit costs, scalar/payload work and retained constraint copies. No bound depends on a declared sparse width alone; ambiguous matching has no claimed linear bound. This increment does not implement progressive stream-prefix publication, later requested-link arrivals or anchored partial publication around an ambiguous gap.

### D03K Open spans retain each scoped member requirement

**Implementation increment, 2026-10-11.** An implicit open tail keeps a request-local persistent chain of checked listSpec requirements. Each link carries its originating edge and checked template frame, plus a reference to the earlier chain. A local listSpec contributes one link; each applicable ancestor projection appends its own link in constant time. Sharing a template value does not erase distinct scoped contributions. These are continuation records over the virtual model, not copied templates, new model occurrences, or a second adjacency graph.

Projecting an ancestor template through an embedded list uses the existing member traversal. Existing logical members receive that new contribution once; already checked local requirements are not replayed. On reaching an open tail, the projection retains the requirement without creating members. When a finite source later supplies a member during the same request, a small cursor visits the retained chain in its original local-to-ancestor order and derives one ordinary member relationship per requirement. Each derived relationship uses the existing merge, prerequisite stack, nested correspondence, and failure behavior. The shared source cursor still stops at the span's proved width. Ordinary nested lists remain one member; their structured requirements are merged at that level.

**Reason and alternatives:** replacing a span's local listSpec with an ancestor listSpec loses constraints. Eagerly merging templates with each other can also reject a valid empty span: `{5| &{6| ...}} = {}` has no member required to equal both values. Separate retained requirements let zero-member spans close without such a comparison. Copying all accumulated requirements at every embedded wrapper is unnecessary; persistent links share the prior chain. An already projected prefix keeps its checked facts when parent correspondence proceeds, so the chain is traversed only for newly supplied tail members.

Open results keep the original listSpecs in their original model scopes; they are discharged only when their finite member contributions are represented. Partial publication can therefore preserve useful prefix facts and future requirements. Unrepresentable new member work, a rejected requirement, or unsupported required semantics cannot leak prepared suffix nodes. A parser wait retains the checked template/prefix frames; arrival continues that request and cancellation publishes nothing. A separate caller-requested normalization after adding finite evidence is a new request and does not reset an old handle.

**Limits and cost:** this does not add a general conjunction solver. For example, an exclusion applied to an unknown member can remain partial before a later positive requirement is tried; all original constraints remain, and no result is invented. Open-source correspondence, ambiguous extent alternatives, contextual/nominal templates, and general later-link delivery remain unfinished. Let Q count visits to retained requirement links and A count actual derived member applications; both are measured separately. Extend the prior structural accounting by Q and the constant-sized retained links. A single ancestor across deep wrappers remains linear in visited wrappers and output. The inherited projection traversal still visits intermediate wrappers for each applicable ancestor; many nested ancestors can therefore require quadratic wrapper work even when few members are supplied. Persistent requirement sharing removes chain copying, not this pre-existing traversal cost. Optimizing that propagation is a separate justified follow-up; no general linear bound or path-cost planning is claimed.

### D03L Batch ancestor projections before their facts are consumed

**Implementation increment, 2026-10-11.** A structural embedded-list occurrence can carry pending listSpec projections instead of executing a separate descendant walk for every ancestor. Checked template frames and their originating edges remain ordered requirements. A persistent binary join combines earlier local requirements with later ancestor requirements in constant time; a retained cursor visits the joins and leaves when a real member needs them. This replaces the D03K backward chain and per-member reversal, and serves both represented members and future tail members.

An occurrence-specific projected frame records unapplied requirements, and a sequence summary reports whether any contained frame has runnable pending requirements. Structural containment transports those records. Before the list's own wrkList, an ordinary nested member, a source relationship, a template, an expression operand, or the requested result consumes their facts, the normal correspondence stack settles the pending projections in one walk. A current list frame continues that walk directly; a borrowed source gets a derived frame. A source cache entry with pending requirements is structural evidence, not proof that those requirements are satisfied. Pending work cannot be used to declare completion or bypass publication checks.

**Reason and alternatives:** D03K shared accumulated requirements but still traversed intermediate wrappers for every ancestor. Carrying requirements through structural containment removes that repeated traversal. Pushing all outer constraints into a child before its own identities would change the established operation order; the boundary before each list's wrkList preserves that order. Copying accumulated requirement vectors at each wrapper would replace traversal with another quadratic operation. Joining immutable requirement references avoids both, and deferring projections does not add a second evaluator or search strategy. Local leaf merging, correspondence, failure handling and publication remain shared.

Within each member the existing local-to-ancestor order remains. Requirements do not flow into sibling scopes or get merged with one another in place of checking actual members. Empty spans therefore keep their existing semantics. A partial member retains its original constraints and is not repeatedly rescheduled; this increment does not solve the partial-conjunction limitation. A parser wait retains the pending projections and read cursors, and cancellation publishes nothing. Source and template frames are never specialized for one consuming occurrence.

**Cost and limits:** for one batch, structural work is O(W + A), with W the inspected sequence/wrapper descriptions and A the required member applications, plus actual nested correspondence, payload work and retained-constraint copies. Joining scopes adds constant work per projection. The requirement cursor charges every join and leaf visit to `requirementVisits`, including applications to an existing prefix; the earlier counter only charged the tail chain. Empty or huge symbolic tails create no members. Explicit intervening identities remain separate semantic operations; the bound does not erase their required checks or claim global linearity for arbitrary inference, multiple source uses, or ambiguity. Planning and path costs remain deferred.

### D03M Discharge represented list facts without demanding final member values

**Implementation clarification, 2026-10-11.** A checked closed list source can be fully represented in the target even when some source member values are unknown. After strict identity adoption or correspondence, the source's known shape, types, sizes, values and represented constraints have already contributed to the target. Completion of every source member is not an additional requirement for removing that relationship. For example, `{7 8} = ({_} {_})` becomes `{7 8}`, and `{_ _} = ({_} {2})` becomes `{_ 2}` while remaining partial.

Use the existing representability summary, together with a supplied value, known endpoint and closed layout. The list merge already validates the endpoint. If member correspondence cannot represent all obligations in the target, its existing completion path retains the RHS; the staged publisher likewise refuses unresolved adoption. Pending comparisons, unresolved expressions and unsupported qualifications cannot be discarded by this rule. Explicit unknown sizes and open sources remain outside this discharge extension. Source expressions and members stay unchanged. A corresponding list records whether its source has other retained meaning, then makes exactly one retention decision after member work returns. It does not append the same source both before and after correspondence.

**Reason and alternatives:** the previous test used semantic completion of the source, so it unnecessarily retained expression syntax containing ordinary unknowns, numeric ranges or negative scalar members even after all their facts had been accounted for. A second descendant scan or a printer exception would duplicate reasoning. Reusing the existing summary removes the redundant edge with no new traversal, evaluator, cache or planning machinery. Unknown information remains in the target as unknown information; it is not upgraded to a completed value.

**Streaming and cost:** no relation is published away until the current required source and member work have settled. Parser arrival resumes the existing source frame and cancellation keeps the relationship. A partial repeated request still examines its stored members, but no longer rereads and remerges a discharged expression source. First-pass correspondence retains its existing linear bound in represented members and sequence nodes, plus payload and applicable template work.

### D03N Adopt open structure without inventing correspondence

**Implemented increment, 2026-10-11.** A strict identity may copy a representable open list into a target that has neither a value nor a size. This copies the actual finite member structure, any known total extent, local listSpecs and symbolic tails. The result remains partial while its layout or member facts remain incomplete. An existing valued list or size-constrained target still requires alignment against an open source and stays outside this increment.

**Reason and alternatives:** an unconstrained destination has no competing shape to align. Requiring a closed source unnecessarily blocks a direct structural copy; expanding an unknown or enormous tail would invent work and members. Reuse the existing adoption and detached publication operations, retaining the source's checked facts and future-member requirements. No special-case enumerator, second solver or planner is needed.

**Representation and continuation:** the adopted frame carries the checked local listSpec and retained tail requirements so later finite identities in the same request use the existing correspondence path. New list bodies retain their applicable listSpec, including nested scopes, so a fresh later request has the same obligations after transient frames are released. Single-operand list expressions use the same metadata transfer. Proven discharged listSpecs stay discharged. Represented adoption with no outstanding source work can discharge the source edge even with an open or explicitly unknown endpoint, because the target now retains that endpoint and its requirements. Each destination has independent member POVs and list bodies; immutable template payloads may be shared. Source bodies and parent links remain unchanged. Parser waits continue at the pending source frame, and cancellation publishes nothing.

**Cost and boundary:** adoption traverses represented prefixes and wrappers, plus the existing cost of their constraints; it does not depend on the number of unrepresented future members. Given symbolic extents use the existing arbitrary-precision number representation. Existing ordered, context-free, strict-identity limits remain. Multiple open-source alignment, open concatenation, inverse member expressions, nominal definitions and general later-link subscriptions are not introduced. A later explicit target request is a new goal, not a reset of its previous partial handle.

### D03O A closed target bounds a strict open source

**Implemented increment, 2026-10-11.** When a target is a represented closed ordered list with a known endpoint equal to its logical member count, that endpoint also bounds a strict identity source. Project the open source against a checked snapshot of the target using the existing correspondence operation. Then merge that checked projection back into the target's original structure. The source's templates, prefix facts and uniquely determined span widths participate through the same existing operations. Both the source and target evidence are checked without changing borrowed source POVs or replaying their wrkLists.

**Reason and alternatives:** strict list identity is symmetric, while publication remains directed to the requested target. The already supported finite-endpoint rule can therefore check the reverse premise without adding a second matcher, copying source slices or introducing path planning. Distinct endpoint bindings get independent derived frames; the source cache retains only its original facts. A target with an open layout, explicitly unknown endpoint or inconsistent represented count supplies no such proof. Qualified relationships keep their existing unsupported boundary.

**Partial results and limits:** an overlong prefix, inconsistent supplied endpoint or ambiguous positive split stays partial. Existing source-comparison and inverse-expression barriers still apply; no unchecked member value is published through them. Existing target occurrences are retained, including embedded wrappers, and compatible independent member facts can still publish. Applicable future templates on new ordinary open members survive in the model for a later request. The printer retains its existing generated-label notation for repeated shared template views; model-level tests check those templates and their later finite closure. General two-open-list alignment, alternative bindings and planning remain deferred. The old `spans/reverseTail` registration also asserts mutation of the source itself and therefore stays pending with that assertion intact.

**Empty-source theorem and measured motivation:** a checked zero endpoint and the existing nonnegative-width extent test prove that every participating span has width zero. Use the checked empty target facts as the source's projected contribution, rather than unfold its possible occurrence paths. The initial projection otherwise created 38/136/522/2060 frames for shared source depths 4/6/8/10. This shortcut uses cached extent and representability evidence; it cannot hide an existing member, nonzero given endpoint, malformed embedded extent or unfinished source prerequisite. It introduces no cost estimate or planner.

**Streaming and cost:** input waits remain at the exact source prerequisite; cancellation and contradiction publish no target values. Ordinary projection adds at most another traversal of the involved represented sequences and derived member constraints, plus their scalar/template work. Count source and target wrapper projections explicitly; no general linear bound is claimed for arbitrary ambiguous inference. For a shared source constrained empty, source reading follows its stored views/edges and the extent proof does not unfold the exponential paths. The bounded-source regressions measure wide lists, deep wrappers, independent endpoint bindings and this shared empty graph.

**Validation:** both CodeDog targets regenerate successfully. All 20 WorldManager suites pass, including 508 bounded-source assertions; the full retained run adds no failures (1582/1882). The 378-case capture corpus changes only 38 previously unsupported results. The implementation adds 23 production lines. Detailed counts, historical-test boundaries and evidence locations are recorded in the consolidated design.

### D03P Single-gap open lists use anchored correspondence

**Implemented increment, 2026-10-11.** For two checked ordered lists with one unbounded gap apiece, expose the represented prefix and suffix around each gap. The starts align with each other, as do the ends. The longest prefix and longest suffix describe a common symbolic remainder only if every shorter overlap is impossible. Try those finite overlaps through the ordinary strict member merge; only a proved contradiction rules out an overlap. If a shorter overlap remains possible or undecided, keep the unsupported boundary rather than select a branch. This is a correspondence premise, not path-cost planning.

The layout scan borrows checked member frames, skips proved empty source spans and keeps signed anchor counts. Once the premise holds, the existing correspondence continuation visits the target's physical members and wrappers. Established prefix/suffix pairs use the same derived relationships as closed lists. Incoming represented members enter the target's existing gap; an incoming suffix can move its symbolic tail into a new embedded remainder. Its width stays unknown. The operation never enumerates an extent or fills an intermediate source infon. Different occurrences have independent anchor positions while sharing source facts.

**Requirements and partial publication:** both inputs' pending ancestor requirements must be settled before their facts can justify alignment. Existing member requirements remain checked once. Newly supplied target members receive the target gap's requirements; existing target members covered by the source gap receive that gap's requirements. Requirements on future members remain on the continuation. If source requirements cannot yet be represented in the stored target alone, keep the original relationship and mark that contribution unrepresented, so containing correspondence retains it too. A later finite identity uses the existing closure operation and checks those requirements. A fresh request can likewise close the stored partial result without restarting the old handle. Contradictions and unsupported alignments publish nothing; target member/parent/sibling identities and source contents are preserved.

**Why this rule:** `{1 ... 5} = {_ 2 ... 4 _}` has no prefix/suffix overlap ambiguity; the middle can receive `2 ... 4`. In `{1 ... 6} = {... 4 5 6}`, the shorter three-member placement would equate `1` and `4`, which the existing scalar merge rejects. The surviving family has a symbolic middle ending in `4 5`. No alternative matcher or scalar comparison implementation is added.

**Ambiguous fixture:** the preserved `merge/dots/align4b` input `{1 4 ... 5 _} = {... 4 5 6}` admits both `{1 4 5 6}` and `{1 4 4 5 6}`. The first needs an empty target gap; the second needs a one-member gap. Its historical expected `{1 4 &{4 5 6 ... } 5 6}` excludes these valid models. The old Proteus4 executable also fails that expectation, returning `{1 4 ... 5 6} = {... 4 5 6}`. The user subsequently confirmed that the historical expected result was incorrect. The fixture now expects `{1 4 ... 5 6} = {... 4 5 6}`: the final member is necessarily `6`, while the retained RHS preserves the unresolved alignment constraint. The fixture input and registration are unchanged, and the correction is recorded alongside the updated fixture hash. The current engine still reports unsupported, so this remains a failing functionality test. The user then proposed an explicit alternative form such as `{1 *_+[4 &{4 ... 4}] 5 6}`, and chose to postpone this case because other representations are possible. The intended alternatives are a single shared `4`, or distinct boundary `4`s with a symbolic span between them. No canonical representation or algorithm is chosen. The retained-RHS expectation currently in the fixture is a provisional partial result, not the settled normalization contract. Keep the test registered and failing, and revisit its expectation with the general ambiguous-correspondence/alternative design.

**Streaming and cost:** parser waits remain on the exact source member or template, before checked alignment. Arrival resumes the saved cursor, and cancellation publishes nothing. This supports normalization of a parsed finite prefix with an open symbolic remainder; it does not implement progressive publication from an unfinished parser stream. Work is O(V + E + S + P + C + O + R), plus actual member/requirement payload work: source views/edges V/E, inspected sequence nodes S, derived member pairs P, shorter-overlap comparisons C, output O and retained constraints R. Each compared pair of anchored positions is visited at most once in one overlap check operation; C can be quadratic in the represented boundary lengths. With no overlapping anchors, correspondence is linear in represented members and wrappers. The unrepresented gap width contributes no enumeration. Several gaps, given endpoints on an open layout, a viable shorter overlap, unresolved source work, and qualified list matching retain explicit boundaries.

**Validation:** the regenerated engine passes all 21 WorldManager suites, including 226/226 new behavioral checks. An independent finite-word recognizer checks 512 input-pattern pairs against all binary words through length six: 331 accepted, 117 rejected, 64 explicitly unsupported, and 56,896 comparisons with no counterexample. Five of the six original fixtures at this boundary now pass unchanged; `align4b` remains unsupported for the reason above. The full run has no new failures or changed remaining failure text. With 32/64/128 members on each side, alignment uses 76/140/268 frames and six sequence-node visits; 32/64/128 nested source wrappers use 43/75/139 frames and 37/69/133 visits. These measurements support the stated unambiguous bounds, not a general linear ambiguity claim. Evidence is in `/tmp/proteus-open-source-alignment/`.

### D03Q A member goal requests RHSs through enclosing correspondence

**Accepted requirement and first implemented slice, 2026-10-11.** The user clarified the next primitive: given a list relationship `A R B`, where `R` is `=`, `==`, `=:` or `==:`, a normalization goal on an occurrence `A.m` can request the corresponding part of B as an RHS for that member. The goal need not first normalize all of A or B. An ancestor relationship projects through the intervening occurrences in the same way. This makes D03A/C directly usable from a member goal and supplies the common mechanism needed by intersections.

**Correspondence has its own completion condition.** The user distinguished this from the old engine, which would completely normalize `A=B`. The enclosing relationship is an input to finding the corresponding occurrence, not a request to finish normalizing that relationship. Establish only the layout, values and matching premises needed for the requested correspondence, then return it. A correspondence can be established while A, B and their enclosing relationship remain partially normalized. The member goal separately requests whatever facts it needs from the projected RHS. Do not use whole-list completion or fully normalized source frames as a blanket prerequisite, mark the parent relationship complete, or discharge unrelated parent work merely because the member's correspondence is known. Verification should include a fresh member with no cached correspondence, a member satisfied by its own work without a parent lookup, and a settled member correspondence alongside unresolved unrelated members, with no normalization or publication of those unrelated members.

**Normal entry is a request for more RHSs.** The member first tries its available wrkList contributions using its retained cursor and attempt outcomes. If those supply the requested facts, it finishes without looking for an enclosing correspondence. Otherwise, after trying the current supplied work, it asks the virtual graph for additional RHSs. One such source is the parent: inspect its list relationships, find a corresponding list, and establish the correspondence needed for this member. Exhaustion means each current contribution has been tried, not that the stored wrkList is empty; meaningful unresolved entries and required waits remain tracked.

Expect this lookup usually to start without any previously found correspondence. Reusing a correspondence from whole-list/span propagation is an optional optimization, not a prerequisite or the primary entry path. The user emphasized that tracking how far the parent has merged must not cost more than the work it saves. Start without a separate persistent parent-progress map: use an already available propagation cursor or derived relationship when cheap, otherwise find the requested correspondence. Add broader reuse bookkeeping only when measured repeated work justifies its lookup, maintenance and memory costs; correctness must not depend on that cache. Retain the cursors and outcomes needed by an active RHS discovery or correspondence attempt, so a newly derived RHS continues the same member frame without replaying its supplied work. This local continuation state is distinct from an optional history of parent merges. If the parent itself needs an ancestor route to discover a corresponding list, discover that route without turning it into a whole-parent normalization request. This fixes the ordering of supplied work versus additional-RHS discovery without selecting a global priority among the other type/listSpec/navigation sources.

When asked for more RHSs, the virtual graph discovers the applicable enclosing relationship and advances a targeted correspondence attempt using required layout and matching evidence. A result identifies the source POV or span, occurrence context, the applicable derived member relationship, and the premises that justify it. Preserve the enclosing relation's qualifications and direction; projection must apply their semantics rather than silently turn every result into strict equality or blindly copy the operator to the child. Qualified or type-changing relationships can require different member interpretations.

Feed justified projected RHSs into the existing member-solving path, alongside the member's own supplied work, type and applicable listSpecs. Requests remain scoped: inspect siblings or source structure when needed to establish placement or validate the route, without publishing or normalizing unrelated members merely because they belong to the same list. Retain which parent relation and scoped template contributions were used, so co-propagation and targeted lookup do not apply them twice. Source views remain immutable and writes belong to the requested occurrence.

A correspondence can yield a proved member/span, correlated alternatives, a local wait, or exhausted/no correspondence. An unresolved placement is not a proved mismatch. Preserve tentative bindings until their required matching conditions hold; an intersection result must not be accepted from a branch whose necessary pattern constraints remain unchecked. Parser or alignment evidence resumes the saved attempt at its missing premise. No recursive whole-parent normalize, fresh scan on each wakeup, or whole-goal restart is introduced.

**Intersection composition:** the intersection exposes its selected body occurrence (last, marked, indexed or otherwise specified) as a target with its containing list relationship. That member requests the projected RHS through the same correspondence operation and ordinary solver. Selection and matching qualifications still supply their own premises; this decision does not declare the remaining D04 semantics implemented. It establishes the operation on which the `defaultTests.tst` list/intersection cases should build, rather than independent implementations for each selection spelling.

**Pre-implementation evidence:** `WorldManager.dog` already used `derivePair` during whole-list correspondence and listSpec application, but `read` rejected contextual targets and qualified list relationships had a separate unsupported boundary. Whole-list normalization therefore demonstrated only one direction of access. The chosen increment was direct finite member projection through the same solving primitives, using the `getLastItem` and related primary fixtures to drive it. All four parent operators retain their projection premises; unsupported qualification semantics stay explicit. `align4b`'s alternative output representation remains deferred.

**Primary acceptance through intersections.** The user directed testing this capability with the existing intersection cases in `defaultTests.tst`. Use those language fixtures as the acceptance tests; additional internal assertions are for work scope, source immutability and continuation, not a substitute for language behavior. `getLastItem/simple`, `embedded` and `list` establish selection of the right occurrence; the six `getLastItem/*ID*` cases exercise constraints on the selected result. `query/first` and `query/getNth` cover position selection. `query/firstStrB` and `query/firstStrC` exercise the key enclosing-correspondence route: their selected `$` has no own wrkList contributions and obtains `'Cat'` through its containing pattern's input. The latter also distinguishes a nested ordinary list from an embedded span. Marked/lookahead cases later check that required matching premises are respected without normalizing unrelated members.

**Baseline before implementation, 2026-10-11:** all nine `getLastItem` cases and both `query/firstStrB/C` cases failed at the unsupported intersection reader check; the reference engine passes both `firstStr` fixtures unchanged. A read-only generated-library client confirms the parsed intersection has an outer selection wrapper, a literal `innr` body, the `<~` input in that body's `funcArgs`, and no supplied RHS on the selected `$`. Stored raw members do not themselves have parent POV pointers; the virtual access path must preserve the enclosing occurrence context when exposing the target. The full replacement baseline remains 1574 failures / 1883 tests. No engine or fixture changes accompany this test-selection audit. Evidence is in `/tmp/proteus-intersection-projection/`.

**Cost:** a single-member request may inspect the represented route to that member and its required matching premises. Known extents permit justified span skips; existing indexes may be used with their construction cost accounted for. Whole-list or span goals share propagation and do not launch a new full lookup for every member. Measure inspected positions, pair attempts, requirement applications, publications and local resumptions. Unknown virtual extent is not an instruction to enumerate it; arbitrary alignment has no assumed linear bound. Strategic planning remains deferred.

**Implemented boundary.** `WorldPosition` is a borrowed occurrence path and `WorldWalk` is a request-local cursor over represented positions. Ordinary sublists occupy one position; embedded lists contribute their members; an open tail is one symbolic event. This access path differs from the existing cursor over already-solved member frames precisely so that it does not require those frames first. The final projected RHS returns through the existing scalar/list merge and publication path. Neither path constructs another model graph or stores persistent parent progress.

After its own work, a member applies its immediate/scoped embedded-ancestor listSpecs once, then requests a parent relationship only if the demanded facts remain missing. An explicit size demand is preserved; the ordinary known numeric value still needs no size. Parent endpoints must permit the requested identity/prefix correspondence. Derived member edges remove the parent's top-only `:` qualification and inherit its deeper loose mode, following the reference relationship semantics. Direct finite parent projection covers all four operators for the supported scalar member semantics. Crossing ordinary ancestor-list boundaries or projecting a contextual RHS still needs further occurrence projection; whole-list qualified matching is not thereby implemented.

An intersection exposes its last, marked, first or positive-indexed represented occurrence. Its `<~` input becomes a virtual parent prefix relation. This relation is a required matching premise even when the selected member itself is known; needed suffix predicates must hold before publication. Simple scalar predicates reuse the existing merge/comparison logic. Repeated prefixes require a scalar predicate proved disjoint from the following predicate. Each consumed candidate must then satisfy the existing scalar comparator; a merely compatible merge cannot choose a repetition count. Type evidence can accept/reject an ordinary nested list without normalizing its contents. If a computed candidate has no directly available type, request its existing normalization path. Overlapping alternatives, repeated range predicates and structured list predicates remain unsupported rather than treating compatible headers as a match.

The selected body's result is published only when complete and represented. Its wrapper removal survives the existing listSpec and correspondence projections. Borrowed bodies, inputs and intermediate predicates are untouched. Parser waits retain the current parent/header/candidate and both structural cursors; arrival continues after already tried member work. Cancellation publishes nothing. Active-context and active-layout sets prevent circular proofs, and are discarded with the request; they are not cross-request correspondence caches.

For a finite represented route with bounded-cost scalar checks, structural traversal is linear in the positions actually visited, with active path storage proportional to nesting. A deterministic repeated scalar prefix takes one initial disjointness check and a constant number of comparisons per candidate. The new counters distinguish structural visits, correspondence checks and parent requests. No known-extent index/skip optimization or arbitrary-alignment complexity guarantee is claimed. Planning and `align4b`'s ambiguous representation remain deferred.

### D03R Overlapping scalar patterns retain a streaming correspondence chart

**Implemented increment, 2026-10-11.** Replace D03Q's disjoint-exit restriction with one matcher for fixed predicates, repeated scalar predicates and their overlapping exits. A repeat exposes both a zero-width exit and a consuming transition. Rejection of a pair comparison eliminates that path, not other viable correspondences. `marked/lookahead` therefore keeps each numeric candidate until the following string establishes `11`. The existing scalar merge/comparator still decides each match; consuming a repetition still requires proof, not merely a compatible substitution. No new scalar semantics or path costs are introduced.

**Attempt identity and packing.** A request-local column contains one state per pattern position at the current source position. Its capture identifies the selected source occurrence and retains any already-normalized facts. Captures made in the same source column share an evidence cell, so later work needed by a competing predicate is reusable by the eventual winner. States reaching the same remaining predicates share their future tests: merge their captures, recording whether distinct selected occurrences remain possible. A fixed selected position is visited at most once along a path, and the supported predicates have no bindings to an earlier capture. Under those premises, an ambiguity bit plus a representative capture is sufficient to prevent guessing; this is not the general binding/oneOf representation. Different source occurrences remain ambiguous even when their printed values happen to agree.

The frontier is sorted by pattern position. Transitions stay at a repeat or advance one position, so equal destinations are adjacent and can be combined without a search/index. Before reading the current source, take all zero-width exits and combine their destinations. Only then compare the consuming states. Each pattern/source pair is evaluated once, and a rejected suffix never rescans an earlier prefix. Patterns and ordinary source values are borrowed; only the requested result is published. The existing direct asserted correspondence continues to use positional traversal and shares final RHS delivery.

**Completion is about the requested occurrence.** A complete pattern match proves its capture. If every still-live path has already captured that same occurrence, none can produce a different answer: finish without consuming optional suffixes. Thus `[<1> &{_|...}]<~{1 rot}` returns `1` without evaluating `rot`, and a trailing symbolic source tail need not be traversed for the same reason. Distinct accepted captures remain partial with the original expression intact. Missing evidence for a needed comparison, or an unresolved source span with a potentially different capture, also retains a partial result. Complete exhaustion after contradictions rejects; exhausting represented input before a required match retains the existing partial-input behavior. No canonical alternative output is chosen.

**Streaming and cost.** The live column, closure/consumption phase, exact predicate/candidate and next-source cursor survive parser waits. Arrival resumes the blocked pair; previous pair checks are not repeated. Cancellation publishes nothing. With P represented pattern positions, N visited source positions and C live consuming pairs, chart work is O(P + N + C), plus actual predicate, value and occurrence-context work. C is at most O(PN); fixed patterns are linear in visited input, while fixed layouts without repetition have only one path. The chart and capture references use O(P) auxiliary live storage plus structural cursor depth; separately retained normalized facts retain their ordinary solver costs. Consecutive repetitions do not enumerate combinations of repetition lengths. This does not promise linear arbitrary inference or cross-request reuse.

**Limits and alternatives.** Greedy or backtracking selection could publish the wrong occurrence or replay prefixes; this chart avoids both. It also replaces the earlier disjoint-only matcher rather than adding a parallel special case. General structured/range repetition, dependent bindings, qualified whole-list lookahead (`merge/lookahead`), arbitrary open-source alignment and `align4b`'s alternative representation remain separate work. An unresolved needed comparison currently stops with its explicit partial/unsupported boundary; continuing all other branches around such a non-parser uncertainty needs later obligation handling. Planning remains deferred.

**Validation:** `marked/lookahead` passes with its original expected `11`. All 23 WorldManager suites pass; the new suite has 9,556 assertions, including 4,732 exhaustive pattern/input cases and parser-continuation/cancellation checks. An independent generated-library client passes 13 checks. For 32/64/128 numeric inputs, a fixed three-position pattern uses 43/75/139 structural visits, 98/194/386 comparisons and six value frames. The full retained run improves from 1557/1884 to 1556/1885, with one old failure resolved and no remaining failure text changed. Protected fixtures and prior registrations are intact. Full evidence and the explicit remaining boundaries are recorded in the consolidated design's implementation status and `/tmp/proteus-overlapping-patterns/`.

### D03S Whole-list prefix matching retains member and span assignments

**Implemented increment, 2026-10-11.** Reuse D03R's chart for represented ordered-list `=:` relationships. Compile the target's already-checked member frames and tail requirements into its predicates. Each successful transition retains the source occurrence/evidence cell and the pattern slot it supplies, with a shared predecessor. Combining paths at the same remaining predicates retains a choice between their mappings. This adds no second matcher, path-cost policy or new scalar comparison semantics.

**What constitutes a unique result.** A member lookup can finish when all successful paths select the same occurrence, even if other span assignments differ. A whole-list goal needs every member/span assignment to be unique, including the matched prefix endpoint. Consequently `{&{_|...} &{_|...} _ $} =: {1 2 'x'}` remains partial even though its corresponding member selection returns `2`. Distinct mappings stay correlated in shared choice records while the request is live; an ambiguous completed search retains the original expression and publishes nothing. This is not yet a canonical oneOf output representation, and does not decide `align4b`.

**Apply proved bounds through existing operations.** For a unique mapping, count contributions to each repeated tail and calculate enclosing extents from the innermost projected wrapper outward. Each physical embedded occurrence has its own derived frame. Materialize only the matched source members, reusing facts already requested by the chart. Feed this bounded target and source prefix to ordinary list correspondence, derived member merges, scoped listSpec application and staged publication. The existing target POVs are preserved and the borrowed source stays unchanged. The outer top-only prefix qualification is discharged by the proved mapping; derived members use the existing member-relationship qualification rule. `==`, `==:` and forced replacement are not silently converted to this case.

Incoming represented positions are walked once. The source cursor carries the tightest enclosing known endpoint in flattened coordinates, so an embedded source cannot supply a member beyond a declared size or silently shift its following siblings across a missing extent. This check concerns only demanded traversal; a proved prefix does not require evaluating the rest of the source. A source with additional unresolved layout relationships remains unsupported, and a given extent constraining an open target pattern remains deferred. Ordinary closed target wrappers and given source bounds are supported. Symbolic source tails stay partial without invented positions. Required source/listSpec evidence is checked; general structured predicates and non-parser comparison obligations remain the existing boundaries.

**Continuation and cost.** The same chart phase, predicate/candidate and records survive parser waits. Source-layout prerequisites are inspected after its header is ready, including relationships supplied by the arriving parse; checking its earlier empty wrkList is not evidence of completed input. Materialization retains its remaining accepted records if a value read waits. Cancellation publishes nothing. No prefix is matched again on arrival. Whole-list bindings require more information than a selected occurrence: with P pattern positions/wrappers, N source positions, and C successful/live pairs, shared mapping storage is O(P + N + C), rather than copying each complete path or enumerating alternatives. Chart work is O(P + N + C) plus scalar/context semantics; checking R inherited requirements adds their actual comparison work. Applying one accepted mapping is O(P + M), where M is its prefix length, plus existing value/merge/publication work. C can be O(PN), while fixed-pattern input growth remains linear. No persistent parent-progress map or planning stage is introduced.

**Validation contract.** `defaultTests.tst:merge/lookahead` must produce its unchanged `{&{33 22} 11 'hi'}`. The new mapping suite independently enumerates complete assignments, checks source immutability, inherited requirements, zero-width repeats, distinct span alternatives, parser arrivals/cancellation, known source endpoints and work counts. Two native assertions that explicitly described the former unsupported boundary are updated: `{_} =: {1}` now completes as `{1}`, and `{_ _} =: {5| ...}` stops partial at an unrepresented source span. All protected language fixtures and registrations remain intact. Final measurements are recorded in the consolidated design and `/tmp/proteus-whole-list-lookahead/`.

### D03 next evidence: partial correspondence

**Investigation at D03O, 2026-10-11; no planning decision.** This snapshot identified three separate problems. D03P above now handles the parsed incoming-prefix example in item 2; the other examples remain pending:

1. `{*10+_ ... ... *10+_} = {*10+1 *10+2 *10+3 *10+4}` returns partial with zero publications. `fitExtent` detects multiple positive width assignments before correspondence starts, so even the forced endpoint members remain unknown. The retained `spans/ambiguousGaps` test requires those endpoint values while preserving both symbolic gaps. Its zero-remainder counterpart already completes through the existing extent proof. The next candidate is retaining unresolved alignment while propagating only bindings that are forced independently of that ambiguity; choosing one gap split is not justified.
2. `*4+{*10+_ ... *10+_} = {*10+1 ... *10+4}` and `{...} = {*10+1 *10+2 ...}` still report unsupported. The first has a known outer endpoint but two open layouts; the second has a finite incoming prefix without an endpoint. These need separate premises from a represented closed target. Knowing a total size must not enumerate missing members, and knowing a prefix must not close a tail.
3. `{_ _} = {!5| ...} = {7 8}` stays partial, whereas supplying `{7 8}` first completes. The projected negative source waits on member facts; `consumePrerequisite` then stops at that unresolved source. A future continuation must retain that local obligation and expose later useful evidence without treating its unchecked facts as accepted or reconstructing the original source walk. This is not solved by changing parenthesis syntax or adding a global restart.

The shared-library probes are in `/tmp/proteus-retained-boundaries/next-probe.log`. They establish current behavior and the next examples, not a claim that the missing chart, link-arrival or partial-publication contracts are settled. Strategic planning and path costs remain deferred until definitions and intersections.

### D03 proposal for the virtual graph

**Proposed, not a final interface.** Expose navigation, assertions, specifications, and transformations through a common operation protocol. A parameterized operation can require several premises and can return facts, a derived relationship, alternatives, or a wait. It need not allocate a persistent edge merely to make it traversable.

| Relationship or operation | What it contributes |
| --- | --- |
| `=`, `==`, `=:`, `==:` | An asserted relationship with its existing matching and qualification semantics. |
| `parent`, `firstChild`, `prevItem`, `nextItem` | Structural navigation; these links do not themselves assert identity. |
| `memberAt(position)` or `spanAt(start, extent)` | Targeted navigation using layout, known extents, or an index, with subscriptions when placement is unknown. |
| `projectMember(correspondence, position)` | A qualified member relationship derived from an enclosing correspondence and established placement or nominal matching. |
| `meaning(type)` and `projectListSpec(scope, member)` | Applicable interpretation/template requirements with their original context and projection. |
| `correspondingMemberInTPrev(member)` | A temporal correspondence route retaining occurrence boundaries and preservation guards, not whole-occurrence identity. |
| Joins, numeric forming, and other transforms | Operations with explicit inputs, prerequisites, and results; potentially several premises rather than a single unrestricted identity edge. |

For example, `a --parent--> A`, an asserted relationship between A and B, and proved correspondence of the positions or nominal roles of a and b can establish `a --derived correspondence--> b`. The direct derived route can then be reused. Parent navigation alone cannot establish it. Get/set permissions follow the resulting semantic relationship, not the mere existence of a navigable path.

A proposed propagation record carries the correspondence, active positions or residual spans, context/bindings, and references to the parent/template contributions already covered. Member work reuses that record. This is operational progress over the existing virtual model, not a second stored adjacency graph.

For a chart, the proposed identity of an attempt includes the operation/rule, participating occurrence positions and offsets, relation qualifications, context, and binding/remaining-constraint state. Two equal node pointers are insufficient when different offsets or bindings can yield different results. Equivalent recognition states can share work while retaining separate derivations. The precise equivalence and indexes still need examples and proof.

### D03 Streaming and complexity checks

For finite fixed-width, unambiguous alignment with constant-cost member checks, shared propagation should be proportional to the total inspected members/spans plus produced output. Repeatedly searching a source of m members for each of n target members can instead take O(n*m). Charge index construction and maintenance when comparing indexed access; do not assume an index is free.

Sparse work should depend on inspected compressed spans and demanded occurrences, not logical extent, unless the requested output requires that extent to be materialized. A known length permits a positional skip only when the relevant layout, units, and relationship semantics support it. Unknown extents can introduce matching alternatives or local waits.

Trying each attempt once does not imply linear matching. Different positions, residual widths, bindings, and alternative combinations can create many distinct attempts. Measure unique states, emitted transitions/alternatives, local wakeups, matching cost, queue/index overhead, and output. Pending input advances the existing chart; it does not rebuild previously processed prefixes. New edge or premise arrivals must join relevant existing states in either arrival order.

Inspecting the LHS template first may exclude impossible alignments cheaply. Record this as a candidate pruning or planning opportunity for D01 after intersections and definitions have been considered; do not select a general planning phase yet.

### D03 cases still to work through

Use an unordered target with useful immediate members and deep internal structure; one member selected through an ancestor ident; a whole span using that same ident; a parent listSpec already applied during propagation; distinct contexts sharing a template; an embedded span skipped by known length; unknown span extents; a late prefix; two different bindings for the same node pair; and three simultaneously constrained lists. Verify partial results preserve correlated alternatives, and that unrelated publications do not repeat completed matching attempts.

Specify how the cheap deterministic case uses the shared correspondence machinery, how attempts and AND/OR records are indexed without duplicating semantic storage, and when a derived edge can be retained or consolidated. These are open implementation choices, not reasons to restore goal restart.

## D04 Intersections

**Basis accepted: D03Q; remaining design open.** An intersection requests its selected body member through enclosing list correspondence. Specify the remaining relationships among the body/pattern, inputs, chosen or alternative matches, result, and constraints on that result. Explain how leaf tests, list traversal, listSpecs, sparse spans, and partial matching compose. Keep source occurrences and their evaluation contexts identifiable.

**Alternatives:** destructive rewriting; retained selection relationships; indexed matching; backtracking; shared matching subproblems. Decide where each is legitimate and how alternatives retain correlated facts rather than mixing facts from different matches.

**Streaming and cost:** state when a prefix or alternative may publish, when exhaustion is proved, and what happens if an earlier ordered candidate is unresolved. Charge candidate search, branching and repeated correspondence explicitly. Include a scalar predicate, list pattern with a selected member, repeated output, negative or end-relative position, and a constraint arriving on an already selected result.

For D01, retain a concrete case if the ordinary forward strategy fails or does avoidable search. Explain the failed assumption and a possible better strategy without choosing a planner yet.

### D04A Finite scalar alternatives reuse ordinary relationship solving

**Implemented 2026-10-11; broader intersection design remains open.** The primary fixtures `merge/num/literalSelect`, `merge/str/literalSelect`, `merge/intersect/onRHS`, `altSelect/simple` and `innr/oneOf` describe a scalar alternative domain, `*_+[...]`. Enumerate a finite domain with the existing borrowed-position cursor. Each candidate uses the ordinary scalar frame, including its own conjunctive work and enclosing listSpec. An unconstrained open body supplied by `<~` takes its alternatives from the finite input domain. Patterned input domains require a separate correlation proof and remain unsupported.

Keep an OR record of checked branch frames in the request facts. Distribute each relationship over those branches using `derivePair`, the same operation used by list correspondence. An RHS OR supplies candidate values under the original relationship; constraints on a target OR descend through the existing member-relationship qualification. Thus `3 = *_+[2 3 4]` leaves `3`, while `*_+[...] <~ {'ARON' 'AARON' 'ERIN'} == 'AARONDacs'` applies the inherited string comparison to each target candidate and leaves `'AARON'`. Two ORs retain each viable conjunction, never independent mixtures of facts from separate matches. Later relationships filter the survivors, without revisiting eliminated branches.

A proved rejection returns to the dependent frame. Required dependencies propagate it; the OR collector alone discards a rejected candidate. Unknown values, pending comparisons, parser waits and unsupported operations are not rejection proofs. A finite domain with no survivors rejects atomically. One survivor supplies its facts. Several provably equal complete scalar results may also collapse, using symmetric comparison and equal size knowledge; distinct or unresolved results stay as alternatives. This equality rule concerns scalar answers, not D03R's occurrence-specific capture identity.

Publication uses the existing staged graph boundary and new destination occurrences. A partial OR is stored with an `emIntersection` outer value and a literal, unknown-size, `fromGiven` inner list. Borrowed candidate/source objects remain unchanged. A representable unknown or numeric range remains a possible answer. Pending checks that cannot yet be represented prevent publication rather than being discarded. A fully determined scalar answer uses ordinary completion, including D02F's unknown numeric size; repeated normalization creates no goals. Direct graph comparisons that cannot distribute an OR report their boundary instead of treating an OR as an unconstrained unknown.

**Streaming:** wait before classifying a domain whose header is still being parsed. Retain the exact structural cursor, candidate frame, source-work index and pair indices. Arrival continues there; it does not clear the goal or reread earlier candidates. A symbolic open domain is not a finite parser wait and does not justify publishing its represented prefix as the entire answer. The shared cursor also checks declared endpoints, including embedded spans: reaching the end of represented members does not prove exhaustion if a given extent disagrees. Arriving layout relationships remain required even if none existed when the wait began. Cancellation publishes nothing.

**Cost:** for N finite scalar candidates and a single scalar constraint, candidate visits and relationship pairs are O(N), plus the ordinary work within each candidate and comparisons of its payload. Two domains of N and M candidates may require O(NM) pairs and surviving conjunction records. Further constraints operate on the survivors. Equality consolidation compares each result with the first once, avoiding a quadratic duplicate search. Nested OR flattening is charged for each copied branch reference; this milestone makes no global linear bound for nested/branching expressions. Model allocation occurs only for the represented output; request frames are not intermediate model copies.

**Alternatives and limits:** merely removing the selection guard would select the last member and lose the other possible answers. Destructively copying outer constraints into every model candidate would mutate shared sources and duplicate intermediate model state. The chosen path retains request-local facts and reuses existing merging/publication. This initial finite-domain path evaluates candidate frames before applying an external constraint; pushing target demand into enumeration, early existential success, and sharing more nested OR structure remain optimization opportunities, not a new planner or path-cost policy. Structured alternatives, OR distribution inside expressions/listSpecs/chart predicates, arbitrary input patterns, unresolved layout relationships and general symbolic tails remain explicit boundaries. `align4b` is still deferred; this scalar OR representation does not decide its ambiguous span representation.

**Validation:** the five original fixtures pass unchanged. The new suite and a separately linked shared-library client both pass 672 assertions, including a 225-case finite-set oracle and exact continuation/work counts. All 25 WorldManager suites pass; the full run is 1550 failures / 1887 tests, with no new failures or changed remaining failure text. Protected fixtures and all previous registrations are preserved. The consolidated design records the complete results; evidence is in `/tmp/proteus-oneof/`.

### D04B Open return facts and fixed-width selected correspondence

**Implemented increment, 2026-10-11.** Seven primary fixtures stopped at “Selected position needs open-span selection.” Six supply a string or list to an unrestricted `[...]`; the seventh, `slice/select`, marks an embedded four-member span after a three-member prefix. These need distinct proofs, followed by the existing merge/publication operations.

**Open result:** a plain, unknown-extent open body with no members, listSpec, input or body work imposes no facts on its result. Initialize neutral facts and consume the outer relationships normally. Do not create a speculative member in the body. Preserve operator qualification: an unconstrained target can adopt a list under `==`, while `=:`/`==:` do not infer its size. Known contents with unknown size can publish as a representable partial result. No supplied value means no selection publication. Bodies with other information remain on their existing semantic path, including D04A's one-of classification.

**Bounded span:** the virtual cursor exposes a requested embedded wrapper rather than flattening away its identity. A span request obtains its known-width source window as one list RHS. Request-local cursors traverse the pattern/source once, and source members use ordinary contextual frames. Final span filling reuses list correspondence, requirements and atomic publication. No intermediate source model copies, whole-parent normalization or lookup from the start for each selected member are needed.

An asserted parent relationship needs coordinates, not sibling values. An input pattern also needs acceptance. This deterministic path can skip an empty unconstrained open span of known width; it must not skip a constrained pattern as if its predicates had passed. Required suffixes are visited before publication. More complex bounded pattern predicates are reported as a boundary until integrated with the existing chart; they are not handled by a second predicate evaluator. Enclosing target listSpec projection is also explicit remaining work. The selected span's own requirements and source ancestor listSpecs already use normal merging.

A counted source wrapper, or an empty given open wrapper, can prove a skip width. Apply inherited endpoint limits even when skipping, retain exact occurrence paths and reject cyclic layouts. An unresolved source layout relationship is not proof of its coordinates. Read source headers after parser completion before deciding this, so an arriving relationship cannot be silently dropped.

**Verification findings:** a direct span behind one ordinary member exposed the native `FlexNum(int)` constructor, which sets decimal formatting rather than numeric value. Unit-width progress now uses the existing arithmetic increment operation, and direct asserted spans plus neutral input-prefix/suffix cases cover it. The older internal `? == {1 ...}` unsupported-boundary assertion now expects the supported partial adoption; protected language fixtures are unchanged.

**Alternatives and justification:** merely deleting the open-tail guard would choose the preceding represented member, and continuing to flatten a marked span loses the requested object. Expanding every virtual position, or normalizing the entire parent, creates work unrelated to the goal. The selected operation instead uses a structural width proof and produces the same kind of RHS the ordinary solver already consumes. Supporting all constrained bounded patterns would require integrating widths/captures with the chart; that is a larger subsequent increment, not a reason to special-case this fixture's numbers.

**Streaming and cost:** both structural cursors, the outstanding width and the selected candidate survive local waits. Cancellation publishes nothing. For fixed depth, traversal is O(V + K), with V represented layout inspected and K selected member evaluation, plus merge/payload/output work. Nested occurrence comparisons add depth-dependent costs. No cost depends solely on the numeric extent of a skipped span. The 10-position and billion-position prefix cases each use five frames and eleven layout visits. Planning, path costs and `align4b` remain deferred.

**Validation:** all seven original fixtures pass unchanged. The new suite and separately linked shared-library client both pass 425 assertions, including 70 offset/length oracle cases. All 26 WorldManager suites pass. The full retained run improves from 1550 failures / 1887 tests to 1543 / 1888, with no new failures or changed remaining failure messages. All protected fixture hashes and previous registrations are preserved. Evidence is in `/tmp/proteus-open-selection/`.

## D05 Definitions and composition

**Status: open.** Specify declaration lookup, meaning, contextual binding, nominal identity, instance constraints, and copying/materialization. Distinguish compiling a declaration from proving that an instance satisfies it. Explain how a definition contributes ordinary wrkList/list/intersection obligations and how those constructs can in turn supply its missing premises.

**Alternatives:** copy a body on every use; share immutable compiled meaning with instance-specific relationships; expose virtual definition routes as demanded. Decide where actual instance data must be allocated and when a proved correspondence can be reused or applied in a batch.

**Streaming and cost:** cover absent and late declarations, partially supplied bodies, recursive definitions, aliases, and ambiguous meanings. Separate one-time declaration processing from per-instance work and changed-binding validation. Work through a direct record and an equivalent typed record, partial typed descriptions, shared definitions with distinct instance contexts, and an intersection inside a definition. Use solved state1 as the larger composition case.

Reconcile late declaration/context events with D02C: an unchanged completed infon does not create goals on ordinary normalize; a semantic change must explicitly request a new goal. Specify which definition events constitute such a trigger instead of relying on blanket revalidation at every submission.

For D01, record where definition expansion or its interaction with intersections makes the obvious strategy inadequate. Separate work required by the requested output from work caused by a poor strategy.

### D05A Named meanings and nominal query predicates

**Increment, 2026-10-11.** Register parsed declarations in a WorldManager-owned catalogue. The catalogue indexes shared `WordDefn`/body references; it is not a second model graph or a restored legacy normalizer. The Agent supplies the request's locale. Reuse the existing word normalization and language-compatibility rules. An explicit model ID filters candidates; otherwise a fresh use needs exactly one applicable declaration. Keep overload/context/morphology selection unsupported rather than choosing the first meaning.

A declaration does not normalize its body. It is staged in its request, visible to subsequent work there, and committed only after publication preparation succeeds. Failed and cancelled requests leave the catalogue unchanged. Repeated requests recognize the same declaration identity. The catalogue belongs to one WorldManager and is shared by its Agent's sessions; cross-Agent knowledge/deduction sharing is not introduced. Existing bound uses retain their declaration pointer when further declarations appear. Later declaration delivery, automatic rebinding, and semantic-change triggers remain open.

A demanded named value requests its meaning once, after the supplied relationships, as a borrowed `trueByDefinition` RHS. Existing expression, scalar, intersection and list operations process that RHS. No intermediate definition instance is copied into the stored graph. Publication copies only the requested result and records its binding and applied meaning. Per-request source-frame reuse evaluates a shared context-free body once while keeping each occurrence's constraints separate. A declaration is an operation, not a value that can be copied by list adoption or selection.

Nominal predicates compare bound declaration identities; equality of scalar payloads or list structure cannot establish a named type. Explicit undeclared labels remain usable on supplied literal values, but a missing named value has no invented meaning. Bound/unbound uncertainty is not proof of mismatch. A pure type predicate can use an already supplied type header, just as the existing numeric/string predicates do. A differing name proves rejection only when both meanings are plain untyped scalar leaves with no other routes that could supply nominal inheritance. An alias or complex meaning remains unresolved until its type relationship is proved; the shortcut must not skip a possible subtype. Ordered search still stops at its first proved matching position, and an unresolved earlier candidate is not treated as a rejection. A selected value then follows ordinary normalization to obtain its needed content. Definition-dependent record correspondence, nominal inheritance/conversions and arithmetic with nominal operands remain explicit boundaries.

For a bare, unconstrained named list in a positional query, the virtual structural cursor borrows its meaning's list layout. It preserves the occurrence path, stops at the requested position, and checks only the demanded member. It does not normalize unused prefix/suffix values or allocate a copied list instance. Layout recursion uses the existing active-path check, with named wrappers removed when their level finishes; a later occurrence is not a cycle. Constraints on a named layout still require a separate correspondence proof and are not bypassed by this shortcut.

**Why this choice:** both routes expose the same declaration meaning. Whole-value normalization needs its requested contents; positional selection needs a represented layout and one member. Reusing ordinary relationships and the existing structural walk avoids a dedicated named-list solver or a copied definition on every query. Registering during parsing or directly while solving would leak declarations from abandoned requests; staging uses the existing publication boundary.

**Streaming and cost:** pending meaning bodies and selected layout members use the existing local input continuation. Prior frames and cursors remain intact; there is no reset. Catalogue lookup costs the applicable word's bucket, with expected constant-time index access; overload buckets are not assumed constant-sized. Bare layout queries cost visited wrappers and positions, plus the selected member's reasoning, without evaluating skipped member values. Reusing a body removes its repeated evaluation, not the per-occurrence merge or requested output cost. Pure nominal search performs one header proof per tested pattern/source pair; the existing chart's ambiguity bounds still apply. Planning and path costs remain deferred.

**Validation:** the original `query/getNthSub`, `query/firstType`, `word/deref` and `word/useTags` fixtures now pass. The new named-query suite passes 299 checks through both generated targets; all 27 WorldManager suites pass. The full retained run is 1539 failures / 1889 tests, with four resolved old failures and no newly failing names. One existing failure, `lang/splitText`, now reaches an unported intrinsic transformation and reports an output mismatch rather than generic unsupported status. This remains a compatibility gap with its original expectation intact. The consolidated status records that distinction, work measurements, protected-file checks and the next dot/record-query boundary.

## D06 Rule representation and normal forms

**Status: open.** Specify both theorem/transformation rules and temporal rules. For a general rule, identify premises, conclusions, bindings, direction, side conditions, and any justified equivalence or inverse. Decide what a rule's normal form exposes to the searcher and which representations may remain virtual. Define how rule declarations themselves enter the graph.

For temporal situations, build a matrix covering an unchanged value, explicit change, unknown later value, `%?`, indirect/model-implied change, nested parts, membership change, an interior cut, a sparse history, and pending input. For each, show source notation, graph relationships, normal form, permitted deductions, and unresolved premises. Distinguish temporal occurrence, value, support, lifetime, structural position, and coordinate.

**Alternatives:** desugar into a small common relation vocabulary; retain higher-level conditional relations; canonicalize equivalent descriptions; preserve several proved-equivalent views. Determine what information canonicalization must retain. A representation choice must not equate successive occurrences merely because their values agree.

**Streaming and cost:** define useful partial normal forms. Charge rule expansion and representation growth; avoid Cartesian expansion of alternatives or enumeration of sparse histories. Compare the graph needed to express a relationship with the additional graph demanded by a particular query.

## D07 Rule search and application

**Status: open.** Given D06, explain how the solver discovers applicable rules and relevant situations. Specify forward, backward, inverse, and equality routes; candidate indexing; unification or correspondence; guards; cycle control; and how proofs remain searchable. Define the relationship between semantic precedence and route cost.

**Alternatives:** scan all rules; index by conclusion/relation/type; goal-directed search; change-driven propagation; a composition of these. Compare completeness for the supported fragment, latency, fairness, and memory. Define whether termination is guaranteed for a fragment or controlled through a resource policy.

**Streaming and cost:** late premises continue affected applications at their waiting links, without clearing their goals. Exceptional semantic changes require explicitly new goals under D02C. A temporal default needs evidence that applicable supplied constraints are settled; queue emptiness alone is not that evidence. Resolve how later contradictory information interacts with committed values. Charge matching, affected dependency closure, and rule firings, including nontermination risks.

**Worked cases:** state2 with one missing zloc; `%?`; an indirect cause of change; an unresolved containing constraint; competing derivations; a conditional shortcut awaiting a premise and a distinct semantic change that triggers a new goal; and a cyclic rule set. Show why irrelevant state1 color work does or does not enter the zloc proof.

## D08 Other constructs as composition checks

**Status: open, after D01–D07.** Inventory the intended `%rules`, including relative/contextual references and any distinct rule forms that need coverage. Cover concatenation, arithmetic, functions, source injection, selectors, and reply/effect boundaries as relevant to the agreed core.

For each, express the graph contribution and execute it with the common loop. A new primitive needs a semantic reason; a difficult fixture alone is not a reason for another scheduler. Concatenation must account for units, widths, embeddings, sparse segments, and open ends. Relative references must preserve the correct occurrence and owner context.

**Streaming and cost:** ask when a completed concatenated prefix or function result is available, which later inputs affect it, and whether work is proportional to inspected spans and produced output. Include an unfinished concatenated operand, a relative reference inside a copied definition, and a streamed reply. Compare lowering to existing relations with retaining a specialized relation and shared leaf algorithm.

## D09 Optimization justification

**Status: open within the accepted D02 lifecycle.** Establish what a theorem must prove to justify skipping, batching, reordering, caching, or indexing work. Keep semantic equivalence separate from an implementation cost model. Preserve contextual bindings, alternatives, qualified relationships, contradictions, and observable effects.

Use the audit's proposed shortcuts as candidates: reuse a proved fact, reuse a common obligation across callers, retain a count after a payload change, transfer corresponding parts in a batch, skip currently represented RHS work, and publish a locally ready guarded result.

**Streaming and cost:** each shortcut states the knowledge required to use it, its maintenance overhead, and any semantic change that must request a new goal. Ordinary arrival of more information resumes local work under D02C. An optimization can be worse on small inputs or frequent updates; compare those workloads too. Demonstrate one new semantic rule and one justified shortcut by changing the graph while leaving the generic solving loop unchanged. This is the test of the intended architecture, not merely of code organization.

## D10 Integrated design and retirement

**Status: open.** Trace fully expressed state1, definition-derived state1, and state2 with one missing value through the selected design. Add ordinary, whole, overlapping, cancelled, and streaming requests. State all remaining obligations at each stop; measure first useful result as well as final completion.

Compare against the audit's reproducible examples and cube baseline. Count helper work, queue churn, subscriptions, support repair, completion checks, allocations, and memory, not only visible trace events. Test scaling over finite fixed-schema records, large sparse spans, and relevant versus unrelated updates.

Validate that a differently implemented `WorldManager.dog` can satisfy the reasoning interface without changing its callers. For experiments made in unpacked generated C++, validate the accepted behavior again after it is ported back to CodeDog and regenerated. Record build-time measurements separately from normalization measurements.

Map every proposed runtime responsibility to the current code it replaces. A staged implementation must name retirement gates and obsolete code; retaining two permanent ways to solve the same obligations requires an explicit decision and justification. The current audit is the candidate deletion map, not an already chosen implementation plan.

## P01 Implementation sequence

**Status: working implementation sequence, accepted to begin on 2026-10-11.** The [implementation roadmap](normalization-design.md#proposed-implementation-roadmap) proposes small working increments behind WorldManager. It leaves D04–D09 choices open and does not authorize an unreviewed semantic resolution merely because a stage needs it.

**Order (updated by P02):** preserve the reference/tests (done); establish the interface and replacement test target; implement scalar acquisition; cut over the normal Proteus Engine/test path and retire its obsolete dependencies; prove local streaming continuation and ownership within sessions; add deterministic then general list correspondence; specify and implement intersections, then definitions; reconsider planning; add theorem/temporal rules; complete composition and the graph-only extension exercise; integrate the application and remove obsolete runtime code. Streaming and cost checks accompany each stage, rather than being postponed to integration.

**Reason:** the first executable scalar request tests the claimed boundary and lifecycle with little machinery. Lists then exercise shared traversal, contribution tracking and alternatives before intersections and definitions compose them. Their actual difficult cases supply the evidence for planning. State2's single missing value follows a working definition-derived state1 and explicit temporal guard semantics. This makes both semantic progress and code retirement reviewable.

**Current source evidence:** Agent owns and exposes GraphNormalizer in `Proteus.Lib.dog`; ThreadedNormalizer uses its input lifecycle; `infonIO.dog` uses GraphPublication/InputPause; GraphFoundation binds the old native runtime; and Slipstream calls graph read/update and known-type operations. Consequently, a replaceable WorldManager needs to own publication and safe access as well as normalize. These are boundary responsibilities, not a requirement to preserve the old method names or status representation.

**Alternatives considered:** designing the whole language engine before running a scalar would delay evidence that the framework is sufficient. Building a wrapper that delegates unfinished cases to the old solver would conceal unsupported features and retain duplicate ownership. Repairing each existing cache or restart mechanism conflicts with the fresh replacement decision. A separate reference executable and explicit replacement coverage permit incremental work without any of these dependencies.

**Test policy:** the previous pruning run exercised the checkpoint engine, not the replacement. Preserve language fixtures and port behavioral assertions through WorldManager as their constructs arrive. Old pass counts and internal API expectations do not settle semantics. D02F/D03 changes, other intentional differences, known failures and unresolved expectations must be distinguished explicitly.

**Retirement and cost:** each increment identifies which old responsibility has moved; delete its obsolete implementation once no retained caller uses it. The historical audit is a candidate map, not a mandate to restore its proof/epoch architecture. Measure actual work and net production-source changes. Local continuation, finite fixed-width list scaling and bounded sparse demand are early acceptance properties; arbitrary ambiguity and inference have no general linear bound.

**First proposed implementation milestone:** stages 1–2, giving a CodeDog WorldManager, a clearly identified replacement test path and verified scalar acquisition/completion. Follow with local streaming before adding complex constructs. The first scalar/session implementation and its validation are now recorded in the consolidated design; this plan entry does not settle the remaining semantics.

## P02 Move the replacement into the Proteus Engine early

**Accepted direction, 2026-10-11, from the user's clarification.** Make the normal Proteus Engine and TestProteus use WorldManager soon, rather than waiting for the remaining language milestones. The existing Proteus3 and original Proteus4 checkouts provide historical behavior; retaining their execution machinery in the replacement is not necessary for comparison.

**Implementation milestone (now implemented for the scalar and parser-completion slice):** migrate Agent's public normalization handle and convenience methods, engine initialization, and the normal language-test path to WorldManager. Move the parser publication and stream lifetime dependencies behind that boundary, with local continuation as those paths become active. Preserve the `.tst` inputs and expected outputs. Port useful compiled behavior tests away from direct GraphNormalizer calls, including the qualified nested-source case in D02D. Report unsupported semantics explicitly even if their unchanged printed form happens to match an expected result. Do not make the real test path fall back to the old solver.

Delete obsolete runtime code and temporary test scaffolding when their callers are migrated. Do not keep the old solver linked solely to compile tests that call its internals; preserve their useful behavioral assertions through the new interface. The separate scalar harness is an initial validation tool, not a second permanent engine test path. More language coverage follows on the normal test path; full language support and Slipstream integration are not prerequisites for the engine cutover.

**Reason and alternative:** staying in a separate scalar harness until the whole replacement is implemented would delay feedback from the actual language tests and prolong duplicate machinery. An early cutover makes unsupported features visible while the reference folders remain available for comparison.

**Validation and cost:** build TestProteus from CodeDog, identify the replacement engine, run the supported historical scalar cases and report a language-suite baseline with unsupported features separate from semantic regressions. Verify that no old solver runs. Exercise input arrival and cancellation when the stream path migrates. Report removed dependencies and production source as well as added code; cutover must not introduce a second traversal or scheduler.

### P02 implementation evidence, 2026-10-11

The normal Agent/ProteusCore/TestProteus path now uses the CodeDog WorldManager. The initial 335 scalar/session checks have moved into TestProteus, with 14 local input-lifecycle checks and an incomplete-RHS parser regression. GraphFoundation and 44 native graph runtime headers have been removed. Unsupported language features never call the reference engine. All protected language fixtures are unchanged; compiled assertions awaiting a port remain named `NOT_PORTED` failures with exact recovery locations in the migration inventory.

Parser readiness and wait registration use the same model lock. Potentially blocking parser reads release it, and completion resumes only the keyed infon's existing waiting frames. This supplies the minimal active stream contract without introducing another queue or restarting a goal. Native C++ is restricted here to thread/lock bindings and the exception-safe parser lock guard; reasoning remains in CodeDog. Full requested-edge continuation and shared deductions are still pending stage 3.

The old printer consulted solver-specific representation receipts. Removing that dependency makes retained constraints visible; the real language tests expose the remaining consolidation/canonical-printing gaps. Preserve those expectations rather than restoring old receipts or silently accepting the new output. The [implementation status](normalization-design.md#implementation-status-proteus-engine-cutover) records the exact baseline, costs, removed source and unsupported boundaries.

### P02 Retained physical-list and signed-number assertions

**Validation increment, 2026-10-11.** Restore the ten original registrations for nested/embedded lists, inferred nested/embedded counts, lazy billion-member lists, conflicting children, list templates, empty lists, signed parsing, signed replay/copy and expression size modes. Independent clients linked to the generated library first verify that these behaviors already work. The ports retain the original value, size, flag, partial-result and source-preservation assertions; no solver behavior changes.

The obsolete count-Set trace is replaced by checking the inferred literal size after removing the parser's original count evidence. A completed-replay work revision is replaced by zero frames/publications and unchanged payload pointers. The lazy-span work bound counts replacement frames, while still requiring zero materialized members. These checks preserve the observable purpose without recreating old scheduler APIs. General subscriptions, definition-dependent signed values and tests requiring mutation of borrowed sources remain pending rather than weakening their assertions. This applies P02's existing test policy; it introduces no normalization semantic or planning decision.

### P02 Retained string and extent-boundary assertions

**Validation increment, 2026-10-11.** Restore the original `graphFoundation/string`, `copyPrefix` and `fractionalSpanExtent` registrations. The first checks string acquisition and inferred size, the second directly checks the existing storage helper's prefix and metadata preservation, and the third preserves an unknown parent size when its embedded child supplies a fractional extent. A generated shared-library client confirms all three original assertions before porting. Status checks replace only the old handle API; there are no production changes.

The sibling `invertedSpanExtent` case still reports unsupported where its original test requires partial information, so its assertions remain pending. The seven existing reference-registry failures were also traced to unsupported selection requests, each stopping at the virtual graph reader with one frame, no publication and no recorded selection reference. Their failure counts and original assertions are unchanged. This distinguishes missing semantics from a newly introduced list or registry regression.

## Settled direction and retained constraints

### G01 Graph semantics and justified optimizations

**Accepted design objective, reaffirmed 2026-10-10.** New semantics should be represented through changes to the graph, and semantic shortcuts should have theorem-based justification. The reason is composability: new constructs should use the common reasoning system, while skipping work preserves meaning. The precise rule representation, proof mechanism, native primitive boundary, and solver architecture remain open in D02, D06, D07, and D09.

### G02 Decision history and consolidated specification

**Accepted process, 2026-10-10.** Maintain this decision history with reasons and alternatives, and maintain a separate consolidated design. Evaluate streaming, Big-O, and competing designs at every stage. The reason is to make both the current design and the rationale for it reviewable without treating tentative suggestions as settled behavior.

### G03 Replaceable WorldManager reasoning interface

**Accepted architecture boundary, 2026-10-10, from the user's clarification.** Factor the complete reasoning interface through the `WorldManager` class in `WorldManager.dog`, following the replaceability described for Proteus3. A different reasoning implementation can be selected by substituting a differently implemented `WorldManager.dog`; reasoning callers depend on that contract rather than implementation-specific scheduler or graph classes.

**Reason:** preserve the ability to replace the reasoning system as a unit. The internal solving architecture can change without forcing reasoning clients to change with it. This chooses the interface boundary, not the previous engine's algorithms or an exact new method list.

**Alternatives:** exposing the present C++ graph implementation directly to callers is not the intended architecture. Internal helpers remain possible behind WorldManager. The request, publication, result, subscription, and cancellation operations needed by the eventual solver must be expressible through its contract; their exact signatures are still to be specified.

**Streaming and cost:** replacing the reasoner must preserve the chosen stream/request lifecycle at this boundary. Measure any dispatch, conversion, or data-copying overhead; the boundary itself is not a reason to duplicate the model or traverse it again. This is a design decision, not a claim that current code already satisfies the replacement test.

### G04 CodeDog source and temporary C++ experiments

**Accepted implementation policy, 2026-10-10, from the user's clarification.** CodeDog remains the maintained source for the reasoning implementation. Faster iteration or runtime optimization in C++ does not by itself justify maintaining a separate C++ reasoning system and losing CodeDog's other benefits.

Two development options remain available: improve CodeDog or its build workflow when build time is the problem; or unpack generated C++ and work there temporarily until a concrete problem is solved. Bring the accepted solution back into CodeDog source, including generator changes when needed. Regenerate and verify the solved behavior before treating the experiment as integrated.

**Reason and alternatives:** this retains CodeDog's benefits while allowing focused experiments at generated-code speed. Permanently maintaining the solution only in C++ is not the chosen outcome. Which build optimization or experiment layout to use remains a practical decision based on a concrete problem.

**Streaming and cost:** the generated-code experiment must exercise the same relevant streaming behavior as the maintained implementation. Distinguish generator/build time from execution time and memory; a faster build does not establish a faster normalization strategy. This policy does not initiate a source migration or CodeDog optimization during the design discussion.

### G05 Virtual graph between POVs and the solver

**Accepted architecture boundary, reaffirmed 2026-10-10.** The solver reads and writes POVs through an interface presenting a virtual graph. WorldManager remains the complete external reasoning interface (G03); this is the semantic access boundary inside the implementation. Reads expose usable relationships and pending links; writes apply semantic results and consolidate fully merged RHS work. A second materialized copy of the model graph is not required.

**Reason:** separate the storage representation from reasoning routes, permit useful intermediates to be skipped, and support directions beyond hard-coded forward copying. Nesting, qualification, and context remain properties of the exposed relationships. Exact operations are still to be specified; accepting the boundary does not claim every edge is an unqualified identity or every write is a scalar copy.

### G06 Separate branch and worktree for the replacement

**Accepted, 2026-10-11.** Develop the replacement on `codex/normalizer-redesign` in `/home/bruce/devl/Proteus4-redesign`, within the existing Proteus4 repository. Keep the project named Proteus4. Checkpoint `b52322a867d2be2c1fda186929b05dc0c70e8be4` on `codex/proteus4` preserves the prior working state, including source changes, regression tests, this design discussion, and diagnostic evidence. The original checkout remains at `/home/bruce/devl/Proteus4`.

**Reason and alternatives:** a separate worktree keeps the reference implementation available for comparisons while the branch permits replacing the solver and retiring superseded code. A separate repository fork is unnecessary for this continuing project and would complicate shared history and comparisons. Preserve parser, POV storage, printing, and tests where their contracts fit; do not adopt existing solver machinery merely because it is present in the starting checkout.

**Implementation boundary:** the new solver follows G03–G05: the complete reasoning interface is WorldManager, CodeDog is maintained source, and POV access presents a virtual graph. Branch creation does not itself replace the runtime. Exact interfaces and the unresolved semantic cases remain on the existing agenda. The initial branch setup changes only development guidance and design documents; no replacement solver is implemented by it.

**Validation and cost:** both checkouts were verified against a manifest of the checkpoint's file contents, modes, symlinks, and deletions. Saved diagnostic logs are included. Generated builds stay in the original checkout; the replacement uses separate build output and the same external public-world resource link. No normalization complexity or performance change is claimed from repository isolation.

### G07 Agent owns sessions

**Accepted, 2026-10-11, from the user's clarification during implementation.** Agent continues to own and track sessions. WorldManager handles reasoning for the requests belonging to those sessions. A session can contain multiple requests; ending one session releases or cancels its unfinished work without ending another session. Agent shutdown closes its sessions.

**Reason:** replacing the reasoning implementation must not remove the Agent's role in managing interactions and their lifetime. The complete reasoning interface remains WorldManager (G03); session ownership belongs to the caller side of that boundary. The initial implementation puts session registration, ownership checks and closure in Agent, while WorldManager contains the scalar solving loop and per-request cleanup.

**Implementation and limits:** Agent now routes both its ordinary normalization entry point and explicit sessions through WorldManager. TestProteus exercises this normal engine path. Runnable solving is synchronous; parser completion continues waiting requests under the model lock. Requests and cancellations remain separate even when they await the same input. Sharing in-progress deductions remains later work. See the [implementation status](normalization-design.md#implementation-status-proteus-engine-cutover).

### B01 Existing semantic constraints

**Retained baseline, not newly decided by this agenda.** Current [AGENTS.md](/home/bruce/devl/Proteus4/AGENTS.md) and the recorded clarifications in [open-questions.md](open-questions.md) constrain the starting examples. They include nominal typed fields, distinct occurrences and contexts, sparse/open safety, partial definitions, selection precedence, and guarded temporal preservation with `%?`.

The consolidated design records these constraints and their sources. The later D02 decisions govern default stopping and the lifecycle of ordinary information arrival; older descriptions of automatic goal restart do not carry forward. Other baseline constraints can be reconsidered explicitly during design; an architectural convenience or existing implementation gap does not silently change them. Current source is evidence of what the engine does, not by itself a justification of what it should do.

### D03S Shared correspondence requests and consumers

**Accepted direction, 2026-10-12.** The user approved the four implementation steps and cross-route validation proposed in the review. Whole-list normalization, member correspondence, intersection captures, and read/write search must request relationships through one correspondence contract. Multiple algorithms remain appropriate when their premises establish the same semantics.

**Evidence and reason:** the review reproduced a whole-list prefix merge that found a member while normalizing only that member failed; concatenated and named sources worked through some routes but not others; symbolic billion-item skipping worked for a one-item span but not a scalar member. These differences arose at layout and dispatch boundaries, not in the shared scalar merge. The initial request/result separation preserves the existing algorithms so subsequent changes have a clear regression boundary.

**Contract:** preserve relation qualifiers, occurrence ancestry, requested coverage, correlated bindings, and local wait progress. Distinguish acquiring asserted facts from proving a query predicate. Reads project checked facts; writes constrain the proved source occurrence and retain atomic publication. Selection does not establish full normalization of the source. Definitions and listSpecs contribute relationships rather than alternate search implementations.

**Validation:** retain every protected fixture and expectation. Compare whole-list/member/span requests; literal/embedded/concatenated/named layouts; constrained/unconstrained prefixes and suffixes; direct selectors/path syntax; and read/write source identities. Include rejection, unknown evidence, parser continuation, cancellation, replay, and work proportional to visited structure and output. General ambiguity has no assumed linear bound; known-width skips must not enumerate sparse counts. No planning or path costs are introduced.

**D03S implementation evidence: logical layouts and captures.** Member capture is a width-one instance of the existing finite-span traversal. Explicit prefix/suffix constraints use the same predicate check as the chart. A variable-width event transfers the unconsumed cursors and established capture into that chart; the old overlap oracle caught a dropped-capture bug in this transfer, fixed before acceptance. The shared header-entry operation also prevents empty patterns from accepting a source whose list type has not been established. Borrowed definitions and concatenations are exposed structurally rather than normalized wholesale. The original 27 suites plus the new 86-check suite pass; the full retained failure set is unchanged. No costs or speculative planning were added.


**D03S implementation: consumers and publication.** Lower dot/index paths into existing selectors rather than introducing separate read and write searches. A dot predicate is the existing negative-repeat/positive-capture pattern. Position and span syntax use the existing positional proof. Whole-list finite pairing now carries the common correspondence request; the duplicated member-coordinate walker was removed. The algorithms remain distinct only where the available evidence justifies different work.

Source occurrence identity is part of a successful result, not recoverable from equal payloads. In particular, later list refinement must not replace the original capture mapping. Span updates apply the same checked occurrence update as scalar writes. Stage updates in the request and let later steps read them; prepare every publication destination before mutating the model. This supports atomic multi-field/span updates and cancellation without a separate write matcher. Borrowed definition bodies remain read-only. Returning the parent after an edit requests that parent's normal obligations; silently omitting unexpanded definition fields is not an accepted shortcut.

Reference-only intermediate steps avoid evaluating unused record fields. Final acquisition and parent normalization are separate demands. Path-valued layouts expose the selected occurrence to the existing structural cursor and continue at that link. Finite structured predicates use recursive ordinary fact comparison; negative templates invert only proved comparisons, and nominal identity remains independent of structural equality. Unordered/open structures and bounded repeated templates still need additional semantics. A zero-width match must have evidence of the source list type; an expression container alone is not that evidence.


Cross-route validation reproduced two scope/proof gaps. A direct indexed list ignored an unused contradictory suffix, but a path-valued layout evaluated that suffix. Propagating the existing reference-only demand through the path removes that extra normalization; tests vary the unused suffix from 8 to 512 members. Indexed source traversal also lacked the known-width skip used for captures. Both now use the same list-width proof. The width must belong to a proved list layout, not a scalar such as a three-character string; pending prefix/suffix predicates prohibit skipping their comparisons. These are shared proof conditions, not syntax-specific answers.


The full protected suite caught an incorrect one-based assumption in the newly added slice probes. The existing `slice/sugar` fixture and Proteus3 `buildIndexIntersection` agree: items use one-based indices, while a slice index is the number of members before its selected span. Correct the lowering and only the new probes; protected expectations stay unchanged. End-relative coordinates preserve this distinction, including boundary zero. The existing `temporal/sBeginField` fixture also prevents dot lookup from treating `.begin` as an ordinary named field; boundary providers remain unsupported. Scalar slice specifications remain an explicit boundary rather than being treated as embedded lists. A plain untyped list definition has no outer inheritance edge, so the existing nominal-exclusion proof applies to it without equating structurally similar named records.


Navigation keywords also need their semantic premise: the predicate must be bare. Cross-checks reproduced `.first:8` returning the first value 7 and `.last:7` returning the last value 8. A shared `pathOperator` classification now distinguishes bare navigation words from ordinary predicates carrying values or shapes. Such constrained predicates go through the same matcher as every other field; boundary/temporal providers remain explicit unsupported operations. The keyword spelling alone is not a valid optimization theorem.


**D03S acceptance evidence.** The four consolidation steps are implemented and validated in the replacement engine. All 29 WorldManager suites pass; the two new suites' 250 checks also pass in an independent shared-library client. The final generated targets have identical bodies for all 103 graph/manager/request/Agent-entry methods. The full retained inventory recovers 20 original default failures, adds two passing suite registrations, and introduces no newly failing names. All protected fixtures, historical registrations and reference assertions remain unchanged. The consolidated specification records the remaining unsupported semantics and the two default output mismatches; successful compilation or a smaller failure count is not treated as proof that those features are complete.

### D03T Structured span correspondence for type-changing list patterns

**Implemented direction, 2026-10-12.** The eight failing `defaultTests.tst:parse/` fixtures describe one operation: a target member consumes a source span. A string terminal consumes its established byte width; a nested list consumes the spans of its members; a listSpec repeats that same operation; a finite OR supplies alternative templates. Keep these under `WorldCorrespondence` and reuse ordinary scalar matching, projected frames, scoped requirements, logical source layouts and atomic publication. Do not add separate handlers for the named fixtures or a second language parser.

**Qualification and proof.** `==` requires an accepted mapping to end at the source endpoint. `==:` admits prefix mappings. A given list size constrains the number of resulting members, independently of the number of source bytes or members consumed. Ordinary nesting remains in the result; embedded occurrences contribute their member count to their parent. Unbounded repetition that can consume zero input remains partial because it admits arbitrarily many outputs; a given finite count can bound such a repetition. Multiple accepted mappings also remain partial, with no arbitrary branch publication.

**Continuation and alternatives.** Memoize each checked template at each requested input boundary. A list state records its member slot, source boundary, accumulated sequence and proof; its key additionally includes member count only when that count constrains future work. Equivalent continuations share a proof node. A later alternative marks that proof ambiguous, which propagates through accepted parent proofs without replaying transitions. Lists and strings use the same goal composition. This chart handles variable-length nested spans; the existing deterministic correspondence and regular member chart remain useful under their narrower premises.

**Definitions.** A type-changing test of a named unknown must first acquire the available meaning that supplies its template. This is a dependency ordering rule, not strategy planning. An unconstrained named OR domain denotes its alternatives, preserving their own nominal types (`Sat`, `Fri`, `Sun` in `parse/lang`); it does not require each alternative to inherit the domain's name. General nominal inheritance remains unsupported. A listSpec may retain an OR frame, and the span matcher uses the existing alternative members. Ancestor requirements are combined through ordinary derived relationships, sequentially before their result is used.

**Storage and cost.** Source lists use the existing logical cursor, caching only demanded occurrences and their acquired facts. Header inspection determines whether a concatenation supplies lists or text without normalizing unused list suffixes. Text expressions supply borrowed leaves. The first experiment repeatedly sought string windows through an unbalanced rope: 100/200/400 fragments visited 5,249/20,499/80,999 text nodes during publication. A lazy leaf index and direct leaf windows reduce these counts to 100/200/400. Sequential windows retain the current leaf; discontinuous branch access uses binary lookup. No substring copies are needed before publication.

Deterministic fixed-width repetition has linear numbers of input visits, chart states, terminal checks and output nodes; ordered map operations add their lookup costs. General ambiguous nested patterns can have polynomially many boundary combinations and outputs; no general linear bound is asserted. Given extents do not enumerate missing source data. Parser waits preserve the exact source/template dependency and chart, and cancellation publishes nothing.

**Scope and validation.** This increment covers whole-list type-changing correspondence with finite available list/text input. Type-changing member-only selection, recursive grammars, general open-source spans, unknown-width string terminals, negative structured patterns and unresolved template relationships remain explicit boundaries. In particular, normalizing just the first member of `{*3+$ *4+$} == 'CatDogs'` still needs extension of the parent correspondence route; passing the whole-list fixtures does not establish that route. `align4b`, planning and path costs remain deferred. Protected fixture expectations and historical assertions stay unchanged. Three replacement-engine unsupported guards become `{1}` success checks; an unknown qualified source becomes a partial comparison while retaining its no-publication assertion.

**Acceptance:** all 19 parse cases, 30 WorldManager suites and 350 new assertions pass; the independent shared-library client passes the same 350 assertions. The full inventory recovers nine existing failures without adding any or changing remaining diagnostics. The consolidated design records build, integrity and cost evidence. Both targets regenerate from CodeDog and have identical reasoning/text-cursor method bodies.


## D03U Selected structured correspondence (2026-10-12)

**Decision.** A member request against text uses the existing structured span chart and returns its captured frame. Literal template layouts are borrowed; predecessor widths or predicates are demanded as needed. An asserted, uniquely determined capture can finish without requesting its suffix. Patterned intersection input still checks the required predicates and preserves ambiguity. Neither the containing list nor its unrequested siblings is published.

List sources retain the existing positional/pattern correspondence path. Source classification borrows named meanings and concatenation headers rather than value-normalizing a named record merely to discover its layout. Target layout traversal may expose members whose parent has relationships; those relationships remain available to the selected member. Source and alternative-domain traversal still require closed layout relationships.

**Reason.** A list member count and a string byte length are different coordinates; comparing those headers cannot establish a token mapping. Reusing the span chart preserves the same widths, grouping, overlap proofs and local parser continuation used by whole-list parsing. Copying or normalizing the parent would lose the demand boundary and occurrence provenance needed by subsequent path steps.

Selected string projections supply facts, not an editable source occurrence. Text-window writes and chained navigation through synthesized grouped output need an additional projection interface. General selected list regrouping, explicit prefix members with a shared structured listSpec, recursive grammars and unknown-width text terminals remain deferred. Planning and path costs are unchanged.


## D05B Nominal record field correspondence (2026-10-12)

**Decision.** A plain field-valued definition supplies nominal field constraints and omitted immediate fields. Index its unique nominal members, then align each supplied occurrence with its corresponding declaration field. Verify nominal identity with the existing comparator and combine facts with the existing derived-pair merge. Preserve supplied field order; insert omitted declaration fields before the next matching supplied field and append remaining omissions. Given total extents remain constraints. Unmatched fields are admissible only through an open declaration remainder.

The index is a candidate lookup, not an inheritance theorem. Duplicate fields or unproved inherited identity remain unsupported. Counted/listSpec definitions continue through list correspondence; field alignment must not erase their extent or repeated constraints. Unrepresented relationships stop publication rather than being discharged with the meaning.

**Demand scope.** Explicit normalization of a supplied named record requests its immediate omitted fields. Its supplied list-valued children are acquired without recursively requesting their omitted fields. Thus returning `spoke` adds `color`, while returning `wheel` leaves its supplied `spoke:{length:123}` intact. A primitive type explicitly supplied in `left:_` or `color:$` is valid partial information even without a separate definition of that field name; a bare unknown name still needs available meaning.

**Publication.** Prepare field order and new occurrences before any mutation, then relink original supplied POVs at publication. Matching payloads do not authorize replacing those occurrences. Borrowed declarations stay unchanged, parser waits retain the current dependency, and rejected/cancelled requests publish neither edits nor new fields.

**Cost and alternatives.** Ordered indexing and two forward passes cost O((N+D) log(N+D)) key work for N supplied and D declaration fields, plus ordinary field reasoning and output. A nested search for every field would add O(ND) comparisons; positional pairing would conflate different nominal fields. No cost planner or new goal restart is introduced. General inheritance, overloaded/duplicate field resolution, unordered records and definition-instance writes remain separate work.


## D03V Known-endpoint selection and occurrence context (2026-10-12)

**Decision.** Evaluate a computed selector coordinate through an ordinary scalar frame. Keep the literal fast path, require a known nonzero integer, and retain the coordinate frame across waits. For a negative intersection coordinate and a proved source endpoint E, lower E+coordinate to an unconstrained prefix span followed by the original borrowed pattern. The shared `offsetDomain` construction also serves zero-based path slices. Item indexing stays one-based; slice boundaries stay zero-based. Unknown or insufficient endpoints remain partial.

**Reason.** The original negative fixtures encode `(-2)` as a concatenation expression, so reading the size slot's numeric field is not a proof of the coordinate. Reusing scalar evaluation covers that syntax without a minus/parentheses special case. Lowering to ordinary prefix/span correspondence reuses symbolic skipping, nested traversal, predicate checks and capture publication rather than adding a reverse traversal solver.

**Occurrence context.** A generated input POV is a relationship adapter, not a new model occurrence. Correspondence retains an existing `inputContext` when available, including ancestry and whether the source is a borrowed definition. The previous adapter discarded this information for spans: `borrowedNested.part@[-2:*2+{...}]={7 8};` could mutate the shared definition of `borrowedNested`. The corrected common source-domain selection prevents both positive and negative span writes through borrowed definition occurrences. Definition-instance writes remain unsupported.

**Streaming and cost.** Coordinate and endpoint waits continue at their exact dependencies. The offset lowering adds constant structure; traversal costs the visited represented spans and selected output, not the magnitude of a symbolic prefix. Cancellation, unknown endpoints and failed ownership checks publish nothing. No backwards scan, planner, path cost, persistent parent-progress cache or goal restart is introduced.

**Layout precondition.** A supplied representation of a named list is not proof of its declared logical member layout. For example, selecting the second minute from an hour stored as seconds must not return the second stored second. If the named meaning supplies an unapplied listSpec, traversal reports that template correspondence is still required; an unfinished meaning waits at that meaning. This guard applies to positive and negative coordinates and spans. It neither converts units nor matches particular type names.


## D03W Numeric domains as correspondence predicates (2026-10-12)

**Decision.** Evaluate a scalar expression through its ordinary frame, then apply its outer predicate negation. Numeric predicate comparison asks whether the supplied value domain is contained, disjoint or unresolved relative to the required domain. Domain membership does not equate representation endpoints; strict identity retains its separate endpoint checks. Complement only a proved match or nonmatch. A partial overlap never permits skipping an earlier candidate.

**Selection.** A proved negative predicate supplies the selected candidate, not the exclusion itself. Positive range selection keeps the checked target endpoint and the candidate's displayed form; it does not mutate or identify the source occurrence. Dot paths and intersections obtain these facts at the same correspondence boundary. The existing matcher, scalar queue and interval arithmetic supply the operation; no range-specific search loop is added.

**Cost and streaming.** A simple first-match search inspects each visited candidate once and retains its predicate/candidate frames on parser waits. The finite search oracle, rational endpoints, source occurrence identity, cancellation and 16/64/256-member work checks validate that path. General ambiguous patterns and later semantic link arrival remain separate contracts.


## D03X Projected patterns and supplied output prefixes (2026-10-12)

**Decision.** A non-OR intersection in a pattern consumes a matched body span and returns its selected member or marked span. Compose this projection as another goal in the existing hierarchical correspondence chart. The consumed source boundary and returned output are distinct. Prepare and retain the projection body once; memoize its matches at requested source boundaries. Last, first, marked and positive literal indexed selection use the proved body result. A pure repeated indexed body requests only its required prefix; an explicit body keeps its own layout and suffix predicates independently of the return index.

A literal list's `<~` input supplies the chart's source relationship. Atomic predicates inside these projections use the common membership comparator. The resulting list supplies ordinary outer relationships and meanings through the existing frame. There is no separate range-filter loop, recursive normalize call, cost planner or restart mechanism.

**Supplied outputs.** A projected listSpec generates output members. Existing members in that list constrain those outputs; they do not consume raw source positions. Evaluate each supplied output constraint once, retain its current chart state across waits, and compare it through the common matcher. This also makes a published partial projected result usable on another normalization request. Its retained `==` input relationship expresses the remaining obligation without a private parser-only representation.

**Partial results.** If an unbounded projected repetition cannot finish its next match against an available finite source, retain the longest unambiguous proved output prefix, the original template and the original input relationship. `range/select1` therefore remains open after `2 3 4 5` when the trailing source is `6 7`. A given output extent is never dropped to enable that publication. Unresolved predicate overlap, ambiguous mappings, zero-width unbounded repetition and required parser waits publish nothing speculative.

**Symbolic candidates.** Containment proves membership, disjointness proves exclusion, and partial overlap proves neither. Selection preserves the candidate's explicit interval and independently checked output endpoint. The `range/select3` output member of size 4 remains distinct from its source member of size 3. New explicit constraints on an output cannot mutate that source. General semantic link-arrival propagation across requests remains deferred; these checks do not claim automatic updates from later source mutations.

**Ownership and continuation.** Output occurrences are independently published; source values and shared templates remain borrowed. A marked span returns the span itself before traversing its contents. Header-only predicate proofs acquire the selected value through its ordinary frame before publication. Projection-body readiness is checked at the original parser-owned infon before constructing a coordinate-adjusted view. Source, body, predicate-operand and supplied-output waits keep the exact current dependency and prior chart work; cancellation publishes nothing.

**Cost and boundaries.** Simple disjoint-prefix filtering uses one source cursor and linear numbers of frames, checked template/boundary pairs and output nodes. Ordered memo tables add lookup costs. Evaluating supplied output-prefix constraints adds one pass; their comparisons are charged to chart states. General overlapping nested patterns retain the chart's ambiguity and potentially polynomial boundary combinations; no general linear bound is claimed. Computed/negative projection coordinates, projection-local extra obligations, recursive grammars and general unresolved source spans remain explicit boundaries. The previously supported standalone negative selectors are unchanged.

**Acceptance.** All six default range fixtures, all 20 focused numeric-range fixtures and all 36 WorldManager suites pass. The three range suites pass 560 checks in both the regenerated executable and an independent regenerated-library client. Simple filters over 17, 65 and 257 source members use 78/270/1038 frames and 67/235/907 correspondence checks. The full inventory recovers six original failures across D03W and D03X, including both every-other-member fixtures, with no new failing names or changed remaining diagnostics.

## D03Y Environment roots compose with ordinary correspondence (2026-10-12)

**Decision.** Agent supplies its current world view when creating a normalization request. The request retains that root, including across parser waits. `%W` exposes that borrowed occurrence to the existing path selector; it does not normalize the whole world before a field can be read. A correspondence source that is itself a path resolves its occurrence once before inspecting its layout. Its original relation qualification is retained. Declaration entries are not matching value candidates; a field search can pass them without treating an absent value as uncertain evidence.

**Reason and ownership.** World access was previously rejected at the generic scalar boundary. Exposing an environment root and resolving a source reference are graph operations, so field reads, selected intersections and writes should reuse the same occurrence selection and staged publication. A read does not identify the result container with the world or copy into shared source definitions. A conflicting write remains atomic. Unavailable world context stays partial.

**Streaming and cost.** Retain the selected root and the exact candidate wait. Replacing the Agent's world while a request waits does not silently rebind it. An unused unfinished suffix is not a dependency. Root acquisition costs constant graph work; lookup and correspondence retain their established traversal costs. No world cache, whole-world prerequisite or goal restart is introduced. Other environment and temporal cursor providers remain separate work.

## D03Z Literal ordering and named record values (2026-10-12)

**Decision.** Ordered/unordered layout and T-list status are facts carried through ordinary literal reads, list identity, definition application and publication. T-list status propagates through a list identity; this does not by itself implement temporal preservation. An unordered description has an implicit remainder: physical member count does not establish its total size. An explicit total smaller than its known members is a contradiction.

Named record meanings use the existing nominal field correspondence. It matches each supplied field, inserts omitted definition fields, retains supplied order, and carries the definition's unordered layout even when the instance initially contains only one field. An unordered meaning without an explicit total permits additional supplied fields. A closed ordered meaning retains its previous boundary checks. Bare named fields inside a described list can remain partial if no definition is available; a standalone unresolved named-value request keeps its explicit unsupported boundary.

**Named values and limits.** A definition whose body is itself named can supply its checked value to the declared name. Preserve that outer name and the body's layout. This is ordinary meaning application, not a proof that arbitrary differently named predicates are interchangeable. Nominal inheritance comparisons still require additional evidence; sibling types with equal structure remain distinct. General unordered alternative search is separate from this unique-field correspondence.

**Streaming, ownership and cost.** Record members and meanings retain ordinary frames and exact input waits; rejected or cancelled requests publish nothing. Shared meanings remain borrowed. A record index avoids permutation search; field work is linear plus ordered-map lookup and the work of each required value. Implicit unknown membership stays partial without fabricating a total size. No planner, new search loop or restart is introduced.

## D040 Homogeneous definitions supply unit coordinates (2026-10-12)

**Decision.** A finite ordered definition consisting only of a positive integral count and one repeated named unit proves a multiplicative coordinate conversion. A chain ends at an unconstrained numeric base definition. Reject this route for cycles, value restrictions, heterogeneous layouts or incompatible base identities. Unit names are ordinary definitions; no time-specific names occur in the solver. The source representation determines the addressed extent, independently of the outer declared count.

A path or finite homogeneous intersection requests a window in these coordinates. Both use the same retained structural cursor, measured segments and projection. Ordinary nested lists are entered only with unit evidence. A sparse gap contributes its arithmetic width; walking past a billion represented members does not create them. Project only the requested composite view, borrowing the checked source leaf information. A containing world's ordinary named-field lookup remains field lookup: unit conversion requires a homogeneous source representation.

**Writes and ownership.** Each temporary projected leaf retains its original occurrence; each projected composite retains its children. Staging a write follows those origins after ordinary scalar/list comparisons have succeeded. Grouping does not equate differently sized containers or rewrite the physical source layout. Conflicts and cancellation publish nothing. Unit-definition T metadata carries into a demanded composite. Sparse positions inside a gap still require a demanded occurrence/split operation; this increment can skip gaps but does not implement those writes.

**Streaming and cost.** Retain each structural cursor and its pending member across an input wait. Positive windows stop when their required suffix is established; negative windows require a known endpoint. Definitions are inspected structurally and the active definition set is shared along a chain. For S stored positions, definition depth D and O demanded output, work is O(SD + O), apart from existing value normalization; depth does not multiply the traversal of source positions. Storage covers the visited segment evidence, active ancestry and demanded view. No persistent parent-progress cache, planner, legacy solver fallback or restart is added. More general pattern/unit alignment remains with the existing correspondence engine until its premises are proved.

## D041 Inherited type evidence is not a primary name (2026-10-12)

**Decision.** Applying a type-only listSpec qualifies a member without replacing its written primary name. If that member has no primary name, retain the checked nominal evidence in the model's existing `addedTypes` storage. Read that evidence into the same scalar facts for later type selection, comparison, unit measurement and writes. An explicit named RHS or a value-producing template keeps the existing primary-name behavior. No printer exception is introduced, and the evidence survives template discharge and later requests.

The current facts representation supports one proved inherited nominal when there is no primary type; broader nominal conjunction and inheritance proofs retain their existing boundary. This change does not declare differently named types identical. Direct and batched ancestor listSpec application mark their contribution as inherited; ordinary relationship qualification is unchanged. Projected unit leaves retain their proved unit type through the same additional-type representation.

**Reason and cost.** The previous merge promoted a qualifier into a primary name, turning every supplied `1` into `second:1` and obscuring the distinction between given members and newly represented named values. Dropping the type on discharge would lose facts. Keeping it in existing model storage preserves both semantics and written names with constant metadata per member and no extra source traversal. Parser waits, atomic publication, source ownership and numeric completion without size remain unchanged. Print/reload is not a requirement.

**D040–D041 acceptance.** Regenerated library and executable agree across all 148 WorldManager-related method bodies. All 40 WorldManager suites pass; independent library clients pass 89 unit-window and 44 inherited-type checks. Default failures fall from 98 to 67 (332 fixtures); the full inventory falls from 1490/1900 to 1459/1902 with no new failing names. All 15 protected fixture hashes, historical registrations and archived assertions are unchanged. The unit-view scale checks visit 33/65/129 structural positions for 32/64/128 leaves. Sparse materialization remains the next increment.

## D042 Demanded sparse occurrences and atomic span edits (2026-10-12)

**Decision.** A proved homogeneous window can expose a base-unit position inside a known sparse extent. Pure numeric pattern members share the pattern's proved numeric base unit; fixed typed prefixes and suffixes contribute arithmetic widths. Other predicates remain with ordinary correspondence. Sparse recipe reads stay partial, and positional acquisition may return the bare named recipe. Unknown output never becomes a numeric zero merely because its old container was an intersection or a path.

A wholly unknown requested window remains a counted listSpec, including a billion-position request. Retain its original source and coordinate offset so subsequent projection composes back to the same source. Checked values supplied to a span retain inherited unit qualifications and their written names. Copy the ordinary list requirements when returning a unit or path result; partial values must not lose their templates.

**Publication.** Stage all demanded sparse writes after ordinary value comparisons. Coalesce writes to the same source position through the scalar merger, sort each source's demanded positions in descending order, then reuse the model's existing sparse prefix split and head exposure. An implicit whole-list tail acquires one compressed remainder at publication. Validate the source and model integer bounds before publishing anything. Rejection, cancellation and waits produce no source split. A changed source extent cannot receive stale coordinates after a wait.

The virtual graph supplies the parent and occurrence to the model edits. This avoids relying on legacy `pParent` population, storing temporary parent links, or repeatedly searching the parent's prefix. Existing model callers retain their original defaults. Existing repetition-family remapping preserves a retained occurrence's identity when it materializes. Open typed descriptions use the existing morphological collection-name operation; opaque named listSpec recipes remain contextual rather than failing as unresolved standalone names.

**Cost and limits.** Structural traversal follows the established unit-window bound and never counts through a represented gap. A wholly unknown window has compressed storage. For K exposed values, staging uses O(K) storage and the publication ordering uses O(K log K) work; splitting receives the already proved source occurrence. Concrete output and constraint evaluation retain their ordinary costs. General ambiguous sparse patterns, native composite-gap materialization and unresolved endpoints remain separate boundaries. No planner, path cost, whole-world prerequisite or goal restart is introduced.

**D042 acceptance.** The regenerated executable and library agree across 152 solver/entry/model-edit method bodies. All 41 WorldManager suites pass; the independent sparse client passes 145 checks. Sparse batches of 32/64/128 writes use 102/198/390 frames and three structural visits. The default suite recovers 22 cases, moving from 67 to 45 failures out of 332; the full retained inventory is 1437/1903 with no new failing names. All protected fixture hashes and prior registrations remain unchanged. Five early WorldManager assertions that expected opaque named listSpecs to be unsupported now check the supported complete/partial results; new checks verify their retained qualifiers. A containing type-only template discharges only when its remaining sparse obligations are already represented by its children; unknown parent extents and uncovered tails retain the requirement.

## D043 Boundary arguments supply ordinary list identities (2026-10-13)

**Decision.** First-item arguments (`<:` and `:>`) expose an ordinary virtual RHS `{argument ...}`. Last-item arguments (`<!` and `!>`) expose `{&{...} argument}`. A known parent extent bounds the prefix. Whole-list requests and selected-member discovery use existing correspondence, requirements, conflict checks and atomic publication. Named targets establish their declared layout before this relationship. A retained source offset lets fixed-coordinate correspondence consume a proved sparse prefix over several smaller target positions without unfolding it.

The RHS borrows the argument; successful publication discharges the applied argument syntax, preserving original member occurrences. A known selected value does not demand an irrelevant argument. Pending argument input resumes at its original parser-owned infon, and cancellation publishes nothing. Non-list arguments and unproved endpoints retain explicit boundaries. Work is linear in visited fixed-width list positions plus ordinary constraint evaluation; no separate matcher, planning cost or restart is added.

**D043 acceptance.** Both regular CodeDog targets regenerate successfully and agree across 153 solver-related method bodies. All 42 WorldManager suites pass. The argument client passes 58 checks; argument, path and end-relative checks together pass 1,106 assertions. Fixed-width lists of 32/64/128 members use 72/136/264 frames. Default failures drop from 45 to 40 out of 332; the full inventory is 1432/1904, with no new failing names. Protected fixtures, archived assertions and prior registrations remain unchanged. Validation caught and corrected partial skipping of represented lists: only empty unconstrained spans admit a retained arithmetic offset; represented lists use structural traversal.

## D044 Finite temporal navigation retains occurrence context (2026-10-13)

**Decision.** `.begin` and `.end` request an identified null occurrence through the virtual graph. Reuse the Agent's existing identity registry and stage new boundary incidence until successful publication. A boundary exists independently of the source's value or available definition. A bare boundary operator takes precedence over a same-named field; a supplied value still denotes an ordinary field predicate. Boundary results use the existing path continuation and may themselves be followed by another boundary request.

`%tprev` and `%tnext` follow adjacent represented occurrences under an ordered T-list, crossing the beginning/end of nested closed T-spans. Ordinary S-lists do not supply temporal adjacency. An absent side of a proved finite layout yields `UNDEFINED`; an open or unrepresented required side stays partial. Named T-spans borrow the existing named-layout cursor. These are finite occurrence-navigation rules, not general temporal preservation, interior cuts, symbolic sparse adjacency or T-rule inference.

**Context and cost.** Carry a borrowed navigation path separately from the frame's demand for parent identities. Ordinary co-propagation already owns those identities and must not request them again. Context-free facts retain the existing shared cache. Context-dependent results are reusable only at the same occurrence path; a completed occurrence can supply a later temporal neighbour. Definition bodies retain their instance's occurrence and remain unchanged. A predecessor chain visits each solved occurrence once, with ancestry comparison proportional to nesting depth, rather than normalizing each earlier prefix again. Mutual unresolved temporal dependencies retain an explicit joint-proof boundary.

**Streaming and publication.** Keep the cursor, named-layout walk and exact parser-owned input across waits. No boundary identity is allocated before publication, and cancellation creates no boundary incidence. Terminal null/undefined values have explicit facts and publication; qualified terminal predicates and new occurrence-identity assertions remain unsupported. Parser/identity allocation adapters are model bindings; all navigation and continuation logic remains in WorldManager.

**D044 acceptance.** All 43 WorldManager suites pass in the regular executable. The independent regenerated-library client passes 29,807 native checks, including the 75 temporal assertions. Predecessor chains of 32/64/128 members use 33/65/129 frames. All 155 relevant generated methods agree between the two regular targets. Default failures drop from 40 to 31 out of 332; the full inventory is 1423/1905, with exactly the nine temporal fixtures recovered and no new failing names. Protected language expectations, historical registrations and archived assertions remain unchanged. Two earlier internal assertions that expected bare boundary access to be unsupported now check canonical identified nulls.

## D045 Checked nominal evidence survives meaning application (2026-10-13)

**Decision.** A successfully applied positive meaning supplies its primary nominal and checked additional qualifications. Retain that evidence in scalar facts and publish it through the model's existing `addedTypes` storage. Keep the destination's primary name. A later request can use the same bound declaration evidence without repeating an ancestry search. Type predicates remain directional; an asserted identity can use a proved subtype relationship in either direction. Equal primitive values or a common parent do not identify sibling declarations. Unprocessed general nominal conjunctions, overloaded meanings and ambiguous inherited field correspondence remain explicit boundaries.

An unresolved nominal identity can use the existing pending-comparison continuation while the target's own meaning is applied. Resume that comparison when nominal evidence arrives. Known leaf mismatches reject immediately. Positive ancestry is not copied out of a negative predicate or combined across alternatives.

**Records and demand.** Unique exact primary field identities can align even when their types inherit another type. The existing field index and ordinary member merger still prove each pair. Unmatched inherited names require additional correspondence evidence. Unordered record predicates compare corresponding named fields independently of storage order; missing or duplicated keys stay unresolved. An omitted field needed as a value continues a copy of its saved predicate frame with the stronger demand, preserving its expression and work-list cursors. Exhausted unknown-valued meanings are partial results, not a reason to requeue them.

**Representation and lifecycle.** An explicit unknown numeric size remains explicit when supplied by a strict meaning. This does not invent size knowledge. Checked additional types remain available after publication, and shared definition bodies remain unchanged. Boundary-source parser errors propagate before any identity is allocated. Reusing a completed occurrence must also satisfy an explicitly requested size demand.

**Cost.** This adds no speculative ancestry walk, planner or restart. Only requested ordinary meanings produce qualifications. For Q retained nominal qualifications, straightforward deduplication/combination is O(Q squared) in the worst case; exact field indexing remains O((N+D) log(N+D)), plus ordinary value work. A larger type hierarchy may justify a shared proof representation later, based on measured need.

**D045 acceptance.** Both regular CodeDog builds succeed and agree across 160 relevant generated methods. All 44 WorldManager suites pass; the independent library client passes 29,865 checks, including 58 new nominal-evidence assertions. Default failures fall from 31 to 30 out of 332; the full inventory is 1422/1906 with no new failing names. `word/setColor` is recovered. Adjective search now selects the correct record but its expanded representation differs from the protected expected string. All protected fixtures, archived assertions and prior registrations remain unchanged. Validation caught an exhausted-meaning requeue loop and a string-format regression; both are fixed and covered by the final runs.

## D046 Selection requests reuse occurrence and boundary proofs (2026-10-13)

**Decision.** A closed anonymous numeric or string literal does not supply an unbound field name. Reject that nominal candidate without evaluating unrelated values; an unknown predecessor still prevents a proved match. A positive type-selection predicate preserves an existing primary name, just as the existing range and exclusion branches return the selected value. For an anonymously qualified member, the explicit descriptor still names the requested view. Copy future list requirements and nominal qualifications along with the facts.

A bracketed literal scalar specification requests ordinary predicate search from its boundary. Lower the boundary through `offsetDomain` and use the same correspondence chart as a dot search. Literal list specifications retain span-window behavior. Negative boundaries require an established endpoint. Dynamically resolved list/expression specifications retain their existing boundary rather than inventing a different span interpretation.

**Occurrence context.** `%` requests the containing definition instance through the retained navigation path. Its owner supplies a given physical layout or a borrowed named layout. Ordinary RHSs and scalar-expression operands retain their caller's occurrence context; this also makes temporal RHSs such as `_ = %tprev` refer to the containing position. Record merges propagate contextual dependence to their consumers, preventing reuse at a different occurrence. A recursively named constructor whose own body is already given does not apply itself again; a value-free alias cycle still needs a joint proof. Reading values represented only in an unfinished enclosing merge remains a separate virtual-layout boundary.

A path forwards an explicit size demand to its selected-member request. Value-only completion cannot satisfy that stronger demand. Publishing a selected result replaces the old containing expression's size, including when the result size is unknown; it must not fabricate an endpoint from the old wrapper. Parser waits retain the selected candidate and consumed prefix; no goal is cleared or restarted.

**Cost.** Boundary search reuses the existing span and predicate scan. Closed unrelated primitive headers require no definition goals; visited source work remains linear for the fixed-width cases tested. Self lookup follows the retained enclosing occurrence path, with cost proportional to its depth. No path costs, planner or separate search engine are introduced.

**D046 acceptance.** Both regular builds succeed and match across 160 relevant generated methods. All 45 WorldManager suites pass; the independent library client passes 29,929 checks, including 64 new provider checks. Default failures fall from 30 to 28 out of 332; the full retained inventory is 1420/1907 with no new failing names. `slice/simpleSugar` and `user/getUser` are recovered. Tests preserve distinct self-dependent instances, requested size, unknown source extents, streaming prefixes and open-list obligations. Protected fixtures and prior registrations remain unchanged. One older unsupported scalar-spec assertion now checks the supported result; the nominal-evidence test now checks preservation of an explicit source name.

## D047 Explicit scalar replacement creates a checked new value (2026-10-13)

**Decision.** `<<=` replaces a positive numeric or string value and its extent with the checked RHS facts. An anonymous RHS retains the destination's declared nominal; an explicitly named RHS supplies the new primary name. The operation creates one new value goal at the destination occurrence, reusing ordinary meaning and retained parent-listSpec discovery. Proofs about the old value do not establish constraints on the new one. A retained edge marker prevents preparing the same replacement twice across continuations; this is not a reset of the original goal.

The RHS's completed expression and identity cursors are not replayed. Parser waits stay at the RHS or newly requested meaning. Source writes use existing staged occurrence publication; rejection, missing values and cancellation publish nothing. Publication replaces a discarded extent even when the replacement size is unknown. Existing source POVs and unrelated members survive. Ordinary identities still reject conflicting values. List replacements and negative/qualified replacements remain explicit boundaries.

**Cost and limits.** Each supported replacement adds one value frame plus any ordinary required meanings or parent constraints. No planner, path cost or fallback engine is introduced. Only retained relationships can constrain the new value: previously discharged anonymous value/extent evidence is deliberately replaced; declared nominal meanings are requested again for this new semantic state. Sparse duplicate replacement ordering remains outside the supported scalar-occurrence write path.

**D047 acceptance.** Both regular CodeDog builds succeed and agree across 160 relevant generated methods. All 46 WorldManager suites pass; the independent regenerated library passes 29,998 native checks, including 69 replacement assertions. Default failures fall from 28 to 27 out of 332, recovering `merge/ident/force`; the full inventory is 1419/1908 with no new failing names. Protected fixtures, historical registrations, archived assertions and unrelated `myTests.tst` remain unchanged.

## D048 External associations are another source of checked RHSs (2026-10-13)

**Decision.** `^` reads or asserts an external property on an exact occurrence. The existing IdentityRegistry owns persistent associations; the normalizer stages new records and value writes until publication. The bearer and property value are not equated. A borrowed definition's navigation context anchors the property to its actual instance, not the shared meaning. A retained parent view carries its proved source alias across publication.

`~` first requests ordinary member correspondence. Only an explicit no-match proof exposes the external tier. Unknown candidates, parser waits, ambiguity and conflicts with a selected direct member do not justify fallback. Existing ordered first-match behavior is retained. Structural dot operators remain dot operators; resolving a word such as `first` still uses a word predicate. The same external-value provider supplies a normally normalized RHS's associated value when a positive requested type differs from the bearer type. Applying the definition itself does not perform that projection.

External values use ordinary scalar/record normalization, predicate checks and staged occurrence writes. New property storage is separate from the descriptor and shared RHS. Existing property updates do not create duplicate associations. Alias lookup recognizes an explicit alias endpoint wrapped by a later POV of the same stored view; it does not broaden general POV identity or identify equal descriptions. Missing properties and multiple external candidates remain unresolved. Preserving the asserted reference's unexpanded presentation remains separate from checking and returning its normalized value.

**Cost and lifecycle.** The direct scan retains existing cursors and no-match evidence; external lookup is requested only at that boundary or a typed RHS projection. Registry snapshots and existing alias proofs are reused, without a second solver or path-cost plan. Current registry lookup scans associations and explicit identity edges; its existing identity traversal can be quadratic in edge count, and endpoint alias candidates can multiply that cost. Indexing should follow measured demand. Parser waits retain frames and staged effects; rejection and cancellation publish neither associations nor source edits.

**D048 acceptance.** Both regular CodeDog builds succeed; 164 relevant generated methods, including registry association lookup, agree between targets. All 47 WorldManager suites pass; the independent library passes 30,078 native checks including 80 new association assertions. Defaults improve from 27 to 22 failures out of 332; the full inventory is 1414/1909 with no new failing names. Direct resolution, external fallback, direct precedence, name-value isolation and typed RHS association projection fixtures are recovered. The asserted-name-reference fixture now returns a checked expanded value and remains a presentation/selection-demand difference. Protected fixtures, historical tests, archived assertions and unrelated work remain unchanged.

## D049 English text yields lexical word occurrences (2026-10-13)

**Decision.** Restore the existing `english-text` to `english-phrase` primitive from Proteus3 `splitLangText` and Proteus4 `WordForms`. Once a positive string value has a proved finite endpoint and its requested work is settled, scan its borrowed text leaves once and produce an ordered sequence of lexical word occurrences. Literal spaces delimit words; repeated spaces contribute no empty word. Spelling bytes, including apostrophes and Unicode, are preserved. The primitive creates lexical descriptions with checked `word` qualification; it does not evaluate a dictionary definition merely because its name matches a token.

This is a deterministic graph value operation, not a second normalizer. Generated member facts use normal staged publication. Source strings and shared definitions stay unchanged, and local parser waits continue the original frame. Unknown string endpoints remain strings rather than being turned into closed token sequences. Token names without requested semantic values remain partial, including on later normalization. General tokenization, punctuation rules, dictionary interpretation and other language transforms are separate work.

**Cost.** O(B+W) work for B source bytes and W output words, with the existing text cursor crossing each borrowed leaf once and one generated member frame per word. No repeated substring scan, planner, path cost or new scheduler is introduced.

**D049 acceptance.** Both regular CodeDog builds succeed and agree across 165 relevant generated methods. All 48 WorldManager suites pass; the independent regenerated library passes 30,121 native checks, including 43 lexical assertions. Defaults improve from 22 to 21 failures out of 332, recovering `lang/splitText`; the full inventory is 1413/1910 with no new failing names. Protected fixtures, historical registrations, archived assertions and unrelated work remain unchanged.

## D050 Future member templates are demanded by members (2026-10-13)

**Decision.** A named unbounded list with no represented members and no local listSpec may request its declared shape even during value acquisition. When an empty open list has no current identities demanding members, retain its listSpec as a deferred frame and future requirement. Read its header for ordinary pattern memoization, without claiming that its body has been checked or subscribing to an unused parser input. A member request evaluates that same frame through the existing prerequisite continuation. No additional scheduler, normalization restart or repeated template scan is introduced.

Two positive unbounded lists containing only a sole unknown tail need no positional alignment. If the target has no local/future/member requirements or retained work, ordinary adoption can supply the source layout and future template. Existing members, bounds, qualifiers and constraints keep the existing correspondence path. Staged publication installs the retained template on an existing empty body as well as on an adopted body. The shared source/template remains unchanged.

**Representation and validation.** Three compiled assertion sites formerly required the future constraint to remain on a redundant RHS. They now require the same constraint directly as a listSpec; subsequent valid and conflicting finite closure assertions remain unchanged. No protected `.tst` expectation changes. The `cube/getLstSpec` query now obtains the declared temporal list and structured member template; its printer still emits an inferred unknown-size prefix inside that template, a remaining presentation difference from the historical fixture.

**Cost and boundaries.** An empty future template costs one retained frame and one header read, with no member applications or input subscription. Later finite closure checks the template once and applies it linearly across requested members. Counts, existing spans, overlapping prefixes/suffixes and qualified relationships retain their previous alignment requirements. This does not settle eager scalar RHS evaluation or expand an unbounded tail.

**D050 acceptance.** Both regular CodeDog builds succeed; all 165 compared generated methods agree. All 49 WorldManager suites pass, and the independently linked regenerated library passes 30,158 native checks including 37 new deferred-template assertions. Defaults remain at 21/332 with no new failing names; the cube result now contains its declared temporal shape and listSpec, leaving only the unknown-size spelling difference. The full inventory is 1413/1911. Protected fixtures, archived assertions, prior registrations and unrelated work are unchanged.

## D051 Inserted streams supply input to the current request (2026-10-13)

**Decision.** `%INSERT` requests a parser-owned child through the existing InfonStreamLoader and its protocol/origin policy. Cache that child on the insertion node under the model lock, so a continued or repeated request does not reopen the same stream. WorldManager retains one child frame and resumes through the normal exact-input subscription. The child has no independent Agent session or normalizer: its declarations, associations and writes belong to the outer request's staging and publish only when that request permits publication.

Connector creation runs on a loader-owned startup worker, outside the model lock. The existing startup/shutdown barrier includes queued workers; exceptions release it, and shutdown joins workers and parser-only streams before the host disappears. The loader's ordinary `startNewConnection` API still starts a normalizer/session as before. Connection rejection and parser errors propagate through the same child input; cancelling one request removes its subscription without publishing its result.

**Publication.** Imported declarations are attached as the original declarations when a parsed list becomes part of a result. They are not copied as value assertions; ordinary declaration-copy restrictions remain. Insertion publication changes the placeholder's item mode and source origin. Closed ordered list lengths inferred from their contents are recomputed after their input members become known. Explicit extents, including an explicitly unknown extent, remain constraints.

**Cost and boundaries.** One connection worker, parser/input stream and dependency frame per inserted source, with ordinary linear traversal of its parsed members. Deferred sources do not restart parent goals. Partial streamed-prefix publication and reply effects remain outside this increment. Kernel loading now parses its 17 source documents; initializing its `world` value encounters the separate unresolved size expression `*(20000+_)+{T million-year|...}`. File insertion does not solve that symbolic-size requirement.

**D051 acceptance.** Both regular CodeDog builds succeed and agree across 175 relevant generated methods, including the input loader and Agent entry. All 50 WorldManager suites pass; the independent regenerated library passes 30,193 native checks, including 35 stream insertion assertions. Defaults remain at 21/332; the full inventory is 1413/1912 with no new failing names. The first full run caught inferred extent recomputation erasing a fractional span boundary; the final implementation retains invalid extents for correspondence and the original boundary assertion passes again. Kernel loading reaches its 17 inserted source documents and then the separate symbolic-size limitation. All protected fixtures, archived assertions, historical cases, prior registrations and unrelated work remain unchanged.

## D052 Anonymous typed lists expose their implied plural description (2026-10-13)

**Decision.** Factor the existing ModelManager plural-name inference into a read-only helper. Virtual list reads can expose that description as nominal evidence before source publication, so ordinary correspondence can find `feet` in `{foot|foot1,foot2}`. Explicit container names keep precedence. Negative templates and selections do not establish a positive homogeneous member type. This does not implement general morphological definition lookup.

A positive type-only parent assertion can qualify a bare named member with no available definition or primitive value/type. Keep its written name and store the checked qualification. Mark that bare name as interpreted for this occurrence so subsequent reads consume the stored evidence rather than treating it as an unchecked conjunction. Bound meanings, primitive values, negative requirements and ordinary nominal identities retain their previous checks. This extends D041's inherited-type representation; it does not identify sibling declarations.

A closed unordered list can discharge a checked type-only listSpec using the same coverage proof as an ordered list. Each member retains the qualifier; open tails and explicitly unknown extents retain the template. Publication omits qualification evidence identical to the published primary name, preventing a redundant self-qualification from becoming an unsupported conjunction on the next read.

A checked RHS with the same nominal identity can carry its meaning-applied proof through the ordinary merge when that proof is context-free. Contextual sources retain occurrence-specific proof obligations. This also prevents later reads of normalized composite units from treating retained implicit plural evidence as an unprocessed nominal conjunction.

**Cost and lifecycle.** Inference uses the existing word inflector without walking member values or changing a source. Applying a template remains one ordinary derived merge per demanded member, and checked qualification is retained through ordinary staged publication. Exact parser waits and cancellation retain their existing continuation. No query-name special case, new solver, planner or restart is introduced.

**D052 acceptance.** Both regular CodeDog builds succeed and agree across 177 relevant generated methods, including the shared ModelManager helper. All 51 WorldManager suites pass; the independently linked regenerated library passes 30,257 native checks including 64 plural/type-evidence assertions. Defaults improve from 21 to 20 failures out of 332, recovering `word/findPlural`; the full inventory is 1412/1913 with no new failing names. The first full run exposed four nested-unit regressions where a checked same-type RHS did not carry its context-free meaning proof into the destination. The final merge preserves that proof; focused tests and the four original fixtures verify subsequent reads and atomic rejection of conflicting writes. Protected fixtures, archived assertions, historical cases, prior registrations and unrelated work remain unchanged.


## D053 Missing relative context controls template applicability (2026-10-13)

**Decision.** Preserve the existing main-branch initial-condition semantics, which the user confirmed already worked there. A template requiring `%prev`, `%next`, or an analogous relative link does not apply at an occurrence where that link cannot resolve because the required neighbor is absent. Mark the template's relative-context requirement and retain the applicability result for the particular occurrence. A missing predecessor can exclude the first application; a missing successor can exclude the last. A template without such a dependency has no first-item exemption. Non-applicability is neither a contradiction nor a rejection of the member or its enclosing list.

**Reason.** A supplied initial value does not by itself authorize ignoring a template assertion. `1 = *256+_` acquires the size and gives `*256+1`; a further applicable assertion of `*256+3` contradicts it. The old `time/t2` expectation relied on bypassing that assertion without a missing-relative-link condition. The user confirmed that the fixture predates the settled initial-condition rule and explicitly requested its removal. Remove that registration rather than weakening ordinary assertion checks or replacing its expected result with current engine output. Record the exact removed fixture and before/after hashes in `retired-normalizer-tests.json`; earlier audit snapshots remain historical evidence.

**Streaming and reuse.** Proven absence is distinct from a neighbor whose existence or value is still pending. Preserve the existing exact-link continuation in the latter case; do not turn a temporarily unavailable successor into permanent non-applicability. Relative-context requirements can be shared with the template, while applicability and resolved values belong to the occurrence. Record attempted applications and continue local waits without resetting the enclosing goal. This rule adds no scan of earlier prefixes and does not require enumerating an unrequested tail.

**Implementation status.** This is an unported main-branch behavior, not a new language feature or an undecided applicability rule. This change records that compatibility requirement and removes the obsolete fixture; the replacement still needs the implementation. Current `WorldPOVGraph`/`WorldManager` temporal navigation represents proved absence as a terminal `UNDEFINED`; ordinary merging can consequently reject a template application with `Incompatible terminal values`. Live probes `{T _ = %tprev | 1 1}` and `{T _ = %tnext | 1 1}` both expose that gap. Implement applicability at the template/occurrence boundary, preserving the reason for unavailable relative context through its dependent expression, rather than treating every terminal mismatch as optional. Direct standalone cursor presentation is a separate contract and is not changed by this clarification. Finite template-tail materialization also remains independent work.

**D053 validation.** The unchanged executable reads the updated fixtures and reports 19 failures out of 331 defaults. The only removed failing name is `time/t2`; every retained failure message is identical to the D052 run. The fixture diff is exactly one removed line. All 15 manifest hashes match the explicitly recorded exception, archived assertions and historical/compiled registrations are preserved, and unrelated `myTests.tst` is unchanged. No solver source changed, so no rebuild or broader runtime validation was needed.

**Main-branch source audit.** In `/home/bruce/devl/Proteus`, `main` at `86321ea` declares `pureInfon.undefined` in `Proteus.Lib.dog:52` specifically for a missing `%prev` or an expression depending on an undefined item. `lookUpCursorRules`, lines 1325–1345, marks missing `%prev`/`%next` this way without setting rejection. The user confirms that applicability worked in this engine. The replacement's `WorldPOVGraph.read` instead converts this flag into a completed terminal value, and `WorldPOVGraph.merge` rejects its mismatch with a numeric value. Preserve the old distinction between unavailable relative context and contradiction when porting template application; do not imitate incidental acceptance of an already-known scalar. This audit inspected source at the named main revision; it did not rebuild or rerun that revision.


## D054 Retain applied listSpecs and hide them only in ordinary printing (2026-10-13)

**Decision.** A listSpec remains part of the stored model after application to a finite list. The user clarified that the old engine hid this redundant syntax when printing rather than deleting the template. Restore that separation. Later member requests and writes must still see and enforce the template, and shared template bodies remain unchanged.

**Reference evidence.** `InfonViewPrinter.isNiceList` in Proteus main (`86321ea`, `infonIO.dog:1345`) returned true for a processed literal list without an unfinished tail, except in debug mode. The printer used it to suppress only the listSpec text. The replacement removed that helper in `82378ee`; subsequent publication code explicitly cleared covered listSpecs. The latter is the representation error being corrected. D054 supersedes earlier D03/D052 wording that describes deleting or discharging a listSpec; discharge of completed work does not authorize deleting its semantic source.

**Implementation.** Keep the original listSpec reference during publication and adoption. Reuse the existing coverage check to record `listSpecApplied` on the published occurrence instead of clearing the pointer. Restore `isNiceList` in the printer, using that checked application marker rather than the retired scheduler's `wasProcessed` flag. Normal printing omits the checked template on a closed literal list; debug printing still shows it. Open or unproved applications remain visible. A new supplied constraint invalidates the display marker through `noteNormalizationWork`; it does not erase the template. For ordinary refinement of the same existing member occurrence, this proof also prevents redundant parent-template application: its constraints already remain in that member or its future template. A replacement value has a fresh frame and must still acquire the parent template. Parent-template requests carry the same containing context as ordinary listSpec application. A deferred projection does not invalidate a type-only coverage proof when each embedded child retains the same positive template; the goal remains partial and the templates remain stored.

**Cost and lifecycle.** Keep one borrowed template reference and one display marker per occurrence; no additional member scan is introduced beyond the existing coverage check. Publication remains staged and atomic. Exact parser waits, cancellation and the no-new-goals path for completed unchanged requests are retained. This does not implement D053's relative-context applicability; that remains a separate compatibility gap.

**D054 acceptance.** Both regular CodeDog targets build successfully (library 143.93 seconds; executable 232.55 seconds), and 180 relevant generated methods agree, including the printer and invalidation hook. All 52 WorldManager suites pass. The independently linked regenerated library passes 30,322 native assertions, including 65 retention/printing checks. Default failures remain 19/331 with identical failing names and messages. The full inventory is 1411/1913 with no new failing names; compared with D052, only the separately authorized removal of `time/t2` changes the old failure set. All protected fixture hashes, archived assertions, historical/prior registrations and unrelated `myTests.tst` are preserved. Two replacement-specific checks that formerly required template deletion now require retention and application proof. Validation caught the missing parent-template context, redundant reapplication during member refinement, and sparse display coverage after deferred projection; the final checks include later reads and atomic rejection of conflicting replacement values. No language expected output changed.


## D055 Audit the nineteen remaining default failures before further implementation (2026-10-13)

**Scope.** The user requested classification of the nineteen default failures. This entry records evidence and prioritization, not a new normalization rule, fixture retirement or permission to replace expectations with current output. The [consolidated classification](normalization-design.md#remaining-default-test-classification-2026-10-13) and per-fixture inventory (`graph/default-test-classification.json` in Git history) contain the findings.

**Finding and rationale.** The fresh default run is unchanged at 19/331. Four are formatting/printed-representation mismatches; seven concern reference expansion, normalization depth or pending/no-match result handling; six stop during world loading in three prerequisite families; one combines a numeric-capture fixture problem with unsupported sparse composite work; one is explicitly deferred. Thirteen names exist in the inspected main snapshot, and fourteen fail in the saved original-Proteus4 comparison. Historical status, fixture validity and present semantic correctness are separate questions. Do not treat nineteen failures as nineteen independent engine defects or accept a historical pass as a semantic oracle.

**Next-action boundary.** Investigate shared prerequisites and general correspondence/demand rules before individual query symptoms. Preserve pending effects and retained listSpecs. Do not turn constant listSpecs into defaults to load an inconsistent model. Review symbolic reference and child-expansion expectations explicitly; the hour probes prove values, not appropriate demand or cost. The already deferred ambiguous-alignment case remains deferred. The 1,384 unported assertions and eight historical compiled failures are outside this step.

**Validation.** Default failure names and messages exactly match D054. Current-library probes confirm partial unknown-endpoint writes, out-of-range sparse selection versus an aligned control, the mixed composite-capture problem, correct hour values, both rejected writes with unchanged values, the symbolic-size boundary, and successive Rubix load blockers. Each probe group used an isolated process with the current generated header/library. Full user-model failure localization and normalization-depth/cost proofs remain incomplete. Protected fixtures and archived assertion sources remain unchanged; no source regeneration is needed for the documentation/inventory-only change.


## D056 Correct agreed displays and isolate default world fixtures (2026-10-13)

**Accepted scope.** The user approved the three recommended expectation corrections and investigation of shared loading blockers. They then required regression tests to avoid changing real user/world files and explicitly chose small synthetic fixtures. This does not approve the other proposed output-contract changes or changes to listSpec assertion semantics.

**Changes and rationale.** Correct `this/lvl1` spacing and the two rejected-write follow-up displays, preserving their inputs, rejection assertions and values. Replace external world resources in the two default load tests with seven small committed synthetic files (753 bytes). Preserve unresolved symbolic world size in those cases and add a finite bootstrap/public-import control. Resolve a public fixture's kernel beside the requested fixture, with the existing origin-relative stream resolver. The reasoning engine and application loader are unchanged. A frozen copy of the full application model was considered and rejected in favor of the user's selected focused fixtures; these tests no longer claim full application-model smoke coverage.

**Diagnostic finding.** A temporary shared-library trace of the current generated test core confirms the first user-world contradiction is `xPos:20` against its parent's `xPos:40` template. Rubix loading first stops at the undeclared `reel`. These are now located failures, not guesses from the fixture contents. The user fixture's intended constraints and the Rubix composition gap remain future work. The symbolic-world limitation is reproducible with the synthetic kernel.

**Validation and cost.** Regenerate TestProteus with CodeDog. In a directory without external public assets, the default run is 16/332, WorldManager is 0/52, the world-loading group is 2/3 (the finite control passes), and the full inventory is 1408/1914. The recovered defaults are exactly the three approved expectations; remaining failure messages are unchanged. Every default world fixture and transitive import is local to tracked/proposed assets. Fixture/hash corrections are explicitly recorded, archived assertions and unrelated work are preserved, and no runtime solving work or planner is added. See the consolidated D056 section and the classification inventory's follow-up for evidence and limits.

**D056 fixture correction follow-up (2026-10-13).** The user authorized changing the first CodeDog TaskList's `xPos:20` to `40`, and the Rubix numeric spelling from `reel` to `rot`. The standalone Rubix fixture now supplies `#rot = *_+_` locally, preserving unknown-size numeric meaning without a live world dependency. These are resource corrections, not changes to listSpec or normalization semantics. Record before/after resource hashes in `retired-normalizer-tests.json`.

**Verification.** The user fixture now reaches `yPos:100` against its parent's `yPos:90`; the earlier x-position conflict is gone. Rubix now reaches `Unordered correspondence needs member matching` at the `edgeCube` definition. Focused results are 3/4 user failures and 1/1 Rubix failure; defaults remain 16/332 with the same failing names and only the Rubix message changed. The exact file-loader traces and logs are indexed in `default-test-classification.json`. All fifteen protected language fixtures and archived assertions remain unchanged. No generated code rebuild or full-inventory rerun is needed for these resource-only corrections.


## D057 Rebuild behavioral tests as semantics are implemented (2026-10-13)

**User decision.** During the semantic-family review of the 1,384 unported registrations, the user chose to start fresh and regenerate tests as work proceeds. Retire those placeholder registrations from the active runner. Keep the default language fixtures, historical parser/model checks, already ported behavioral cases and current WorldManager suites. The sixteen default failures remain deferred as requested. This supersedes the earlier requirement that every unported registration remain selectable and fail as `NOT_PORTED`.

**Reason.** The old registrations mix language behavior with solver-specific assertions. Representative source review found automatic subscriptions for quiescent unknowns, forced intermediate copies, exact cache/evidence identities, priced strategy cutoffs and expectations encoding old unsupported boundaries. Those do not automatically define the replacement's contract. Twenty-one isolated input-level probes also show that several old results already work while their registrations remain placeholders; probes are not complete ports of those assertion bodies. The old first-ambiguous-parse expectations differ from the accepted preservation of unresolved alternatives. Retiring the placeholders makes the active failure count informative without claiming semantic parity.

**Archive and scope.** Preserve every original registration, source line, checkpoint and assertion-source hash in `engine-test-migration.json`; mark the 1,384 retired entries `archived_reference`. Verify each archived registration against that source and repair 25 stale line locations, retaining their previous line numbers for provenance. Keep all 198 active compiled registrations and all 332 defaults. Retain the fourteen standalone native drivers as historical references, not a migration backlog. Remove the placeholder macro, its wrappers, the stream-insertion placeholder helper and the unused failure helper. Record exact counts and source hashes in `retired-normalizer-tests.json`. No solver, parser, production loader, language fixture, expected output or active assertion body changes.

**New-test contract.** Develop tests alongside a coherent semantic capability. Prefer small self-contained models that exercise the real Agent/WorldManager path. Check results and constraints, qualification/context and occurrence identity, source preservation and atomic failure. Add partial-input, exact-link continuation, cancellation and ownership checks where the feature needs them. For traversal optimizations, test work growth and sparse materialization with justified bounds; do not reintroduce path pricing, exact legacy trace strings, cache layouts or a requirement to fill every intermediate infon. Use the archive to discover missed cases, not to prescribe implementation. Preserve active failures and record any separately authorized expectation changes.

**Review limits.** All 1,384 names were assigned a primary semantic family; representative assertion bodies were reviewed across the families. This is not an individual keep/delete verdict on every old assertion. The user chose the fresh-suite policy before a full assertion-by-assertion port. The consolidated design retains the family counts as a coverage checklist, not quotas. The unported Rubix palette/transition cases depend on live world assets; any regenerated versions require fixed local fixtures under D056.

**D057 acceptance.** CodeDog regeneration/build succeeds in 187.43 seconds. The isolated full run is **24/530**, with exactly the same sixteen default failures and eight existing compiled failures as before the reset, including identical diagnostics. All 52 WorldManager suites pass within that run; no `NOT_PORTED` result remains. All 198 active registrations, 58 protected historical cases, fifteen fixture hashes, the archived source hash and unrelated `myTests.tst` are preserved. Every archived source location was verified, including the 25 repaired pointers. The original source also matches the pinned Git checkpoint. This is a harness/documentation change; no solver or shared-library rebuild was required.

## D058 A unique positive qualification can identify an anonymous record field (2026-10-13)

**Decision.** Field correspondence uses a written primary name when present; otherwise one positive nominal qualification can identify the field. Multiple qualifications require further proof. This uses D052's existing inferred plural description of a listSpec, without renaming the anonymous list or introducing a separate matching algorithm. Both record comparison and record merging use this identity accessor. The index still rejects duplicate descriptions and checks bound nominal identity before pairing.

**Reason.** A piece containing named location/orientation fields and an anonymous `{side|...}` list already carries the `sides` description. Ignoring that evidence unnecessarily blocked composition as unordered correspondence. Positional pairing of unordered members would be incorrect; the existing indexed field alignment is justified by the same unique type evidence used in selection. Shared definitions and templates remain immutable.

**Completion and cost.** The fully specified synthetic piece gives explicit unordered extents; an implicit unordered remainder retains its existing partial semantics. Known rot values still complete with unknown numeric size. At fixed piece width, existing linear field traversal and member pairing remain unchanged. Completed requests create no new frames or relationship reads.

**D058 validation.** Both CodeDog targets regenerate successfully. All 53 WorldManager suites pass; the new suite passes 87 checks in both the executable and an independent shared-library client. Full validation is 24/531 failures with no new failing names. The Rubix fixture now loads past the former unordered-field blocker and stops later at an unsupported request. Protected test inputs and unrelated work are unchanged.

## D059 Member correspondence follows field identity and ancestor containment (2026-10-13)

**Decision.** A member request can request identities on higher containing lists. A retained correspondence projects one non-embedded containment level at a time; intermediate frames request only a source occurrence, not its normalized value. Embedded spans retain the existing coordinate machinery. Whole-list requests retain their shared field/positional traversal.

For unordered records, a member correspondence reads each side's field headers once and requires unique effective field names plus bound nominal identity. It does not pair by physical position or choose the first duplicate. This is the member-demand form of D058's existing whole-record field alignment; the resulting RHS still enters ordinary merge/publication. Equal explicit finite extents may use whole-record alignment, retaining the target's extent check. Other unresolved explicit extents remain outside this fast path.

**Layout proof.** A represented list of uniquely described, non-embedded members with strict identities has stable named occurrences. Selection may traverse those headers before all identities are solved; the demanded member requests its corresponding parent constraints. Unknown/embedded layouts, replacements and type-changing relations retain the existing guard. An unrelated member's value is not a prerequisite of the requested field. No persistent parent merge-progress cache is introduced.

**Cost and continuation.** A single field request makes linear header scans at the relevant containing levels and creates value goals only on its requested route. The ancestor route is built once and retained by shared ownership; each descent consumes one entry. Neither repeatedly walking the remaining depth nor copying the remaining route is allowed. Thus route work and storage are O(depth), in addition to the field-header scans. Depth and width scaling checks measure ancestry steps, correspondence positions and created frames. Whole-list work still makes one shared merge at each level. The current header, cursors and intermediate occurrence projections survive parser waits; unrelated arrivals do not restart them. Ambiguous or conflicting writes publish nothing, and reads leave their source untouched.

**Validation.** Both CodeDog targets regenerated. The independent library client passes all active WorldManager checks, including 440 checks in the new piece-correspondence suite. Depth 8/32/128 fixtures construct the graph directly to isolate solver cost from parser cost; width and whole-list scaling remain covered. The full Engine run is 24 failures / 532 registrations with the same failing names as the starting baseline. All 15 protected fixtures and unrelated `myTests.tst` are unchanged.

## D060 Apply contextual templates at occurrences; absence is not contradiction (2026-10-13)

**Decision.** Evaluate context-free listSpecs once as before. A recipe requesting a neighbour without an occurrence records `contextRequired`, rather than claiming an absent neighbour or publishing a value. Application binds that immutable recipe to the member occurrence. A contextual recipe at another occurrence is a different goal; no live goal is cleared or restarted. Pure finished subexpressions remain reusable. Agent continues to own the session and request lifetime.

A closed occurrence boundary can prove the requested neighbour absent. That evidence propagates through the required recipe, including expressions and nested lists. At the template-application boundary, discard only that inapplicable application; its type, size and value constraints contribute nothing. The immutable listSpec stays attached. Applicable constraints still reject conflicts atomically. Direct absent neighbour reads, including a root with no containing list, retain the established UNDEFINED result. An unbound template cursor, open future, or unfinished parser header does not prove absence; they remain partial or wait at the exact input link.

`%prev`/`%next` use adjacent physical ordered members and stop at that list's boundary. `%tprev`/`%tnext` retain temporal traversal across nested T-lists. The providers return ordinary occurrences, allowing the same correspondence to serve an entire state or a demanded coordinate. This does not add arbitrary descendant projection of ancestor listSpecs: a leaf can use its enclosing state's explicit temporal identity; a whole state can request its parent's template.

**Reuse and cost.** A normalized source already supplies the facts for list correspondence. Mark its source resolved so merging never re-evaluates the source expression at a child's context. This both removes a redundant traversal and preserves temporal meaning. Completed occurrence proofs also serve members reached ahead through `%next`, so the later containing-list pass does not repeat that inference. Completed scalar template applications discharge their source edge when no unresolved work remains.

For fixed-width states in a flat chain, expected work and storage are linear in the number of states (the occurrence maps use hash tables). The 8/32/128-state checks bound created frames, relationships and applications in both directions; the 128-state probes use 515 frames/258 relationships backward and 769 frames/512 relationships forward. This is not a linearity claim for arbitrary ambiguous alignment, graph inference, or unbounded nesting. Waits preserve current frames/cursors; unrelated arrivals perform no work. Completed whole-list requests produce no new goals.

**Alternatives rejected.** Deferring every listSpec changed existing future-template and parser-continuation behavior, so only contextual evaluation is deferred. Treating missing context as UNDEFINED before an occurrence was supplied silently skipped valid future work. Reusing an active contextual expression merely by its source infon conflated different occurrences; non-contextual active cycles and same-occurrence temporal cycles still terminate without a circular proof.

**Separate performance observation.** A textual fixture with 32 nested named records exceeded the bounded parsing probe before normalization began. The depth regression therefore constructs small wrappers directly. It establishes the solver route bound, not a parser scaling claim; the parser cost still needs separate profiling.

**Validation.** Both maintained CodeDog targets regenerate successfully (final library 155.80 s; executable 202.19 s). The independent library client passes all active WorldManager checks; the fresh temporal suite passes 110/110 checks and the earlier piece suites pass 87/87 and 440/440. The full regenerated executable reports 24 failures / 533 registrations, with exactly the same failure names as the starting baseline. The only changed baseline diagnostic is the D058 Rubix fixture advancing past model loading to a later unsupported request. All 15 protected fixtures and unrelated `myTests.tst` remain unchanged. This validates the synthetic Engine model; it does not establish Slipstream rendering or general ancestor-template projection.


## D061 Asserted named identities combine compatible checked definitions (2026-10-13)

**Decision.** The user confirmed that `#front-color=color`, `#red=color:{hue:0}` and `front-color=red` must succeed after checking their definitions. An explicit positive identity can combine separately named meanings when ordinary scalar/list merging proves their constraints compatible. This revises D045's blanket rejection of sibling names for assertions. A shared parent or equal payload does not, by itself, prove that an unasserted type predicate matches another declaration; nominal selection remains directional.

**Mechanism.** When a positive asserted identity has no existing nominal proof and the target has an unchecked definition, keep the already resolved RHS and its relationship on that frame. Request the missing target meaning through the ordinary definition edge, then continue the saved identity through the existing scalar/list merge. This is a local prerequisite, not a change to the general relationship order. Once both meanings have been checked, nominal merging retains their qualifications and ordinary value/field correspondence decides compatibility. Unknown contents remain partial; conflicting values or sizes reject atomically. Undeclared labels do not gain fabricated definition evidence.

**Reason and rejected alternatives.** The live cube's first directly requested state rejected `front-color=red`. Merely relaxing the nominal rejection was unsound: the pending scalar-comparison path could accept two record headers without propagating the red hue or detecting a later blue conflict. Acquiring every definition before supplied identities fixed that example but regressed replacement and temporal-context behavior. The missing prerequisite belongs to this identity merge; it must preserve the resolved RHS and later resume the normal merge. No cube-specific rule, fallback solver, goal reset or copied adjacency graph is introduced.

**Cost and coverage.** Meanings and RHS relationships are requested through their existing retained cursors; each is consumed once per frame. Fixed-width repeated color assertions use 55/175/655 frames and 44/140/524 relationships for 8/32/128 occurrences. These measurements establish linear work for that family, not for arbitrary type conjunctions or ambiguous search. Regression checks cover actual hue propagation, both assertion directions, chained assertions, unknown values, distinct compatible scalar declarations, incompatible record/scalar values and sizes, source preservation, nominal search, local parser waits and cancellation.

**Integration boundary.** This change removes the initial nominal rejection; it does not implement implicit temporal preservation or complete the real cube's inherited field/face selections. The actual model remains an integration probe, not a dependency of permanent tests. Slipstream still needs migration to the replacement WorldManager interface and visible rendering validation after the engine can supply the demanded fields.

**D061 validation.** Both maintained CodeDog targets regenerate successfully (library 136.75 s; executable 193.69 s). The independent library client passes the five focused suites, including all 111 nominal-evidence checks and the unchanged replacement and temporal suites. The full executable reports 24 failures / 533 registrations, with the same failing names and diagnostics as the baseline (apart from the crash-protection engine banner). Assertions formerly rejecting compatible named scalars and listSpec siblings now follow the user-approved semantics. All protected `.tst` inputs and unrelated `myTests.tst` remain unchanged. The real-model probe still reports unresolved inherited layouts and implicit temporal preservation; no Slipstream GUI claim follows from these results.

## D062 Temporal preservation is a constraint; compound independence covers the whole value (2026-10-13)

**User clarification.** In the example with earlier `{x:1, y:2}` and later `{x:9, y:_}`, x:9 causes the whole thing to fail. The user also confirmed that `%?` applies to the whole compound value where it is placed.

**Decision.** An ordinary supplied later value does not authorize overriding preservation. If preservation applies, x:1 and x:9 conflict, and the containing state constraint is rejected. Do not infer an exception merely because the later value is already known or disagrees with the earlier one. Other language-defined change relationships remain a separate semantic question. This corrects the assistant's proposed example that accepted x:9 while inheriting y:2.

`%?` on a compound value withholds implicit preservation throughout that value, including descendant requests reached through ancestor correspondence. An explicit relationship can still supply a component; independence neither provides a value nor asserts inequality. Independence on one component leaves its siblings' preservation intact. Later occurrences remain distinct from earlier ones.

**Semantic examples, not implemented test results.**

| Earlier value | Later constraints and independence scope | Required result |
| --- | --- | --- |
| `{x:1, y:2}` | `{x:_, y:_}`, no independence | Preserve x:1 and y:2 |
| `{x:1, y:2}` | `{x:9, y:_}`, no independence | Reject the whole state constraint |
| `{x:1, y:2}` | `{x:9, y:_}`, independence on x only | x:9 is permitted; preserve y:2 |
| `{x:1, y:2}` | `{x:9, y:_}`, independence on the whole record | Keep the supplied x:9; y remains unknown absent another relationship |

**Consequences for the design.** Implicit temporal correspondence must carry the applicable preservation/independence scope into member requests. Known later values still require checking an applicable preservation constraint; preservation cannot be limited to filling unknown holes. Required conflicts reject atomically through ordinary solving. Whole-span work should propagate the same scope with its shared cursor rather than repeat ancestor discovery for every member. These are obligations for the upcoming implementation, not claims that the current engine implements implicit preservation.

**Validation.** Documentation-only clarification; no engine, tests, or fixture expectations changed. Reviewed the examples against the user's two answers and checked the documentation diff. The prior engine validation remains unchanged.


## D063 Project ancestor requirements through ordinary correspondence (2026-10-13)

**Decision.** After its local evidence, a member request can obtain an RHS from an unchecked containing definition or from a structured listSpec on a higher ancestor. The listSpec constrains the ancestor's member; correspondence then projects from that member to the demanded descendant. A containing definition is requested after that parent's supplied relationships, when the member still needs facts. Existing completion policy is unchanged: a member request is not a coherence check of every enclosing object.

**Mechanism and scope.** Retain the ancestor's immediate member alongside the existing parent relationship cursor. Reuse the field/span correspondence and shared descent route; do not normalize every enclosing list or add another matching algorithm. Definition correspondence uses uniquely identified record fields even when their physical representation is ordered and omits another definition field, just as whole-record definition merging does. Ambiguous identities, unresolved template layouts and embedded-span projection retain their existing boundaries.

A borrowed definition/template is a boundary for discovering additional ancestor requirements. Its source still carries the caller's occurrence and navigation context. When the borrowed source has exhausted its own evidence, return that partial result to the caller, which continues its remaining relationships. Otherwise an unknown borrowed field can climb back into the caller's constraints and falsely rediscover the same requirement. There is no reset operation, persistent parent-progress cache or copied source model.

An unspecified corresponding container contributes no descendant facts; exhaust that route without demanding a nonexistent layout. A synthetic partial-color record reproduces the real cube regression that exposed this case. Self references bind to the represented containing object rather than the surrounding query expression. Borrowed named layouts retain their definition marker, so `%` has the same containing definition context through direct definition and listSpec routes. Source definitions stay immutable; direct normalization and successful selected assertions retain target occurrence identity, and failed assertions publish nothing.

**Cost.** One member request scans the relevant field headers and follows one retained route. The extra ancestor-member pointer advances once per containing level. Depth and width costs are linear for the unambiguous fixed-layout families tested here. Whole-list normalization keeps its existing shared propagation and makes no per-member parent lookups in the regression family. This does not claim linear bounds for arbitrary ambiguity, parser depth, or repeated independent member requests.

**Reference precedence.** The user requested main-branch behavior over conflicting recent semantic suggestions. A clean snapshot of `/home/bruce/devl/Proteus`'s literal `main`, commit `86321ea75fbdbfa3cfd8cf356eb3eb2495b4efc8`, was built and probed. `{T 1 _}` remains unknown; `{T 1 9}` succeeds; the compound changed-value example remains unchanged; `%?` is parsed as two items. This reference therefore does not establish implicit preservation or whole-compound temporal independence. D062 describes the proposed behavior if preservation applies; it is not evidence that this main branch implements it. No implicit temporal or `%?` rule is added in this cycle. The user has been asked which historical implementation should supply that later comparison. The independent ancestor-correspondence work proceeds without assuming an answer.

**Short-cycle evidence.** The specification, representative traces, scaling counts, remaining boundaries and final validation are consolidated in `Docs/Design Docs/Historical/member-requirements-2026-10-13.md`. New permanent tests use synthetic models, not mutable world resources.

**D063 validation.** Both maintained CodeDog targets regenerate successfully (library 139.12 s; executable 199.16 s). The independent regenerated-library client passes all 470 ancestor-requirement checks. The full executable passes all 56 WorldManager suites and reports 24 failures / 534 registrations: the same sixteen default and eight existing compiled failures, with identical names and diagnostics. The known cube coordinate still returns zLoc:1.1; state 2 and the hue/layout probes retain the documented incomplete boundaries. All 15 protected language fixtures and unrelated `myTests.tst` are unchanged. No Slipstream source or live world resource was modified.


## D064 Share graph cursors between ordinary normalization and temporal navigation (2026-10-13)

**User decision.** The proposed temporal cursor should also be reused by regular normalization. Treat temporal movement as an operation of the common virtual-graph traversal mechanism. WorldManager continues to own the reasoning interface and Agent the session lifetime.

**Reason.** Ordinary normalization already needs occurrence context, retained traversal progress, corresponding RHSs and borrowed facts. A temporal cursor needs those same facilities. Sharing the interface lets member requests, list co-propagation and time navigation use the same correspondence and constraint solving. Movement need not copy the previous state's values or materialize intermediate infons. Existing specialized walks can retain the state required by their traversal; this decision does not require replacing them with a second solver or one oversized cursor implementation.

**Lifecycle and semantics.** A cursor identifies an occurrence and its navigation context. A normalization frame can retain cursor positions while requesting relationships and continuing at waiting links. Independent goals and branches retain independent positions, while sharing immutable structure and applicable proved facts. Moving a caller's cursor does not retarget an outstanding request. Reusing a value across temporal occurrences still requires the relevant relationship or proof; cursor movement alone establishes no preservation, override, or independence rule. Existing publication and atomic-write rules remain in effect. Cross-session deduction sharing remains deferred.

**Cost obligations.** Cursor movement should reuse structure rather than copy a complete state. Whole spans retain shared traversal instead of restarting a member lookup for each element. Retained source references must avoid repeated walks through the entire temporal history for already justified reads. Constant-time adjacent movement is possible with an established neighbour link; nested traversal, correspondence and newly demanded inference keep their actual costs. Exact API, storage and proof-reuse lifetimes remain implementation choices to validate, not accepted performance claims.

**Status and validation.** Design decision only. The existing positions, walks and temporal providers are implementation starting points; a persistent common cursor interface has not been implemented. Reviewed both design documents for consistency and checked the diff. No engine, test, fixture or runtime behavior changed, so the D063 validation remains the current baseline.


## D065 TCursor owns temporal materialization and uses lower-level solving (2026-10-14)

**User clarification.** Name the proposed class `TCursor`, leaving room for an eventual `SCursor`. Initialize it with a POV, preserving parent and surrounding occurrence structure. Do not require an Agent or session merely to construct or move a lightweight cursor; supply execution context only when needed. Start with `goNext()` and `goPrev()`. Integer/rational movement and detailed materialization controls are deferred. For T-lists the cursor is responsible for materialization to the requested level, using the item's wrkList, applicable ancestor listSpecs, correspondences and other available evidence. It can schedule work at the goal-queue level and use justified local strategies. This corrects the earlier example that made the cursor only find a reference and then called public `normalize()` to fill it in.

**Integration recommendation, not yet implemented.** TCursor is a resumable producer of temporal occurrences materialized to its current demand. A normalizer handling a T-list consumes/drives that producer. TCursor uses the same lower-level graph reads, correspondence, constraint merging and publication machinery as ordinary solving. When necessary, it submits a specific prerequisite to the existing work context and yields; the scheduler resumes its retained continuation. It does not call public `normalize()` or recursively drain a second queue. Ordinary non-temporal walks need not become TCursors; an SCursor or common base class is not part of this decision.

**Ownership and local work.** TCursor retains the occurrence, traversal/correspondence progress and materialization demand. WorldManager retains scheduling, dependency lifetime, cancellation and publication. Reuse the caller's work context when already inside normalization. A standalone cursor can attach a minimal execution capability when actual queued work is required; Agent/session lifetime remains external. Without that capability, expose the deferred work honestly rather than claiming materialization. The exact attachment API remains to be designed. Known structure and already justified values can be reused locally; inline deterministic merges must use the shared semantic primitives, with the same constraint and publication rules as queued work. Heuristics choose justified work or its order; they do not introduce different semantics or the deferred cost planner.

**Avoiding cycles and repeated work.** The active materialization demand has one retained continuation. Do not schedule a second whole-item normalization that immediately asks the same cursor for the same materialization. Request the missing premise, definition, correspondence or member constraint instead. Waiting retains the candidate occurrence and tried sources. Repeated driving of a pending step continues that step; moving elsewhere must not silently retarget its outstanding work. A proved boundary, unresolved extent, parsing wait, partial value and contradiction must remain distinguishable. Exact public status representation and movement behavior while work is pending remain API details to settle.

**Scope and spans.** Materialize what is needed at the requested level; do not fill all descendants or copy the previous state as an incidental consequence of movement. Preserve symbolic spans, hierarchy, occurrence context and correspondence where available. A possible TSpan can later consume aligned ranges or spans at different levels through the same machinery. This motivates avoiding an interface that only exposes flattened leaf values, but does not require a TSpan API now. The precise default meaning of level across nested T-lists remains to be specified before implementation. Existing temporal semantic questions are unchanged; this decision concerns how established relationships are executed.

**Cost and implementation starting points.** Known adjacent movement can reuse its position and structure; materialization costs the inspected relationships, newly required output and applicable inference. Whole spans should retain co-propagation rather than perform independent ancestor searches for every member. No general O(1) materialization claim follows from cursor movement. `WorldPosition`, `WorldWalk`, `WorldPath.cursor`, `cursorSource()` and `walkReady()` supply existing context, traversal and prerequisite-continuation mechanisms; `makeFrame()` and `WorldRequest.ready` are current internal scheduling machinery, not a proposed public API. Public `normalize()` currently constructs/registers a request and runs it, confirming that it is too high-level for a cursor prerequisite.

**Validation.** Design/documentation only. Read the current implementation and revised the consolidated cursor section to distinguish the user's requirements from the recommended integration. No source, tests or fixture expectations changed; the D063 runtime baseline remains current.

## D066 Reuse one shallow cursor infon; write back explicitly (2026-10-14)

**User clarification.** Initialization may allocate an empty infon shaped from the initial POV, for example a rubix-cube state. This same cursor infon always represents the current item. Its `mySymbol()` must not establish semantic identity. `goNext()` fills what is available at that level and applies the given changes, without recursively filling substructure. Other represented states stay as they are. Add `writeBack()` to materialize the cursor's current view. This supplies the initial level contract left open in D065; TSpan can address associated substructure levels later.

**Representation and scope.** Keep the reusable working infon separate from the current temporal POV/occurrence and its surrounding context. The initial POV determines the working shape and level; allocate or borrow the known main-level shape without expanding a complete object tree. Unknown shape can remain unresolved. Movement updates the working view at that level from the destination's supplied information, applicable relationships and already justified facts. Nested structures remain references or deferred descriptions; merely moving does not enumerate their members. `goPrev()` has the same scope and obtains evidence for its destination rather than assuming every forward change has an arithmetic inverse. Storage reuse does not by itself prove preservation, authorize an override, or resolve the deferred temporal semantic questions.

**Write-back interpretation.** Filling the working infon and storing it in the model are separate operations. `writeBack()` explicitly materializes the represented current occurrence through the existing checked publication machinery. It stores the available shallow result and retained constraints/references; it is not a request to fully normalize descendants. Later cursor movement must not mutate an earlier written result through the reusable buffer. Reuse immutable payloads and the existing safe publication/copying rules where possible. The returned value/status is an implementation detail; unresolved work and contradictions must remain visible, and failed publication remains atomic.

**Identity and continuation.** The buffer's address and debug symbol describe storage, not the temporal occurrence it currently represents. Preserve the real occurrence/context in traversal and prerequisite work. Completion facts from one position cannot become facts about another solely because the same buffer is reused. Existing `WorldPosition` occurrence/context handling is the starting point; no new global identity registry or cache-generation scheme is required by this decision. Pending work remains tied to its original position and continues at its waiting link. Moving to another temporal position is a new demand, not clearing and restarting that pending goal.

**Integration and cost.** Keep D065's shared lower-level solving: normalizer drives TCursor, which performs justified local work or requests a specific prerequisite in the caller's work context. Agent/session attachment stays optional until needed. One reusable current container avoids allocating a full state at every step. Initial shape discovery, inspected main-level relationships, changed slots, demanded inference and explicit write-back output still have their actual costs. Reuse established correspondences and borrowed substructure; do not rebuild the whole shape, scan the entire history, or independently search from the parent for each sibling on every step. Constant-time movement/materialization is not promised without those premises.

**Implementation evidence and future checks.** `WorldManager.dog` has no `mySymbol()` calls. `samePosition()` uses occurrence paths; request maps also use POV/infon pointers, so cursor integration must retain the occurrence distinction when using those maps. No identity mechanism is being changed in this documentation increment. Behavioral checks should cover stable cursor-infon storage across moves, distinct temporal occurrences, main-level filling without descendant traversal, unchanged source states before write-back, a written result surviving later cursor movement, and continuation at the correct position. Measure work against inspected shape and supplied changes rather than total nested object size.

**Validation.** Documentation-only clarification, checked against the user's requested model and the current occurrence/publication implementation. No engine, tests or expected outputs changed; D063 remains the runtime baseline.


## D067 Compute new values normally; retain unaffected cursor values (2026-10-14)

**User clarifications.** A face-turn rule supplies the changed cublets; the cursor does not proactively ask for more facts once that work has drained. Definitions and listSpecs can supply work, but the first implementation should get the wrkList path working. A transition can provide a changed value. The user corrected the proposed use of `=:` as replacement: the colon permits a partial match from the left/start. Finally: “The regular process will or should already get the new values. It just won't carry over the old ones.”

**Decision.** Use ordinary normalization for the next item's given constraints, and add only retention of unaffected previous values. Keep source occurrence, previous value and mutable output separate. Retention happens after given work drains and uses the existing correspondence/merge queue. It is an internal relation, not parsed `=`, `==`, `=:` or `==:`. New concrete values are not compared to old values merely because the cursor retains other fields. There is no transition-provenance/change-authorization subsystem. Explicit conflicting new constraints still reject atomically.

This narrows D062 for the cursor execution mode: its old-versus-new equality interpretation is not imposed on a computed transition result. Ordinary explicit identity constraints retain their meaning. `%?` still withholds retention throughout its scope. D066's “level” is semantic, not an arbitrary tree-depth cutoff: supplied work may produce nested changes; unrelated retained material is not recursively requested. No parser qualifier is changed. `:` remains `looseTop`; the existing `<<`/`forceID` behavior is unchanged.

**Why this representation.** A frozen prior view lets all rules in a step read the same inputs, including permutations. Staging keeps the visible cursor intact on wait, rejection or cancellation. The normalizer receives an occurrence context separately from source/destination storage; no fake prev/next/parent links are installed in the model. A shallow payload snapshot and owned immediate wrappers avoid copying all descendants or mutating source wrappers when the current view is reparented. Explicit write-back stores the available description, independent of later cursor movement.

**Shared machinery.** `solve()` is the common internal entry below public normalize. The same request owns ordinary rule work, the retention relation, subscriptions and atomic publication. `TCursor.adjacent()` is also used by ordinary relative navigation. An already proved scalar snapshot carries its completion witness, preventing each `%prev` expression from replaying earlier transition recipes. Successive steps retire the previous internal request when it is last in the dedicated cursor session; explicit write-back and other requests remain Agent-tracked, and caller-retained handles remain valid. Agent retains session ownership.

**Scope.** This milestone applies supplied wrkList work and definitions, including their required expression dependencies. It stops new ancestor-source discovery after that work. Ancestor listSpecs and model-specific primary-level materialization remain subsequent integration work. A bare parsed initial literal list is borrowed without evaluating descendants, so it can still contain unresolved child recipes. Immediate backward navigation uses the saved prior value; further backward movement uses available destination evidence, without a general history reconstruction or inverse solver. The interface exposes partial and unsupported outcomes rather than filling those gaps with the old engine.

**Alternatives discarded.** A fixed depth limit hid work required by nested rules. A special transition-authorization flag duplicated semantics already handled by normal solving. Interpreting `=:` as replacement changed language meaning. Copying previous values into the new constraints would assert unwanted equality and make permutations order-dependent. Re-running normalize for the whole prior history would violate the intended work bound. These alternatives are absent from the maintained implementation.

**Validation.** See `Docs/Design Docs/Historical/tcursor-work-2026-10-14.md` for regenerated builds, behavioral checks, scaling measurements and remaining boundaries. Slipstream rendering requires separate visible validation; engine checks do not establish that the real cube is rendered.

## D068 Materialize deferred definition fields in their occurrence (2026-10-14)

**Decision.** Acquiring a composite definition's record layout borrows its literal composite fields. An unbound field-name requirement adds no demand for that field's interior. When correspondence actually combines two such fields, the ordinary frame evaluates the deferred input and retains its list requirements. A requested inherited literal record still acquires its own definition; having a literal body is not proof that inherited constraints were applied. No subtree is declared complete merely because it was borrowed.

**Context and identity.** Published deferred fields retain their definition context. On demand, a direct definition field binds that context to a borrowed wrapper for its materialized parent occurrence. The same shared definition can therefore read different `%self` values in different instances, including fields supplied by the definition. Binding does not mutate the source definition, infer identity from storage, or store a parent/child ownership cycle in the published view. Traversal supplies membership for borrowed expression operands; their original physical parent need not equal the virtual expression parent. Record-field matching requires unique field identities; ordered repeated types remain distinct positions.

**Supplied work.** An explicit selection's input correspondence is part of executing supplied work. Suppressing unsolicited ancestor discovery must not suppress that input. The retained parent cursor consumes it once; there is no restart. When ancestor discovery is suppressed, an unknown named value can still request its own definition rather than repeatedly deferring to a parent it will never request. `materialize` exposes this execution option through the same solve path used by TCursor.

**Viewer integration.** Slipstream TimeView uses TCursor. Renderer demands publish into scene-owned occurrences and keep their handles across waits. Each demand is requested once per displayed state, and changing state closes that scene session. Rendering requests parts, poses and surface colors without demanding unrelated face-slot metadata. This is a consumer of the generic graph and materialization interface, not a model-specific solver.

**Cost and alternatives.** Borrowing does not visit composite descendants. Binding copies a position and parent wrapper on demand, not a containing subtree. Field uniqueness and correspondence scan their immediate lists. Retaining the same deferred value payload adds no information and returns the incoming description immediately. Fixed layout work remains proportional to inspected fields; explicit selection can still scan the relevant source and context depth. This makes no general linear claim for arbitrary patterns or repeated independent queries. Eagerly normalizing all definition descendants was rejected because unrelated metadata could prevent a valid view and multiply work. Dropping definition context or replacing unknown fields with renderer-specific facts was rejected because it loses supplied constraints.

**Validation.** Synthetic tests cover two differently colored instances, inherited defaults, repeated ordered types, deferred contradictory metadata, explicit rejection when that metadata is demanded, inherited composite conflicts, source preservation and state 2 retaining its coordinate. Final build and runtime evidence is recorded in `Docs/Design Docs/Historical/tcursor-viewer-2026-10-14.md`.

## D069 Fixed cube verbs are ordinary state-change definitions (2026-10-14)

**User decision.** A sequence such as `{T solvedState turn-red-face-clockwise turn-blue-face-counterclockwise}` describes successive states. Each verb evaluates the new changes; the cursor retains unaffected facts. Start with fixed, non-parameterized face names.

**Implementation.** Keep move geometry in the public model. Twelve ordinary definitions update nine named positions, reading their occupants and orientation indices from `%tprev`. An independent occupant scope prevents merging the new occupant with the former occupant of a destination. The existing cursor merge supplies the remaining positions and overall pose. A 24-entry orientation catalogue and per-turn permutations compose exact cube frames; this avoids incorrect Euler-component addition and numeric wrapping. Positions name places, not cubie identities. The render projection retains the existing parts/faces vocabulary and is supplied in each new state's definition, so deferred self references bind to the new occurrence.

**General prerequisites.** Cursor retention does not treat a closed previous record as a constraint excluding newly supplied fields; new explicit extents still constrain the resulting record. Temporal relative references climb structural ancestors to the nearest T-level, matching `CursorPathResolver.getTemporalRelativeAnchor` in the reference checkout. Ordinary `%prev` remains local. Computed index frames inherit the caller's navigation and template context. Finally, remove the old scalar-milestone rejection of multiple `addedTypes`: the current parser does not produce that representation; WorldManager produces checked nominal/unit evidence and already binds each qualification on read. A partial occurrence can retain these facts before its own meaning is demanded. Keep that meaning pending rather than falsely marking it applied. The new repeated-projection regression failed on the preceding engine and now succeeds.

**Alternatives and costs.** The older two-state turn relations remain available; they are not the verb API and their locally edited files are untouched. The new model uses named-field correspondence rather than repeated unstructured occupant searches for each destination. Each step has 26 inventory entries, nine supplied moves, and a bounded 24-entry orientation lookup. Model tests measure constant frame count per step over 8, 32 and 128 mixed turns, with one cursor request retained. This is linear in steps for this fixed-size cube, not a claim of constant work for arbitrary-width models. Renderer projection costs are separate and checked explicitly. Planning, path-cost calculation, ancestor temporal listSpec transitions, parameterized verbs and general backward reconstruction remain deferred.

**Validation.** See `Docs/Design Docs/Historical/rubix-verbs-2026-10-14.md` for regenerated-engine, geometry, regression and GUI evidence. Core tests use small synthetic definitions; model integration tests deliberately validate the public model and do not become engine fixtures.

## D070 Pragmatic normal form, explicit print modes, and cursor follow-up (2026-10-17)

**User decision.** Keep the current normal form until a concrete problem requires
changing it; generally this is the compressed stored representation. Storage and
printing are separate choices. Do not deepen template evaluation, expand large
listSpecs, or change wrkList merging as part of preparing the printer. Normalizing
templates before application is a possible optimization, and multiple or nested
templates should compose by ordinary wrkList merging; neither is implemented by
this printer change.

**Printer step.** Add `InfonPrintMode` with compressed as the default, expanded,
and debug. Pass the choice through recursive printing, including nested values,
work and paths. Debug retains the printer's existing size and retained-template
diagnostics. Expanded intentionally uses compressed output until cursor-based
expansion is ready. Keep the one-argument `toString` overload because CodeDog's
overload resolution needs that exact signature; the overload supplies compressed.
A seventh `.tst` column selects the print mode independently of execution mode.
No existing fixture expectations change. The two native retained-listSpec checks
now request debug explicitly instead of setting the removed mutable debug flag.

**Cursor audit of this checkout.** `TCursor` in `WorldManager.dog` already stages
ordinary solver work, preserves unaffected shallow values, keeps the visible
value intact during a wait or rejection, and implements explicit `writeBack()`
through `solve()`. `settle()` copies each result into its reusable `current` POV;
it does not return an existing materialized source directly. Movement follows
physical prev/next members and requires a temporal parent. Immediate backward
movement can reuse the saved previous value; general backward history
reconstruction, virtual sparse positions and a general S-list cursor are not
implemented by this class. `WorldWalk`, `WorldListCursor` and correspondence are
existing structural traversal machinery to reuse, rather than duplicating list
interpretation for printing.

The live request already retains its continuation when a particular parser input
is unfinished, and `inputReady()` resumes it. That is narrower than a general
wait-for-member facility: a missing successor returns partial, and the cursor has
no subscription for that successor's eventual insertion. There is no member-load
priority API. `WorldRequest.waitForCompletion()` waits for active work under the
model lock; it can return with the request still waiting. No list-backed automatic
cursor cache or cache-clear policy is implemented here.

**Updated direction.** This revises D066's requirement that the reusable working
infon always be the exposed result. When a materialized occurrence exists, return
that occurrence directly; otherwise use the cursor's working infon. Cache values
by publishing into the list through ordinary inference, with explicit provenance
identifying entries that may be cleared. Explicit `writeBack()` makes that item
persistent. Clearing must preserve supplied or subsequently asserted facts,
constraints and occurrence identity. Start with a simple explicit clearing policy;
no eviction budget is chosen yet.

**Suggested sequence.** First implement returning existing occurrences together
with list-backed cache ownership, reversible clearing and writeBack promotion.
Verify multiple forward/backward steps, independent occurrences, asserted facts,
and cancellation. Then expose member readiness using the existing request and
continuation machinery, including missing members, and let demanded fields carry
priority when a loader can honor it. Next unify virtual T-list/S-list navigation
through structural traversal before enabling bounded expanded printing. Measure
template pre-normalization only when repeated application supplies a concrete
optimization case; occurrence-dependent expressions must keep their context.
These are follow-up steps, not changes to the normalizer in this commit.

**Alternatives.** A detached value cache would bypass normal inference and require
separate validity rules. Eagerly materializing a whole repeated list would defeat
compressed storage. Requiring the same returned address forever would prevent
using an existing stored occurrence directly. Those approaches are not adopted.

**Validation.** Regenerated with `codeDog Proteus.Lib.dog`; build successful.
The ten new print-mode fixtures pass in one process, including a default-mode
case after debug output. All 57 WorldManager suites pass, including all 273
TCursor checks and the retained-listSpec checks. Default fixtures remain at
16 failures out of 332; the full active suite remains at 24 failures out of 535.
WorldManager, cursor, default and full-suite logs are byte-identical before and
after the change. All 15 protected fixture files and the user's `myTests.tst`
are byte-for-byte unchanged. No cursor behavior or GUI behavior was changed.


## D071 Return stored cursor occurrences, clear disposable values, replay history (2026-10-17)

**User decision.** Implement the first three cursor follow-ups in Proteus4-redesign:
return materialized occurrences directly, cache in the list with explicit permanent
write-back, and support repeated backward movement. Keep the pragmatic stored normal
form and existing printer modes. General member waiting/priority, virtual sparse
T/S navigation and expanded printing remain subsequent work.

**Storage and lifetime.** `current` is the represented list POV on a hit or successful
writable step. `working` is the read-only fallback. A cache record belongs to that
occurrence, points to its current infon/revision, and retains its original description
for clearing. Computed values live only in the list. A shared use token protects each
live cursor's current entry; destruction releases it without retaining the cursor.
`clearCache()` restores unused disposable descriptions. Explicit `writeBack()`, a
successful ordinary assertion into the value, or new unprocessed work makes it
permanent. Dirty or unrecognized changes are conservatively retained. No automatic
eviction or size budget is introduced.

**Inference and sharing.** Cache misses use the existing solver and publication.
An unchanged writeBack still performs checked publication and promotes the item.
Ordinary selected writes into a borrowed nested value prepare private headers and
copy only the represented lists on that write path. Untouched descendants stay
shared. All validation precedes attaching the private branch. Publishing against
the real destination keeps parent links and identity witnesses valid. This is
necessary because shallow retention deliberately shares infon headers and child
storage; merely marking the root cache permanent would allow a nested assertion to
mutate an earlier state. Rejected writes neither change that state nor pin the cache.

**Navigation.** Cache hits expose the existing POV and require no request. A miss
scans backward to the nearest retained state and replays forward once, caching the
successful prefix. Suspended replay retains its current request and keeps the old
visible value until the destination is ready. No inverse transition is inferred.
Forward and backward cached movement no longer depends on a single saved predecessor.
Slipstream's scene index now follows the returned occurrence when its timeline moves.

**Costs and boundaries.** Clearing and locating a missing predecessor each scan the
represented list/prefix once. Replay uses ordinary per-step inference; fixed-width
prefixes are checked through 128 steps. Writing a shared branch copies each demanded
containing level once per transaction, rather than expanding unrelated descendants.
Physical temporal neighbours remain the navigation boundary. This does not implement
virtual positions in compressed listSpecs, a general S-list cursor, general missing-
member subscriptions, loader priorities or automatic invalidation of every derived
fact when arbitrary external storage is changed. Mutations must use normal inference
and normalization-work tracking. Existing unsupported sparse/ambiguous correspondence
remains unsupported; no fallback solver is introduced.

**Validation.** Regenerated `codeDog Proteus.Lib.dog` (215.82 s),
`codeDog BuildProteusGraph.dog` (142.17 s), and the linked
`codeDog Slipstream.dog` (108.28 s) successfully. All 58 WorldManager suites pass,
including the 273 existing cursor checks and 464 cache checks. The latter cover
shared cursors, unchanged and changed write-back, exact source restoration,
borrowed scalar/nested/composite writes, parent links, rejection, read-only views,
independent occurrences, pending/cancelled replay and cleared prefixes through 128
states. All ten print-mode fixtures pass. The full active suite reports 24 failures
out of 536 registrations; all failing names and diagnostics exactly match the
24/535 baseline. The 15 protected fixtures, print-mode fixture and user's
`myTests.tst` are unchanged.

The maintained World Rubix driver now checks this stored-occurrence contract in
place of its obsolete reusable-address/source-list expectations. The independent
geometry oracle is unchanged: all 12 verbs, 206 turns, inverse/four-turn identities,
78 projected cubies and 468 surfaces pass. Each of its 16 sequences also verifies
backward/forward cache hits and a full reconstructed state after clearing the
prefix. Mixed sequences of 8, 32 and 128 turns retain the same 536–554 frames per
new step; hits create no request. Shared definitions and original recipes remain
unchanged. No World or Slipstream model resource was edited.

On a private Xvfb display, the rebuilt Slipstream renders a colored cube and accepts
three Next clicks followed by three Previous clicks. Every move succeeds; backward
steps return the earlier occurrence pointers with a null request. Screenshots
confirm that the cube remains rendered. Existing preview rotation remains active,
so pixel equality across wall-clock times is not a state oracle. The two existing
world-loading NRMERR messages remain; no DISPLAY failure occurs. Evidence is in
`/tmp/proteus4-cursor-cache/` (`world-final.log`, `full-final.log`,
`model-final.log`, build logs and `gui/`).

## D072 Retire native test scaffolding and remove the failed checkout (2026-10-17)

**Request and scope.** The user retired the failed Proteus4 fork and requested
cleanup of Proteus4-redesign, especially verbatim test implementations. The scope
is tests; production native bindings are reserved for a later review.

**Decision.** Remove all native C++ test bodies from ProteusTests.dog, their 140
compiled registrations, and the 14 standalone drivers for the removed graph
engine. This includes 57 WorldManager suites and 83 graph compatibility wrappers.
Retain 64 ordinary CodeDog checks: the 58 historical parser/model checks, five
plain string assertions and incomplete-RHS parsing. CodeDog macro expansions
remain CodeDog even though their quoting syntax also uses `<% ... %>`.
All 16 tracked .tst files and the user's untracked myTests.tst are unchanged.
Future tests use ordinary CodeDog or language fixtures. This supersedes earlier
instructions to retain the native suites or migrate their implementation APIs.
Retirement reduces test coverage; it is not evidence that those behaviors pass.

**Provenance.** The active inventory lists current registrations only. The
retirement manifest records removed names, files and protected fixture hashes.
The full pre-cleanup source and inventory remain at `bd7078b`; the failed engine
remains at ancestor `b52322a`. Historical reports are retained as evidence, not
current build instructions. No engine or language implementation was changed. The standalone streaming
test retains its checks; its one native lock adapter now uses the existing
WorldModelLock binding from ordinary CodeDog.

**Repository cleanup.** Proteus4-redesign formerly used Git metadata stored in
Proteus4/.git. Copy and verify that metadata as an independent repository before
removing the failed checkout and merged codex/proteus4 branch. Preserve the
redesign branch, other refs, untracked myTests.tst and the external World resource
symlink. The original Proteus repository remains untouched. Remove Slipstream's
unused GraphFoundation proxy into the deleted checkout; its current engine
interface already targets Proteus4-redesign.

**Validation.** `codeDog Proteus.Lib.dog` regenerated and compiled successfully.
The complete retained suite reports 24 failures out of 396 registrations versus
24/536 before cleanup. Every failure name and diagnostic is identical. All ten
print-mode fixtures pass. All 16 tracked .tst files and untracked myTests.tst
retain their pre-cleanup hashes; implementation source is unchanged. Git fsck
passes in the independent repository after removing the failed checkout and
merged branch. The obsolete WorldManagerBuild outputs were also removed.
`codeDog streamReplyTest.dog` also regenerated and compiled successfully. Its
read adapter holds Nested_Lock_Mutex over the same recursive mutex; standalone
streaming behavior was not rerun. After deleting the failed checkout, `ldd -r`
on Slipstream resolves the redesign shared library without missing dependencies
or undefined symbols. No GUI runtime claim is made by this test-only cleanup.

## D073 Suspend the parser lock at the actual input wait (2026-10-17)

**Request.** Remove the failed fork's per-operation input wrappers while retaining
safe streaming. The user approved a shared input-wait facility, correct lock
ordering on wake, and explicit progress opportunities during buffered parsing.

**Decision.** The parser owns a scoped `InputLock` supplied by CodeDog's C++ thread
runtime (CodeDog commit `e3dec0e` or later). All 15 input adapters now call the ordinary buffer/iterator methods
directly; `pauseInput` and `inputMayBlock` are removed. The one actual blocking
buffer loop uses `waitForInput`. The buffer knows nothing about WorldManager.
A thread-local scope chain identifies owned locks, including nested registered
recursive acquisitions. Other readers cannot release this parser's lock.

The wait suspends the registered model locks while retaining the buffer lock
until the condition-variable wait atomically releases it. On wake it releases
the buffer lock, restores model ownership, then retakes the buffer lock and
rechecks the predicate. Exception unwinding releases the buffer before restoring
the model lock too. This prevents inversion against model-then-buffer users.
All recursive acquisitions spanning input waits must use InputLock; arbitrary
raw/protect acquisitions cannot be discovered or fully released by a scope.

`waitingForBuffer` only inspects state, so it uses ordinary CodeDog `protect` and
never suspends model ownership. Host syntax-error callbacks use a lexical
`InputPause` with exception-safe restoration. The existing native `doParse`
exception/lifetime boundary remains; unrelated verbatim blocks are outside this
change. The recursive-mutex support in `protect` alone cannot substitute for
suspending an owned outer lock during a condition-variable wait.

After an item or list prefix is consistently published, the parser calls
`yieldInputLock`. After at least five milliseconds since the previous yield, the runtime suspends the
registered locks for one millisecond, providing a scheduling opportunity without
per-character unlock/relock. This is cooperative progress at safe boundaries,
not a real-time bound for a single large token or solver computation. Deferred
prefix publication retains its existing semantics.

**Tests.** Four new ordinary CodeDog tests cover another query completing before
EOF, another query completing before a 10,000-item buffered parse finishes,
a syntax-error callback waiting for another model-using thread, and the real
InfonStream cancellation-before-close shutdown sequence. The progress tests use
nested registered acquisitions. CodeDog's native runtime tests separately force
a model/buffer wake-up race and verify exception restoration, nested callbacks,
and ordinary waits without registration. No native assertion bodies were added
to the Proteus tests. Existing language fixtures remain unchanged.

**Validation.** Regenerated and compiled `codeDog Proteus.Lib.dog` (156.16 s),
`codeDog BuildProteusGraph.dog` (142.23 s), and linked `codeDog Slipstream.dog`
(104.43 s). The four new tests pass, including ten consecutive scheduling runs.
The full retained suite reports 24 failures out of 400 tests; all failing names
and diagnostics exactly match the 24/396 cleanup baseline. All ten print-mode
fixtures pass. All 16 tracked .tst fixtures and untracked myTests.tst retain their
pre-cleanup hashes. The CodeDog runtime checks report nine passes and one skipped
toolchain test, including a generated ordinary CodeDog producer/consumer test
that releases nested scopes through strBuf. Its library-manager test also passes.
`ldd -r` resolves the rebuilt Slipstream to the redesign shared library without
missing dependencies or undefined symbols. No GUI behavior was changed or
visually retested. Evidence is in `/tmp/proteus-input-lock-fix/`, including the
build logs, focused and repeated concurrency runs, `full.log`, `print-modes.log`,
`runtime-all.log`, `runtime-generated.log` and `validation.json`.

## D074 Use UInt64Parser for null identities and review document completion (2026-10-17)

**Decision.** `parseNullIdentity` now uses CodeDog's `UInt64Parser` in ordinary
CodeDog. Keep the identity scope's positive int64 domain, including its exact
range errors and source positions. Unlabelled `NULL:` remains valid; leading
zeros are accepted. Values above INT64_MAX are rejected even when UInt64Parser
can represent them. Conversion overflow is reported as a parser error.

**History and lifecycle review.** Commit `d4f3bf487` introduced the native
`doParse` wrapper and renamed its previous body to `parseDocument`. That body
is still identical to both its pre-wrapper version and the original Proteus
checkout's `doParse`. The current WorldManager ownership, root-completion guard
and error-before-completion ordering came from redesign commit `82378ee1`.
The old GraphPublication mechanism is gone.

Keep the parsing body in CodeDog and retain the current lock and exception
boundary. WorldManager resumes waiting requests synchronously in `inputReady`,
so a root must not complete before trailing syntax checks and error publication.
The old bare parsing body cannot replace that lifecycle. CodeDog currently has
no try/catch action syntax, so exception notification still needs a native
boundary. D073 supplies lock suspension and restoration at actual input waits.

The current split is useful, but its bookkeeping can be simplified in a later
cleanup: `parseDocument`'s final streaming `parseDone` notification is suppressed
by the root guard and is redundant. Success/failure completion and context cleanup
could be consolidated into ordinary CodeDog helpers while keeping a small native
exception boundary. No production lifecycle behavior is changed by this review.

**Tests.** Two ordinary CodeDog tests exercise accepted identities (including
INT64_MAX and leading zeros), zero/negative IDs, values above INT64_MAX, uint64
overflow, and unchanged error locations. A third test starts a waiting normalize
request, parses `1#3`, and verifies that the request receives the trailing syntax
error when the root completes. The four D073 input-lock regressions also pass.

**Validation.** `codeDog Proteus.Lib.dog` regenerated and compiled successfully
(140.45 s), as did `codeDog BuildProteusGraph.dog` (132.54 s). All seven focused
parser checks pass. The complete suite reports 24 failures out of 403 tests;
every failure name and diagnostic matches the prior 24/400 baseline. All 16
tracked .tst fixtures retain their protected hashes, and the active inventory
matches all 71 compiled registrations. `ldd -r` resolves Slipstream to the rebuilt
redesign library without missing dependencies or undefined symbols. GUI behavior
was not retested. Evidence is in `/tmp/proteus-identity-parser-review/`, including
`history.json`, the focused logs, `full.log`, `validation.json` and build logs.

## D075 Simplify parser entry and permit later input (2026-10-17)

**Decision.** The user does not require a migration diagnostic for the retired
`#` indexing spelling. Parsing one infon must permit later input, including input
from a stream; it must not require the entire buffer to be exhausted. Remove the
trailing-input check entirely, with no replacement end-of-document validation.
`@[...]` remains the indexing syntax, and `#` still introduces list definitions.

Remove `parseDocument` and its redundant streaming `parseDone` notification.
Its two remaining calls, `clearError` and `parseInfonView`, run directly inside
`doParse`'s existing try block. The native call supplies the same parse context
and notification arguments previously supplied by CodeDog's default-argument
expansion. `doParse` retains responsibility for publishing success or failure
and final completion.
This supersedes D074's recommendation to retain the separate helper and its
trailing-validation rationale. The lock, actual parse-error publication and
exception responsibilities still apply.

Replace the dedicated legacy-index test with ordinary CodeDog checks that later
input is accepted and the parsed infon completes in both parser modes. Change
D074's waiting-request test to use an incomplete RHS (`1 =`), an actual syntax
error within the infon. The compiled test count remains 71. Existing .tst fixtures
and expectations are unchanged.

**Validation.** Regenerated and compiled the final sources with
`codeDog Proteus.Lib.dog` (148.68 s) and `codeDog BuildProteusGraph.dog` (141.58 s).
All 26 focused parsing, input-lock, identity and completion checks pass. The full
suite reports 24 failures out of 403 tests, with every failing name and diagnostic
identical to the prior baseline. All 16 tracked .tst fixtures retain their hashes,
and the inventory matches all 71 compiled registrations. Removing the exported
helper changes the library interface fingerprint. Regenerated and relinked
Slipstream with `codeDog Slipstream.dog` (92.98 s); `ldd -r` then reports no
missing dependencies or undefined symbols.
GUI behavior was not retested. Final evidence is in
`/tmp/proteus-document-syntax-cleanup/`: `build-tests.log`, `build-shared.log`,
`parse.log`, `lifecycle.log`, `input.log`, `identity.log`, `full.log`, and
`validation.json`, `build-slipstream.log` and `linked-library.log`. Superseded
build logs there are not validation of this change.

## D076 Checked decimal input and ordinary CodeDog parser entry (2026-10-17)

**Evidence.** Before removing the native catch, parsing `1/` reproduced
`std::invalid_argument` from GMP's `mpq_set_str`; the same input followed by a
newline parsed the valid prefix `1`. `strBuf.scrapeFlexNum` removed an unfinished
separator at a delimiter but retained it at EOF. The string assignment also
used auto-base conversion: `08` and `08/09` threw, and `010` parsed as `8`.
These are numeric parser defects, not reasons to retain the exception wrapper.

**Decision.** The user chose to fix the throwing numeric path before converting
`doParse`. CodeDog's numeric scanner now leaves a trailing dot or slash outside
the numeric prefix at EOF, matching its existing delimiter behavior. Add
`FlexNum.parseDecimal`, an ordinary CodeDog checked base-10 conversion using the
existing nonthrowing BigFrac conversion. It accepts signed integers, fractions
and decimals, rejects invalid tokens and zero denominators, and preserves the
previous value on failure. Existing `fromString` behavior for other clients is
unchanged. Proteus uses the checked conversion for decimal tokens and reports
failure through `setError` at the token's start; binary/hexadecimal parsing and
null identity ranges are unchanged.

Convert `doParse` to ordinary CodeDog with a nullable `InputLock`. Preserve the
root-completion guard, prefix acceptance, error publication before final
completion, and scoped lock release. Remove the unused `failDocument` helper
and the catch that marked an unexpected exception then rethrew it. This
supersedes D074/D075's retained exception-boundary recommendation. Unexpected
native failures still propagate; there is no generic failure publication on
that path. The lock's destructor still releases ownership. Explicit identity
scope exhaustion and runtime/callback failures are not claimed impossible.

Add ordinary CodeDog checks for the reproduced numeric cases, valid arbitrary
precision and zero-denominator syntax errors; extend prefix checks with `1/`
at EOF and before a newline. CodeDog's generated runtime regression covers the
conversion's failure contract and numeric prefix lengths. No native test
assertions or tracked .tst fixture changes are introduced. The inventory now
contains 72 compiled checks. Only three macro definitions in infonIO.dog still
use `<% ... %>`; they expand to ordinary CodeDog.

**Validation.** CodeDog dependency commit `2a6c20e` contains the numeric scanner,
checked conversion and generated CodeDog regression. Its runtime suite passes
10 tests with one unavailable-toolchain skip. Regenerated and compiled
`codeDog Proteus.Lib.dog` (154.94 s) and `codeDog BuildProteusGraph.dog`
(143.73 s). All 27 focused parser, input-lock, identity and completion checks
pass. The full suite reports 24 failures out of 404 tests; every failing name
and diagnostic matches the prior 24/403 baseline. All 16 tracked .tst fixtures
retain their hashes and the inventory matches all 72 compiled registrations.
Regenerated and relinked Slipstream (99.41 s) for the changed exported interface;
`ldd -r` resolves the rebuilt redesign library without missing dependencies or
undefined symbols. GUI behavior was not retested. Evidence is in
`/tmp/proteus-codedog-parser/`: the pre-fix exception and decimal probe logs,
`runtime-decimal.log`, `runtime-all.log`, all three build logs, `parse.log`,
`lifecycle.log`, `input.log`, `identity.log`, `full.log`, `validation.json` and
`linked-library.log`.

## D077 Remove obsolete native identity helpers and inline origin publication (2026-10-17)

**Decision.** After reviewing the 15 remaining native C++ uses outside infonIO,
the user approved cleaning up the obvious remnants first and reviewing the rest.
Defer large Proteus and shared-library builds, and any Slipstream rebuild, until
this native-block cleanup is finished.

Remove `IdentityRegistry.bindOccurrenceName` and its otherwise unused
`namedOccurrences` map. The only former production caller was in the retired
GraphFoundation; neither the replacement nor Slipstream calls the function.
Remove `infonView.hasRHS`, replacing its three callers with the existing
`workListContainsInfonView` check. Its extra registry scan only consulted
`IdentityRelation.leftWorkOwner`/`rightWorkOwner`; the replacement never assigns
those fields. Remove the fields and their obsolete consolidation comment too.
Ordinary identity relationships and current work-list duplicate checks remain.

Inline `parentAgent!.worldManager.setOrigin(toFill, originID)` in stream
initialization and remove the C++ `publishOrigin` wrapper. Preserve its position
before worker startup and the WorldManager lock; do not replace it with an
unprotected field assignment or rely on later parser initialization.

Twelve native uses remain: five identity operations, five stream-loading
operations, the POV self-identity comparison, and the cursor shared-use check.
Independent identity imports remain an optional parse feature with no current
callers enabling it; retiring that option is separate from removing unreachable
binding code. The stream workers and cursor cache lifetime behavior serve the
replacement engine and are not discarded because they use native code.

**Validation for this pass.** The CodeDog parser accepted Proteus.Lib.dog and all
its included sources (49.98 s), with no code generation or compilation. Source
checks confirm that all six retired names are absent from tracked .dog files,
12 native uses remain, and the unchanged inventory matches all 72 compiled test
registrations. `git diff --check` passes. No tests or language fixtures changed.
Runtime tests and export/consumer regeneration are deferred with the large builds
at the user's request; existing binaries do not validate this source revision.
Evidence: `/tmp/proteus-native-cleanup/syntax-check.log`.

## D078 Convert identity comparisons and consolidate the identity allocator (2026-10-17)

**Decision.** Implement the three follow-ups approved after D077. Express
`POV.isThisPOV` with ordinary CodeDog pointer identity. Convert
`IdentityRegistry.areIdentical` to ordinary CodeDog, extracting its existing
local equality predicate into `sameOccurrenceIdentity`. Keep direct POV identity
and shared scoped token identity distinct from equal numeric IDs or equal values.
Preserve null rejection at the public query, traversal of identity edges in both
directions, the pending stack, pointer-based visited list, and the registry lock.
No graph traversal or matching semantics are changed intentionally.

Move the unchanged native allocation body into `freshIdentity` and remove
`allocate`. The forwarding wrapper existed because CodeDog rejects a call named
`allocate`; there is now one entry point. Keep the scope lock, ID allocation and
registration, and exhaustion exception exactly as before. This step does not
change identity error policy or the optional independent-import feature.

Nine native uses remain: `claim`, `freshIdentity`, `importIdentity`, the five
stream-loading operations, and the cursor shared-use check. Large builds and
Slipstream regeneration remain deferred until the full native-block review is
finished, as requested under D077.

**Validation for this pass.** A small standalone CodeDog program extracts the
exact edited methods plus their identity types and dependencies into minimal
containing types. It generates and compiles successfully and passes 24 ordinary
CodeDog checks: null handling, self comparison, shared tokens versus distinct
tokens with equal IDs, symmetric/transitive edges, token aliases through edges,
cycle termination, repeated calls after traversal, and allocation/registration
around claimed IDs. The renamed allocator body is byte-for-byte identical to
`allocate` in `e5fc512`, including its overflow behavior. Generated code confirms
pointer comparison and scoped mutex ownership. The first isolated compile caught
a pointer-list literal generation limitation; initialization now uses the normal
append operation. No generator change was needed.

`git diff --check` passes and the source inventory confirms nine native uses.
No maintained test registrations or language fixtures changed. No full engine,
shared-library or Slipstream rebuild was run; integration and full runtime checks
remain deferred under D077. Evidence and the repeatable extraction/build check
are in `/tmp/proteus-identity-cleanup/verify.py`, `identityProbe.dog`, `build.log`
and `run.log`. Test assertions in that probe are ordinary CodeDog.


## D079 Express the cursor shared-use check through CodeDog (2026-10-17)

**Decision.** Retain the cursor-cache protection accepted in D071 and replace
`WorldCursorValue.inUse` with ordinary CodeDog:
`return(sharedUseCount(use)>1)`. A cache entry owns one use token reference;
current cursors own additional references. Moving or destroying a cursor releases
its reference automatically. Keep the existing model lock, token ownership,
cache clearing rules, and permanent-value policy unchanged.

Add `sharedUseCount(value)` as a CodeDog ownership operation. It requires one
`our` argument and returns `int64`. The C++ backend inspects the pointer directly
with `use_count()`, without copying it or adding an inspection reference. The
expression is evaluated once at runtime. Empty typed pointers return zero;
other ownership types, bare NULL, and unsupported backends are rejected during
CodeDog generation. Reference counts do not replace the model lock. Do not add
manual counters and cursor destruction hooks just to remove a native expression.

The user also chose to retain `independentIdentities=true` and its consistent
remapping for independent inputs. Its current lack of callers does not authorize
removing that feature. No identity parser or remapping code changes in this pass.

**Validation.** All 27 focused CodeDog shared-ownership and explicit-dereference
unit tests pass, including a generated ordinary-CodeDog program with 14 lifetime
checks. These cover empty pointers, aliases, nested fields, container elements,
holder destruction, scope exit, and evaluating an argument once. Diagnostics
cover argument count, ownership types, and unsupported backends.

A second small program extracts the exact production `WorldCursorValue` type,
`cursorValueReady`, `selectValue`, and `clearCursorCache` methods into minimal
containing types. Its 15 ordinary-CodeDog checks pass for multiple live cursors,
movement, destruction, scope exit, restoring original descriptions, and retaining
permanent or changed values. Evidence: `/tmp/proteus-cursor-use-check.py` and
`/tmp/proteus-cursor-use-check/{cursorUseProbe.dog,build.log,run.log}`; CodeDog test
output is `/tmp/codedog-shared-use-tests.log`.

No native function bodies remain in WorldManager.dog. Eight remain elsewhere:
three identity operations and five stream-loading operations. No maintained
Proteus tests or .tst fixtures changed. `git diff --check` passes. Full Proteus,
shared-library, and Slipstream rebuilds remain deferred under D077; the isolated
checks do not establish full engine or consumer integration.


## D080 Convert identity scopes and use standard file open modes (2026-10-17)

**Decision.** Convert `InfonIdentityScope.claim`, `freshIdentity`, and
`importIdentity` together to ordinary CodeDog. Keep positive-int64 validation,
identity registration, token-based independent-import remapping, null imports,
reuse of local/imported tokens, and exhaustion exceptions. `protect(scopeMutex)`
retains locking and release on exceptions. Both allocation paths share
`freshIdentityLocked`; its callers already hold the nonrecursive mutex, so it
must not acquire that mutex again. Keep the existing order of checks: reusing a
known identity remains possible after new-ID capacity is exhausted.

CodeDog exposes `throwInvalidArgument` and `throwOverflowError` through its public
runtime API, with C++ bindings to the existing standard exception types. Preserve
the messages and stack unwinding instead of changing these methods to return NULL
or terminate on errors. This does not introduce general CodeDog try/catch syntax.
The isolated compile also exposed the missing `<climits>` include required by
CodeDog's existing `INT64_MAX` binding; its C++ runtime now includes that header.

**File streams.** Add the standard mode-string overload `FileStream.open(path,
openMode)`, with C++ support for `r`, `w`, `a`, their `+` variants, and binary `b`
modifiers in either order. Read mode requires an existing file without requiring
write access; write mode creates/truncates; append mode writes at the end. Invalid
mode strings set stream failure without opening or truncating the file. Keep
one-argument open, openNew, and openAppend behavior unchanged. The non-C++
backends explicitly reject the new overload as unsupported while retaining their
existing methods; this pass does not implement their incomplete file I/O APIs.

Replace AssetFileConnector's native `openForReading` wrapper with the direct
CodeDog call `dataFile.open(filePath, "r")`. Normal file-open failures continue
through the connector's existing failure/status handling.

**Validation.** Twenty ordinary-CodeDog checks pass against the extracted exact
identity scope: claims, fresh allocation, local and independent imports, colliding
numeric IDs from different scopes, consistent remapping, empty slots, and the
last usable ID. A small native caller of the generated methods checks ten error
and concurrency outcomes, including exception types/messages, unchanged state
on validation/exhaustion failures, lock release, and four concurrent workers
claiming/importing/allocating without duplicates or lost registrations. No native
assertion body was added to a .dog test.

Fourteen focused CodeDog tests pass (stream modes, existing stream writes,
standard-error bindings, and library API visibility). The file program exercises
all fifteen supported mode spellings, missing and read-only files, invalid modes,
content preservation/truncation/append, reopening after EOF, and single evaluation
of arguments. The exception test passed on its focused rerun after correcting a
test-only C++ field/type name collision. Evidence: `/tmp/identity-scope-check.py`,
`/tmp/proteus-identity-scope-check/`, `/tmp/codedog-identity-file-tests.log`, and
`/tmp/codedog-standard-errors-test.log`.

Four native blocks remain, all in InfonStreamLoader.dog: the input worker, open
worker, requestParsedInput, and startConnection. No maintained Proteus test or
language fixture changed. `git diff --check` passes. Full Proteus, shared-library,
and Slipstream rebuilds and integration validation remain deferred under D077.


## D081 Express stream exception cleanup in CodeDog (2026-10-17)

**Decision.** Use the new CodeDog try/catch support for the four remaining native
function bodies: `ThreadedStreamInput.run`, `ThreadedStreamOpen.run`,
`InfonStreamLoader.requestParsedInput`, and `startConnection`. The requested
read-only file open was already inline as `dataFile.open(filePath, "r")` under
D080; no additional wrapper remains there.

Keep both startup counts (queued open worker and connection creation), origin
allocation, relative path resolution, parse-only mode, registration order, and
cleanup of partially started streams. Release each startup count after success,
rejection, or an exception, including exceptions raised during cleanup or error
reporting. Connector/parser/stream locals unwind before releasing the connection
startup count. Keep the manager mutex out of connector creation and stream
startup. Register a pending job before incrementing the startup count so an
allocation failure cannot leave shutdown waiting on a nonexistent worker.

Add `rethrow()` to CodeDog because cleanup must preserve the original exception,
not swallow it or replace it with a message-only error. It is a standalone action
with no arguments, valid only within a catch block. C++ uses native rethrow;
Java, Kotlin, and Swift throw the original hidden exception binding. Nested
handlers retain their own bindings, and changing the visible message does not
change the exception. Native binding names remain collision-free. Document and
test the operation rather than retaining native cleanup wrappers in Proteus.

CodeDog's general catch also handles nonstandard C++ exceptions. Consequently,
the input and open workers now report these through their existing error paths
with `Unknown native exception`, whereas their former std::exception-only
handlers let them escape. Existing standard exception messages and cleanup
ordering remain intact. Typed catches and finally remain unsupported.

**Validation.** Thirteen focused CodeDog tests pass, including compiling and
running the portable exception/rethrow program on C++, Java, Kotlin, and Swift.
They cover nested and unnamed handlers, catch scope and invalid arguments,
message mutation, original C++ exception object/type preservation, nonstandard
thrown values, and exception-safe lock release. The tightened C++ object-identity
check also passes separately. Evidence: `/tmp/codedog-rethrow-tests-final.log`
and `/tmp/codedog-rethrow-identity-test.log`.

Thirty-six ordinary-CodeDog checks pass using the exact four production method
bodies in an isolated program, with deterministic stand-ins for thread start,
connectors, parser, and stream lifecycle. They cover successful and rejected
starts, parent origins, parse-only mode, nested startup counts, failed connector
creation/init/start, finish/join exceptions, failed job registration, exceptions
from error reporting, and input-pump errors. Evidence:
`/tmp/proteus-stream-exceptions-check.py` and
`/tmp/proteus-stream-exceptions-check/{streamExceptionsProbe.dog,build.log,run.log}`.
These checks do not establish real worker scheduling or full engine integration.

No native C++ function bodies remain in the maintained Proteus .dog sources;
CodeDog macro definitions still use the same delimiters. No maintained Proteus
test or .tst fixture changed. Full Proteus, shared-library, and Slipstream
rebuilds and integration validation remain deferred under D077.


## D082 Finish parser failure and thread completion in CodeDog (2026-10-17)

**Decision.** Restore an unexpected-exception boundary in `doParse`, using
CodeDog try/catch and rethrow. D076 fixed ordinary decimal input failures but did
not make identity imports, callbacks, allocations, or publication nonthrowing.
The user approved cleanup and failure publication at the parser boundary, plus
containment at the native parser-thread entry point. This supersedes D076's
choice to propagate unexpected failures without document failure publication.

`doParse` retains the registered InputLock across parsing and failure publication.
On an exception, set finishingDocument, clear documentRoot/worldManager, mark
the document parseDone with its graphParseError, and notify inputReady when a
manager exists. Do not invoke successful-parse completion on this path: that
would also resolve end-relative references, which can itself have failed. Do not
synthesize a syntax error or invoke syntax-error callbacks. A secondary failure
while resuming input waiters is contained so rethrow preserves the original
exception. That notification is an attempt, not a guarantee of recovery from a
failing continuation. An empty exception message gets a nonempty document error
(`Unexpected parser exception`), since an empty graphParseError denotes no error;
the original rethrown exception is unchanged.

`ThreadedInfonParser.run` catches runtime failures and reports a system error
through its connector after the parser's model-lock scope has unwound. Contain
exceptions from that report as well: they must not escape the native thread or
skip completion signaling. The original document error remains available even
if the host cannot accept the report. Set parseCompleted and notifyAll under
chkParseDone once, after either outcome. Preserve the normal syntax-error path;
multiple completion waiters can now all finish. No new native bodies or wrapper
functions are introduced.

**Validation.** Before editing, an isolated probe using the exact production
doParse/notification methods reproduced missing terminal state, retained parser
references, missing runtime error text, and skipped failure publication after
injected exceptions. The final probe passes 157 ordinary-CodeDog checks. It uses
the exact ThreadedInfonParser implementation and actual CodeDog/C++ thread,
condition-variable, and input-lock bindings, with small parser/host/model stand-ins
for deterministic faults. Cases cover success, ordinary syntax-error status,
parse/host-callback/completion/publication exceptions, empty exception messages,
preserving the original error when failure publication also throws, clearing
temporary state, parser reuse, and model-lock release. Real parser threads with
two waiting callers finish even when syntax or system error reporting throws.

Evidence: `/tmp/proteus-parser-exception-check.py` and
`/tmp/proteus-parser-exception-check/{parserExceptionProbe.dog,build.log,before.log,after.log}`.
The small stand-ins do not establish full parser/reasoner or consumer integration.
No maintained Proteus tests or .tst fixtures changed. Full Proteus, shared-library,
and Slipstream rebuilds remain deferred under D077.


## D083 Complete full validation after native-block cleanup (2026-10-17)

**Decision.** The user authorized the previously deferred large builds and tests,
lifting the D077 validation boundary. Retain CodeDog's `_strm_open` helper and
`dataFile.open(filePath, "r")`: the helper translates string modes to C++ stream
flags and remains necessary independently of try/catch. No further production
changes were needed. The maintained Proteus .dog files contain only CodeDog macro
uses of the verbatim delimiters, with no native C++ function bodies remaining.

**Validation.** Regenerated and built `codeDog Proteus.Lib.dog` (155.53 s),
`codeDog BuildProteusGraph.dog` (139.05 s), and linked
`codeDog Slipstream.dog` (101.42 s). All three report Build Successful. The full
active Proteus suite with per-case crash protection reports 24 failures / 404
tests: 380 pass, and all 24 failure names and diagnostics exactly match D076's
last full-build baseline. All ten print-mode fixtures pass. The full discovered
CodeDog Python suite passes all 110 tests, including native C++, Java, Kotlin and
Swift runtime checks for the relevant compiler features, with no skipped tests.
Existing compiler warnings remain; no new failure required a source change.

`ldd -r` reports no missing dependencies or undefined symbols for either the
shared library or Slipstream. Slipstream resolves the rebuilt redesign library.
A 20-second startup smoke test on a private Xvfb display runs until the deliberate
timeout. Its log includes the same two existing normalization errors (semantics
beyond the scalar milestone and open-source correspondence needing span alignment),
with no syntax or system errors. This is startup validation, not renewed visual
rendering, navigation or graceful-shutdown validation.

Hashes of all tracked source files and existing nonignored untracked user files
remain unchanged across all three repositories before adding this decision.
All 16 tracked .tst fixtures are unchanged, and the test inventory still matches
all 72 compiled registrations. Existing CodeDog and Slipstream working-tree edits
were preserved and included as they stood in these builds.

Evidence: `/tmp/proteus-cleanup-full-validation/` contains per-stage logs and
timings, the source snapshot, exact baseline comparison in `validation.json`,
linkage output, and the bounded startup log. The comparison baseline is
`/tmp/proteus-codedog-parser/full.log`. No maintained tests or expectations were
modified, and the known unsupported semantic cases remain visible as failures.


## D084 Promote the redesign to canonical Proteus main (2026-10-17)

**Decision.** The user approved importing and publishing the cleaned redesign
as `Proteus:main`, then requested an annotated Git tag named `Proteus4` on the
final integration commit. The label is a Git tag; runtime and language version
labels are unchanged. Remote main at `86321ea` is an ancestor of redesign
checkpoint `d49e557`, with 325 incoming commits and no main-only commits.
Fast-forward main to that checkpoint, then add the canonical-path documentation.
Preserve the full history, `proteus3b` at `73752c4`, and the imported
`codex/normalizer-redesign` checkpoint. No force push or history rewrite is needed.

Develop in `/home/bruce/devl/Proteus`. Update current repository instructions,
build documentation and the design's working-location references; historical
reports retain their original evidence paths. Slipstream's proxy and exported
interface both select `../Proteus`, with its generated RUNPATH pointing to the
canonical SharedBuild directory. `Resources/public` still resolves to the
sibling World repository. Solver code and all language fixtures remain identical
to the validated redesign.

Coordinate publication with CodeDog `e0dae8e`, including the nine compiler
commits ahead of its previous remote main, and Slipstream's existing
`codex/proteus4-insert-compat` branch. Stage only Slipstream's two build-path
changes and canonical build instructions. Preserve its broader local README
additions, model edits and untracked files, plus CodeDog's local ReactGUI edit.
The original Proteus untracked research files remain in place. Recovery copies
and hashes of existing local work are in `/tmp/proteus-main-migration/`.

**Validation.** Rebuilt from the canonical checkout using a clean checkout of
committed CodeDog `e0dae8e`, so publication does not rely on its local ReactGUI
edit: test executable 160.86 s, shared library
140.15 s, Slipstream 107.54 s.
All three builds succeed. All 110 discovered CodeDog tests and all ten print-mode
fixtures pass. The full Proteus run has 380 passes and the same 24 failures out
of 404 tests; failure names and diagnostics exactly match D083. All 16 tracked
.tst files and 72 compiled registrations are unchanged.

Both shared-library and Slipstream `ldd -r` checks report no missing dependencies
or undefined symbols; Slipstream resolves the canonical engine library. Its
20-second private-Xvfb startup run reaches the deliberate timeout, reporting only
the two existing normalization errors and no syntax/system errors. This run uses
the existing local application/world resources; it does not revalidate visual
rendering, navigation or graceful shutdown. Existing local edits are preserved.

Retire the unused standalone checkout by moving it intact, including its Git
metadata and ignored files, to
`/home/bruce/devl/.retired/Proteus4-redesign-2026-10-17-d49e557`.
The canonical repository retains the imported history independently. Evidence:
`/tmp/proteus-main-migration/` contains build/test logs, timings, source and local-
work checks, linkage/startup output and the retirement record. The publication
checkpoint retains the documented unsupported semantics and known failures.


## D085 Organize documentation under Docs (2026-10-17)

**Decision.** The user requested moving Markdown documents into `Docs`,
`Docs/Design Docs`, and `Docs/Design Docs/Historical`. Move the build, shared-library
and test-ambiguity guides into Docs. Keep the main design, decision log, semantic
contracts and working temporal/identity notes in Design Docs. Place dated
checkpoint reports and superseded implementation plans in Historical, including
reports for the current engine whose later milestones changed their details.

Move 95 existing documents without changing their semantic content. Preserve the
six previously untracked research documents in the documentation commit. Keep
the root README, AGENTS.md and Resources/README.md in their conventional locations.
Add navigation indexes and update documentation references, including links to
unchanged graph fixtures and diagnostic assets. Historical Git commands and
source/evidence references retain their original meaning. Existing user cleanup
deletions and the Archive ignore rule remain separate from this organization.

**Validation.** Verified all 95 moved documents against the original contents,
allowing path updates, the documented navigation additions, and three Markdown
hard breaks rewritten without trailing spaces. All 127 previously
valid local links still resolve; the move introduces no broken links and repairs
four old documentation links. The remaining 23 broken links refer to historical
source or evidence. Engine, fixture and unrelated tracked-file hashes are
unchanged, and the user's earlier cleanup deletions remain intact. This
documentation-only change does not require engine or Slipstream rebuilds.


## D086 Refresh README guidance for the current engine (2026-10-17)

**Decision.** Following the documentation moves, update the root README to
describe the maintained WorldManager implementation, Agent sessions and request
handles. Replace obsolete agenda-engine claims, unsupported application promises,
missing-file entries and the old Tic-tac-toe example with a current project map
and links to the design references. Preserve authorship and license information.

Keep build and test details in Docs/README.md, with a short getting-started path
in the root README. Document separate test/shared-library builds, focused fixture
selection and the printed-failure-count requirement. Date the D084 full-suite
baseline instead of presenting it as a newly run result. Point historical report
references to their new folder and identify Proteus4 as the promotion Git tag.

**Validation.** Checked local links and heading anchors in all five README files
and verified every path in the root project table. The documented focused command,
`./TestProteus -t defaultTests.tst:parser/list/simple -T 3`, passes 1/1 with the
existing executable. The previously recorded full run remains 380 passes and
24 known failures out of 404 tests. No full suite or engine/Slipstream rebuild was
needed for these documentation edits. Tracked-file hashes confirm source and
fixtures are unchanged; earlier user cleanup edits remain separate.


## D087 Centralize language fixtures in Tests (2026-10-17)

**Decision.** Move all 16 retained `.tst` suites from the repository root and
graph/ into Tests/. These remain runnable fixtures, including separate suites
for unresolved semantics. Preserve their exact contents and expected outputs.
Generated copies in ignored build directories are not maintained fixture sources.

For a bare filename, ProteusTests.dog now reads ../Tests first, then retains the
working-directory and parent-directory fallbacks. This applies to the default
suite, command-line selections and @include names, with optional .tst extension.
Explicit relative and absolute paths keep their existing meaning. Prefer Tests
over a stale build-directory copy without introducing a legacy-path alias map.

Update both Python audit helpers, current build guidance and the moved fixture's
Markdown link. Historical result inventories and recorded commands retain their
checkpoint paths. While verifying run_language_baseline.py, its selected cases
passed but it rejected the run because it still expected GRAPH_CASE lines from
the retired implementation. Remove that obsolete marker check; retain the exit
status, failure summary and exact selected-test-count checks.

**Validation.** Regenerated TestProteus from CodeDog successfully in 149.29 s.
The full run remains 24 failures / 404 tests. The twelve separate suites add
117 cases with 41 existing failures. All 521 case results and diagnostics match
the pre-move runs exactly after normalizing file qualifiers. No fixture input,
expected output, compiled registration or engine source changed.

Eight temporary loader probes pass: preferred-directory precedence over a stale
local copy, extensionless selection, explicit relative and absolute paths,
current-directory and parent-directory fallbacks, repeated includes, and the
existing empty-file fallback. The crash-protected runner requires ./TestProteus
in its working directory; the isolated probe directory supplies a symlink to
the rebuilt executable. The regular audit helper passes its selected parser
fixture. The baseline helper passes ten numeric fixtures and still returns a
failure for the known failing merge/dots/align4b fixture.

All 16 moved fixture hashes match their originals. Current documentation links
resolve, and existing user cleanup edits remain separate. This test-loader change
does not require a shared-library or Slipstream rebuild. Evidence, before/after
logs and the loader probes are in /tmp/proteus-tests-folder/.


## D088 Remove graph artifacts and their historical reports (2026-10-17)

**Decision.** The user requested removing graph/ and the Markdown files moved
from it into Historical, provided the files were already stored in Git. All 61
remaining graph files are tracked and unchanged, with no untracked or ignored
contents. Commit 3c5e4cc records the 78 report moves from graph/ into
Docs/Design Docs/Historical/. Remove those 139 files from the checkout; retain
their complete history at the pre-cleanup commit 5dbbe10.

Keep all 16 active .tst files in Tests/, the current design documents and guides,
and the four historical documents that came from theory/. Remove the graph audit
helpers and inventories along with its diagnostic assets, as requested. Update
navigation and current instructions so they no longer depend on graph/. Preserve
citations in semantic notes as explicit Git-history paths, with recovery commands
in the Historical index. The fixture-ambiguity notes remain as historical
semantic questions; the ordinary TestProteus run does not apply the retired
audit helper's skips.

**Validation.** Verified that all 139 removed files are recoverable from 5dbbe10.
All remaining engine, test-loader and fixture hashes are unchanged. The full
existing executable run reports the same 24 failures / 404 tests, with output
identical to the pre-cleanup run. Checked surviving documentation links and
introduced no new broken targets. No engine or Slipstream rebuild was needed.
The user's earlier cleanup edits remain separate. The exact removal lists,
source snapshot, link validation and test output are in
/tmp/proteus-graph-cleanup/.


## D089 Require selected cursor fields before presenting a temporal state (2026-10-17)

**Problem and decision.** The user accepts pink progressive cube colors but wants
to remove the initial incorrect placement during navigation. They explicitly
identified this as a cursor change as well as a Slipstream change and approved
caller-selected readiness requirements. Add `TCursor.requireField(path)` before
initialization. Slipstream requests the six location/orientation coordinates for
3D timelines; other cursors retain their existing shallow behavior. Acquire those
requirements before optional scene/color demands, and retain the old visible
cursor occurrence until all of them have known values. Priority reduces delay;
readiness prevents presenting invented zero coordinates.

**Implementation.** Compile canonical represented field paths into a shared-prefix
tree. Acquire each demanded parent/field through the ordinary WorldManager solver
with the original occurrence/definition context. Index each acquired parent's
represented members once and reuse explicit contexts for its children, rather
than locating every sibling from the parent again. Frozen source views and owned
output branches remain separate. The virtual graph stores checked required facts
in the selected cursor occurrence after the whole requirement set succeeds;
read-only traversal transfers the same result to `working`. Owned staged members
are transferred with valid parent pointers. No second value cache, solver,
recursive whole-state normalization or restart API is added.

`preparedView()` recognizes headers already acquired for the current occurrence.
Slipstream's scene reset consumes these values directly. Renderable child objects
also wait for all their represented pose coordinates before contributing geometry,
so pending cublets do not accumulate at the origin. Their colors remain pink until
available. Slipstream retains the last rendered object while the new parts acquire
their poses, avoiding a blank interval between states. Structural grouping nodes
retain their identity transform. Timeline
initialization records that its first demand was attempted, so an exhausted unknown
cannot become an idle-loop retry.

**Waiting, failure and scope.** A parser dependency retains its WorldRequest and
subscription; repeated refresh and unrelated input do not recreate the request.
Cancellation, rejection, a missing/ambiguous field or an exhausted unknown does not
advance the visible cursor or expose partially prepared axes. Numeric readiness
requires a value, without demanding size. A cache hit must still satisfy this
cursor's field requirements. This increment covers represented named record paths;
embedded/sparse field layouts and general later member arrivals remain explicit
boundaries. Registration order supplies priority within preparation, before
Slipstream's optional requests. It does not add loader priorities or preempt a
solver operation already executing.

**Alternatives.** Reordering color and location reads alone still exposes missing
coordinates across redraws. Blocking the GUI thread would prevent useful progress.
Waiting for the complete cube would discard the desired progressive colors.
Keeping acquired values only in detached cursor views would conflict with D071's
list-backed cache policy. Hard-coding Rubix fields into the engine would couple the
cursor to one renderer. These alternatives are not used.

**Validation.** Regenerated successfully with `codeDog Proteus.Lib.dog`,
`codeDog BuildProteusGraph.dog` and `codeDog Slipstream.dog`. Eleven ordinary
CodeDog readiness checks pass, covering shared prefixes, zero/unknown values,
retained parser waits, cancellation/rejection, missing fields, cache hits,
navigation, atomic publication, read-only traversal and unchanged shallow cursors.
For 8/32/128 represented sibling fields, requests, frames, queue steps and members
scanned each equal the width; relationship attempts remain zero. The full
`./TestProteus -T 3` run reports 391 passes / 415 tests, with the same 24 failure
names and diagnostics as the pre-change 380/404 run. The explicit
`./TestProteus -t defaultTests.tst: -T 3` run retains 16 failures / 332 cases.
No fixture expectations changed.

The regenerated consumer resolves `libproteusGraph.so` from this checkout;
`ldd -r` finds no missing dependencies or undefined symbols for the library or app.
A private Xvfb display reproduced the tiny pink cube before the change. The stale
generated baseline had an obsolete first-field-only presenter predicate; that
predicate was aligned with the existing maintained CodeDog source for the visual
reproduction. The final consumer was fully regenerated, with no dependency-source
edits. Four final 60 fps recordings (two new states, backward and cached forward)
show correctly placed progressive pink geometry and retain the old object while
the new poses load. All 960 captured frames contain visible cube geometry; none
contains the tiny pink fallback or blank transition seen in the baseline.
Build/test logs, screenshots and frame measurements are in /tmp/cursor-readiness/.
Existing unsupported startup normalization notices and the 24 baseline failures
remain outside this change.


## D090 Prepare contained Things in the cursor before publishing (2026-10-17)

**Decision and reason.** After D089 the cube stayed correctly positioned, but
preparing child poses through successive display demands still froze the retained
mesh briefly. The user requested trying built-in Thing preparation in the cursor.
When a state has represented `thing` or `parts` fields and no explicit caller
requirements, TCursor now prepares the finite containment tree before selecting
that state. `thing` means its location and orientation coordinates. An explicit
`parts` collection supplies children; structural collections preserve member
occurrence order, including repeated types. A Thing leaf's other fields and an
assembly's reference/metadata fields remain optional. This follows containment,
not all references reachable from a model, and adds no Rubix-specific type names.
Explicit `requireField` callers retain their selected requirements; ordinary
nonspatial states retain shallow cursor behavior.

**Progress and publication.** Preparation uses the existing staged field path,
solver, original contexts and atomic list-backed publication. The previous cursor
occurrence remains selected while every contained Thing is acquired, so Slipstream
continues drawing and animating it. On completion, the renderer reads the prepared
parts and poses directly; colors still arrive progressively. Slipstream no longer
registers six root-only paths or discovers the cublet poses over successive redraws.
The renderer also starts at most sixteen optional display demands per pass, so
color refinement cannot drain the whole pending collection inside one draw.

Readiness metadata lives on the existing stored cursor value and identifies its
prepared headers. A valid cache hit reuses those facts without re-solving derived
parts or allocating another value tree. Replacing the value or publishing into any of
its descendants invalidates that readiness metadata. This is not a second cache
of computed values.

WorldManager can yield an ordinary runnable request after a queue-step budget;
the same frames and ready queue resume later. Default requests still drain
synchronously. Cursor preparation defaults to 4096 queue steps per refresh; zero
permits synchronous draining. Budget exhaustion is `worldReady`, creates no input
subscription and does not publish an incomplete field. Parser arrivals keep their
request's budget; real input waits keep their existing subscription. This bounds
queue work per preparation call, not the elapsed duration of an individual solver
step. No worker thread, reset/restart path or second solver is introduced.

**Boundaries and alternatives.** Unknown poses or containment stop partial without
presenting zero coordinates. Open/embedded containment remains unsupported;
repeated source headers along a containment ancestry reject cycles. Shared sibling
instances remain distinct. Eagerly normalizing colors, metadata and the complete
model would add unnecessary work. Keeping child acquisition in rendering retains
the extra discovery/redraw cycles. An unbounded synchronous preparation batch could
freeze the GUI. None of those alternatives is used.

**Validation.** Seven new ordinary CodeDog checks cover recursive/repeated Thing
occurrences, parser waits, unknown poses, read-only navigation, cooperative
continuation, containment boundaries and scaling. They also check cache reuse,
readiness invalidation after descendant publication, and optional fields staying
unprepared. All seven and the eleven D089 checks pass. The full regenerated suite
reports 398 passes and 24 failures out of 422; the failure names and diagnostics
exactly match D089. Default language fixtures retain 16 failures out of 332, with
no fixture or expectation changes.

Widths 8, 32 and 128 take respectively 641, 2561 and 10241 queue steps;
relationships are 280, 1120 and 4480, and preparation header allocations are
82, 322 and 1282. These measurements cover finite fixed-depth containment, not
arbitrary inference. Revisiting a valid prepared state starts no field requests.
Yielding introduces no parser subscriptions or accumulating request handles.

`codeDog BuildProteusGraph.dog`, `codeDog Proteus.Lib.dog` and
`codeDog Slipstream.dog` regenerate and build successfully. The consumer resolves
this checkout's shared library, and `ldd -r` reports no missing dependencies or
undefined symbols. Four private Xvfb recordings at 60 fps exercise two new states,
backward navigation and cached forward navigation. The longest unchanged cube
image drops from 25-26 frames (0.417-0.433 seconds) under D089 to 5-6 frames
(0.083-0.100 seconds). This measures visible animation pauses in these captures,
not a general latency bound. All 960 final frames contain visible cube geometry;
none contains the tiny pink fallback or a blank transition. Colors fill more
gradually while the correctly placed pink cube keeps rotating. Existing unsupported
startup notices remain. Build/test logs, screenshots and frame measurements are in
`/tmp/cursor-things/`; comparison captures are in `/tmp/cursor-readiness/`.


## D091 Restore display-driven child materialization (2026-10-17)

**User decision.** The user found D090 substantially slower overall and requested
returning to the preceding method, accepting its animation stutter. Revert the
paired engine change `1f86d87` and Slipstream change `523b82f`. D089 remains active:
explicit root pose requirements and the last rendered geometry preserve placement
while renderer demands acquire child fields.

**Restored behavior.** Remove automatic recursive Thing preparation, its readiness
metadata and cooperative solver budgets. Ordinary requests drain synchronously.
Slipstream again registers the six root pose fields for 3D timelines and processes
all pending scene demands in each pass. Keep the canonical engine linkage,
stored cursor occurrences, CodeDog portability fixes and existing SceneModel.

**Evidence boundary.** D090's recorded reduction in frozen animation frames did
not establish faster state presentation or color completion. The user's observed
latency takes precedence over that narrower metric. Future performance work must
measure time from navigation to a usable complete view as well as animation pauses.
The D090 record above is historical; its implementation and seven feature-specific
checks are reverted. The eleven D089 cursor-readiness checks remain active.

**Rollback validation.** The shared engine, TestProteus and Slipstream regenerate
and build successfully. The full suite reports 391 passes and the same 24 failures
out of 415 tests, with identical failure names and diagnostics to D089. The eleven
cursor-readiness checks remain covered. Slipstream resolves the canonical shared
library without missing symbols. Private Xvfb next/previous navigation captures
480 frames with visible cube geometry throughout and no tiny pink fallback;
the retained animation stutter and brief progressive pink coloring remain. The
application exits with status zero after normal window closure. Evidence is in
`/tmp/slipstream-d090-rollback/` and `/tmp/proteus-d090-rollback-*.log`.

## D092 Optional definition roles; binding and application syntax open (2026-10-18)

**Accepted direction.** Word definitions may expose optional role names as an
interface to participants already described by their meaning. A role header is
not required on every definition. The user chose named roles over requiring a
narrow model type for each parameter. The meaning still supplies its actual
constraints; moving a restriction from a header into the body does not remove it.
No parameter syntax or engine behavior is implemented by this decision record.

**User proposals, not settled syntax.** A body could refer to roles with `%item`,
`%actor`, and similar references. Verbs might conventionally expose `actor`,
`item`, and `object`; whether this is a fixed inventory remains open. Explicit
application could use positional `gave(Bruce, the box, Arthur)` or named
`gave(actor::Bruce, item::the box, object::Arthur)` bindings. A natural-language
form such as `Bruce gave Arthur the box` could acquire its bindings by examining
alternative interpretations. These examples are schematic: phrase boundaries,
entity resolution, and meaning selection need their own contracts.

**Binding questions.** Specify which application owns each role reference,
how nested definitions and forwarding preserve that scope, and how a binding
refers to the model participant without mutating the shared declaration.
Distinguish an omitted header, an omitted argument, an unknown participant, and
a participant that does not exist. State-change roles must retain the distinction
between the participant and its earlier/later occurrences. Positional application
needs a declared order; natural-language word order need not match that order.

**Grammar evidence and alternatives.** `infonIO.dog` currently recognizes a fixed
set of percent references, including `%self` and `%args`; it rejects arbitrary
role tags. Parentheses introduce `emConcat` expressions, and commas are currently
restricted to brace lists, where they select unordered correspondence. A bare
name has no parenthesized call suffix. A role call therefore needs an explicit
grammar rule, with a decision between a new postfix-parentheses production and
a distinct application marker. Named binding punctuation must also be checked
against existing colon qualification and boundary operators. Grammar must not
silently change according to whichever definitions are currently loaded.

**Proposed composition.** Named, positional, and interpreted applications should
produce the same occurrence-specific role relationships and use ordinary meaning
application. Natural-language constructions can propose correlated bindings;
model constraints can reject proved contradictions without treating missing
evidence as a mismatch. Remaining interpretations must stay separate. The search
strategy, ambiguity representation, role inventory, argument rules, and concrete
syntax remain open; this does not accept a planner or add a separate evaluator.

**Follow-up proposals.** The user suggested requiring the opening parenthesis to
immediately follow the word, distinguishing `gave(...)` from `gave (...)` without
consulting declarations. Whitespace/comment treatment and compatibility with
existing adjacent word/expression forms still need specification and checking.
The user also proposed `turn(%ctx.rubix-cube.red-face, clockwise)` (using the
existing `%ctx` spelling). This motivates definition-specific parameter names,
such as `item, direction`, beyond participant conventions such as `actor, item,
object`. Direction is itself a supplied description; it need not be an actor or
another physical participant. A possible header is `#turn: item, direction = ...`,
with body references `%item` and `%direction`. These remain syntax proposals.

**Cube example obligations.** The argument's context path must retain the caller's
context and the selected face's occurrence/containing-cube relationship when used
inside the body. Clockwise needs a viewing convention, and direction alone does
not specify an angular amount. D069's fixed cube verbs use the view from outside
the named center color; a parameterized model must state its own corresponding
convention and turn extent. Supplying a face does not limit the changed facts to
that face; the model describes the affected layer and related cube positions.
No general overload selection or parameterized cube implementation is accepted
by this example.

**Current scope: explicit application first.** The user considered receiver syntax
such as `%ctx.rubix-cube.red-face.turn(clockwise)`, then deferred it because of
the additional result/receiver questions. Continue with `turn(...)`. A definition
may also evaluate to an independent value, as in the proposed `sqrt(9)` example;
application must not universally identify its result with an argument or presume
that the result becomes the next receiver. This does not claim an implemented
`sqrt` definition or settle the result of `turn`.

**Exploratory composition example.** The user supplied the schematic sequence
`%ctx.file.paragraphs[1]~; paragraphs[3]~; 'primary font'=Ariel; paragraphs[2].sentences[2]~`.
Use it to examine how calls compose with selection, reads, assertions and retained
path bases. Keep the exploratory base, selected source occurrence, definition
bindings, and resulting value distinct. The later relative paths appear intended
to remain relative to the file; that interpretation is not yet a specified scope
rule. Questions remain about how that base is established, whether argument
expressions use it, how a call result is selected or queried, and how a described
state change is applied. A call returning a scalar is not by itself authorization
to change the next path's base or update an argument. Current parser syntax uses
`@[...]` for indices and `>>` for replies; the example is not a runnable fixture.
`%ctx` parses but WorldManager's cursor path still requires an occurrence provider
for it. No exploratory-interface implementation is accepted by this record.

**Application on a temporal identity RHS.** The user proposed
`%ctx.rubixCube.next = turn(direction::clockwise)`, possibly supplying
`item::%self`, then observed that the assignment-like use of identity in a T-list
could make `.next` unnecessary. Continue examining
`%ctx.rubixCube = turn(...)` as a description of the destination state selected
by the enclosing temporal step. This is not a general rule that `=` outside that
context advances time, nor an assertion that the preceding and resulting values
are equal. D067 already separates given new-state work from retention of
unaffected previous values.

The remaining binding choice is whether `item` denotes the participant whose
temporal states the body relates, or a particular input-state description. The
prior input and resulting occurrence must remain distinct. `%self` currently
refers to a containing definition occurrence; selecting an arbitrary LHS does
not by itself rebind it. Explicit `item::%self` therefore requires an established
caller scope, and omitting `item` requires a specified default or contextual
binding relationship. Neither convenience is implemented or accepted as an
automatic rule for all applications. A cube face turn also needs its face/layer
identified; naming the cube and a direction alone does not select the red face.

**User clarification: selected face and an unnamed direction.** Applying the
example to `rubixCube` means turning the whole cube. For a face turn the user
instead intends `rubix-cube.red-face = turn(clockwise)`, with the more realistic
LHS `rubix-cube.faces~red`. The intended participant comes from the selected face,
and `clockwise` supplies the direction role. This does not establish a universal
rule binding every definition's first argument to its result destination.

The user expects the model to recognize that `clockwise` fits `direction`.
Distinguish two possible mechanisms: context first supplies `item`, leaving
`direction` as the sole remaining role; or an unnamed argument is matched to
candidate roles through their model relationships. The latter extends the earlier
purely positional-call proposal. Named `direction::clockwise` remains explicit.
Role compatibility must be justified by the meaning and context, not a special
case for the token `clockwise` or an inferred global parameter type. Unknown
compatibility is not rejection, and multiple viable assignments remain correlated
alternatives rather than being selected merely by evaluation order. The general
rules for mixing positional, named, contextual and inferred bindings remain open.
The face selector retains its existing correspondence/occurrence semantics;
parameter binding does not authorize replacing it with a separate name matcher.

**Validation boundary.** Documentation only, based on the current parser source
and the accepted definition/context contract. No parser, engine, fixture, or
test expectation is changed, and no runtime support is claimed.
