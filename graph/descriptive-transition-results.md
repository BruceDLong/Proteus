# Descriptive state 2 checkpoint (2026-10-03)

Slipstream's `Resources/userKB.pr` now contains a second temporal occurrence.
The same input is preserved in `Resources/TestFiles/descriptiveRubixHistory.pr`.
It selects the top-front edge by its location `(0, 1.1, 1.1)` and supplies only
`orientation={xRot:-1.570796326794897,yRot:3.141592653589793}`. The intended
half-turn exchanges the red and white sticker directions without moving the
cubie. No intrinsic cubie index, duplicated solved state, or verb is introduced.

## Ordering repair

A descriptive path could normalize a tentative target while still locating it.
That allowed temporal preservation to fill the old scalar before the supplied
change was attached, producing `x:0 = 5` in the reduced two-part example.

`exposeStructure` lets a graph subgoal expose immediate structure and relationships
without recursively normalizing member values. Retained writes locate their targets
and attach their RHSs before requesting the enclosing values. Provisional write
candidates are not normalized until the update is ready. Field predicates request
the facts they need, trying existing read-only definition evidence first. Explicit
RHS representation checks retain their previous normalization requirements.

The generic scheduler, scalar equality rules and numeric extent rules are unchanged.
The reduced `graphPreservation/descriptiveTarget` regression checks the changed
scalar, its preserved sibling, both neighbor fields, and the original state.

## Full-model result: rotation remains pending

The integration exercise obtains 26 cubies in both states. The second state keeps
the old rotation: the new RHS is retained but not applied. The new
`graphDescriptive/modelTransition` test intentionally asserts the requested flip
and currently fails. This is not a successful transition yet.

A read-only diagnostic classified the twelve edge candidates as one proved match,
eight proved mismatches, and three unknown comparisons. The unknown candidates are
at `(0,-1.1,1.1)`, `(0,1.1,-1.1)`, and `(0,-1.1,-1.1)`. Therefore the descriptive
selector has one known match plus unresolved alternatives; withholding its write
is consistent with the existing unique-target contract.

`GraphTypeFacts::pendingNumericValue` preserves extent-relative interpretation:
when neither size is known, `1.1` versus `-1.1` remains undecided because an extent
of `2.2` could translate the latter to the former. A two-cubie reduction confirms
that changing only the competing Y coordinate from `0` to `-1.1` prevents the
write. Positive-coordinate examples, nested locations, partial color definitions,
and inherited side references pass.

The open modeling question is how to specify ordinary, non-wrapping signed
location coordinates while retaining extent-relative negative values elsewhere.
The user was asked this explicitly; the numeric rule has not been relaxed and
negative coordinates have not been removed from the model.

## Validation

- The broader graph audit passed 1,400 checks in 73 groups.
- Temporal tests passed 51/51; the existing descriptive checks passed 16/16.
- The new full-model transition is a separate known failure: 1/1.
- World geometry checks still verify 26 cubies, 54 face-local slots, and nine
  outward stickers per color.
- The optimized Slipstream display was exercised with `>`, `<`, then `>`.
  Both temporal occurrences render a complete cube after materialization, but
  the expected flip is absent. The first display of state 2 is initially partial
  and still takes substantial time. These runs included concurrent diagnostics
  and builds, so they are not isolated performance measurements.
- This is graph coverage, not a claim that the entire old language suite passes.

Logs and screenshots: `/tmp/p4-state2-check/`, `/tmp/p4-state2-regressions/`,
`/tmp/p4-state2-predicate/run.log`, and `/tmp/p4-state2-display-replay/`.
