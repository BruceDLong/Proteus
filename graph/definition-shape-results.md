# Sparse definition shape before member meaning

A type application can now publish its known sparse list header even when the
member meaning remains unresolved. For example:

```proteus
#part={...}
#history=*1000000000+{T {&part}|...}
history
```

The result has its known billion-position bound, T-list interpretation, and
shared member template. No history member is allocated by that publication.
An indexed goal can subsequently reach a requested virtual occurrence directly.
The unknown member extent remains a required obligation with a subscription;
this publication does not certify the type or mark the result `isNormed`.

The existing type goal first attempts its full read-only compilation. If that
walk settles without completing, its ordinary `finishPass` hook can offer a
partial Set. `ProteusDefinitions.exposeShape` combines the headers along named
meanings and top-level RHS relationships, without descending into members.
Header contradictions reject before any write. A distinct competing template,
ambiguous meaning, contextual header, or unsupported registry requirement stays
pending. Inspection and alias traversal contribute to the Set's cost. Rejected
or cancelled goals cannot publish; cost filtering uses the existing runtime.

The Set reuses `adoptDefinitionList`, `applyTypeFacts`, `setDefinitionOrder`, and
`installMemberTemplate`. Existing members and occurrence POVs are preserved.
The template remains the declaration's read-only infonView. Full validation,
late refinements, and explicit re-normalization continue through the existing
type and member goals. No new graph node kind, scheduler branch, or application
RHS was added.

The unchanged `cube/getLstSpec` fixture now exposes the desired T-list and
`{&thing}` template. It still fails its exact string comparison: the printer
retains the unknown embedded extent as `*_+{&thing}`. A parse-only probe confirms
that this marker already appears on the original unnormalized template.
The source and expected output have not been changed, and the fixture is still
counted as failing.

The next composition gap is unordered typed definition bodies and prefixed
member templates. This increment does not enforce omitted parts, equate field
names with unique record keys, project parent/child temporal intervals, or
normalize shared declarations.

## Validation

- Full CodeDog source regeneration/build passed.
- All 14 new shape checks pass, including billion-position sparse access,
  aliases, loaded prefixes, size/overlay and joint-header conflicts, ambiguous
  templates, cost exclusion, replay, late declaration, cancellation, and late
  member meaning. Source bodies remain unchanged outside the explicit late
  update case. The 15 language-tag tests also pass.
- Compiled audit: **1,206 pass / 8 existing fail**, including **1,156/1,156**
  graph tests. Every pre-existing compiled status is unchanged.
- Six old language suites: **265 pass / 88 fail / 8 documented skips**.
  Defaults: **247 pass / 13 fail / 8 skips**. Every individual status is
  unchanged; no fixture, expectation, registration, or skip was removed.
- Standalone streaming source build passed; supported selections 1, 2, 4, 5,
  6, and 8 pass **33/33 checks**. Selections 3 and 7 remain outside the migrated
  coverage.
- `git diff --check` passed. No new sanitizer or GUI integration run is claimed.

Logs and per-fixture results are in `/tmp/p4-definition-final`. Build logs are
`/tmp/p4-definition-shape-finalbuild.log` and
`/tmp/p4-definition-stream-build.log`.

