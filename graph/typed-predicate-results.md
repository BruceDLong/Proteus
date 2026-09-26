# Read-only scalar type meanings in sequence matching

The original checkpoint read scalar meanings across differently labeled types.
The user subsequently retracted structural type equivalence. The current rule
requires nominal compatibility for already-typed input, including explicit aliases:

```proteus
{#left=*10+_ #right=left}
{[left]|...}<~{right:7}
// {left:*10+7}
```

Equal scalar constraints without that explicit relationship do not equate the
types. Untyped parser input can still acquire a type from its accepted grammar
pattern. See [structured-field-results.md](structured-field-results.md) for the
corrected implementation and validation.

## Shared definition walk, separate demand

`compileTypeRequirements` uses the existing definition traversal to record a
meaning certificate. Unlike `typeRequirements`, it does not apply the definition
to the node being considered. A rejected candidate and the shared grammar remain
unchanged. Ordinary type application can later reuse the same certificate.

`SequenceCorrespondence` combines this meaning with each scalar's stated facts.
Positive matches retain the pattern's type; a proved negative match retains the
source type. Ambiguous declarations, conflicting candidate facts, and unsupported
contextual meanings yield no successful continuation. Missing definitions are
now optional: use explicit data and nominal identity without adding an empty
type requirement. Unresolved searches retain subscriptions; later definition
publication can resume them, and cancellation prevents that resumption.

An accepted correspondence uses the existing atomic definition plan and remembers
its scalar meaning evidence on the output occurrences. This introduces no
application RHS and avoids a second type expansion for each copied item. A
represented scalar identity can reuse that checked evidence on replay. Finite
list and context-dependent meanings retain their existing conservative guards.
The generic graph scheduler and existing data layout are unchanged.

## Boundaries and selection semantics

This checkpoint implements scalar sequence predicates. The subsequent
[field-selection-results.md](field-selection-results.md) adds single-step scalar
dot queries and unordered result construction. Matching a structured predicate
against a list-valued input member, unordered omission semantics, and contextual
overload resolution remain pending. The current catalogue requires a
unique declaration for a word; it does not choose one overloaded meaning by
spelling, declaration order, or an arbitrary first result.

Context is part of a type's meaning. Future contextual certificates need to carry
that binding context rather than reuse context-free proof across unrelated uses.
Primary and additional types constrain matching. A common structural shape or
ancestor alone does not equate sibling types. `.wheel` therefore chooses the
wheel member in the older `query/byType1` example, preserving its original
expectation. Missing definitions are optional; ambiguous known declarations
remain deferred. Completed searches reconsider new definitions when explicitly
normalized again.

## Original checkpoint validation

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
