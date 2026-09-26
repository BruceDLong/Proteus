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

These are consequences of the selection rule. Detailed handling of partial oneOf
publication remains implementation work. Reuse source POVs and their checked
meaning evidence; avoid copying/reapplying definitions for every candidate.
The earlier permission to choose the first complete ambiguous grammar parse is
separate from this unordered field-selection requirement.

## Current implementation boundary

Single-step dot queries and `GraphNormalizer.findField` use nominal compatibility,
scalar constraints, and finite ordered structural constraints. See
[structured-field-results.md](structured-field-results.md). Chained paths,
writes/retained parents, contextual meanings, unordered structure correspondence,
and partial publication of an unordered result remain pending.
