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

## Retained bare-word meaning (2026-10-05)

The small temporal reproduction above now resolves its prior `x:7` during the
original search. A normalization goal that first sees a bare typed value retains
its type-meaning demand after another route materializes a generic list. Previously,
the new structure made `describesParts` suppress that demand; the scheduler then
retired a type goal that already had a valid compiled plan to set `x`.

This is a goal-local obligation, not a second graph or a history of visited POVs.
Initially supplied partial descriptions keep the existing lazy policy. The
ordinary declaration goal and Set planner still resolve and apply the meaning.

`graph/test_retained_meaning.cpp` covers the original and an extra alias, each
with bounded and symbolic records. It verifies the known source fact without
explicitly normalizing the first state, the second state's result, distinct
occurrences, and unchanged declarations. Bounded fields resolve to `*10+7`;
unbounded unordered fields retain a symbolic oneOf with known candidate 7.
The previous library fails because the source fact remains unknown.

The graph audit remains 1,459/1,468 with the same nine existing cost-policy
failures. An initially missing asset caused the cube-orientation fixture to fail;
it passes when rerun with the regular test assets. Existing depth and expression-
publication regressions pass. Logs are in `/tmp/p4-state2-route/`.

This repairs the small dependency gap. The full cube still has unresolved
placement-description selection and derived poses; complete state-2 rendering
is not claimed by this checkpoint.


## Transition selected as a record RHS (2026-10-06)

A selected later state can supply an ordinary record RHS without first normalizing
its prior state explicitly. Graph access now advertises the selected occurrence's
preservation route, resolves path-valued work RHSs even when their type qualifier
is retained, and exposes prior/source definition parts through type goals rather
than recursively normalizing unrelated fields. Record constraint checks do not
request value preservation: that bypass initially broke
`graphPreservation/descriptiveTarget`, and its explicit guard restores the test.

Definition application preserves an explicit unordered extent even when it equals
the current member count. This closes a bounded candidate set without changing
scalar ranges or ordered repetition. Missing-field traversal may request retained
part requirements; initially omitted fields still remain omitted. Property
comparison carries the correct parent stack and can reject a conjunction using
known literal facts while unrelated RHS obligations are pending; such incomplete
facts do not establish a positive match.

`graph/test_rhs_transition.cpp` exercises direct selection, selection inside a
record, and selection inside a later T-item. The changed part transfers cargo 9
and maps pose 2 to 10; the omitted part preserves cargo 9 and pose 2. It checks
replay and unchanged shared declarations. All three cases pass. The graph audit
is 1,459/1,468, with precisely the same nine baseline cost-policy failures and no
new failure. The descriptive path-write regression passes again.

This remains an intermediate correctness checkpoint. Actual Slipstream shows the
first Cube but an empty second display at 80 seconds. A more reduced failure is
`change:{T known ?}@[2]~part:{label:'left',}.pose`: selecting the whole later state
and then normalizing its parts produces pose 10, whereas the direct descriptive
chain remains unresolved. The requested label has a prior correspondence, but
its preservation guard waits on containing transition constraints. Those
constraints include unrelated changed fields. Investigate that dependency cycle
before claiming the Cube's derived poses or complete rendering work. Temporary
membership and query-part experiments did not repair the chain and are not part
of this checkpoint.


## Projection through pending unordered fields (2026-10-06)

The direct descriptive chain above now yields a candidate with pose 10. Its
missing label could not be projected from the declared `left` meaning because
`DefinitionProjection` used the positional span reader on the containing record.
That reader requires each ordinary member to support positional correspondence;
the unrelated cargo/pose expressions did not yet meet that requirement.

Unordered projection now enumerates typed member headers through the existing
`RecordStructure::members` reader. The same nominal ambiguity checks and scalar
requirement compiler then select the requested field. Ordered/embedded layouts
retain their span reader. This adds no Cube-specific rule and does not weaken
preservation guards. The public regression includes the direct descriptive chain
and still checks unchanged shared declarations.

Validation: all four public transition cases pass against the rebuilt production
shared library. The graph audit is again 1,459/1,468 with no changed result relative
to the earlier baseline; 191 focused tests pass against the production executable.
The unchanged Slipstream executable links to this library without unresolved
symbols.

The actual front-center turn query now returns a symbolic choice containing
`cubie-frame:10`. The frame-catalog lookup using that result remains unresolved:
the selection still has an open oneOf tail, so this is not yet a uniquely resolved
numeric coordinate. A full Cube-history probe timed out after 90 seconds. In the
isolated Slipstream run the first Cube was visible (pink at 30 seconds), and the
second display was empty at 80 seconds. Complete state-2 rendering remains
unverified and unsuccessful in that run. Next investigate why the selected
placement/field remains open and how the finite definition's extent reaches that
intermediate value; do not arbitrarily collapse an open choice to its first item.
Logs and screenshots are under `/tmp/p4-state2-route/` and
`/tmp/p4-state2-record-projection/`.
