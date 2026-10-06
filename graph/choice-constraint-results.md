# Constraints against oneOf alternatives

An equality between oneOf and a scalar or finite structural constraint now
lowers to candidate filtering. The existing field predicate reads the explicit
constraint, the shared list walk visits alternatives, and the path publisher
retains the compatible results. The equality may put the choice on either side.

Known mismatches supply no candidate. Unknown comparisons remain in a symbolic
choice and retain subscriptions; an open source also keeps the result open.
A sole proved candidate supplies its original value, size, and context. Multiple
candidates remain a oneOf, and no match yields an empty oneOf. A collected Set
publishes after traversal so cheaper findings cannot hide slower alternatives.

Additional RHS constraints on the published result feed the same predicate.
The shared candidate source is not overwritten to force a match. Normalization
can refine a positive explicit predicate, and result aliases reuse the original
cursor binding only through a registered correspondence with the same infonView.
A new cursor occurrence without that evidence still fails context revalidation.

This increment supports one strict equality with one explicit choice. It does
not implement Cartesian products of choices, repeated grammar alternatives,
choice-producing definitions, or loose text-prefix choice matching. The later test-completion checkpoint updates `merge/intersect/onRHS` to the
ordinary symbolic oneOf spelling without legacy tentative `t` markers. Its
three candidate values remain alternatives.

Validation:

- Full CodeDog source build succeeded.
- 866/866 focused checks, including 16 choice cases: both equality directions,
  empty/multiple results, unknown and late candidates, cancellation, open/late
  ends, structured/embedded/nominal candidates, new constraints, relative source
  context, and identity-stable replay.
- Original default suite: 189/270 passing, up from 186. Recovered
  `merge/num/literalSelect`, `merge/str/literalSelect`, and `altSelect/simple`.
  No prior pass was lost; no fixture expectations were changed.
- The generic search and span planners are unchanged in this increment.

Logs: `/tmp/p4-choice-final-build.log`, `/tmp/p4-choice-verified-summary.log`,
`/tmp/p4-choice-default/results.json`.

## Declared fields with choice values (2026-10-05)

Choice filtering preserves the containing field's declared type and additional
types through lowering and result publication. It also retains a `%?` declaration
on that field. This does not change ordinary path navigation, whose result takes
the selected source's type.

`knownField` uses an entry's declared type before trying to unwrap its value.
An unresolved value in `slot` therefore cannot hide an unrelated, known `thing`
field later in the same record. Untyped unknown entries and ambiguous nominal
matches still withhold a unique result.

Validation:

- Rebuilt `libproteusGraph.so` and the existing `TestProteus` executable.
- Public shared-library regression `graph/test_choice_declarations.cpp` passes:
  ordered/unordered fields, unresolved and resolved choices, `%?`, type aliases,
  late publication, replay, ordinary navigation, and unresolved ambiguity.
- Focused existing checks: 191 pass / 193 total. The two failures are the
  previously disabled cost-policy assertions in `graphField/cost` and
  `graphFieldSource/cost`; expectations are unchanged.
- The 15 small descriptive-selection fixtures pass. Full-model palette/transition
  fixtures remain outside this focused claim, as in the prior graph audit.
  The attempted full descriptive group timed out after 90 seconds in the
  palette fixture; it did not reach the transition fixture.
- The unchanged Slipstream executable loads the rebuilt shared library. An
  isolated GUI run still has an empty state-2 display at 60 seconds. The direct
  display probe now finds `thing`, but rotation fields remain unresolved.

A separate minimal reproduction identifies the remaining lazy-definition gap:

```proteus
#base = *1+{item:{x:_,}}
#start = base:{item:{x:7,}}
#act = {T base base:{item:{x:%?=%@[1].item.x,}}}
act:{T start ?}
```

The second state's query initially sees `x:_`. Explicitly normalizing the first
state exposes `x:7`, after which the second state's query sees that value. This
change does not repair that route and does not claim complete cube rendering.
Depth-limited expansion must still discover definitions needed by requested
values; no new depth parameter is implemented here.
