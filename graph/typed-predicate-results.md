# Read-only scalar type meanings in sequence matching

Sequence predicates can follow unique, context-free scalar type definitions and
compare their constraints rather than reject a candidate because its label differs.
For example:

```proteus
{#left=*10+_ #right=*10+_}
{[left]|...}<~{right:7}
// {left:*10+7}
```

This is compatibility evidence for a scalar pattern. It does not establish that
similarly shaped domain concepts are interchangeable in every context, or that a
matching member is the unique answer to a field query.

## Shared definition walk, separate demand

`compileTypeRequirements` uses the existing definition traversal to record a
meaning certificate. Unlike `typeRequirements`, it does not apply the definition
to the node being considered. A rejected candidate and the shared grammar remain
unchanged. Ordinary type application can later reuse the same certificate.

`SequenceCorrespondence` combines this meaning with each scalar's stated facts.
Positive matches retain the pattern's type; a proved negative match retains the
source type. Missing definitions, ambiguous declarations, conflicting candidate
facts, and unsupported contextual meanings yield no successful continuation.
Unresolved searches retain subscriptions; later definition publication can resume
them, and cancellation prevents that resumption.

An accepted correspondence uses the existing atomic definition plan and remembers
its scalar meaning evidence on the output occurrences. This introduces no
application RHS and avoids a second type expansion for each copied item. A
represented scalar identity can reuse that checked evidence on replay. Finite
list and context-dependent meanings retain their existing conservative guards.
The generic graph scheduler and existing data layout are unchanged.

## Boundaries and semantic question

This implements scalar predicates only. Dot/path lowering, matching a structured
predicate against a list-valued input member, unordered omission semantics, and
contextual overload resolution remain pending. The current catalogue requires a
unique declaration for a word; it does not choose one overloaded meaning by
spelling, declaration order, or an arbitrary first result.

Context is part of a type's meaning. Future contextual certificates need to carry
that binding context rather than reuse context-free proof across unrelated uses.
Written type IDs alone cannot prove structural incompatibility. Conversely,
structural compatibility alone must not silently establish unique field selection.

The old `query/byType1` fixture gives `pedals` and `wheel` identical definitions
`{position:_ torque:_}` and expects `.wheel` to choose the second member. The user
has been asked whether explicit `wheel` type evidence gives it priority or whether
this now requires a distinguishing property/context. No dot selection policy or
original fixture was changed while that question is pending.

## Validation

CodeDog source regeneration and the native build succeeded. All **611 focused
checks pass**: 14 typed-matching, 18 scalar-predicate, 24 source-injection, 36
text-correspondence, and 519 established regression checks. Coverage includes
aliases, distinct labels, scalar contradiction, negative predicates, late
definitions, cancellation, ambiguous/contextual deferral, read-only compilation,
replay without extra identities, and preservation of shared templates/definitions.
The output checks also verify no redundant scalar type application on accepted
members. An enclosing selection must preserve its chosen child's meaning proof,
not replace it with the input cursor's type evidence.

The older default suite remains **159/270 passing, 111 failing**. Every per-fixture
status and process result matches the scalar-predicate checkpoint; no original
input or expected output was edited. No crash or timeout occurred. The other
language suites, standalone streaming executable, and sanitizers were not rerun.

This regeneration also exposed a contradictory assertion accidentally copied
into `graphNavigation/repeat` in the preceding checkpoint. The same case required
both symbolic and non-symbolic status. It now checks its fully known result's
`isNormed` flag and retains the repeat/identity stability checks. The earlier
597-check executable result did not cover that final source edit; the corrected
source is covered by this regenerated run.

Logs: `/tmp/p4-typed-verified-build.log`,
`/tmp/p4-typed-verified-focused.log`, `/tmp/p4-typed-verified-regression.log`, and
`/tmp/p4-typed-verified-default/results.json`. `git diff --check` passed.
