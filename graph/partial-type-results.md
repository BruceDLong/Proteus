# Partial typed descriptions and chained writes

Returning a selected parent with `;` or `;;` now normalizes its supplied typed
parts without imposing the entire positional layout of the parent's definition.
For example, `#spoke={color:$ length:_}` does not make
`spoke:{length:_}.length=123;` fail because color was omitted. It returns
`spoke:{length:123}`. The original assignment already worked; the failure was a
required whole-type expansion during normalization of the returned parent.

The user confirmed that these ordered typed-field descriptions may be partial.
This is a normalization policy, not a special path-write rule. A direct supplied
description behaves the same way. The adapter recognizes present typed members
through the shared list walk, including embedded positions, and leaves their
own normalization goals intact. Their values, scalar type requirements, and
explicit RHS constraints still apply. Bare type references and positional value
sequences retain definition expansion; repetition constraints are not skipped.
The generic graph search and path-write algorithms are unchanged.

Omitted parent members create neither inserted RHSs nor missing-field
subscriptions. Unknown supplied members can still subscribe and resume. A
finished description can be marked normalized when its own value, extent,
overlay and members are known; that is not a certificate that it enumerates and
validates the complete parent definition. List size counts supplied members.
Definition bodies remain unchanged, and replay adds no duplicate identities.
When size is learned before value, the later value copy also preserves the
explicit numeric-size rendering metadata; otherwise a printed result could lose a fact.

## Boundaries

A later increment supplies missing scalar facts from the corresponding parent
field through a separate graph goal; see
[definition-projection-results.md](definition-projection-results.md). It does not
force the full parent layout or manufacture absent parts. Contextual and general
nested container requirements remain deferred.

Supplied bodies with untyped positions, empty bodies, templates, and unresolved
structure keep their existing rules. This change does not infer order from a
field's name or alter ambiguous-write behavior.

## Validation

- Regenerated from `Proteus.Lib.dog`; `Marker: Build Successful`.
- 890/890 focused checks, including nine partial-description cases. Both original
  `query/chain3` and `query/chain4` now pass. Coverage includes direct partial
  descriptions, late member values, contradictory writes and member types,
  bare-type expansion, unordered descriptions, replay, and an omitted field
  whose definition depends on context.
- Unchanged original defaults: 192/270 passing across these increments, up from
  186/270 before oneOf filtering. No prior pass was lost; no crashes or timeouts.
  The remaining 78 exact assertions are still reported as failures.

Logs: `/tmp/p4-composition-source-build.log`,
`/tmp/p4-composition-final-summary.log`, and
`/tmp/p4-composition-final-default/results.json`.
