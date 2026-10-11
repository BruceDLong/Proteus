# Proteus4 normalization issues and design decisions

The design discussion proceeds from the meaning of normalize and its solving loop through lists, intersections, definitions, and rules. Each choice must explain its streaming behavior, computational cost, alternatives, and justification. Semantics should be expressed through graph relationships, with optimizations justified by theorems.

This is the issue agenda and decision history. The companion [consolidated design](normalization-design.md) contains the resulting contract. The [framework audit](/home/bruce/devl/Proteus4/graph/normalization-framework-audit.md) supplies implementation evidence and proposals; its recommendations are not automatically accepted decisions.

Created 2026-10-10; updated 2026-10-11. Current issue: **D03, list correspondence, expansion scope, and shared span propagation**. D02 establishes directed acquisition, completion, and continuation without restart. Exact leaf and list interfaces remain under discussion, followed by intersections and definitions. D01 planning design is deferred until intersections and definitions have been considered. Develop the request contract alongside those examples. “T-rules” covers both theorem/transformation rules and temporal/T-list rules, as clarified in this discussion.

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

When a decision changes, update the specification and this history together. Distinguish a changed semantic choice from an implementation that has not yet met the choice. Existing unresolved topics in [open-questions.md](/home/bruce/devl/Proteus4/graph/open-questions.md) remain open unless explicitly resolved here; do not create contradictory resolutions in the two documents.

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

**Clarified 2026-10-11:** parentheses usually do not change the result of ordinary `A=B=C` versus `A=(B=C)`. Do not impose a separate solving strategy merely because the parser represents a group. The important distinction is a type-changing relationship such as `A==(B=C)`: C may have a different type from A, so its type cannot be applied directly to A by flattening the chain. Preserve the qualifications and prerequisites that govern the transformation. The existing `unit/graphText/identityInSource` case in [ProteusTests.dog](../ProteusTests.dog) exercises `{*3+$ *4+$} == ($ = 'CatDogs')`, expecting `{'Cat' 'Dogs'}`. It currently uses the checkpoint engine and must be ported with the real tests; it is not evidence that the scalar replacement already supports this case.

Stopping A's search and proving a work entry removable are distinct decisions. A completed scalar goal does not by itself erase a nested or qualified relationship that still carries meaning. The interface may discard fully represented pending work while retaining any identity or context still needed by other uses. Removal from one wrkList is not deletion of a source infon used elsewhere. Exact consolidation criteria for each relationship belong with the leaf, list, intersection, and definition rules. Current [RHS consolidation](/home/bruce/devl/Proteus4/graph/rhs-consolidation-results.md) is implementation evidence, not the chosen limit of the new design.

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

### Streaming and complexity checks

For a simple acyclic chain of k constant-cost compatible links, locating a value available only at the far end requires O(k) link inspection. It need not require k target publications: the requested A may need only one, plus any semantic publication actually required by a relationship. If B already supplies the answer and no further premise is needed, inspect that route and stop without scanning the rest of the chain. Account separately for payload-copy cost and type/size compatibility; these are not always constant.

For incremental input, measure newly exposed work and the continuations actually woken. Reject an implementation that revisits the entire goal after each publication. This is a workload bound and a design requirement, not a proof that arbitrary matching or inference is linear. Include pending subscription storage, sparse occurrence allocation, and the extra cost of coherence mode.

Validate direct copying; a longer chain with only its end known; a flat versus nested RHS; a rejected B that blocks its route to C; a qualified relation requiring B to resolve first; an arrival at one pending link while another route proceeds; completed A creating no goals on another normalize; no sparse intermediate clutter; and useful partial oneOf/range results. Include parser completion metadata for literals versus unfinished containing expressions. Check a numeric value with unknown size and exhausted idents completing without size search; a named range type doing the same; a required pending ident continuing locally; and a stronger size request remaining unsatisfied by numeric `isNormed` alone. Preserve and enforce supplied constraints. With maintained cursor/status bookkeeping, checking numeric completion must not require another ident traversal. Print/reload checkpoint tests are out of scope.

### Remaining D02 questions

Specify exact goal requirements and `isNormed` scope within D02F's numeric completion rule, how the interface reports a usable or fully merged edge, and how local subscriptions retain and deliver their continuation without missed arrivals. Establish ownership when goals share useful deductions within a session; sharing among sessions is deferred under D02H. Define the coherence mode's comparison and reporting behavior, without making it a new default validation pass. Carry special meaning-change triggers into D05 and D07. None of these questions reopens the no-restart decision.

## D03 Lists and listSpecs

**Status: partly accepted, 2026-10-11, from the user's list discussion.** Identity sources, main-level scope, reuse during co-propagation, and deduplication of matching attempts are the design direction below. The proposed graph operations and chart layout remain to be reviewed. Planning remains deferred under D01.

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

**Status: open.** Specify the relationships among an intersection's body/pattern, inputs, chosen or alternative matches, result, and constraints on that result. Explain how leaf tests, list traversal, listSpecs, sparse spans, and partial matching compose. Keep source occurrences and their evaluation contexts identifiable.

**Alternatives:** destructive rewriting; retained selection relationships; indexed matching; backtracking; shared matching subproblems. Decide where each is legitimate and how alternatives retain correlated facts rather than mixing facts from different matches.

**Streaming and cost:** state when a prefix or alternative may publish, when exhaustion is proved, and what happens if an earlier ordered candidate is unresolved. Charge candidate search, branching and repeated correspondence explicitly. Include a scalar predicate, list pattern with a selected member, repeated output, negative or end-relative position, and a constraint arriving on an already selected result.

For D01, retain a concrete case if the ordinary forward strategy fails or does avoidable search. Explain the failed assumption and a possible better strategy without choosing a planner yet.

## D05 Definitions and composition

**Status: open.** Specify declaration lookup, meaning, contextual binding, nominal identity, instance constraints, and copying/materialization. Distinguish compiling a declaration from proving that an instance satisfies it. Explain how a definition contributes ordinary wrkList/list/intersection obligations and how those constructs can in turn supply its missing premises.

**Alternatives:** copy a body on every use; share immutable compiled meaning with instance-specific relationships; expose virtual definition routes as demanded. Decide where actual instance data must be allocated and when a proved correspondence can be reused or applied in a batch.

**Streaming and cost:** cover absent and late declarations, partially supplied bodies, recursive definitions, aliases, and ambiguous meanings. Separate one-time declaration processing from per-instance work and changed-binding validation. Work through a direct record and an equivalent typed record, partial typed descriptions, shared definitions with distinct instance contexts, and an intersection inside a definition. Use solved state1 as the larger composition case.

Reconcile late declaration/context events with D02C: an unchanged completed infon does not create goals on ordinary normalize; a semantic change must explicitly request a new goal. Specify which definition events constitute such a trigger instead of relying on blanket revalidation at every submission.

For D01, record where definition expansion or its interaction with intersections makes the obvious strategy inadequate. Separate work required by the requested output from work caused by a poor strategy.

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

**Retained baseline, not newly decided by this agenda.** Current [AGENTS.md](/home/bruce/devl/Proteus4/AGENTS.md) and the recorded clarifications in [open-questions.md](/home/bruce/devl/Proteus4/graph/open-questions.md) constrain the starting examples. They include nominal typed fields, distinct occurrences and contexts, sparse/open safety, partial definitions, selection precedence, and guarded temporal preservation with `%?`.

The consolidated design records these constraints and their sources. The later D02 decisions govern default stopping and the lifecycle of ordinary information arrival; older descriptions of automatic goal restart do not carry forward. Other baseline constraints can be reconsidered explicitly during design; an architectural convenience or existing implementation gap does not silently change them. Current source is evidence of what the engine does, not by itself a justification of what it should do.
