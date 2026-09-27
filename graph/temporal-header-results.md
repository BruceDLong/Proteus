# T-list headers and repeated requirement representation

The `T` marker is positive list meaning. It does not change how an ordinary list
stores members, counts them, or resolves their size/value requirements. Physical
and repeated-template list access therefore use the existing paths for T-lists.
Type facts and their cached witnesses retain that marker. Successful list value
acquisition and list correspondence carry it; a size-only query does not.
An ordinary non-T list makes no negative temporal assertion.

A repeated RHS is represented only when the existing definition plan proves its
compiled requirements on the target itself. A source that has checked its own
members is insufficient. This is a read-only proof and uses normal graph costs;
no separate list matcher or scheduler rule was added. Existing RHS retention and
consolidation rules still decide whether the relation stays in work or registry.

The new 12 focused cases check member normalization, identity and open-header
meaning, unknown-list acquisition, definitions, nested counts, positional access,
rejection, later member publication, a billion-item sparse occurrence, size-only
queries, and replay. All 1,103 previous focused checks also pass: **1,115/1,115**.
Full CodeDog regeneration/build succeeds. The six old suites report 251 pass,
106 fail, six explicit ambiguity skips; no previous passes were lost. Legacy
helper checks remain 102/125, without crashes, timeouts or incomplete summaries.

Two unchanged fixtures now pass: `temporal/identityCarriesT` and
`temporal/openShapeCarriesT`. Three representation expectations changed:

| Fixture | Previous expected text | Current expected text |
|---|---|---|
| `merge/intersect/onRHS` | `*_+[t5 t6 t7]` | `*_+[5 6 7]` |
| `merge/lookahead` | `{&{_| 33 22} 11 'hi'}` | `{&{33 22} 11 'hi'}` |
| `temporal/definitionCarriesT` | `timeline:{T _| 1 2 ... }` | `timeline:{T 1 2}` |

All inputs/modes stay unchanged. The choice still contains all three alternatives.
The list template is already satisfied, and the concrete T-list contains exactly
two items; the open definition does not reopen the finite supplied list.
These follow the earlier authorization to use consolidated printing.

`slice/simpleSugar` is documented separately as ambiguous after Proteus3 probes
showed the literal spec being returned even at an out-of-range position. Its
expectation is unchanged. Temporal cursor navigation, identity-bearing boundary
views, contextual recurrence and context-history design are not implemented by
this change. Parser-owned streaming T-list access also remains deferred.

Logs: `/tmp/p4-test-migration/temporal-*` and `/tmp/p4-compat-audit-*`.
