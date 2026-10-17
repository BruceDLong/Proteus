# Descriptive parts and the face-based Rubix model

The model uses 26 physical cubies in unordered typed collections. Color-role
descriptions distinguish center, edge and corner cubies. Six faces contain
54 local slots whose occupants reference those cubies. Slot coordinates belong
to the face frame; they are not asserted as the physical cubie's world location.
No cubie has an intrinsic numeric identifier. The initial poses preserve handedness.

The model is `World/public/Things/rubixCube.pr`. Slipstream's `userKB.pr` now
instantiates `rubix-cube` with `solved-rubix-state`. The subsequent change-only
state-2 exercise is documented in [descriptive-transition-results.md](descriptive-transition-results.md);
its rotation is currently pending on coordinate-comparison semantics. The separate draft in Slipstream's `Resources/rubixCube.pr`
is untouched; runtime loading uses `Resources/public/Things/rubixCube.pr`.

## General engine changes

- Finite unordered descriptions require nominal compatibility and the supplied
  properties, recursively. Unrelated unknown properties need not be resolved.
  Missing or ambiguous relevant information remains pending.
- Bare type selection uses nominal identity. A distinct same-shaped type does
  not match. Definition aliases remain explicit evidence.
- Repeated nominal parts in a new definition create independent occurrences.
  Existing occurrences use distinguishing facts through `MemberCorrespondence`;
  ambiguous evidence does not trigger permutation search.
- Unordered omitted membership and an explicit unfinished positional tail are
  distinct when deciding whether a supplied nested description may stay lazy.
- `~` resolves each oneOf branch with its own direct-before-external precedence.
  Retained path effects are available to those searches. Implicit membership
  metadata survives filtering, so known provisional results stay inspectable.
- Acquiring an LHS list shape no longer hides an unresolved named RHS. The
  definition supplying that RHS is still a graph route.

The generic scheduler and list correspondence algorithm were not changed.

## Renderer

The existing hue palette remains in use. Chromatic colors remain partial.
Known achromatic gray/white use saturation zero and brightness; unresolved
colors temporarily display pink. White was
added to Foundation. Rendering requests each visible side, consistent with the
one-level normalization rule. Read-only field access can inspect a singleton
selection whose only open information is implicit membership, without committing
a choice or treating an explicit unfinished alternative list as complete.
An assembly's `parts` collection supplies its geometry, avoiding duplicate draws
of its descriptive face references.

## Validation and remaining work

`World/tests/check_rubix_cube.py` independently checks 26 cubies, 54 transformed
slot positions, proper outward sticker normals and nine stickers per color.
The compiled palette test reads the actual Foundation/model assets and requests
all 156 physical surfaces. It is intentionally stricter than a screenshot:
renderer fallback is not evidence that a color reference resolved. All 54
outward stickers must resolve to their specified colors. The other 102 backing
surfaces may remain unresolved; at the user's request these render in pink while
color semantics are developed. The deeper inherited color-meaning experiment was
reverted, rather than expanding this task into additional color work.

A complete chained face-slot query exceeded a 35-second exploratory limit and
still needs investigation. Partial colors can also leave candidate distinctions
unknown; this increment does not invent saturation, brightness, or a hue for
white to force a unique match. Verbs and state transitions are outside this model
increment. The existing temporal cut test uses the red-white and red-blue edge
descriptions instead of old positional indices.

### Verified 2026-10-03

- Proteus4 and Slipstream both completed full CodeDog source regeneration and
  optimized builds (`Marker: Build Successful`).
- The 73 other graph groups passed 1,399/1,399 checks. Final-source focused runs
  passed `graphDescriptive` 16/16 and `graphTemporal` 51/51: 1,466 checks total.
  This is graph regression coverage, not a claim about the entire legacy suite.
- The static geometry checker passed, and the model/Foundation source files
  exactly match the assets installed in both generated builds.
- An isolated Slipstream run used those installed assets and the pink fallback.
  Screenshots are in `/tmp/p4-new-model-render-pink/`; resolved stickers replace
  pink as their display demands finish. Centers are present.

Build logs: `/tmp/p4-rubix-accepted-build.log`, `/tmp/p4-rubix-pink-build.log`.
Test logs: `/tmp/p4-rubix-installed-regressions/`,
`/tmp/p4-rubix-final-descriptive.log`, `/tmp/p4-rubix-final-temporal.log`.
