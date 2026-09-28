Proteus4 development
--------------------

This is an independent architecture experiment. Favor the new graph reasoner
and remove code that is no longer used; compatibility with every legacy API or
test is not a requirement at each step. Do not retain obsolete scaffolding only
to preserve the old test baseline.

The word/grammar checkpoint supports finite oneOf alternatives in sequence
correspondence, preserving shared definitions and accepting the first complete
parse. WordForms.h owns collection-type inference and the small English-text
splitter; these are not general word/context interpretation. Arithmetic operands
must not borrow a list occurrence for %prev/%next. See graph/word-grammar-results.md
and graph/time-word-design-plan.md for the remaining design discussion.

Nulls use known size/value zero with value overlay tUnknown, printed as NULL:
or NULL:<positive scoped ID>. Bare NULL remains a word. Every infon has begin/end;
only T-list composition currently equates adjoining boundaries. Boundary nodes
are requested lazily through graph/BoundaryGraph.h and published with priced Sets.
Keep occurrence identity separate from list position; do not merge adjacent null
entries or expand sparse tails to discover boundaries. See graph/boundary-results.md.

Temporal value references must not identify successive occurrences or their
supports. `%tprev`/`%tnext` use RelativeNavigation's temporal policy; ordinary
`%prev`/`%next` retain their structural boundary. An S-list is one temporal value;
only nested T-lists and explicit embedded spans continue temporal navigation.
TemporalGraph supplies symbolic support/cut goals. Reading either side of an
interior S-state cut yields the same source value. A cut in a changing T-history
stays unresolved until its location is known. Do not infer time duration from
member count, expand sparse histories, or copy missing fields by persistence.
See graph/temporal-context-results.md for the current implementation boundary.

Definition catalogues select language-only tags using the existing base/full
locale rule. Each graph context captures its vocabulary at initialization;
additional usage qualifiers and multiple applicable meanings stay unresolved.
Do not drop qualifiers or silently rebind an existing context to another locale.
See graph/definition-language-results.md.

Current priority: make the existing test suites work before further context,
T-list, or word-model redesign. If a test's intended semantics are ambiguous,
record the exact fixture and question in graph/test-ambiguities.md and skip it
explicitly in the compatibility audit. An unimplemented feature with clear
semantics is still a failure, not an ambiguity. Do not change expectations merely
to match incomplete behavior.

Validate the behavior being changed with focused tests. Existing language tests
remain useful semantic examples for later migration, and can be run selectively.
A full legacy-suite run is optional, not a gate for every change. Clearly report
which behavior was checked and which features remain unsupported.

Organize reasoning by the semantics of language constructs. Combinations of
constructs should compose their contributions; avoid adding an algorithm for
each fixture or representational combination. List boundary/embedding/span
traversal is shared in graph/ListStructure.h. Consumers retain their semantic
obligations and must not silently ignore unsupported constraints.
POV structure and stored relationships are described in graph/POVDescription.h,
independently of goals. Reuse that live view; do not create parallel adjacency
storage or cache a POV as permanently expanded. Publications may expose new
relationships, and sparse inspection must not materialize occurrences.

Current checkpoint: scalar/finite definitions, lazy member templates, and
fixed-width whole-list grouping use graph requirements and priced plans.
Empty open definition bodies retain virtual positions; their known extent
feeds the shared span layout without allocating members.
Loaded instance prefixes can be checked without expanding their virtual tails.
A proved contradiction uses GoalRejected (or its DefinitionConflict subtype),
so rejection propagates only through required goal dependencies. Exploratory
rejection must not abort unrelated routes, mutate the RHS into a global failure,
or establish an identity. Keep source nodes and contexts available for reuse.
Missing information is still deferred. Engine/input/ownership errors remain
session failures; do not classify them as ordinary match rejection.
Sequence matching uses successful continuation edges and accepts the first
complete grammar correspondence; alternatives for ambiguous grammar parses remain
deferred. This does not apply to unordered field selection, which must retain
its matching alternatives as oneOf. Direct and repeated
<~ injection share that walk without mutating the listSpec. The constrained
repeated-output example in graph/source-injection-results.md intentionally waits
symbolically with a subscription until more information arrives. Unsized open
root lists can acquire a known prefix while their unresolved tail stays symbolic.
Completed repeated <~ results may publish while the unfinished repetition stays
open and subscribed. Shared span access keeps those result members available to
other RHS constraints. A prefix does not prove final size or whole correspondence.
See graph/repetition-prefix-results.md; parser-owned open input remains deferred.
Scalar exclusion predicates distinguish a known mismatch from missing information.
Accepted list-valued input members retain their atomic identity on replay; do not
reinterpret their copied values as more grammar. Composite exclusion,
contextual typed predicates, prefixed definition bodies, recursive grammar,
and unordered record matching remain later boundaries.
Single-step field predicates support finite ordered structures. Context-free scalar type
meanings can be compiled read-only for speculative matching and reused on accepted
outputs. Compilation is not proof that a candidate satisfies its type.
Dot and bracketed item paths share `graph/PathSelection.h` and its
result publisher. Positional steps wrap existing indexed navigation; do not
copy its sparse or embedded-list algorithms into path handling. Negative
indexes demand extent evidence. Competing routes to one position are not separate
oneOf alternatives. Collected positional results use the generic finishPass Set
hook so a partial publication cannot starve a slower successful route.
See graph/positional-path-results.md, graph/slice-results.md, and
graph/path-write-results.md. Unambiguous path writes add constraints to the
selected source occurrence; ambiguous targets remain pending. Retained parts
are required subgoals, while partRef determines the returned descendant.
Preserve the original owner stack when normalizing a selected occurrence;
copied source RHSs must not run again under the query alias.
Fixed-extent slices use shared logical layouts and span windows. Concrete result
members use identity-backed views so contextual expressions resolve at their
source occurrences; compressed runs share repetition families and ordinals.
Reuse window pieces and source-revision proofs across refinement and replay.
Empty open ordered bodies constrain only list headers, not individual members;
predicate and RHS-representation checks share that interpretation.
Dot queries (including read-only chains) and GraphNormalizer.findField share the field walk:
ordered lookup selects the first proved compatible logical member; unordered
lookup publishes known alternatives with an open oneOf tail while candidates or
membership remain unresolved. Chained paths traverse those alternatives using
the same field walk and preserve unresolved branches.
Unresolved earlier candidates cannot be skipped to choose a later ordered result.
See graph/chained-field-results.md for supported boundaries and validation.
Preserve read-only definition bodies, already-demanded occurrence POVs, and
reusable type evidence; avoid redundant
application RHSs. Redundant RHS/template rendering uses revision-scoped proofs;
keep semantic relationships searchable and do not relax isNormed to hide unknown
sizes. Closed, context-free strict equalities may leave wrkList after their
identity and original infonView owner are retained in the registry. Producers
use hasRHS() so replay does not recreate consolidated work. Partial, qualified,
typed, and contextual work stays pending. See graph/rhs-consolidation-results.md.
Size and value should be acquired together when both are available; partial
acquisition is for cases where only one is available. Qualified scalar string
relationships use ScalarRelation: a colon can supply prefix value evidence but
must not infer a missing target size from the available bytes. A known target
width selects that prefix; whole matching still requires matching ends. Keep
qualified work searchable and preserve RHS payloads on mismatch. See
graph/qualified-text-results.md.

Function arguments and symbolic last-result positions reuse indexed navigation
and the existing end-relative registry. A constraint on an intersection result
must not be reinterpreted as input to its body after selection. Qualified RHS
payloads expose their own graph prerequisites; no recursive normalize call is
hidden inside a matcher. Explicit-source ~ lookup uses the field walk and only
falls back to external associations after proved direct absence. Unknown direct
candidates retain precedence. External values own their mutable state; predicates
remain separate. See graph/input-resolution-results.md for supported boundaries.

Strict scalar exclusions and positive scalar acquisition share the compatibility
helpers in ScalarPredicate.h. Unknown comparisons stay unknown under complement;
never copy positive facts into a negative template. Preserve rejected endpoints
and check explicit member contradictions before publishing list identities.
%ctx binds the live Agent topic-history POV through ordinary path publication;
unary ~word uses the same lookup. Publish context edits under runtime.update().
AgentContext invalidates counted size on membership edits; explicit sizes remain
constraints. Definition applications must retain their original context binding,
even on explicit re-normalization. Future caller-authorized rebinding is tracked
in graph/open-questions.md; do not infer permission to rebind from normalization.
See graph/scalar-compatibility-results.md for tests and remaining boundaries.

Supplied typed-field descriptions may be partial even when ordered. Normalizing
such a description (including a parent returned by `;` or `;;`) normalizes the
present members without forcing the complete parent definition onto it. Bare type
references and positional value sequences still expand; member type constraints,
explicit RHSs, and repetition requirements retain their own goals. See
graph/partial-type-results.md for the boundary on projecting parent-specific
meaning. Do not add missing-field subscriptions merely because a definition
mentions omitted parts.

For unordered lists, the user clarified that omitted items are non-relevant to
that instance. Do not create missing-field subscriptions or eagerly insert all
definition members. Instance size counts the items it holds. Ordered lists can
also contain typed fields (as in a C struct); field selection does not imply
unorderedness. Preserve the space-separated query/chain fixtures. The latest
clarification says unordered descriptions have an implied open tail: unlisted
parts may exist, including parts beyond a type definition. Physical end alone
must not prove absence. An explicit total size bounds membership; once that many
members are known, the remainder is empty. Lookup does not require a size when
known parts already supply useful results. Size constrains membership, not the
members' states: describing a change to one part says nothing about changes to
omitted parts. See graph/unordered-field-results.md for partial publication and
continuation semantics. A completed field search with no matches returns a
closed empty oneOf; incomplete membership or matching still leaves an open
choice. Track unresolved design decisions in graph/open-questions.md, keeping
implementation gaps separate from unanswered semantic questions.

Fields are typed members, not string-keyed fields. An explicit name is modeled
as field^{name=X}. The user retracted structural type equivalence: .wheel must
not select a pedal merely because their definitions have the same fields.
Require nominal compatibility, including explicit aliases/additional types;
a shared ancestor alone does not equate sibling types. Untyped structural
predicates still test explicit constraints. Ordered selection chooses the first
compatible member; unordered selection retains matching alternatives as oneOf.
Cost chooses routes, not a different semantic winner. For now typed lookup
requires the correct type word; recognizing it solely from a sufficiently rich
description is future work. Check the explicit query before expanding meaning:
selection is not full validation of the selected object's definition. Definitions
can supply missing queried facts or prove exclusions; they cannot establish
nominal identity solely from matching structure.

Separate definitions are optional. Use supplied constraints and type identities;
an absent declaration adds no validation requirement, while existing declarations
still supply meaning. Track absent bindings in reusable definition evidence so a
later declaration cannot leave a stale proof. Completed queries reconsider later
definitions when explicitly normalized again, not automatically. See
graph/field-selection-contract.md for the current contract.

Agent.normalize() is the public graph entry point and returns a handle.
Agent.normalizeAndWait() is the synchronous convenience. Each Agent owns a
persistent GraphNormalizer context; no agenda fallback is used. See
graph/README.md for architecture, stream lifetime, and current semantic limits.
The AItem engine, lifecycle machinery, old reasoners, and generated merge-rule
matrix have been removed. Do not restore them to satisfy obsolete internal
tests. See graph/aitem-cleanup-results.md for the retained coverage.

Intro
-----

Proteus is a modeling language for representing causal structures and acting upon or querying the models. Queries and knowledge are represented in Proteus which can be stored in files or streamed for more interactive usage. After parsing, the function normalize() is called which stores the knowledge, answeres the query or does an action.

A typical way to use the library to make an interactive session is to open a parser thread / normalizer thread.

Proteus is written in the C++ like language CodeDog. It should be fairly easy to figure out but if not, docs can be found at https://codedog.world/Docs.

Program Architecture
--------------------

The primary structure in the library is an infon which represents a stateful piece of information. This could be a pyhsical system, a file or stream. Anything pretty much. It's a Proteus model. It contains POVs which are point-of-views into other infons that it contains -- it's "sub-infons". The POV is used because the same sub-infon may be used elsewhere so in a different POV.

Normalize starts a goal-driven graph search over POVs and their relationships.
The graph runtime owns traversal costs, subgoals, subscriptions and cancellation.
Proteus goal specifications turn findings into Set commands. Parser publications
resume subscribed searches as streaming information arrives.

Typical usage of the library 
 1. Load proteus files to use as a knowledge base
 2. Start a parser thread working on a proteus stream to use as querys, etc.
 3. Connect the infon being generated by the parser thread to a normalizer thread.

 If a sub-infon (in the streaming Proteus code) in a list is followed by `>>`, it requires that when the infon is normalized
 the resulting infon should be stringified and sent back to the GUI or other consumer. This allows query to yield results.

 Word definitions begin with `#`, for example `#minute = *60+{second| ...}`.
 `@` is reserved for bracketed postfix selection and is not a definition marker.

 Ordered item and span selection use bracketed postfix syntax: `items@[3]` and
 `items@[3:*5+{ ... }]`. The former unbracketed `items#3` spelling is not part
 of the current syntax.

Files
-----

The main files are
 - infonIO.dog:       parser / printer
 - infonList.dog:     the POV class
 - Proteus.Lib.dog    infon class and support classes such as Agent which hold normalize() 
                      and ProteusCore which handles loading libraries and multiple normalize() events.
 - ProteusTests.dog is built when the library is built when the library is built (codeDog Proteus.Lib.dog).

Other files:
 - ModelManager.dog:  Handles part of natural language interface
 - GraphFoundation.Lib.dog: CodeDog binding and semantic access primitives
 - graph/ProteusGraph.h: Proteus graph goals and relationship access
 - graph/GraphSearch.h: Generic asynchronous graph runtime
 - clip.dog:          A CLI for Proteus. Not often used currently because it does not stream
 - Functions.dog:     Some built-in functions for the engine to run if needed.

 - ./Resources:       Folder containing some input files for various purposes
 - ./LocalBuild/codeDog.log is the test/runtime log. Search handles retain their execution steps.

 The remaining files are experiments, tests or artificts.

 Agent Instructions
 ------------
 This code is very precisely written to handle a wide variety of exceptional cases. Be very through in looking for side-effects of seemlingly innocent code changes. Change as little as possible and use good programing techniques like avoid special-casing or quick fixes for a bug that fix the symptom but not the underlying cause. Always test to verify that your theory about what is causing a bug is actually the real cause. Discuss with the user when in doubt.

 Make sure the user understands what changes have been made. If the user cannot grok what is going on progress will or should stop.


 =========================
Build the current C++ target with:
    codeDog Proteus.Lib.dog

Run the focused graph suite from LocalBuild:
    ./TestProteus -t unit/graphFoundation/

The old language suite is still available there as ./TestProteus. Run it when
useful for the change, rather than preserving its pass count as a standing gate.
Check the printed failure summary; the harness may exit zero on test failures.

For a quick generated-C++ experiment, rebuild from LocalBuild with:
    codeDog -b TestProteus
Regenerate from the CodeDog source before accepting the final change.
