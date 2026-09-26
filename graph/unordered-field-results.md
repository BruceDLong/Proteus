# Partial unordered descriptions and field paths

An unordered description names relevant parts without asserting that those are
all the parts. The graph adapter exposes its implied remainder through the
shared list-boundary interface. The parser and normalizer no longer infer a
total size by counting the stored members of an unordered description.

An explicit size bounds total membership. Two stored members of a size-two list
exhaust it; two stored members of a size-three list leave a virtual remainder.
A field query does not allocate that remainder or demand its total size merely
to use a known part. The size constrains membership, not member states.
Describing a change to one part makes no assertion about changes to omitted
parts.

## Selection and continuation

Ordered selection still requires the first compatible logical member.
Unordered selection publishes the known matches, retaining an open oneOf tail
when candidates or membership remain unresolved. Even one known match stays a
choice while other matches remain possible. No known matches in an open source
produce an open, empty choice rather than a proof of absence.

For example:

```proteus
{wheel:{position:*10+1}, wheel:{color:'red'}}.wheel.position
```

publishes `*_+[position:*10+1 ... ]`. The missing position remains unresolved.
When it is supplied, the same result can acquire another alternative. Neither
wheel must first satisfy every member of its definition.

Each following dot segment traverses the preceding choice's existing POVs and
uses the ordinary field walk inside each alternative. Ordered-first selection
applies within each ordered alternative. Unknown earlier candidates, spans,
or missing fields keep an obligation while the walk visits other alternatives.
Nested oneOf payloads and unsupported source constraints remain deferred.

The preceding segment is a candidate-set demand: a complete choice satisfies
that demand even though the choice is a symbolic value. The final value demand
still reports a oneOf as symbolic. This permits two routes to the same source
field to yield one definite result without an unnecessary subscription.

Publication reuses the choice and retained alternative POVs. Only new matching
source occurrences acquire new identity records. Reaching the same source POV
by more than one route does not duplicate that alternative. Selected views keep
their provenance; result collection does not mutate candidate models or insert
whole type definitions. The generic graph scheduler is unchanged.

## Representation and remaining boundaries

Printing preserves explicit unordered sizes and the explicit `*_+` marker for
an open oneOf, so reparsing does not silently change those meanings.

Unordered structural correspondence, writes/retained-parent paths, and
contextual descriptions remain later work. A closed selection with no match
still uses the existing unresolved behavior pending the user's choice between
normalization failure and a closed empty oneOf. Invalid size/member combinations
are not accepted as completed field results.

## Validation

CodeDog regeneration and the native build succeeded. All **720 focused checks
pass**, including 26 new unordered-membership and continuation cases plus the
previous 694-check regression set. Tests cover implied and explicit bounds,
partial results, late members and values, subscriptions/cancellation, stable
identities on replay, shared source occurrences, guarded choices, two/three
path segments, and printer round trips. A billion-member bound is inspected
without materializing its missing members.

The unchanged older default suite remains **164/270 passing, 106 failing**. Every
fixture retains its preceding outcome; no crash, timeout, or incomplete summary
occurred. Other language-suite inventory rows retain their earlier checkpoints.
No full legacy compiled-unit, sanitizer, or GUI run is claimed.

Logs: `/tmp/p4-unordered-final-build.log`,
`/tmp/p4-unordered-final-native.log`,
`/tmp/p4-unordered-verified-summary.log`,
`/tmp/p4-unordered-verified-focused.json`, and
`/tmp/p4-unordered-verified-default/results.json`.
