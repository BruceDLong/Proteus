# Field-selection contract

The user clarified this rule on 2026-09-26:

- Ordered list: `.wheel` selects the first compatible member in logical list order.
- Unordered list: matching members remain alternatives in a oneOf,
  `*_+[wheel1 wheel2]`.

The user subsequently retracted structural type equivalence. Type compatibility
requires the same nominal type or an explicit compatible type relationship;
equal field layout alone is insufficient. Proteus3's primary/additional-type
comparison provides the starting rule. Two sibling types do not become equal
merely because they share a parent type. Untyped structural predicates continue
to compare explicit constraints.

## Consequence for the older examples

```proteus
{#pedals={position:_ torque:_} #wheel={position:_ torque:_}}
{pedals:{position:321 torque:4} wheel:{position:210 torque:5}}.wheel
```

The result must be `wheel:{position:210 torque:5}`. The original `query/byType1`
and `query/chain` expectations are consistent with this corrected contract;
the previous claim that they were obsolete has been withdrawn.

A missing declaration is permitted. It does not erase the distinction between
`pedal` and `wheel`. Completed queries reconsider newly supplied definitions when
explicitly normalized again, as clarified by the user.

For now a typed query requires the correct type word (including explicit alias
or additional-type evidence). A description such as "the round thing with
spokes" may eventually establish identity when its definition is sufficiently
complete; deciding when that is justified is deferred. The same type-word gate
applies to ordered and unordered selection.

Selecting an existing member checks its type and the extra constraints written
in the query. It does not first validate every field of its type definition.
Definitions may supply a queried fact or prove an exclusion, but matching
structure alone cannot supply missing nominal identity. Selection and full
normalization of the selected source remain separate goals.

## Implications for graph search

Use the same compatibility evidence for both order modes. Order mode controls
which result to publish, rather than changing what counts as a match:

1. An ordered selector needs a compatible candidate and evidence excluding earlier
   logical members. A cheap route to a later candidate is not sufficient. If an
   earlier candidate cannot yet be classified, keep that dependency visible.
2. An unordered selector retains alternatives instead of committing to whichever
   graph route finishes first. If relevant candidates or the list extent remain
   unresolved, keep the result provisional with the necessary subscriptions.
3. Cost controls exploration and materialization. It must not silently change the
   winner in an ordered list or discard an unordered alternative while reporting
   a complete result.

These are consequences of the selection rule. Partial oneOf publication keeps an
explicit unresolved tail. Reuse source POVs and their checked
meaning evidence; avoid copying/reapplying definitions for every candidate.
The earlier permission to choose the first complete ambiguous grammar parse is
separate from this unordered field-selection requirement.

## Current implementation boundary

Dot queries, including read-only chains, and `GraphNormalizer.findField` use
nominal compatibility, scalar constraints, and finite ordered or unordered
structural constraints. Each path segment is another selection goal over the preceding
result. See `Historical/chained-field-results.md` (Git history) and
`Historical/structured-field-results.md` (Git history). Unambiguous writes
and retained parents are covered in `Historical/path-write-results.md` (Git history)
and `Historical/partial-type-results.md` (Git history). Partial unordered
predicates compare only the requested typed properties, including nested
properties. Missing relevant facts remain subscribed; unrelated unknown fields
do not need normalization. General contextual theorem matching remains pending. Partial publication and continuation are described in
`Historical/unordered-field-results.md` (Git history).

## Unordered descriptions are partial

The user clarified that an unordered list has an implied open tail: a model need
not enumerate every part of an aircraft carrier, and a particular bike may have a
water bottle holder beyond the parts in its definition. Therefore omission is
not evidence of absence. A missing `position` cannot by itself exclude an
already-selected wheel from `.wheel.position`.

The graph adapter exposes that virtual remainder through the same list boundary
interface used for explicit tails. Do not infer exhaustive membership from
physical end alone. An explicitly supplied size limits the total membership;
when all that many members are known, the remainder is empty. A large bound is
not a request to materialize its missing members for a field query.

This also permits describing a change to one part while making no assertion
about changes to other parts. A membership bound does not freeze member states.
A partial query can publish known alternatives and retain a subscription for
unresolved candidates, branches, or membership. Later dot segments traverse the
published alternatives without requiring complete parent normalization.

A completed field search with no matches returns a closed empty oneOf (`*_+[]`),
not a normalization failure. An incomplete search with no known matches remains
an open choice. The user's motivation includes temporal models of disassembly;
absence at a particular time and omission from a partial description must remain
distinct. See [open-questions.md](open-questions.md) for unresolved temporal rules.
