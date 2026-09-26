# Field-selection contract

The user clarified this rule on 2026-09-26:

- Ordered list: `.wheel` selects the first compatible member in logical list order.
- Unordered list: matching members remain alternatives in a oneOf,
  `*_+[wheel1 wheel2]`.

Compatibility follows type meanings, structure, distinguishing properties, and
context. A member's written type label is not a record key and does not outrank an
earlier compatible member. The labels `wheel1` and `wheel2` above identify the
alternative results; they do not require renaming the stored source members.

## Consequence for the older examples

```proteus
{#pedals={position:_ torque:_} #wheel={position:_ torque:_}}
{pedals:{position:321 torque:4} wheel:{position:210 torque:5}}.wheel
```

Both members have compatible structure. The ordered query chooses the first
member, whose position is 321 and torque is 4. Its explicit `pedals` label does not
exclude it. The old `query/byType1` expectation chooses the second member and is
therefore obsolete under this clarified rule; `query/chain` has the corresponding
position-selection expectation. This note identifies semantic targets without
changing the historical default fixtures or their recorded results.

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

Scalar sequence predicates already compare context-free type meanings read-only.
Dot/path lowering, structured-member predicates, contextual meanings, and
unordered oneOf publication are not yet implemented in the graph adapter. This
checkpoint records the resolved contract; it makes no runtime behavior change.
