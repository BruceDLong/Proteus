# Missing field facts from a parent definition

A supplied field can now obtain missing scalar facts from its containing type:

```proteus
#spoke={color:$ length:*1000+123}
spoke:{length:_}.length
// length:*1000+123
```

The graph advertises a parent-meaning route for the unknown member. The goal
uses the shared list layout and nominal matcher to locate its corresponding
field, then uses the existing definition compiler on only that field's subtree.
Literal facts, its RHSs, and its own scalar type meaning are combined read-only.
The goal emits the ordinary scalar Set, acquiring size, value, and overlay
together when available. It does not create a type-application RHS or insert
omitted siblings. The shared definition body is never normalized or changed.

Repeated typed fields use their ordered occurrence. An unordered duplicate has
no distinguished correspondence and stays unresolved. Embedded lists preserve
logical membership; ordinary nested lists form a new owner boundary. Each
published path candidate retains its original owner stack, including candidates
inside a oneOf, so a value demand reaches that candidate's own parent meaning. Definitions
with unresolved overloads or contextual selected fields remain deferred. An
unrelated field's contextual expression or contradictory value does not block
the selected field. Multiple conjunctive descriptions of the selected field
must agree before its Set is published.

This is a route for missing information, not compulsory validation of an entire
parent definition. A supplied complete scalar keeps its explicit value. Reusable
compiler witnesses are checked before reuse, and later facts or definitions wake
an unresolved normalization. Cancellation stops that subscription. Completed
queries retain the existing explicit re-normalization policy.

## Boundaries

This increment projects scalar facts onto existing fields. It does not synthesize
absent parts, select a meaning among overloaded definitions, execute contextual
requirements inside a shared declaration, or project arbitrary nested container
requirements. Those need their own graph evidence. A field's own explicit
constraints and type requirements still run independently.

## Validation

- Regenerated with `codeDog Proteus.Lib.dog`; `Marker: Build Successful`.
- 934/934 focused checks, including 26 parent-projection cases, 13 crossing-span
  cases, five canonical language checks, and all 890 previous focused checks.
- Shared span planner: 374/374 cases passed.
- Unchanged original defaults: 192/270 passing, 78 exact assertion failures.
  Every fixture retained its prior pass/fail status; no crashes or timeouts.

Logs: `/tmp/p4-field-cross-final-build.log`,
`/tmp/p4-field-cross-final-focused.log`, and
`/tmp/p4-field-cross-final-default/results.json`.
No sanitizer, GUI integration, or full compiled legacy-unit pass is claimed.
