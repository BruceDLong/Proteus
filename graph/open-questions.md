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

2. **State across time.** When a later description mentions only the part that
   changed, how can the reasoner obtain other parts' states? Is persistence an
   explicit relation or theorem, a property of a temporal model, or something
   else? Omission itself must not assert either change or no change. When does a
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

## Decisions already made

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
- The graph interface exposes existing POV/infon structure and relationships.
  Goals translate discovered information into Sets; normalization returns a
  controllable handle with completion waiting and cancellation.
- Cost chooses routes and limits unreasonable exploration. It is not an
  unconditional reason to abort a normalization that can still return a useful
  symbolic or subscribed result.

## Implementation gaps are tracked separately

Unordered structural correspondence, contextual resolution, parent-definition
projection onto selected fields, crossing open spans, recursive grammars, fuller
streaming publication, and remaining temporal navigation need implementation.
Unambiguous field updates and retained-parent paths are implemented; ambiguous
updates still need the decision above. Some require the decisions
above; others should follow existing Proteus semantics and tests. A failing old
test is evidence to investigate, not automatically a new language-design question.

See [field-selection-contract.md](field-selection-contract.md),
[unordered-field-results.md](unordered-field-results.md), and
[language-compatibility-status.json](language-compatibility-status.json).
