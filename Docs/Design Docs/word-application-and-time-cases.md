# Word application and temporal description coverage

This draft gives a basis of examples for specifying systems through words,
parameterized definitions, part selection and T-lists. The user has limited the
first design to unambiguous application syntax, and clarified that `&` before a
verb describes a sequence of state changes. Role inference, receiver calls
and natural-language argument assignment remain deferred. The proposed rules and
unsettled temporal cases below need discussion before implementation.

There are arbitrarily many compositions, so a finite case list cannot enumerate
every program. Coverage comes from independent semantic dimensions, examples at
their intersections, and equivalence laws that must survive composition.

The accepted starting points are the occurrence and definition rules in
[normalization-design.md](normalization-design.md), the relative assignment and
boundary agreements in [time-word-design-plan.md](time-word-design-plan.md), and
D067/D069 in [the decision log](normalization-design-decisions.md). D067 governs
current cursor retention; the earlier D062 old-versus-new equality interpretation
must not be reapplied to a computed transition result. Historical implementation
claims in older notes do not establish current engine coverage.

## Proposed application rules for the first examples

1. Use explicit named bindings, for example
   `turn(item::%prev, direction::clockwise)`. The examples require no positional
   binding, inferred roles, receiver sugar, or implicit LHS argument. Fully
   supplied positional syntax can be discussed separately without introducing
   role inference. A missing binding stays missing; required results cannot be
   claimed if they depend on it. This does not yet settle optional-argument or
   default syntax.
2. An attached opening parenthesis marks application: `turn(...)`. A separated
   `turn (...)` retains the non-call interpretation. Inside the argument list,
   ordinary whitespace is allowed. Named binding order does not affect meaning;
   duplicate and undeclared role labels should be diagnosed rather than assigned
   an implicit precedence. The attachment rule and these diagnostics are proposed
   grammar, not implemented syntax.
3. A word or an application contributes its definition's description. The result
   may be a scalar, an object description, a state, a process/span, or a sequence.
   The result is not automatically the first argument, and evaluating the
   description does not by itself execute an external action.
4. Argument expressions retain their call-site occurrence and context. A body
   reference such as `%item` refers to that application's binding; nested calls
   forward it explicitly. Shared declarations remain unchanged. Binding a role
   need not fully normalize the participant or copy it into the declaration.
5. Distinguish the selected destination, a participant, its prior state, and the
   resulting occurrence. Ordinary equality constrains its target; it does not
   overwrite a contradictory fact at that same occurrence. In a T-step the
   target is the state being described, so a new value need not equal the old
   value. Outside temporal context, equality does not create a successor.
6. Relative references must have one specified anchor. In the user's direct
   T-member example, `%prev` denotes the preceding member at the call site.
   `%prev` remains structural; `%tprev` is used when crossing ordinary structure
   to a temporal ancestor. Proposed rule: an explicit reference is not shifted
   again merely because the call is on an identity RHS. The implicit relative
   assignment context must compose with this rule; examples must catch a double
   predecessor shift. Until this is settled, explicit references to named source
   and destination occurrences are the comparison form.
7. T-step work computes the described new facts; D067 retention supplies
   unaffected previous descriptions. Direct and model-implied changes must both
   be accounted for. Ordinary unknowns do not assert independence; `%?` withholds
   retention at its scope. These do not exempt explicit contradictory constraints.
8. Describe simultaneous requirements in one state and ordered changes in
   successive states. Same-step computations read the same prior occurrence;
   serialization of evaluation must not create an unintended ordering of changes.
   Interacting changes need their joint relationship or remain conflicting or
   unresolved; arbitrary ordering is not a resolution mechanism.
9. The user's `&verb(...)` form describes the verb's sequence of state changes.
   Compare it with the ordinary resulting-state use of `verb(...)`. Proposed
   consistency law: for a finite completed change sequence, its terminal state
   agrees with the ordinary resulting-state description for the same inputs.
   The model must supply the intermediate relationships; `&` does not invent a
   path, sample rate, duration, or extra states from a known endpoint alone.
   How the endpoint and sequence views are exposed by one verb model remains to
   be specified. This is not a general rule collapsing ordinary list-valued
   words to their last member.
10. A T-member's temporal support comes from the model. Neither one member nor one
   verb application implies a duration, a zero-duration event, or one second.
   Member ordinals, boundary identities, duration, coordinate origin, and units
   remain separate. The sequence form must compose with existing embedded-span
   semantics; ordinary list-valued descriptions do not silently splice themselves
   into their surrounding history.

## Dimensions of coverage

| Dimension | Cases to distinguish |
| --- | --- |
| Structural scope | Whole system; one part; several parts; nested part; overlapping views; membership and part identity |
| Temporal scope | Instant/cut; one state support; interval; transition boundary; finite history; open history; nested or embedded span |
| Description source | Literal; unparameterized word; explicit application; identity/constraint; selected description; shared listSpec |
| Interval meaning | At both endpoints; throughout; at some time; trajectory over the interval; event occurring within it; aggregate over it |
| Composition | Same-state conjunction; ordered transitions; embedded sequence; whole/part projection; aligned or overlapping histories |
| Knowledge | Known; explicitly unknown; missing source; conflicting; ambiguous selection; pending parser/link information |
| Identity and context | Same entity at different times; equal distinct entities; aliases of one occurrence; different part roles; changing caller context |

For every case, specify what is requested. A selected fact, a whole state, a
history, a proof that a condition holds throughout an interval, and a query
following future states impose different obligations.

## Concrete examples

All call syntax below is a design specimen, not a runnable current fixture.
`S0`, `S1` and `H` below are explanatory aliases for already identified occurrences
or a history, not new declaration syntax. In the face examples, assume `turn`
describes a resulting face state plus the necessary relationships to its cube.
If a definition instead returns a whole cube, its destination must match that
result; no automatic projection from cube to face is assumed.

### E01 Literal and named descriptions at one instant

Describe the same cube state once with explicit fields and once with a word such
as `solved-cube`. Query an immediate property and a nested face property. Require
the same requested facts and qualifications. Applying the same word to two cubes
must not make them one occurrence or mutate the shared meaning.

Also describe only one face at that instant. It must remain a partial description
of the system, not evidence that the other faces are absent.

### E02 An application with an independent result

```text
S0.display.font-size = sqrt(value::144)
```

The model for this example specifies the nonnegative square root. The result is
12, not the argument 144 or the display. Argument and destination identities
remain distinct. A second contradictory font-size assertion at S0 must conflict.
Querying the result must not change the exploratory base for a later path.

### E03 The direct face history

```text
cube-face:{T|
    state1
    state2
    turn(item::%prev, direction::clockwise)
    &move-sequence
    ...
}
```

The turn takes state2 as its input in this layout. Its result describes the next
face occurrence; state2 is unchanged. The model must account for affected cube
parts as well as the selected face. The ellipsis leaves later work unspecified;
it does not assert a final extent. Whether state1/state2 represent instants or
intervals, and what duration the turn represents, require separate support facts.

### E04 The same change through an explicit selected destination

```text
S1.cube.faces~red =
    turn(item::S0.cube.faces~red, direction::clockwise)
```

S0 and S1 are successive system occurrences. Compare the selected face and all
affected cube facts with E03 under explicit correspondence of their temporal
supports. The selection retains the identity of the face and its containing
cube. The word `red` must select the face under the established model; it does
not acquire a new hard-coded color lookup.

Compare this explicit-reference form with a relative form inside S1. Verify that
the RHS observes S0 exactly once, not S1's unfinished result or the state before
S0. A selector that has no proved destination or several unresolved alternatives
must not silently choose an update target.

### E05 Whole-system and part-local descriptions of the same change

Give the whole cube's S1 description, then independently describe the red face's
change over the same support. Compatible descriptions must combine; disagreeing
descriptions of the same cubie occurrence must conflict. A face view is not an
isolated copy whose updates can disagree with the system view.

Repeat with two different cubes that happen to have identical colors. Facts about
one cube must not transfer to the other merely because their values are equal.

### E06 Two simultaneous computations and two sequential computations

Start with S0 containing `a:1` and `b:2`. In the same destination state assert:

```text
S1.a = S0.b
S1.b = S0.a
```

Require `a:2, b:1`, independently of evaluation order. Compare with an explicitly
sequential history where the second operation reads the first operation's result.
Those need not agree. Also combine independent changes to two machines in one
system state; both should survive without imposing a time order between them.

### E07 Interacting and contradictory simultaneous changes

Apply two adjacent cube-face turns at the same temporal support. Their affected
positions overlap, so the model must define a joint operation, prove consistency,
or retain a conflict/underdetermination. It must not silently execute them in
whichever order a queue happens to visit them. Two incompatible values explicitly
asserted for one resulting cubie orientation must reject that required conjunction.

### E08 Retention and explicit independence

Use a small record with `a:1, b:7` to separate four later-state cases: change a
while b is unaffected; leave b unknown with retention applicable; place `%?` on
b; place `%?` on the whole record. Verify the scope of retained facts. Then use a
bicycle example where pedaling changes the chain and wheel through the model:
those effects cannot be frozen merely because they were not written explicitly.

### E09 Verb expansion and parameterized sequence composition

Compare the resulting-state and sequence forms with identical bound inputs:

```text
cube-face:{T| initial-state turn(item::%prev, direction::clockwise)}
cube-face:{T| initial-state &turn(item::%prev, direction::clockwise)}
```

The second exposes the state changes described by the verb. For a finite
completed sequence, require agreement of its endpoint with the first form while
retaining its own intermediate temporal occurrences. An endpoint-only meaning
does not justify inventing the intermediate changes. The `item::%prev` binding
belongs to the outer call site and must not be recaptured at every expanded step.

Also compose an explicitly described sequence:

```text
#two-turns: item = {T|
    turn(item::%item, direction::clockwise)
    turn(item::%prev, direction::clockwise)
}

cube-face:{T|
    initial-state
    &two-turns(item::%prev)
    ...
}
```

This proposed sequence returns the two resulting states, excluding its input
seed. Compare it with writing both turns inline. Preserve caller bindings,
sequence entry context, final result and temporal boundaries. Embedding must not
duplicate the seed, invent an extra transition, or conflate equal states. Compare
with a nested, unembedded sequence to specify whether it is a compound span or
an ordinary contained value. Do not assume `%prev` and `%tprev` are interchangeable
across that boundary. Seed exclusion is a proposed convention to settle, not an
accepted consequence of the `&` notation. Cover an empty expansion, a one-change
expansion, nested expansions, and an open sequence with no established terminal
state. A plain verb's endpoint need not be available just because a prefix exists.

### E10 Whole and part histories

Represent a system as a T-list of whole-system descriptions. Separately represent
histories of its motor, switch and temperature, with explicit support alignment.
The two representations should yield the same facts where they describe the same
occurrences. A component can have a boundary where the containing description
does not, and a cut in the parent can fall inside a constant component span.

Repeat with one part sampled more often than another. Equal item indices do not
establish simultaneity; align by temporal relationships, coordinates and units.
Unknown alignment remains unknown instead of zipping the histories by position.
For example, motor observations at times 0, 1 and 2 and temperature observations
at times 0 and 2 do not have simultaneous second entries. Also compare seconds
with tenths of a second, and equal numeric coordinates with different origins.

### E11 One instant and an interval are different obligations

For a heater, distinguish all of the following over the same interval I:

- Temperature is 20 at I.begin and 30 at I.end; the interior is unspecified.
- Temperature is 20 throughout I.
- Temperature reaches 30 at some time within I.
- Temperature follows a specified trajectory throughout I.
- One heating event occurs within I, with its own start and finish.
- Average temperature over I is 25; this does not establish each instant's value.

These are semantic examples. They deliberately do not invent `at`/`during`
surface syntax before the quantification, boundaries and supports are settled.
At least one interior-time query must distinguish each description from the others.

### E12 Turning as an endpoint result or a process

Compare a quarter-turn's resulting orientation with a process that rotates from
0 to 90 degrees over two seconds. Query the midpoint. The endpoint result alone
must not imply a 45-degree midpoint. A model with an explicit uniform angular
rate can establish it; a nonuniform process can have the same endpoints.

Use `&turn(...)` to describe the modeled change sequence. Its members may need
interval-valued process descriptions rather than a fabricated series of instant
snapshots. Specify how a state is projected from those descriptions and how that
projection relates to the ordinary turn result. The `&` distinction does not
itself settle duration, trajectory or boundary-value conventions.

### E13 Boundaries and overlapping spans

Cover a beginning, an ending, a cut inside an unchanged span, adjoining spans,
overlapping spans, and a zero-duration boundary. Adjacent T-span boundaries share
the established boundary identity without identifying the whole spans. A cut in
an unchanged value does not invent a change. At a discontinuity, distinguish the
value on either side or leave the boundary-value convention explicit.

Compatible constraints on overlapping supports combine. Incompatible ones at the
same occurrence/support conflict; disagreement on disjoint supports is allowed.
The meaning of open/closed endpoints and claims exactly at a discontinuity still
needs a temporal-value contract. Counted list slices alone do not supply it.

### E14 Parts that move, appear, disappear or change roles

Remove a wheel from bicycle A and attach it to bicycle B. The wheel can keep its
entity identity while its membership relationship changes. Compare omission of
the wheel, explicit non-membership, destruction, and replacement with a different
wheel. These must not collapse to one operation. Include a wheel with a lifetime
longer than its interval as part of A, and queries just before/after the change.

### E15 Missing information, open histories and later knowledge

Start with a history prefix, an unfinished next member, an explicitly unknown
direction, and a query beyond the represented end. Distinguish a required input
wait from exhausted unknown knowledge; an open tail is not proof that a future
member exists. Later parser completion continues its existing attempt without
reapplying completed changes or replacing earlier occurrence identities.

Compare learning an omitted fact about S0 with adding a genuinely later S1, and
with explicitly replacing a mistaken stored fact. They are not the same temporal
operation. Completed queries and subscriptions retain their accepted lifecycle;
this example does not introduce automatic restart or general belief revision.

### E16 Missing causes and nonunique answers

Supply a before-state and an after-state with the operation unspecified. A model
may constrain possible changes, but the engine must not invent a unique action.
Also describe a process by its endpoints with unknown duration and ask for its
rate. Unambiguous argument labels do not imply sufficient semantic information.
Inverse reasoning and planning remain deferred capabilities, not prerequisites
for implementing explicit forward descriptions.

### E17 Descriptions and physical execution

Describe a motor turn twice, query its result twice, and revisit its cached state.
None of these alone should command the motor twice. If external execution is
later added, specify the action occurrence, authorization and delivery lifecycle
separately. This keeps pure value definitions and state descriptions usable by
the same application syntax without pretending that external effects are solved.

### E18 Shared specifications across a history

```text
cube-face:{T face-state-requirement|
    initial-state
    turn(item::%prev, direction::clockwise)
    &move-sequence
    ...
}
```

The named requirement contributes constraints at each applicable member using
ordinary listSpec relationships, including members supplied by an embedded
sequence. It must combine with each verb's result rather than be replaced by it.
Test an accepted state, a contradictory state and a member with unknown evidence.
Also test an occurrence-dependent template at the first history member, where
its preceding-state prerequisite is unavailable.

A requirement on each sampled instant does not establish an invariant at every
intermediate instant. If the member descriptions cover whole intervals, the
required temporal quantification must be preserved through their projection.
Applying ancestor temporal listSpecs remains an implementation boundary; these
examples specify the question rather than claiming the current cursor supports it.

### E19 Parameterized object descriptions without change

```text
S0.panel = rectangle(width::3, height::4)
```

This ordinary application supplies a rectangular object description at S0. Its
model relates the dimensions to properties such as area; it need not be a verb
or advance time. Compare it with the corresponding literal and named description.
Two panels with these dimensions can still be different objects.

Repeat with an explicitly unknown width and a supplied area constraint. Keep the
partial description and its relationships; deriving width requires the model's
available reasoning, not role inference. A conflicting area must reject the
required conjunction. Reusing one declaration with different arguments must keep
each application's bindings and contextual occurrences independent.

## Equivalence and failure checks

For the supported fragment, use these as semantic checks rather than printer
string equality:

- Literal meaning versus a word versus explicit parameterized application.
- Permuting named bindings without changing their roles.
- Explicit source occurrence versus the corresponding relative temporal reference.
- Inline changes versus an embedded sequence with explicit entry binding.
- A verb's ordinary resulting-state description versus the endpoint of its finite
  completed `&` expansion, preserving intermediate occurrences in the latter.
- Whole-system description projected to a part versus that part's description at
  the same temporal support; recombination needs explicit correspondence and all
  relevant coupling relationships, not a blind union of independent histories.
- Splitting an interval at a cut versus the unsplit description when the same
  support facts and constraints are preserved.
- Equivalent coordinates in different units/origins after their relationship is
  established; equal ordinal positions alone must fail this equivalence test.
- Same-state conjunction independent of evaluation order, with conflicting
  conjunctions rejected and unknown prerequisites kept unresolved.

For each example record the selected source and destination occurrences, argument
bindings and call-site context, temporal support, requested result, expected facts,
retention obligations, contradictions, and current implementation status. Re-run
each important family with one missing fact, one contradictory fact and a split
input arrival where meaningful. These cases become ordinary CodeDog or `.tst`
checks only as their semantics are accepted and implemented; existing expectations
and known failures remain intact.

## Decisions to settle first

1. How does one verb model expose its resulting-state and `&` sequence views?
   Does the resulting state describe a cut or a following span, and can sequence
   members describe continuous processes? Which support comes from context, and
   which from the definition? Does an expansion include its input state?
2. What is the exact call-site anchor for `%prev`, `%tprev`, `%self` and `%ctx` in
   direct T-members, identity RHSs, nested records and embedded sequences?
3. How does explicit whole/part correspondence transport temporal support,
   membership and indirect effects without equating different occurrences?
4. How are instant selection, interval selection and throughout/sometime
   requirements expressed using the existing relationships and qualifiers?
5. Which missing-role/default rules belong in the first explicit application
   subset? Until decided, the examples supply every role they use by name.

Start with E01–E06, E09 and E19 to settle application, context and composition. Then
E10–E13 determine whether the same rules cover time spans and whole/part views.
E07/E08 and E14–E18 test interactions and boundaries without requiring every
broader capability to be implemented in the first milestone.
