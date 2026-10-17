# Numeric literal normalization, 2026-09-26

This ports the local rule in Proteus3 `WorldManager.rangeRules` into the graph's
ordinary Set path. Literal numeric fractions are canonicalized, and an inverted
numeric value with known size becomes `size - value`. A negative value with
unknown size remains symbolic. The generic scheduler is unchanged.

The GoalSpec plans the local value change before comparing it with RHS evidence.
The Set publishes a revision, so the same graph relationships are reconsidered
with canonical evidence. A representation-only change such as `6/12` to `1/2`
also publishes a revision; mathematical equality alone cannot detect that change.

Definition facts use the same canonical numeric projection without mutating a
shared body. Thus `{#half=6/12 half:1/2}` satisfies the definition while retaining
`6/12` in the stored declaration. The process handle is checked, not just the
printed result.

Validation:

- Full CodeDog regeneration and C++ build passed.
- Existing focused graph/parser suites: 508/508.
- Numeric composition checks: 11/11, including rational equality, sized and
  unsized negatives, RHS evidence, nested members, and read-only definitions.
- Older default suite: 141/270, up from 138/270. `parse/frac`, `neg/simple`, and
  `fracs/simple` now pass; no previous pass was lost and no old expectation changed.
- Remaining default failures: 129. Other older suites, the standalone streaming
  binary, and sanitizers were not rerun for this numeric change. The focused
  graph stream/lifecycle checks are included in the 508.

Run from `LocalBuild`:

```sh
./TestProteus -t ../graph/numeric-literal-tests.tst:
```

## Rejected RHS contract awaiting a design decision

Proteus3 `DirectConsolidationStrategy` reports a mismatched literal as rejected.
`AItemLifecycle.completeSourceWorkConstraint` classifies it as `cdContradiction`
and removes the terminal work item. Consequently the older test for
`*_+8 = *16+9` expects `*_+8`, without copying the RHS size. The Proteus3 executable
also confirms this behavior, including a rejected overlay type and a second
conflicting RHS.

The graph foundation deliberately retains conjunctive work. A contradictory
literal fails its handle and leaves the RHS visible; its conflict tests require
both failure and retention. An incompatible overlay currently stays symbolic.
Those are policy differences, not numeric normalization bugs. Before migrating
selection/matching rejection, decide whether to consume rejected work as in
Proteus3 or retain the current graph reporting contract. The old text tests do
not themselves specify a process-handle failure policy, so rendering and retained
diagnostics can also be separated if that is the intended contract.

This change does not alter rejection handling or either set of conflict tests.
