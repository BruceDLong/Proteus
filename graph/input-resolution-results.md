# Graph inputs and explicit-source resolution

The graph now composes function result constraints, argument placement, and
explicit-source word resolution through shared navigation and result publishers.
The generic search scheduler is unchanged.

| Expression | Result |
|---|---|
| `{_ 6 _}<:5` or `5:>{_ 6 _}` | `{5 6 _}` |
| `{_ 6 _}<!7` or `7!>{_ 6 _}` | `{_ 6 7}` |
| `[_ 456 789]<:+123` | `789` |
| `[...] = 'Hello'` or `[...] == 'Hello'` | `'Hello'` |
| `[...] =: 'Hello'` | `*_+'Hello'` |
| `[...] == {1 2 3}` | `{1 2 3}` |
| `[...] ==: {1 2 3}` | `*_+{1 2 3}` |
| `$ =: ($ = 'Hello')` | `*_+'Hello'` |
| `{a:1 b:2}~b` | `b:2` |
| `{a:1}^badge=7;~badge` | `badge:7` |
| `{name:'local'}^name='external';~name` | `name:'local'` |

## Shared mechanisms

`EndpointDemand.h` translates argument first/last placement into ordinary indexed
navigation, including sparse and embedded positions. The selected member receives
an ordinary RHS constraint. Unknown endpoints wait and resume through the normal
handle lifecycle. The parser now marks a completed left-side argument as done
when swapping it with the function body.

An open intersection may expose the existing registry's symbolic end-relative
node. Outer RHSs constrain that selected result, not the body. A later concrete
endpoint gains an identity and an explicit normalization prerequisite, so facts
can reach the actual member. An unclosed body can still leave the handle
subscribed even when the result's useful value is already available.

Qualified RHS payloads expose their own constraints and refinements as graph
prerequisites. `PayloadReadiness` shares the finite proof check and charges its
inspection work. No matcher recursively normalizes an operand. `ListValueShape.h`
uses `DefinitionPlan` to acquire a finite list's shape; existing sequence
correspondence supplies the members. Value acquisition can preserve an unknown
root size for a colon relationship. Once an intersection has selected its result,
sequence matching uses that result instead of reopening the body's grammar.

`Resolution.h` wraps the existing field walk. Only proved direct absence exposes
the external-property tier; cost cannot select an external property ahead of an
unresolved direct candidate. Registry associations are enumerated through
`POVDescription`, and matches use the same field predicates and result publisher.
Explicit `^` writes create independent value storage and ordinary RHS obligations.
Multiple registered matches remain a oneOf, and replay reuses the associations.

Selected views now read their source's live RHS-rendering proofs. This avoids
printing work whose proof finished after the view was published. Typed proofs
still require matching nominal information; an optional definition does not
permit hiding an unmatched type name.

## Rejection expectation audit

Three old fixtures expected rejected work to disappear. Their expected text now
preserves it:

- `merge/directLooseStr/exactLiteralReject`: `'Hello' == 'World'`.
- `merge/directAtomic/numReject`: `*_+8 = *16+9`.
- `merge/directRange/endReject`: `*4+_ == 4`.

Dedicated tests check unchanged source/target facts, retained work, and absence
of a false identity. The numeric assertion fails its required goal; the two
qualified mismatches supply no successful match and remain symbolic. Inverted
atomic and cross-overlay mismatch fixtures still expose implementation gaps;
their expectations were not changed to conceal those gaps.

## Validation

- Full CodeDog regeneration and C++ build passed.
- **1,057/1,057** focused graph/parser checks passed, including 20 input cases,
  nine lookup cases, and three rejection-contract cases added here.
- Cases cover delayed endpoints and RHS values, cancellation, cost exclusion,
  replay, rejected payloads, embedded traversal, and a 100,000-item sparse
  argument lookup that materializes only the selected occurrence.
- The stream test now verifies no loose publication while parsing is pending,
  followed by successful finite matching after closure.
- **222/270** defaults pass, up from **205/270**: fourteen behaviors recovered
  and three expectation updates. The fourteen reference examples passed on the
  Proteus3 executable before implementation and failed on the earlier fork.
- All six older suites were rerun: **237/363** pass, with no lost passes,
  crashes, timeouts, or incomplete summaries. Legacy helpers remain **102/125**.
  Exact statuses are in the compatibility inventories.

Logs: `/tmp/p4-compose-final-build.log`, `/tmp/p4-compose-focused-final.log`,
`/tmp/p4-compose-audit-*`, and `/tmp/p4-input-reference/results.log`.
The earlier scheduler sanitizer results are not a new full-engine sanitizer run.

## Remaining boundaries

Implicit context (`~word`/`%ctx`) and contextual definition expansion remain
unimplemented. Resolution through oneOf source branches needs a precedence proof
per branch; this implementation leaves it deferred. Finite qualified list-value
acquisition requires a closed, loaded source layout; streaming sources wait for
closure. General computed/contextual RHS expressions, unresolved type meanings,
and sparse source-value copying retain their existing limitations. These are
implementation boundaries, not changes to the language's intended semantics.
