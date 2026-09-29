# Locating values at a temporal cut

`findTemporalSide(cut, after)` now searches existing T-list members and explicit
history identities. A boundary identity locating the cut supplies the adjoining
S-occurrence. Nested T-lists contribute their existing composition identities;
an ordinary S-list remains one value and is not searched as a sequence of times.

For example, in `{T 0 0.5}`, identify an interior cut with the first occurrence's
end. A before query returns the first occurrence, and an after query returns the
second. An explicit identity to another history makes this information reachable
from that history too. Equating cuts alone does not equate histories or values.
An unchanged S-value with an interior cut aligned to this transition returns the
same source POV on both sides.

The traversal is in `TemporalGraph.h`. `temporalSideSpan` inspects a span and
`temporalSideMember` advances through stored members. These are ordinary graph
queries, with priced edges and existing boundary subgoals. The scheduler is
unchanged. Existing demanded sparse occurrences are also visible; absent sparse
members are never enumerated or allocated by this search. No new adjacency
cache, cube-specific branch, or state-copy operation was added.

Unlocated cuts and missing side values remain subscribed. Publishing an identity
or the missing value resumes those searches; cancellation stops the request.
A strict-interior cut identified with an outside endpoint is rejected. A cut
that identifies two distinct state occurrences on the same side is also rejected.
Completed results reconsider later constraints when explicitly queried again.

## Real cube data and explicit mappings

`unit/graphTemporal/cubeOrientationMapping` reads the existing
`public/Things/rubixCube.pr` asset. It registers the original declarations and
parses a separate occurrence of the stored `solvedState` body, preserving the
shared declaration. Fixture preparation reads the explicitly stored orientation/location of edge
cubelet 10 and the orientation of edge cubelet 11, without invoking the blocked
whole-definition/positional application. The test supplies an x-rotation
history changing from the selected 0 to 0.5, then explicitly identifies a second
history with that history. Both histories are queried at the same cut.

The y/z rotations, location, and neighboring cubelet orientation receive only
interior-cut alignments. They remain the same source S-values on both sides.
The original x-rotation and shared declaration remain unchanged. These are
explicit graph constraints in the test harness, not new Proteus verb syntax.

This exercises temporal mappings over real stored cube data. It does **not** yet
instantiate the entire cube from a bare `solvedState` word, apply a face-turn
verb, derive parent/part mappings, or infer preservation exceptions from arbitrary
causal rules. The test must not be cited as completing those features.

## Separate blockers exposed by the full model

1. `%INSERT` is still parsed as `mIncludeStream`, but no graph goal dispatches
   that directive. The old `rubixTransition.pr` import fixture was therefore not
   loading Foundation or the cube declarations. This is independent of numeric
   sign handling and temporal cut location.
2. Loading `rubixCube.pr` directly registers its definitions, but the known edge
   rotation query still waits while the whole definition compiler tries to read
   the unrelated contextual `centerCube` template. Removing only that declaration
   in a diagnostic copy allows the query to return its known open oneOf. No
   production model was changed. `graph/lazy-definition-blockers.tst` retains
   a small passing control and a deliberately failing regression for this issue.

The next general repair is demand-driven access to known finite definition parts
while unresolved contextual sibling requirements remain visible. It must avoid
mutating shared declarations or certifying an incompletely checked type. Restoring
`%INSERT` also needs the existing asynchronous loader lifecycle; a graph effect
must never synchronously wait for its own worker.

## Validation

- Full CodeDog regeneration and C++ build passed from the final source.
- **48/48 temporal tests pass**, including the 12 new cases. They cover located
  and late cut identities, nested histories, explicit mappings, unchanged
  S-values, endpoint and competing-location contradictions, cancellation,
  missing values, sparse lookup, and the actual stored cube-data exercise.
- The sparse located-cut test requests exactly two occurrences in a billion-item
  T-list. Reading both sides creates no further occurrence and no physical list
  member. An unlocated cut creates no repetition occurrences at all.
- Full default inventory: **1,615 pass / 40 fail / 1,655 total**. The failure
  names exactly match the prior 40/1,643 baseline. All **1,269 registered graph
  tests pass**. No existing expectation or skip was changed.
- The separate definition-blocker suite is intentionally **1 pass / 1 fail**;
  the known-field control passes while the contextual sibling still blocks the
  query. This regression is recorded, not included in the default inventory.
- `git diff --check` passed. No GUI, automatic preservation rule, parent/part
  projection, or complete verb application validation is claimed.

Final logs: `/tmp/p4-temporal-final-regeneration.log`,
`/tmp/p4-temporal-focused-final.log`, `/tmp/p4-temporal-default-final.log`, and
`/tmp/p4-lazy-definition-regression.log`. The focused run used temporary runtime
assets; its warnings about absent default language-suite files do not affect its
explicit unit-test selection. The full inventory ran from `LocalBuild` with the
normal staged resources.
