# Proposed next design discussion: time and words

This is a discussion plan, not authorization to implement the remaining rules.
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

## 1. Agree on examples before adding time rules

Use a one-page matrix of inputs, requested relations, expected identities and
expected values. Include a finite T-list, an open T-list, two adjacent nested
T-spans, a sparse span with a known extent, and two timelines with different
units/origins. For each ask for: begin/end, tprev/tnext, a position, and an item
at a stated time. Include missing information, late arrival, and a contradiction.

Keep three notions distinct in that matrix:

1. An item occurrence and its structural ordinal.
2. A boundary/cut between occurrences, including the two ends.
3. A temporal coordinate or interval, with its unit and origin when known.

The existing boundary tests print zero, but two boundaries that both print zero
need not be the same node. Decide what their identities mean before coding the
boundary lookup. Also settle what a temporal query outside a T-list means:
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

Recommended first discussion: the time example matrix, starting with boundary
identity and nested tprev/tnext. After those contracts are agreed, implement
boundaries, then temporal adjacency, with cost/cancellation/sparse/replay checks.
The word matrix can establish the name/value/reference distinction before
implementing further name rules. Continue reporting old-suite failures and semantic ambiguities
separately throughout.
