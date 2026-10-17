# Proteus4 open design questions

This is a discussion list, not a list of features that must be added immediately.
The temporal questions below come from the user's connection between missing
parts and disassembly. Other entries collect deferred design topics and questions
exposed by their composition; they are not claims that previous decisions have
been reopened.

## Questions to discuss

1. **Temporal membership and identity.** How do we state that a wheel stops being
   part of a bicycle at a particular time? Does the same wheel retain its identity
   as an independent object or as a part of something else? What evidence
   distinguishes an unmentioned part from a removed, destroyed, or replaced part?

2. **Implementing preservation across change.** The default preservation rule is
   settled below. How do graph dependencies represent its exceptions from implied
   changes, and how should tentative preserved results respond to later evidence?
   How are consequences with different temporal supports related? When does a
   membership bound apply to one time versus an interval?

3. **Subscriptions to temporal results.** Should a query follow a part as the
   model evolves, describe a fixed time, or explicitly choose between those
   goals? What wakes a previously empty result when a part is assembled or
   removed? This is separate from the settled rule that completed queries
   reconsider newly supplied definitions on explicit re-normalization.

4. **Sufficient descriptions.** When is a definition complete enough for a
   description such as "the round thing with spokes" to identify a wheel without
   its type word? How is that completeness represented, and which evidence can
   be reused? For now the correct type word or explicit type relationship is
   required; matching structure alone does not establish the type.

5. **Context and overloaded meanings.** How does the graph select the intended
   meaning when the same word has multiple definitions? Which POV/context
   relationships bind that meaning, and what context must travel with a cached
   proof so that it cannot be reused incorrectly elsewhere?
   Definition applications keep their original context binding for now, including
   on explicit re-normalization. When the engine can interact with the caller
   better, how can it request and record authorization to rebind an application?
   What scope should that authorization cover, and how should dependent evidence
   and results be reconsidered? Rebinding must not happen implicitly. Ordinary
   `%ctx`/`~word` queries use live topic-history source edges; completed queries
   reconsider those edges on explicit normalization.
   The user suggested representing context as a T-list. This could make the
   original binding an ordinary POV into a particular context occurrence;
   subsequent context occurrences would not move that binding. The remaining
   representation question is whether entries describe successive context
   states (which may be partial) or record mentions/updates from which context
   must be reconstructed. The current topic history refreshes recency and culls
   entries, so a T flag alone does not preserve historical context. Retention
   must keep bound occurrences meaningful; learning more about an existing
   occurrence must remain distinct from creating a later context state. This
   proposal does not decide component persistence or require full snapshots.

6. **Related alternatives.** How do oneOf results preserve which facts belong to
   the same alternative? For example, position from wheel A must not be combined
   with color from wheel B as if they describe one wheel. How should nested
   choices and eventual multiple grammar parses share their derivations without
   enumerating every combination? The current grammar policy remains first
   complete parse; unordered field selection retains alternatives.

7. **Updates through a selection.** If an unordered selector has several matches,
   does an assignment require a unique target, constrain each alternative, or
   express alternative possible updates? How should retained-parent (`;`) paths
   and shared POV identities participate? The rule that unmentioned parts remain
   unconstrained must also hold for changes.

8. **Publishing partial parses.** What evidence is sufficient to publish a useful
   parsed prefix before the enclosing parse finishes? Which results are
   provisional, which can be committed to a stream consumer, and how should
   later information invalidate a provisional result without exploring all
   failing paths?

9. **Theorems, inverse relations, and commutators.** How do model definitions and
   future theorems expose conditional or virtual graph relationships, including
   inverse routes? What proof makes reordering operations valid, and how does
   traversal discover those possibilities without materializing the entire
   graph? Future fees or external-data costs can extend route costing later;
   graph-owned cost calculation and cheaper-route selection are already decided.

10. **Selecting definitions themselves.** Declarations currently publish meaning
    when encountered during a field walk and are not ordinary value candidates.
    Later, searches must be able to refer to definitions themselves. Which goal
    expresses that request while keeping ordinary value selection distinct?

## Decisions already made

- A change, including one expressed by a verb, preserves other values unless
  the applicable model implies changes to them (2026-09-29). Consequences can
  propagate: forward pedaling on a functioning bicycle implies chain and wheel
  movement, and typical riding can imply movement of the bicycle itself.
  Preservation therefore applies beyond both direct and model-implied effects;
  it is not limited to the fields explicitly written in the change. The graph
  implementation is pending. On 2026-10-01 the user clarified that T-list
  adjacency supplies this preservation: `{T {a:0, b:7} {a:1}}` retains `b:7`
  in state 2 unless the model implies a change. A later-state part reference
  targets that occurrence; its RHS describes the change without modifying the
  known prior state. No separate transition operator is needed. Disconnected
  descriptions outside such a history do not acquire this relationship.

- Assigning a conflicting known value is a contradiction unless the relationship
  explicitly uses `forceID` (written `<<=`). Ordinary `=` may supply an unknown
  value but must not overwrite a known one, through units or any other path.
  This was clarified on 2026-09-29. The graph implementation of `forceID`
  remains a separate gap; its parser and printer already preserve the flag.

- Preserve a rejected RHS and its context: it may match something else later.
  A failed comparison establishes no identity and supplies no continuation for
  that match. Exploratory goal rejection leaves other routes available; a
  contradictory explicit assertion still fails its owning required goal.
  Completed or failed requests can be normalized again after new information.
  Rejection evidence belongs to the query and revision, not globally to the RHS.

- Completed repeated `<~` selections are published while an unfinished
  repetition remains open and subscribed. A loaded source end alone does not
  supply its missing selection or final result size. The broader provisional
  parse/consumer/retraction questions above remain open.

- `=` and `==` require matching ends at the current pair; `=:` and `==:`
  permit unequal ends. The colon applies at that pair, while `==` propagates
  loose matching into members. A partial numeric match keeps the target size;
  a bare unknown RHS contributes no new value. It does not establish a whole
  identity between differently sized infons.

- A completed field search with no matches returns a closed empty oneOf
  (`*_+[]`) for now. An incomplete search retains an open choice.
- Unordered descriptions have an implied open remainder. An explicit total size
  bounds membership; it does not freeze the states of the members.
- Omission does not assert absence, change, or lack of change.
- Ordered lookup selects the first matching logical member. Unordered lookup
  retains matching alternatives; cost cannot change that semantic policy.
- Fields are typed members. Equal structure does not make `.pedal` match `.wheel`.
  Literal names can be expressed using `field^{name=X}`.
- Separate definitions are optional; selection does not first validate every
  member of a type definition.
- Supplied typed-field descriptions may be partial even when ordered; returning
  a parent normalizes its present fields without enforcing the full parent layout.
- Completed queries reconsider new definitions when explicitly normalized again.
- Existing definition applications retain their original context binding, even
  when explicitly normalized again after the Agent context changes. Future
  rebinding may be authorized through interaction with the caller; it is not
  currently automatic.
- The graph interface exposes existing POV/infon structure and relationships.
  Goals translate discovered information into Sets; normalization returns a
  controllable handle with completion waiting and cancellation.
- Cost chooses routes and limits unreasonable exploration. It is not an
  unconditional reason to abort a normalization that can still return a useful
  symbolic or subscribed result.

## Implementation gaps are tracked separately

Unordered structural correspondence, contextual resolution, general nested
parent-definition projection, ambiguous crossing spans, recursive grammars, fuller
streaming publication, and remaining temporal navigation need implementation.
Scalar parent-field projection and crossings with proven relative placement
are implemented.
Unambiguous field updates and retained-parent paths are implemented; ambiguous
updates still need the decision above. Some require the decisions
above; others should follow existing Proteus semantics and tests. A failing old
test is evidence to investigate, not automatically a new language-design question.

See [field-selection-contract.md](field-selection-contract.md),
`Historical/unordered-field-results.md` (Git history), and
`graph/language-compatibility-status.json` (Git history).

## Boundary names and typed fields

The `.begin` / `.end` boundary syntax can collide with ordinary typed-field
selection using those words. The user explicitly deferred resolving this
overload until a concrete modeling need arises. Keep the current boundary
behavior; do not redesign syntax as part of the AItem cleanup.

## Temporal independence marker (settled 2026-10-02)

T-list adjacency supplies default preservation unless the model implies a change.
Use `b:%?` to explicitly withhold preservation for the later value. Ordinary
`b:?` and `b:_` are infon descriptions, not declarations about preservation;
no authored-versus-generated unknown distinction is needed. `%?` permits other
constraints to determine the value, including the same value as before. It does
not erase a known value, assert inequality, or suppress explicit relationships.
`% ?` remains two infons. The user does not request new `!%` functionality.

Scalar and finite unambiguous part preservation now use guarded graph routes;
see `Historical/temporal-context-results.md` (Git history). Pending supplied
constraints prevent a default Set. Evidence arriving after a value has been
committed, general causal closure, membership changes, and sparse correspondence
remain separate implementation/design boundaries.
