# Chained field selection

Read-only dot paths now compose the existing field-selection goal. Each segment
selects from the preceding segment's result. It does not demand full
normalization of the intermediate object or insert the object's entire type
definition before proceeding. A typed source list can expose its stored members
through the same walk as an untyped source.

The unchanged older `query/chain` and `query/chain2` fixtures now pass. The latter
finds `.wheel.spoke.length` even though the spoke's color is omitted. That is
selection of a known typed member, not proof that the containing wheel or spoke
is fully normalized. Bare numeric length still has unknown size.

## Representation and publication

`FieldPath` creates one query occurrence per parsed dot segment. The original
source and candidate POVs remain shared. The parsed predicate headers stay
unchanged; each segment's following path becomes its continuation, not an extra
condition on that segment's candidate. The graph sees the preceding selection as
a required subgoal and follows its published source link. The generic scheduler
is unchanged. Preparation cost scales with the number of path segments.

A published selection records its source infon and revision. When a subscribed
source changes, the working view's header is refreshed while retaining query
provenance and reply metadata. This fixes a single-field bug exposed by chaining:
shared value slots could become numeric while the copied view header remained
`vmAny`. Replays with unchanged sources reuse the result. New explicit normalize
calls reconsider earlier segment choices after a new type alias is declared.

## Unordered boundary

This checkpoint originally deferred chained oneOf traversal. The subsequent
[unordered-field-results.md](unordered-field-results.md) checkpoint implements
implicit membership tails, explicit size bounds, partial result publication,
and continuation through alternatives. Omission does not prove absence, and
an unresolved branch must not be discarded to obtain a definite answer.

Writes and retained-parent semicolons are now implemented; see
[path-write-results.md](path-write-results.md). Contextual descriptions and
unordered structural correspondence remain later semantic work.

## Validation

CodeDog regeneration and the native build succeeded. **694 focused checks pass**,
including 11 chain/publication cases and the previous 683-check regression set.
Coverage includes two/three segments, partial intermediate definitions, typed
sources, embedded members, explicit constraints, delayed values, cancellation,
oneOf deferral, re-normalization, and single-field publication refresh. Source
models and shared definitions remain unchanged except for explicitly supplied
late data.

The unchanged older defaults are **164/270 passing, 106 failing**. The two recovered
fixtures are `query/chain` and `query/chain2`; no prior pass was lost and no crash,
timeout, or incomplete test summary occurred. Other language-suite rows retain
their previous checkpoints. No sanitizer, GUI, or full compiled legacy-unit run
is claimed.

Logs: `/tmp/p4-chain-verified-build.log`,
`/tmp/p4-chain-verified-focused-summary.log`,
`/tmp/p4-chain-verified-focused.json`, and
`/tmp/p4-chain-verified-default/results.json`.
