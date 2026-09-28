# Time and words: agreed foundation and next discussion

The null/boundary foundation below was approved and implemented; see
[boundary-results.md](boundary-results.md). The remaining time and word rules
still require discussion.
Use a small set of semantic relationships and shared graph operations. A test
should demonstrate a language construct or a composition of constructs, not
introduce another dispatch exception.

## Existing commitments

- A list's size is its number of items. Duration, coordinates, and rates are
  separate facts. Preserve sparse membership and streaming access.
- `%prev` and `%next` identify structural neighbors. Their use inside arithmetic
  has been removed, including the old `+1` recurrence examples. Do not reintroduce
  it as part of temporal navigation without an explicit design decision.
- Normalization can settle symbolic and subscribed; a missing temporal relation
  is not an excuse to fall back to an ordinary structural neighbor.
- Typed fields require the relevant word or an explicit alias. Shape alone does
  not equate siblings. Selection does not validate all parts of a definition.
- Ordered lookup selects the first compatible member; unordered lookup preserves
  alternatives and possible unseen members. Cost chooses computational routes,
  not a different semantic winner.
- Existing context bindings are retained. Completed queries reconsider new
  definitions on a new normalize call; rebinding requires caller authorization.
- Definitions should eventually be searchable directly. The current catalogue
  is an implementation bridge, not a permanent second knowledge system.

## Agreed null parsing and printing contract

The null state is exactly `infSize=0`, `value=0`, and
`value.overlayType=tUnknown`. Both zeros are known values, not default payloads
of unresolved states. This allows null to be recognized from the existing state
fields without requiring a separate null flag. Unknown overlay alone is not
sufficient.

Print this state as `NULL:` if no identity ID is assigned, or `NULL:<ID>` when
an ID is assigned (for example, `NULL:647`). Always retain the colon; bare
`NULL` remains a word in the printed syntax.

Parsing `NULL:` constructs the same known-zero size/value state with unknown
value overlay. A positive ID suffix identifies the occurrence, not a numeric
value assignment. Recognize this literal alongside `?` in the inner-infon
parser, ensuring the outer word parser does not consume the reserved `NULL:`
form first. Bare `NULL` retains its ordinary word meaning.

Parsing `?` must explicitly set `value.overlayType=tUnknown`, as well as its
existing unknown size/value evaluation modes. The parser now makes that
overlay assignment explicitly. Null's size and value
are known, while those of `?` remain unknown.

Assigned IDs must be positive and never zero. Zero may serve internally as the
unassigned sentinel, but the printer omits that sentinel rather than emitting
`NULL:0`. Allocation must start at one and must not wrap around to zero.
The ID identifies the occurrence; it is not the null's value. Zero is the only
valid null value, which is why printing `NULL:0` would be misleading.

IDs belong to an explicit identity scope, normally the Agent's registry.
Repeated IDs within that scope identify the same occurrence, including across
inputs sharing that scope. Independent model imports remap their IDs into the
destination scope unless identity sharing is explicitly intended. Remapping
preserves repeated references within the imported model; matching numeric IDs
from independent scopes alone do not establish identity.

The graph implementation now uses this null state and identity scope. The
legacy AItem boundary constructor and its numeric placeholder were removed
with the old engine.

## Agreed boundary availability

Every infon occurrence has `.begin` and `.end`; boundary access is not limited
to T-lists or explicit temporal projections. This includes scalar, list, null,
and incompletely known infons. Boundary existence does not require knowing its
coordinate or the infon's extent. Which relationships identify boundaries with
one another is a separate semantic question.

For a null occurrence `N`, `N.begin`, `N.end`, and `N` identify the same
occurrence. Boundary access returns that occurrence rather than allocating a
new boundary. If an ID is assigned, all three references denote that same ID
within its identity scope. This rule does not identify unrelated null
occurrences merely because their size, value, and overlay state match.

## Agreed identity at adjoining boundaries

For now, only T-list composition implies shared adjoining boundaries:
`A.end = B.begin` for adjacent members of `{T A B}`. Use this identity
rather than introducing `A.end.next = B.begin` as a separate
connection between two boundaries. Retain the contextual POVs that describe
A's end and B's beginning; their roles and the containing structure supply
ordering and navigation. Boundary identity does not require physically merging
POVs or identifying the whole spans A and B.

This also applies when an adjoining T-list member is null. Together with the
null self-boundary rule, adjacent null entries in a T-list may refer to one boundary while
remaining separate list positions with their own POVs. Do not remove entries,
change list size, or discard structural order merely because their boundaries
are identical. The full singlet theory is not required for this implementation.

Ordinary ordered-list adjacency in `{A B}` does not imply this identity under
the current contract. Every infon still has `.begin` and `.end`. Extending the
composition rule beyond T-lists is deferred until the user develops the theory
further and explicitly changes this decision. The implementation need not wait
for that theoretical work.

## Agreed T-list container boundaries

For a closed, nonempty T-list `L = {T A B}`, establish:

```
L.begin = A.begin
A.end   = B.begin
B.end   = L.end
```

The same rules compose through nested T-lists using ordinary boundary identity
relationships.

For an open T-list `L = {T A B ...}`, the first two identities still hold, but
the received prefix does not establish `B.end = L.end`. `L.end` denotes the
eventual end and can have an identity before its location is known. Closing the
stream with B as the final member establishes the last identity. Pending closure
does not assert that the two boundaries are unequal.

For a closed empty T-list `L = {T}`, `L.begin = L.end` identifies one boundary.
An open T-list having no received members does not by itself establish that
identity. If it closes empty, establish the shared begin/end boundary then.

## Agreed state contexts and interior cuts

In a T-context, `LHS[n+1] = RHS[n]` is the normal assignment reading:
`LHS.current = RHS.prev`, equivalently `LHS.next = RHS.current`. These are
relative evaluation contexts, not whole-occurrence or lifetime identities.
An unchanged S-value can be reached from both sides of an interior cut. Those
are two routes to the same value; the cut does not require copied states or a
new transition. Equal values at different temporal occurrences still have
distinct supports.

The September 28 design note also distinguishes an instance lifetime from a
state occurrence's support and a part-role interval. A part's lifetime may
extend outside its role. Projections preserve their source, schema and support;
a parent cut need not be a child state transition. These relationships extend
the existing POV graph rather than requiring separate classes of graph nodes.
General supported role/schema mapping remains a later implementation step.

The initial graph implementation is described in
[temporal-context-results.md](temporal-context-results.md). It implements
explicit temporal value reads, symbolic support references, and interior cuts.
It does not change word/context binding or infer persistence from omitted parts.

## 1. Agree on examples before adding time rules

Use a one-page matrix of inputs, requested relations, expected identities and
expected values. Include scalar, null, unknown, and ordinary-list boundary
queries, a finite T-list, an open T-list, two adjacent nested T-spans, a sparse
span with a known extent, and two timelines with different units/origins. For
each ask for: begin/end, tprev/tnext, a position, and an item at a stated time. Include missing information, late arrival, and a contradiction.

Keep three notions distinct in that matrix:

1. An item occurrence and its structural ordinal.
2. A boundary/cut between occurrences, including the two ends.
3. A temporal coordinate or interval, with its unit and origin when known.

Boundary fixtures now print identity-bearing nulls. Equal null states alone do
not identify their boundaries; the graph establishes the agreed identities
while retaining contextual POVs. Boundary access outside T-lists is agreed; separately
settle what temporal-neighbor queries such as `%tprev` outside a T-list mean:
unknown/subscribed, empty result, or a definite rejection.

## 2. Implement time relationships in dependency order

First expose identity-bearing boundaries and their incidence with existing
occurrences. Then define tprev/tnext using those relations, including crossings
between adjacent nested T-spans. Only then connect coordinates, origins, and
unit conversions to those same occurrences/boundaries. Reuse the existing span
layout and sparse indexing rather than materializing every intermediate item.

After navigation works, decide how a state change is represented. A rule that
relates states should contribute ordinary conditional relationships and
requirements. Do not smuggle arithmetic recurrence or temporal inheritance into
list traversal. Omitted unordered parts remain non-relevant; omission alone
must not become a removal or an assertion that a part stayed unchanged.

## 3. Separate the meanings involved in a word

Build a corresponding example matrix for:

- Written spelling and its language/model metadata.
- Declared type/description and explicit aliases.
- The value described by a definition.
- An occurrence selected using that description in a context.
- A name value, an asserted name reference, and the entity bearing the name.
- Collection descriptions and singular/plural spellings.

Use the existing Morgan/name, typed association, color and descriptive-search
fixtures as acceptance examples. Add one counterexample per distinction, such
as two siblings sharing a structure and a name remaining a name after a bearer
is associated with it.

## 4. Make word-derived relationships explicit

Keep spelling changes, collection-type inference, definition lookup and context
selection as distinct contributions to the graph. They can share goal/Set
interfaces without becoming a large catalogue of rule classes. Preserve the
written WordUse separately from the normalized comparison key.

Discuss when English inflection is sufficient evidence versus a proposed
interpretation: ambiguous or invariant plurals, singular words ending in s,
explicit declarations that override spelling, locale/model differences, and
unknown words. The small current rule restores ordinary plurals; it does not
settle these wider language questions.

For text, distinguish tokenization from interpretation. Specify whitespace,
punctuation, quoting and Unicode behavior before broadening the legacy
space-delimited splitter. A conversion from text to phrase must account for
source identity, character extent versus item count, and any RHS constraints;
it must not silently turn a character-count constraint into a word-count one.

## 5. Use evidence to avoid redundant work

For both time and words, record which source relationships and context justified
a result. Reuse valid evidence across aliases and repeated normalization. Keep
unknown premises visible as subgoals or subscriptions. New facts may expose
new routes; a cached completed expansion must not hide them. Proposed routes
must not specialize shared definitions or commit tentative alternatives.

Cost estimates belong to the graph's operations. Semantic providers expose the
work required, including sparse jumps, scans and conditional refinements. Do
not enumerate all combinations merely to estimate their cost.

## 6. Choose one narrow implementation slice together

Boundary identity, explicit nested temporal reads, symbolic support, and interior
S-state cuts are now implemented with cost/cancellation/sparse/replay checks.
The next temporal slice is a supported parent/part projection: one parent
interval mapped to a child history containing several states, without creating
child transitions at parent cuts. Reuse the source span and selection machinery.
The word matrix can establish the name/value/reference distinction before
implementing further name rules. Continue reporting old-suite failures and
semantic ambiguities separately throughout.
