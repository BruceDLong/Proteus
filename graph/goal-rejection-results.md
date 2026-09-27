# Goal-local rejection

A failed comparison must leave its RHS available to another comparison. In
sequence matching, a mismatch already supplies no continuation and consumes no
input. The same isolation now applies to exploratory normalization subgoals.

## Shared rule

`GoalRejected` represents a proved contradiction in the current goal. The
manager records it on the existing search task, invalidates that task's queued
visits and Sets, and propagates rejection only through required dependencies.
An exploratory parent keeps its other routes. If a required chain reaches the
root, the handle still reports `failed` with the reason.

Rejection does not delete a source POV, rewrite it as invalid, or establish an
identity. Failed assertions remain inspectable on the existing work list.
Scalar, numeric-range, grouping, and definition contradictions use this common
outcome; engine/input/ownership errors still fail the whole session. Missing
information remains deferred. No syntax-specific failure scheduler, shadow
adjacency, or separate store of retired RHS nodes was added.

One query is rejected at most once per revision. A cheaper route cannot repair
a proved contradiction. Publication restarts subscribed searches under the
existing context-wide revision policy; failed root handles are terminal until
the caller normalizes again. No semantic information changes during settlement.

## Concrete behavior

- `*10+8 = *10+9` fails, retaining the original constraint. The same source
  `*10+9` can then supply a different unknown target successfully.
- A rejected exploratory goal cannot abort a slower route using the same source.
- A conflicting optional parent-definition projection leaves a missing field
  symbolic and subscribed; another RHS can still supply the requested value.
- Required definition or scalar contradictions still fail the owning request.
- A failed grammar match leaves the same input available to another grammar.

The old `graphProjection/conflictAtomic` assertion expected an optional lookup
to fail its entire request. It now checks local rejection, unchanged unknown
value, and a subscribed parent. No language fixture expectations were changed.

## Validation

- Full CodeDog generation and C++ build passed.
- **1,009/1,009** focused graph/parser checks pass, including seven new
  source-reuse/projection/grammar regressions.
- **38/38** standalone scheduler scenarios pass, including eight new rejection
  scenarios. The same suite passes with address, undefined-behavior, and leak
  sanitizers outside the ptrace sandbox.
- All six older language suites retain exactly their previous per-fixture
  outcomes: **218/363** pass, including **203/270** defaults. No crashes,
  timeouts, missing summaries, or lost passing fixtures.
- Registered legacy helper/parser-error checks remain **102/125**.

Logs are `/tmp/p4-rejection-verified-build.log`, `/tmp/p4-rejection-focused.log`,
and `/tmp/p4-rejection-audit-*`. The initial standalone regression failed on the
previous runtime: one exploratory mismatch aborted the slower successful route.

## Boundaries

This is failure isolation, not transactional backtracking of already published
Sets. Earlier valid publications remain inspectable as before. Ordered lookup
still cannot skip an unresolved earlier candidate merely because a later one
matches. This change also does not discard failed assertions or suppress their
printing to satisfy old output expectations such as `merge/directRange/endReject`.
