# Word application and temporal description coverage

This document organizes the agreed design and coverage examples for word
applications, part selection and temporal descriptions. Definitions expand
templates whose constraints participate in ordinary normalization. The first
examples use explicit named arguments and discrete models. Realistic walking,
continuous models, role inference and natural-language argument assignment are
later work.

**Design checkpoint, 2026-10-19.** D099 records participant scope, time-slice and
session context. D100 records template application, missing parameters,
model-defined sequence completion and the accepted `^T` / `^S` view notation.
The sections below state that baseline, followed by examples, verification
properties and the remaining work. Suggestive grammar and provisional examples
are identified where they occur. This is not a claim of parser or engine support.

**Parser checkpoint, 2026-10-21 (D101).** Declaration headers now use
`#give(actor, item, object) = ...`, with local `%actor`/`%item`/`%object`
references. The parser also recognizes `^T` and `^S` as distinct view operations.
Call-site argument binding is implemented in D102; temporal conversion remains
unimplemented.

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

## Definition application

### Template meaning and uses

A definition supplies a template. Applying it contributes constraints at the
application site using the bound references and ordinary normalization rules.
The same mechanism supports several uses, and one model may combine them:

| Use | What the model can describe |
| --- | --- |
| Noun-like | Members, allowed values, evolution and allowed transitions |
| Verb-like | One or more states or constraints in a T-context, related to current or previous aspects of a situation |
| Adjective-like | Constraints on an existing object in an S-list |
| Adverb-like | Constraints on an activity in a T-list |
| Function word or selector | A related occurrence or span, such as before an event or during an epoch |
| Independent value | A result such as `sqrt(value::144)` |

These are uses of templates, not exclusive definition kinds. A word's model
determines its description; there is no universal first-argument/result identity.
Describing a change does not itself execute a physical action.

For example, the World color definitions include:

```text
#color <en> = {hue, saturation, brightness}
#red <en> = color:{hue:0}

object.color = red
```

The assertion applies the red description to the selected color occurrence.
There is no need for a separate `isRed(object)` mechanism. Constraints can supply
missing information and propagate through the model's relationships. They cannot
overwrite a contradictory known fact at the same occurrence. In a T-context,
constraints on a later occurrence can describe values different from the earlier
occurrence while preserving both. Outside a temporal context, ordinary equality
does not create a successor.

### Explicit bindings and partial information

Declare optional role names after the word, before any usage tags:

```text
#give(actor, item, object) <en verb> = { %actor %item %object }
#walk(destination) = *_+{T takeStep | ...}
```

The first body is only a syntax illustration. Parameters are comma-separated,
case-sensitive identifiers using the existing `%name` identifier syntax. Empty
`()` and omitted parameter lists are accepted; whitespace and comments are
allowed in the header. Duplicate names, trailing commas and reserved names are
rejected. Reserved names are the context tags (`self`, `W`, `U`, `ctx`, `args`,
`vars`, `prev`, `next`, `tprev`, `tnext`) and syntax names `INSERT`, `PROTEUS`,
`NULL`, `T`, `S`. Parameters do not declare types or defaults in this syntax.
Usage tags follow the closing parenthesis. The former suggestive `#walk:
destination = ...` spelling is superseded.

Body references must name a parameter in that definition or a reserved context
reference. Ordinary nested lists retain the definition's scope; nested
definitions introduce their own scope and do not capture outer parameters.

Use named bindings such as `turn(item::%prev, direction::clockwise)`. Definitions
may expose role names without imposing narrow model types merely to declare those
roles. Any actual type/value constraints belong to the model. Parameter names
cannot be reserved words, and bindings have the definition's local scope.

Argument expressions resolve in the caller's scope; the body uses its own bound
parameters, such as `%item`. Forward outer parameters explicitly. Binding a
reference need not fully normalize or copy its referent, and does not freeze its
state at call entry. Shared declarations remain unchanged.

**Missing arguments permit partial normalization.** Normalize as far as available
information and model constraints allow. Independent facts can resolve while
facts depending on missing parts remain unresolved; constraints may also determine
an omitted value. Authors should write models that work correctly with missing
parts. Omission alone is not an invalid call or a request to guess an argument.
Automatically filling the first parameter is deferred, along with role inference,
receiver sugar and natural-language application. No general default mechanism is
required for these examples.

The call grammar uses an attached opening parenthesis: `turn(...)`.
A separated `turn (...)` retains its non-call interpretation; ordinary whitespace
is allowed inside the argument list. Named binding order does not affect meaning.
Duplicate and undeclared labels are diagnosed rather than given implicit
precedence. Positional calls bind in declaration order, including an omitted
suffix; named and positional bindings cannot be mixed. This introduces no role
inference. Empty `()` and a bare word leave all declared parameters unknown.

The D102 implementation uses an application-local binding scope and the ordinary
meaning, path, correspondence and staged-publication machinery. It keeps lexical
bindings when a listSpec is projected to another member position. Unused arguments
remain undemanded, and a field read through a parameter can skip unrelated fields.
For example:

```text
#increment(item) = (%item +1)
increment(item::4)  // normalized value 5
increment(4)       // same binding by declaration order
```

Tests cover explicit caller references, nested forwarding, independent calls,
partial applications, constraints on missing arguments, embedding and finite
temporal/listSpec examples. Inverse argument binding through correspondence,
recursive proofs, joint inference requiring an
already partial expression to continue, instantiating borrowed definition literals
for writes, and general temporal retiming remain implementation boundaries.

### Temporal composition and embedding

The normalizer owns production and ordering of the history. Cursors should
observe ("surf") that history rather than duplicate its normalization or require
normalization to stop at the cursor's position. The detailed cursor integration
will be discussed before implementation; the current TCursor is not evidence
that this direction has already been implemented (D101).

Describe simultaneous requirements in one state and ordered changes in successive
states. Computations reading the prior state in one T-step read the same prior
occurrence. Interacting constraints combine through their model relationships;
evaluation order must not impose an unintended sequence or resolve a conflict.
D067 retention supplies unaffected previous descriptions after the new facts are
computed. Model-implied changes count as changes. `%?` withholds retention at its
scope; ordinary unknowns do not assert independence.

A list-valued application describes its list. `&walk(...)` embeds its logical
members exactly as `&{...}` does for a literal. Without `&`, that list remains a
contained value; the application does not select its final state. The definition
supplies the states, their relationships, completion constraints and whether an
initial state is included. Embedding preserves that model and does not recursively
flatten ordinary nested lists.

A T-member's temporal support comes from the model. Member count, boundaries,
duration, coordinates and units are distinct. One member or verb application does
not imply one second or an instantaneous event. The first development cases are
discrete; continuous trajectories and realistic activities are later work.

## Participant and context references

This section consolidates the working design accepted in D099. D095–D098 retain
the earlier discussion and alternatives. The proposed distinction between `%`
and `%self` is the working basis for these examples, not an implementation claim;
the user still intends to revisit their ergonomics after substantial temporal
models exist. In particular, levels of being such as cube, material and lattice
need concrete models, not a new hard-coded ancestor search.

| Reference | Working meaning |
| --- | --- |
| `%` | The member being described by the current application |
| `%self` | The model subject/scope in which that member is described |
| `%prev`, `%next` | The current member's neighbours in its ordered scope, including across embedded spans |
| `%tprev` and other temporal references | An explicit temporal occurrence/span in the model's T-list |
| `%item`, `%actor`, other declared roles | Parameters bound in this definition application |
| `%ctx` | Session interaction context containing recent model references |
| Other supplied global references, such as `%U` | Explicit external context governed by that reference's own meaning |

The percent prefix supplies explicit context references with their respective
scope rules. A declared parameter is not thereby placed in a shared global
parameter namespace. An unordered scope can have a current member and `%self`,
but does not supply structural `%prev` or `%next`.

### Ordinary nesting and embedding

Consider `{ A {b c} E }`, calling the ordinary nested list `{b c}` B. The outer
list has A, B and E as members. If its listSpec constrains their type, B receives
that same requirement; b and c do not replace B at the outer level. Inside B,
b has no local predecessor and c's predecessor is b. B's own outer predecessor
is A, but its internal definition cannot discover A by climbing its data-structure
parent. The outer listSpec may relate `B.b` to `A.b`. A relationship authored
inside B needs A supplied as an argument or through an explicit global reference.

An embedded list shares the surrounding list context:

```text
{ A &{ B C D } E }
```

Its logical neighbour sequence is A, B, C, D, E. B's `%prev` is A, D's `%next` is E,
and E's `%prev` is D. The wrapper is not a neighbour-visible member. Grouping
consecutive members into an embedded list preserves these neighbour relationships.
This does not recursively flatten ordinary lists nested inside those members.

`&word(...)` gets its primary explicit external bindings through arguments, while
embedding supplies its surrounding list context. It does not hide `%self` or
the neighbours of its first and last logical members. In `&{ LS | ... }`, LS can
refer to the parent context while describing each embedded member. Scope and
coverage remain distinct: in `{ before &{ LS | a b } after }`, LS applies to a
and b, not automatically to before and after. Applicable ancestor requirements
still project into the embedded members through ordinary relationships.

### Definitions, arguments and paths

A definition has class/function-style scope. It cannot refer implicitly to a
data-structure parent whose identity and structure depend on where it is used.
Explicit arguments and supplied global/context references provide external
connections. Parameter names cannot be reserved words. Each definition owns its
parameter bindings; nested definitions do not search containing lists for an
outer parameter of the same name. Forward such arguments explicitly.

Argument expressions resolve in the caller's scope before the definition body's
own context is established. For example, in the schematic assertion
`%.thing.location = f(item::%)`, the argument `%` has the caller's binding; selecting
the LHS does not silently rebind it to the selected location. Likewise an argument
`item::%prev` to an embedded word identifies the predecessor at that application
site, rather than being recaptured at every generated member. Body-relative
references still use their respective member contexts.

Bindings retain their application provenance across projection, embedding and
delayed evaluation; ordinary expression traversal does not create a new subject
binding. In `x.y`, the dot finds a member fitting description y. `x^y` follows an
external relationship, such as `x^size`. Modeled spatial location and orientation
are fields of `thing`, distinct from list positions. `x.thing.location` selects
the location through thing; `x~location` searches deeper. In `%.thing.location`
the selected path target advances without changing the original `%` binding.

### Time-slice and model level

Unless another time-slot or T-span is explicitly referenced, references describe
states at the current time-slice. This includes `%`, `%self`, `%prev`, `%next`,
parameter referents and selected model references. Everything in an S-list is
described at the same time-slice. Structural neighbours do not themselves mean
earlier or later states of the same thing.

In a T-list, L-values refer to the next state, retaining the agreed relative
reading `LHS[n+1] = RHS[n]`. Explicit temporal references such as `%tprev` or an
explicit time-slot/span select their stated temporal occurrence instead of
receiving a second implicit shift. Binding a referent in the caller's scope is
not the same as freezing its state at call entry. A referenced entity's state
follows the applicable time-slice unless another time is explicit.

To obtain intermediate or parallel states at a different level, select the
subject and use its `^T` view as described below. `%self` follows these same time
and level rules; it does not silently choose a deeper material or lattice history.
Neither list-member count nor a contextual lookup supplies duration or an
interior trajectory by itself.

### Session interaction context

The initial `%ctx` contract is a session-scoped, recency-ordered collection of
model references established through the agent's explicit interactions:

1. It works in ordinary assertion lists as well as T-lists. Source order governs
   contextual reference resolution; that order does not advance modeled time.
2. References explicitly addressed or supplied in an assertion or query become
   available, including referents on either side of an assertion. Internal
   searches performed while evaluating definitions do not silently populate the
   session context.
3. Selection uses ordinary model matching. `%ctx.planet` finds the most recently
   established reference fitting `planet`. Mars fits through its model; no rule
   extracts a type from the spelling `favoritePlanet` or invents a new name lookup.
4. Each assertion resolves its `%ctx` references against the context available
   before that assertion, then contributes its resolved referents for subsequent
   assertions. Once resolved, a reference stays bound when session context later
   changes. The binding preserves a reference, not a frozen copy of its state.
5. Context lasts for the session; another assertion list does not automatically
   clear it. An unordered assertion collection cannot establish recency through
   evaluator scheduling: use an already established context or explicit bindings.

The user's example is an agent interaction containing three assertions:

```text
{
    %U.bike.color=red
    %U.favoritePlanet=Mars
    %ctx.planet.color = %ctx.bike.color
}
```

| Assertion | Information and context contribution |
| --- | --- |
| `%U.bike.color=red` | The user's bike is red; that bike becomes available in context. |
| `%U.favoritePlanet=Mars` | The user's favorite planet is Mars; the planet referent becomes available. |
| `%ctx.planet.color = %ctx.bike.color` | That planet and that bike have the same color; with the earlier assertion, red follows for the planet. |

These readings assume the models support the stated selections and identify
Mars as a planet. All three assertions describe one time-slice. Source order
resolves the contextual references; the bound information is a conjunction of
assertions, not a sequence of physical changes. Later references to another
planet or bike cannot redirect the already-bound third assertion.

### Scope of applied constraints

D100 clarifies the earlier question about external modification in D099:
definitions expand templates, and their constraints are applied through ordinary
normalization. Explicit parameters and supplied global/context references can
connect those constraints to other occurrences. Deductions through those
relationships, including facts about unknown arguments, are part of normalization.

The no-escape rule continues to restrict implicit references: a definition cannot
discover its data-structure parent or capture arbitrary outside occurrences.
Embedded context and explicit relationships provide the connections described
above. Constraint application preserves qualifications and occurrence identity;
it does not rewrite a known prior state to make a new-state description succeed.
Shared declarations remain read-only. Physical execution and explicit replacement
of erroneous facts retain their separate contracts.

## Temporal views and selection

The accepted short forms are `^T` and `^S`, corresponding to the previously
suggested `^asTSpan` and `^asSList`. They use the existing external-relationship
notation, with dedicated parser operations for the exact uppercase tokens.
These are semantic views of the selected subject and support. Other names such
as `^size`, `^Timeline`, `^t` and `^s` remain ordinary external-property paths.
Selections, paths and identities after a view apply to its result, as in
`cube^T@[1]^S.location` and `cube^T = {T ...}`. Parsing these forms does not yet
implement their normalization; a demanded conversion reports unsupported.

| Expression | View |
| --- | --- |
| `rubixCube^T` | The cube's T-span corresponding to the selected cube state |
| `rubixCube.part^T` | The selected part's T-span, giving access at that lower level |
| `%self^T` | The temporal view of the enclosing model subject |
| `%^T` | The temporal view of the current member |
| `tItem@[1]^S` | The S-item view of the first selected temporal member |

For a T-span obtained from an object state, its temporal parent evaluates to that
object's lifetime T-list. This is an explicit semantic relationship supplied by
the view, not an implicit climb through storage containers. The selected span
and the full lifetime retain their distinct supports. A part's lifetime can
extend beyond the period when it belongs to the selected whole.

Selecting a subpart before taking `^T` provides a handle into a lower model level;
selecting `%self` instead of `%` can provide the enclosing level under their
working scope meanings. Storage may place or share state descriptions at whichever
level minimizes space. Storage placement must preserve semantic subjects, supports,
boundaries and references, and must not redefine `%self` or erase distinct demanded
occurrences.

T-list subspans use the same selection mechanisms as S-lists:

- `tItem@[n]` selects a temporal member using one-based item indices.
- `tItem@[start, end-desc]` is the proposed simple span-selection sugar. Preserve
  the end description as a selection constraint; its full grammar still needs
  validation alongside existing slice notation.
- Intersection-based selection handles starts and ends constrained by position,
  description, internal pattern, duration or combinations of these.

In `tItem@[1]^S`, selection happens first; `^S` gives that occurrence's S-view,
preserving its subject and temporal support. This is the agreed basic operation
for obtaining an S-item from a multi-state span. Applying `^S` directly to an
unselected span containing several different states is not yet specified; it
must not silently choose a member or discard temporal distinctions. View rules
remain open to pragmatic refinement through examples.

## Completion and interval constraints

Sequence extent is established through model constraints and ordinary list
correspondence. A constraint applicable only at the terminal member can identify
that member. A proved nonmatching continuation can end a repeated pattern.
Missing information remains unresolved, rather than proving a nonmatch. This
uses the Earley-like correspondence approach already present in normalization;
it does not introduce a separate loop or goal-termination operator. General
temporal applications still need their own implementation validation.

For the simple completed walk, let G describe satisfaction of the destination:
the final state satisfies G, and every preceding state of that walk fails G.
This is the agreed first-arrival condition for this example, not a requirement
on every verb. The model determines whether it includes an initial state, how
an initially satisfied goal is represented, and which transitions are allowed.
`&` does not insert an extra arrival state or decide these matters.

`location=destination` is sufficient for a core example. A realistic destination
can constrain a range of acceptable locations, such as being at a store or in
L.A.; it need not be a single coordinate. The completion condition is unchanged
when G becomes a richer constraint. Realistic destination models, resting,
tripping, traffic lights and finishing on either leg are deferred until walking
itself is being modeled. Well-written models are an assumption for these examples;
model repair and recovery are separate work.

The user's interval sketch combines several descriptions of the activity:

```text
walking:{
    &{ ! location=atStore | ... }
    location=atStore
}
= { <Another set of constraints or changes that may not align with the location description> }
== { <A third set of constraints that is not in terms of steps> }
```

This is suggestive notation: the angle-bracketed prose is a placeholder, not a
selection marker, and `location` stands for the model's actual location path.
Read the negation as applying to the whole location constraint in the walking
T-context. Every member of the embedded prefix must fail `atStore`; the final
member must satisfy it. The prefix's listSpec covers only its embedded members.

The other descriptions may use different partitions and need not be expressed
in steps. Their constraints combine through established correspondence over the
activity. Preserve the existing qualifications of `=` and `==`; they are not
interchangeable spellings or permission to zip unequal histories by ordinal.
A cut needed in one view need not be a state change in another. E10 supplies a
small discrete sketch; the user's fuller asynchronous example is forthcoming.

An activity can also be bounded without achieving a destination. In "walking at
6:00, studying by 8:00," models that make those activities incompatible prevent
walking from extending into studying. An intervening `&{...}` accommodates the
unspecified history. Those facts alone do not fix the exact walking endpoint or
prove arrival. A domain model can further constrain feasible continuations, for
example through a human's sleep/wake cycle. No special walking logic is required
in the application syntax.

## Dimensions of coverage

| Dimension | Cases to distinguish |
| --- | --- |
| Structural scope | Whole system; one part; several parts; nested part; overlapping views; membership and part identity |
| Temporal scope | Instant/cut; one state support; interval; transition boundary; finite history; open history; nested or embedded span |
| Description source | Literal; unparameterized word; explicit application; identity/constraint; selected description; shared listSpec |
| Temporal view | Selected state to `^T`; part versus enclosing subject; span to lifetime parent; selected temporal member to `^S` |
| Interval meaning | At both endpoints; throughout; at some time; trajectory over the interval; event occurring within it; aggregate over it |
| Composition | Same-state conjunction; ordered transitions; embedded sequence; whole/part projection; aligned or overlapping histories |
| Knowledge | Known; explicitly unknown; missing source; conflicting; ambiguous selection; pending parser/link information |
| Identity and context | Same entity at different times; equal distinct entities; aliases of one occurrence; different part roles; changing caller context |

For every case, specify what is requested. A selected fact, a whole state, a
history, a proof that a condition holds throughout an interval, and a query
following future states impose different obligations.

### Coverage index and development order

Example identifiers remain stable so later fixtures and design discussions can
refer to them. Work on core discrete examples before the deferred models.

| Group | Examples | Purpose |
| --- | --- | --- |
| Core application | E01–E06, E09, E19 | Literal/template equivalence, explicit bindings, changes, composition, embedding and partial models |
| Next discrete temporal cases | E10, E11, E13 | Whole/part views, asynchronous and implied changes, interval constraints and boundaries |
| Interaction and knowledge checks | E07, E08, E14, E15, E18 | Conflicts, retention, changing membership, partial histories and shared listSpecs |
| Later capabilities/models | E12, E16, E17 | Continuous motion, general inverse reasoning and physical execution |

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

Use `object.color=red` as the smallest constraint-template case. Compare it with
the corresponding explicit color description, retaining the model's nominal
qualifications and the selected occurrence. Applying the definition contributes
ordinary constraints without changing the shared declaration.

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
The input/output reading here comes from the established T-list context; an
S-list `%prev` does not independently request another time-slice.

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

### E09 Walking as an embedded, parameterized temporal list

Use the user's schematic definition and an explicitly named argument:

```text
#walk(destination) = *_+{T takeStep | ...}

walker:{T|
    initial-state
    &walk(destination::door)
    following-state
}
```

The definition describes a T-list of unspecified extent. Its `takeStep` listSpec
supplies each successive state as the result of a step. `&walk(...)` embeds those
members in the surrounding history. Compare it with the corresponding embedded
literal using the same bindings and context. The input state precedes the span
in this example; embedding does not insert or remove it.

The ellipsis stands for the remaining model. Bind the walker and starting state
through permitted embedding context, explicit arguments or supplied globals.
Relate the destination to the stepper's modeled location. The header alone does
not establish arrival. Completion uses the first-arrival constraints described
above: all earlier walk states fail the goal, and the final state satisfies it.
That final occurrence remains a valid result of the step model.

Check the first member's relationship to initial-state, later members' relations
to their predecessors, and following-state's relationship to the completed span.
The destination remains bound to this application while body-relative references
vary by member. Structural and temporal references retain their distinct rules.
A missing destination permits partial normalization; an unknown span endpoint
must not be guessed merely to place following-state.

#### Optional refinement: alternating steps with reorientation

The user's suggestive sketch illustrates simultaneous and sequential composition:

```text
*_+{T
    {T
        {step(left-foot) reorient(direction)}
        {step(right-foot) reorient(direction)}
    }
  | ... location=destination
}
```

Each phase combines step and reorientation constraints on the same stepper;
left and right phases are sequential, and cycles repeat. `step` relates the
stepper's earlier and later location fields. The quoted positional calls and
abbreviated location path are illustrative. Concrete applications use explicit
role bindings, for example `step(foot::left-foot)`, with the shared stepper and
other required relationships supplied by the model.

`&walk(...)` embeds the outer cycles without flattening their ordinary nested
T-lists. Inside a nested cycle, its first phase has no local structural predecessor.
An outer relationship or explicit argument/context connection supplies its
relation to the preceding cycle. A final location constraint must refer to the
appropriate final stepper occurrence or its projection, not an unrelated field
on the cycle description.

The sketch's final positive constraint alone does not express first arrival;
combine it with the earlier-state negative constraint from the completion model.
The simple example need not yet handle either leg finishing, resting, tripping,
traffic lights or realistic route selection. Those are walking-model refinements,
not prerequisites for the language composition rules.

### E10 Whole and part histories with asynchronous changes

Select a system's temporal view with `system^T` and a part's with `system.part^T`.
For a selected temporal member, `tItem@[1]^S` gives its S-view. The corresponding
whole and part views must agree wherever they describe the same occurrences and
supports, even when their state-change boundaries differ.

**Provisional discrete sketch.** This table illustrates the consequences of the
agreed rules; the user will supply a fuller synchronous/asynchronous and implied
change example. S0 through S3 are successive whole-system states, not time units.
The table gives the complete four-state history for this sketch. A and B are
independently changing binary parts. The model also requires C's value to equal
A's at every state, so changes to C are implied by that constraint.

| Whole-system state | A | B | C | Change from the preceding state |
| --- | --- | --- | --- | --- |
| S0 | 0 | 0 | 0 | Initial description |
| S1 | 1 | 0 | 1 | A changes; C follows through the model |
| S2 | 1 | 1 | 1 | B changes |
| S3 | 0 | 1 | 0 | A changes; C follows through the model |

A's constant span covers S1 and S2; B's covers S0/S1 and S2/S3. The whole-system
cut between S1 and S2 remains accessible in A's temporal view without inventing
an A transition. C's values and changes follow from the model rather than an
extra scheduled operation. The equality of A's and C's values does not identify
their occurrences or entities.

Check the same facts through whole-system states, part T-spans and their selected
S-views. Preserve source occurrence and temporal support when projecting or
recombining them. Compact representation of constant spans must retain every
requested cut and occurrence distinction. Unknown alignment remains unknown;
equal member indices in two differently partitioned histories do not prove
simultaneity. Concrete model syntax and the full correspondence cases will be
refined with the user's example.

### E11 Discrete interval constraints and different partitions

Use the walking interval sketch above as the first case. Combine its not-at-store
prefix and final at-store member with a second description of changes and a
third description using a different partition. Require the same selected facts
where the descriptions overlap, while preserving each relationship's qualifiers.

For a smaller discrete case, describe an indicator over an interval I and compare:

- It is on at both endpoints; interior states are unspecified.
- It is on at every state in the selected discrete span.
- It is on at some state within the span.
- One on-to-off change occurs within the span.

An interior selection must distinguish these descriptions. A constraint on every
member covers what those members describe; samples alone do not establish a fact
at all omitted intermediate states. Combine a constant indicator span with a
second part that changes inside it, using E10's correspondence principles.

Definitions used as function words may select spans, such as before an event or
during an epoch. Express their models through view relationships and ordinary
intersection-based selection; no dedicated activity/interval controller is
assumed. Duration constraints can use explicit model coordinates and units.
Continuous trajectories, rates and averages remain later cases.

### E12 Step results and motion within each step

**Deferred continuous-model case.** Complete the discrete examples first.

Compare a walk described by its successive foot-placement states with a model
that also describes the motion during each step. Query a time between two known
placements. The endpoint positions alone must not imply a straight path, uniform
speed, or a particular position at the midpoint. Those require relationships in
the step model; different motions can have the same endpoint states.

Members may describe intervals with internal motion rather than only sampled
instants. Specify how a state is projected at a requested time within a member's
support. Embedding the walk preserves whatever temporal description its model
supplies. Neither the number of steps nor `&` establishes elapsed time,
trajectory, or boundary-value conventions.

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
For the discrete cases, express the same boundary in different model units or
origins and require their established coordinate relationship; equal numeric
coordinates alone do not identify boundaries.

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

Also omit a declared argument from an application. Facts independent of that
argument should still normalize; dependent facts remain partial unless other
constraints determine them. Distinguish a missing argument from a contradictory
supplied value, without introducing automatic argument selection.

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

Repeat with E09's walk embedded in a history that has its own listSpec. The
walk's `takeStep` requirement and applicable outer requirements must both reach
the step occurrences through their respective contexts; neither replaces the
other. The shared template must not identify distinct step occurrences.

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

Then use `rectangle(height::4)` with width omitted. Normalize the available
description, and resolve width if the model and other supplied constraints
determine it. There is no requirement to reject an application solely because
one role was omitted.

## Equivalence and failure checks

For the supported fragment, use these as semantic checks rather than printer
string equality:

- Literal meaning versus a word versus explicit parameterized application.
- Permuting named bindings without changing their roles.
- Omitted arguments versus explicitly unknown arguments where the model gives
  them the same constraints; independent facts normalize in both cases.
- Explicit source occurrence versus the corresponding relative temporal reference.
- Inline changes versus an embedded sequence with explicit entry binding.
- `{ A &{ B C D } E }` versus the corresponding inline sequence: B's predecessor
  is A, D's successor is E, and E's predecessor is D. In `{ A {b c} E }`, b has no
  local predecessor; outer relations require an outer assertion or explicit input.
- An embedded literal temporal model versus `&walk(...)` supplying the same
  model with preserved bindings, listSpecs, member occurrences and span boundaries.
- Whole-system description projected to a part versus that part's description at
  the same temporal support; recombination needs explicit correspondence and all
  relevant coupling relationships, not a blind union of independent histories.
- `^T` views reached from the whole and a selected part preserve their respective
  subjects and supports; each span's temporal parent is its subject's lifetime
  T-list. Storage-level changes must preserve these observable relationships.
- `tItem@[1]^S` selects the first temporal member before obtaining its S-view.
  Check the subject, occurrence and support against an explicitly selected state.
- The simple completed walk rejects earlier goal states and satisfies the goal
  at its final state. The embedded prefix's negative listSpec excludes that final
  member. Unknown goal evidence is not proof of non-arrival.
- Differently partitioned interval descriptions retain their constraints and
  operator qualifications after correspondence. A cut in one view need not be a
  transition in another. Repeat with a proved nonmatching continuation and with
  one whose compatibility is unresolved.
- Splitting an interval at a cut versus the unsplit description when the same
  support facts and constraints are preserved.
- Equivalent coordinates in different units/origins after their relationship is
  established; equal ordinal positions alone must fail this equivalence test.
- Same-state conjunction independent of evaluation order, with conflicting
  conjunctions rejected and unknown prerequisites kept unresolved.
- Argument names resolve in the caller's scope; body parameters are local and
  cannot use reserved words. Refactoring a definition must not capture an outer
  parameter merely because it has the same name.
- Ordinary S-list references share a time-slice; an explicit time/span selects
  the stated occurrence. Stable argument and `%ctx` bindings must not silently
  freeze state values or be redirected by later session interactions.
- The bike/planet assertion list resolves references in source order, then
  combines the bound facts independently of solver scheduling. Internal reads
  do not populate `%ctx`, and an unordered collection does not obtain recency
  from whichever assertion happens to be evaluated first.
- Ordinary definition scope cannot be escaped through aliases. Embedded fragments
  inherit the specified surrounding context, and their listSpecs retain their
  own member coverage. Explicit bindings connect applied template constraints to
  their referents through ordinary normalization, preserving occurrence identity
  and qualifications. A contradiction cannot be repaired by rewriting a prior
  state or the shared declaration.

For each example record the selected source and destination occurrences, argument
bindings and call-site context, temporal support, requested result, expected facts,
retention obligations, contradictions, and current implementation status. Re-run
each important family with one missing fact, one contradictory fact and a split
input arrival where meaningful. These cases become ordinary CodeDog or `.tst`
checks only as their semantics are accepted and implemented; existing expectations
and known failures remain intact.

## Development order and remaining work

The settled baseline covers template application, partial normalization with
missing arguments, ordinary list embedding, participant/context scope, common
time-slice references, model-defined sequence termination, the simple walk's
first-arrival condition, and `^T` / `^S` with one-based `@[...]` item selection.
These are no longer open alternatives in the core design.

### Next discrete examples

1. Refine E10 with the user's synchronous/asynchronous and implied-change model.
   Establish explicit whole/part correspondence and repeat its queries through
   temporal and S-views. Use the small table as a provisional check, not a final
   definition of the user's intended example.
2. Make E11's differently partitioned descriptions concrete. Exercise a terminal
   constraint and a proved incompatible continuation using ordinary matching.
   Preserve unknown alternatives and the distinctions between `=`, `==` and
   prefix-qualified relationships.
3. Validate the proposed application and span-selector grammar. The view names
   `^T` and `^S`, `tItem@[1]^S`, and one-based item indexing are agreed; call-header
   details and `@[start, end-desc]` sugar still need concrete parser examples.
4. Exercise initial-goal, zero-new-step, open-span and missing-part cases through
   simple models. Their outcomes follow those models; they do not require a
   universal walking policy or a new normalizer termination mechanism.

### Deferred refinements

- Direct `^S` on an unselected span containing different states, and the eventual
  spelling of the temporal-parent relationship to an object's lifetime T-list.
- Final `%` versus `%self` ergonomics after substantial temporal models exist.
- First-argument autofill, positional conveniences, receiver calls and natural
  language role inference.
- Realistic walking, complex destination/location constraints, and continuous
  models such as E12. Continuous trajectories and exact discontinuity conventions
  are not prerequisites for the discrete examples.
- Richer `%ctx` organization, reset and incomplete/rejected interaction behavior.
  Existing bindings remain stable and evaluation scheduling cannot set recency.
- General inverse planning, physical execution and model-repair/recovery policies.

Use the development groups in the coverage index to stage implementation and
verification. Design acceptance does not change the engine's supported-feature
boundary; each implemented increment needs focused behavioral evidence.
