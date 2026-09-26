# Nominal fields, optional definitions, and finite structural constraints

The user retracted structural type equivalence. A query for `.wheel` does not
select a `pedals` member just because their definitions have the same fields.
Type compatibility follows Proteus3's primary/additional-type rule, extended with
explicit alias evidence from the definition reader. A shared ancestor does not
make two sibling types interchangeable. Untyped structural predicates still
compare their explicit constraints. For now typed lookup requires the correct
type word; description-based identity awaits a future rule for deciding when a
definition is sufficiently complete.

```proteus
{#pedals={position:_ torque:_} #wheel={position:_ torque:_}}
{pedals:{position:321 torque:4} wheel:{position:210 torque:5}}.wheel
// wheel:{position:210 torque:5}

{{a:3 b:'yes'} {x:3 y:4}}.{_ $}
// {a:3 b:'yes'}
```

## Optional declarations

A separate definition is not mandatory. An absent declaration adds no constraints,
and the field's nominal type still exists. Existing definitions can supply
queried facts and explicit type relationships. An ambiguous declaration prevents
using that description, but does not erase a directly known type word. Bare values may still have unknown size/value despite a successful
field lookup. Do not mark those values normalized just because selection succeeded.

The definition reader records absent bindings when a compiled definition depends
on them. A later declaration invalidates that certificate. Nodes with only
undeclared types need no empty certificate or type-application Set; skipping
those redundant operations also keeps loaded-model normalization bounded.

## Shared matching and traversal

`FieldPredicate.h` reads finite ordered structures using the existing list walk
and definition requirements. It constructs temporary proof data, never new POVs
or application RHSs. Known contradictions can exclude a candidate even while
another member is unresolved. Missing facts expose ordinary graph refinements,
including the original enclosing context for relative references. Inspections
contribute to the edge's work estimate.

Selection checks the explicit query first. Nominal comparison follows type words
and explicit alias links without demanding the rest of a definition. Only missing
facts require further meaning evidence. A known definition may prove an untyped
candidate incompatible, but its shape cannot establish missing nominal identity.
Thus a `.wheel` selection can succeed on a partially known wheel even while full
normalization of that wheel is still symbolic. Explicit query constraints still
have to hold. This does not claim that selection validates the whole object.

`FieldSelection.h` retains the same ordered-first/unordered-oneOf result policy.
`ScalarPredicate.h` supplies scalar and nominal comparisons to field and sequence
matching. An accepted grammar may still type untyped input; it cannot silently
identify two already-typed inputs just from equal structure. Shared definition
bodies remain unchanged.

## Results and explicit re-normalization

The user chose explicit re-normalization for completed queries. A later definition
does not restart a completed handle. Normalizing the same query again must use
current type evidence and can choose a different result after an explicit alias
becomes available. Searches still waiting on missing values retain their usual
subscriptions and cancellation behavior.

A working field query now exposes its current selected source through
`POVDescription`. It no longer records an everlasting registry identity to each
past result. Thus a later result does not remain constrained to the old result.
The view shares the selected source's state slots, and validation follows that
source directly instead of imposing a second positional correspondence.
OneOf alternatives keep their source identities; their alternatives are fixed
members of that published symbolic value.

## Boundaries

Only finite ordered structural comparison is admitted. Unordered structural
omission/mapping, nonempty virtual spans, contextual/overloaded meanings,
chained paths, writes, and nested/complemented literal structure predicates remain
pending. The outer container may be unordered and return multiple matching
structured members as a oneOf.

The next field-selection boundary is composing these results through chained
paths. Description-based recognition without the type word remains deferred;
it must not be enabled merely because two unfinished definitions share a shape.

## Validation

CodeDog regeneration and the native build succeeded. All **683 focused checks
pass**, including 37 finite-structure checks, the existing field/sequence/type
checks, graph regressions, parser tests, and numeric fixtures. The new coverage
includes distinct types with identical structure, sibling types sharing a base,
explicit aliases, optional declarations, late ambiguity, re-normalization with a
new selected source, read-only nested matching, subscriptions, and cancellation.
Additional checks cover partial/contextual definitions behind a known type word,
aliases to those definitions, unordered partial members, explicit query
constraints, and a definition supplying a requested value.

The unchanged older default suite is **162/270 passing, 108 failing**, recovering
`query/byType1` with no previous pass lost. All 270 fixtures completed without
crashes, timeouts, or incomplete summaries. The other five language-suite rows
retain their earlier checkpoint; no sanitizer, GUI, or full compiled legacy-unit
run is claimed.

Logs: `/tmp/p4-word-policy-build.log`,
`/tmp/p4-word-policy-focused-summary.log`,
`/tmp/p4-word-policy-focused.json`, and
`/tmp/p4-word-policy-default/results.json`.
The original language fixtures and expected outputs were not edited.
