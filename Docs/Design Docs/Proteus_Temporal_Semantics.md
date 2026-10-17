# Proteus Temporal Semantics — Working Notes

**Status:** Living design document\
**Project:** Proteus / Slipstream\
**Purpose:** Collect and refine the semantic rules governing T-lists, S-lists, temporal identity, sequencing, temporal refinement, rooting, and cross-level alignment.

> This document intentionally distinguishes **established semantics** from **derived consequences** and **open questions**.\
> It should be updated as the temporal model is clarified.

**Syntax decision, 2026-09-12:** `%prev` and `%next` are structural navigation
in ordered lists. Temporal navigation uses `%tprev` and `%tnext`, relative to
the nearest enclosing T-level. Older temporal examples in this working document
that still say `%prev` or `%next` should be read as `%tprev` or `%tnext` until
those examples are revised individually.

---

## 1. Core Distinction: S-Context vs T-Context

### 1.1 S-context

An **S-context** describes a state with no state change at the level currently being described.

This does **not** imply that nothing changes at lower levels.

For example:

- At the switch-level, a light switch may simply be `ON`.
- At a lower mechanical level, the switch may pass through many intermediate positions.
- At sufficiently low levels, these lower-level transitions may themselves refine into further state sequences.
- At the limiting physical representation, World may be regarded as a sparse sequence of S-lists at Planck-scale temporal resolution.

Thus, **S vs T is level-relative**.

### 1.2 T-context

A **T-context** represents an ordered sequence of states or state transitions.

Temporal order matters.

A T-list need not be measured directly in physical time units. Its items may instead represent state transitions at whatever level of abstraction is being modeled.

Time units can later be used as a metric or as another view on the same temporal structure.

---


## 1.3 T-lists are specialized ordinary lists

**Status: Clarified 2026-09-11**

A T-list should not be treated as an entirely separate data structure.

It is fundamentally an ordinary list with additional or modified semantics appropriate to temporal interpretation.

Much of the local behavior of a T-list could, in principle, be reproduced using an ordinary ordered list plus explicit references such as:

```proteus
%prev
%next
```

and enough additional identities describing:

- which state precedes which;
- how nested temporal spans connect;
- the beginning and end of each temporal span;
- how adjacent sublists share entry/exit state;
- how references are interpreted across state boundaries.

However, doing this manually would produce substantial structural boilerplate and obscure the intended temporal meaning.

Thus `T` is partly a semantic compression mechanism: it makes a common pattern of ordered, stateful identity relations first-class.

---

## 1.4 What `T` adds beyond `%prev` and `%next`

**Status: Working formulation**

The semantics introduced by `T` appear to include at least:

1. **Temporal interpretation of order**
   - list position denotes temporal succession at the current level.

2. **Temporalized identity**
   - in T-context, references such as the RHS of an identity may resolve through `%prev`.

3. **Implicit sequence continuity**
   - adjacent T-items and nested T-spans are interpreted as belonging to one continuous temporal sequence unless otherwise constrained.

4. **Entry / exit semantics**
   - temporal sublists have meaningful beginnings and endings that connect to surrounding T-contexts.

5. **Relative temporal references**
   - `%prev`, `%next`, and eventually `now` have T-specific meanings.

6. **Temporal rooting**
   - a T-item or T-list may be positioned relative to another T-item/T-list that supplies its temporal context.

The first five could potentially be expanded into ordinary list identities, although awkwardly.

The sixth is more fundamental because temporal meaning is not merely ordinal; it is relative to a temporal world/context.

---

## 11.3 Temporal rooting includes a world/context identity

**Status: Important clarification**

A temporal sequence is not necessarily rooted in the real-world `%W`.

Proteus must support multiple independent temporal worlds or models, for example:

- the physical World;
- a hypothetical scenario;
- a simulation;
- a fictional world such as the Buffyverse;
- a person's belief-model or imagined future;
- an alternate-history model.

A T-list may therefore have meaningful local temporal order without yet being rooted in any particular world.

Rooting establishes that its temporal positions are to be interpreted relative to a selected parent/world temporal structure.

Conceptually:

```text
local T-list
     |
     | temporal rooting
     v
Buffyverse T-context
```

is different from:

```text
local T-list
     |
     | temporal rooting
     v
%W
```

Even if both lists use concepts such as:

```text
1997
Tuesday
now
age 16
```

the meanings of those references depend on the world/context in which the T-list is rooted.

Therefore:

> Temporal position is not globally absolute merely because something is a T-list.

---

## 11.4 World-root identity must be explicit or inherited

**Status: Design requirement**

Two T-lists should not automatically synchronize merely because they use comparable time units or indices.

They synchronize only when a chain of identities/relations places them in a shared temporal context.

For example:

```text
Buffyverse.Sunnydale
Buffyverse.Buffy
Buffyverse.school-year
```

may share one temporal root.

Meanwhile:

```text
%W.Sunnydale
%W.some-person
```

belong to the physical World unless explicitly identified otherwise.

This prevents accidental cross-world temporal inference.

A T-list may inherit its root from:

- its parent temporal system;
- an enclosing world/model;
- a rooted sibling relation;
- an explicit temporal identity;
- a chain of parent/child identities.

If no such root exists, its temporal ordering may remain purely local.

---

## 11.5 T-list rooting and level alignment are separate

**Status: Clarified**

Rooting answers:

> **Which temporal world/context is this sequence situated in, and where?**

Level alignment answers:

> **How do states at one temporal resolution correspond to states/spans at another resolution?**

These often interact, but they are not the same operation.

For example, a fictional character may have:

```text
character high-level life events
```

and:

```text
character lower-level moment-to-moment states
```

Both can be rooted in the Buffyverse while still requiring separate identities to align their different temporal resolutions.

Likewise, two systems may both be rooted in `%W` but still have unknown relative phase at a finer level.

---

## 11.6 Consequence for `now`

**Status: Derived**

`now` cannot be a single universal cursor independent of temporal root.

The interpretation of:

```proteus
now
```

must be relative to a temporal world/context.

Thus conceptually there may be distinct values such as:

```text
%W.now
Buffyverse.now
simulation.now
belief-model.now
```

even if user-facing syntax normally resolves the appropriate one from context.

This also explains why an unrooted T-list may have meaningful `%prev` and `%next` but no determinate externally meaningful `now`.


## 2. Identity Semantics

### 2.1 Identity in S-context

**Status: Established**

Within an S-context, identity is co-temporal.

If the current state is \(S_i\), then:

```proteus
A = B
```

means:

\[
A_i \equiv B_i
\]

No temporal advance occurs.

Therefore:

```proteus
{
    B = 3
    A = B
    C = A
    B = 7
}
```

is contradictory, because it requires the same S-state to satisfy both:

\[
B_i = 3
\]

and:

\[
B_i = 7
\]

---

### 2.2 Identity in T-context

**Status: Established**

In a T-list, the **right-hand side of an identity refers to the previous state**.

Thus:

```proteus
A = B
```

inside a T-context behaves similarly to declarative assignment.

Formally, if the transition is from \(S_i\) to \(S_{i+1}\):

\[
A_{i+1} \equiv B_i
\]

This is not fundamentally an imperative update rule. It is an **identity relation across a temporal transition**.

This rule was explicitly documented in *The World as Information: Part 7*.

---

## 3. Sequential Meaning of T-Lists

### 3.1 Identity assertions in T-context create state transitions

**Status: Strongly supported / needs exact engine formulation**

Because an identity in a T-context takes its RHS from the previous state and constrains the next state, it necessarily introduces or participates in a temporal boundary.

For example:

```proteus
{T
    B = 3
    A = B
    C = A
    B = 7
    ...
}
```

is coherent as a temporal sequence.

Conceptually:

```text
S0 --B=3--> S1 --A=B--> S2 --C=A--> S3 --B=7--> S4
```

With persistence of unspecified values:

```text
       B    A    C
S1     3
S2     3    3
S3     3    3    3
S4     7    3    3
```

The equivalent statements in a single S-context would be contradictory.

---


## 3.2 State descriptions and transition descriptions are different kinds of T-items

**Status: Established / clarified 2026-09-11**

A T-list may contain at least two semantically different kinds of descriptions:

1. **S-description** — describes the state of the system at a temporal position.
2. **T-description / transition description** — describes how a next state relates to the previous state.

A first S-description can initialize a system:

```proteus
thing:{T
    initial-state
    ...
}
```

A transition description instead constrains a state by reference to `%prev`.

For example:

```proteus
a = a + 1
```

in T-context is interpreted conceptually as:

```text
current.a = %prev.a + 1
```

or, in identity form:

\[
a_i \equiv a_{i-1}+1
\]

Thus the transition itself is a value/description whose internal references are mapped through `%prev`.

### Named transitions: verbs

A named transition description is called a **verb**.

For example:

```proteus
#take-a-step = {T
    ...
}
```

or a repeated transition:

```proteus
#walk = *_+{T
    take-a-step
| ...}
```

These examples are deliberately simplified.

A verb is therefore not fundamentally an imperative procedure. It is a reusable T-description constraining one or more transitions.

### Can a T-description be the first item?

The sequence-composition rule may allow a T-description to appear without an explicit preceding S-description.

However, a transition that references `%prev` still requires an entry state from somewhere.

Possible sources include:

- a containing T-context;
- a parent/rooted system;
- another level of description;
- an externally supplied initial state;
- facts sufficient to infer the needed prior state.

Therefore:

> A T-description may syntactically begin a sequence, but semantically it still has an entry S-context whenever it depends on `%prev`.

A completely standalone transition whose required previous values are unknown is valid only as a partially specified sequence; its resulting state remains correspondingly underdetermined.

---

## 3.3 Sequence rule

**Status: Clarified working rule**

A T-list is a sequence of state descriptions and/or transition descriptions.

For consecutive elements:

- an S-description constrains the S-context at that point;
- a T-description maps from the preceding/entry S-context to a following S-context;
- a nested T-description inherits the current S-context as its entry and yields an exit S-context;
- the exit S-context becomes the entry/current context for whatever follows.

This permits sequences such as:

```proteus
{T
    initial-state
    verb-a
    verb-b
    later-state
    ...
}
```

and potentially:

```proteus
{T
    verb-a
    verb-b
    ...
}
```

when the entry state is supplied or left partially unknown.

---

## 8.2 Repeated transition descriptions may be injected with `&`

**Status: Established conceptually**

Repeated transitions can be inserted into a containing T-list as a temporal sub-process.

For example:

```proteus
& *10+{T
    take-a-step
| ...}
```

represents ten sequential applications of the `take-a-step` transition description.

A more general verb might be:

```proteus
#walk = *_+{T
    take-a-step
| ...}
```

where the number of steps is not fixed.

The `&` relation allows the repeated T-description to be added as a temporal component of the surrounding system rather than replacing the whole enclosing state description.

Exact parser/normalizer behavior for `&` with nested T-lists should be verified against the current engine.

---

## 8.3 T-list ListSpecs can constrain allowed transitions

**Status: Important design rule**

A T-list's ListSpec need not merely describe the structural type of each state. It can constrain which transitions are permitted at that level.

For a Rubik-style cube, a high-level T-list might conceptually constrain successive changes to members of a transition set such as:

```proteus
*_+[
    turn-front-left
    turn-front-right
    turn-top-left
    turn-top-right
    ...
]
```

The exact syntax should remain concise; for example, faces may already be constrained by a generic `turn-left` / `turn-right` transition model.

The semantic principle is:

> The ListSpec of a T-list can define the admissible transition descriptions at that level.

This transforms a T-list from merely an ordered history into a constrained state-transition space.

If the current state is \(S_i\) and the allowed verbs are \(V\), then the next state must satisfy at least one applicable transition relation:

\[
S_{i+1} \in \{v(S_i) \mid v \in V,\ v\text{ applicable}\}
\]

where `v(S_i)` is only shorthand: Proteus should represent this declaratively by identities, not by imperative function application.

---

## 8.4 Constraints at multiple levels coexist

**Status: Core principle**

Constraining the transitions at one level does **not** exclude transitions defined at other levels.

For example, a high-level Rubik-style cube model may allow:

- turn front left,
- turn front right,
- turn top left,
- etc.

while lower or alternate levels may independently allow processes such as:

- partially rotating a face;
- disassembling cubelets;
- deforming components;
- melting plastic;
- breaking material bonds.

Therefore a system's actual possible evolution is constrained by the compatible transition relations available across all relevant descriptions/levels.

This is important: a high-level transition vocabulary is **not a closed ontology of everything physically possible** unless the model explicitly asserts that it is.

---

## 8.5 Allowed verbs constrain possibilities without forcing a transition

**Status: Derived clarification**

A list of allowed verbs should normally mean:

> If a transition occurs at this level, it must conform to one of these transition descriptions.

It should not automatically mean:

> One of these transitions must occur immediately.

Otherwise persistent states would be impossible unless an explicit `stay-the-same` verb were included.

The exact treatment of duration/persistence still needs formalization, but the distinction between **allowed transition** and **required transition** should be preserved.

---

## 18.2 Meaning of `...` in T-context

**Status: Clarified 2026-09-11**

After `...` in a T-list, the state/history is unknown or incompletely specified.

This does **not** mean that anything whatsoever is permitted.

Unknown states may be partly or fully reconstructable from constraints including:

- allowed transitions / verbs at the current level;
- known states before or after the ellipsis;
- transition constraints at lower or higher levels;
- parent/child relations;
- sibling synchronization;
- conservation or dynamical constraints;
- momentum or other physical state;
- externally asserted facts.

Thus:

```proteus
{T
    known-state-A
    ...
    known-state-B
}
```

means that the intervening history has not been explicitly stated.

It may range from:

- completely undetermined;
- constrained to a finite set of possible histories;
- uniquely reconstructable from surrounding identities and transition rules.

The same applies to:

```proteus
{T
    known-state
    ...
}
```

where future states may be unknown but constrained by the system's transition models.

### Important distinction

`...` represents **missing specification**, not an explicit "random transition" operator and not an assertion that all states are possible.

The normalizer/search system may therefore infer structure inside an ellipsis whenever existing identities and transition constraints are sufficient.

---

## 19.3 Repetition can quantify transitions as well as objects

**Status: Clarified**

Repetition has at least two temporal uses that should not be conflated:

1. **Repeat/quantify objects**
   ```text
   many galaxies
   ```

2. **Repeat/quantify transitions**
   ```proteus
   *_+{T take-a-step | ...}
   ```

The second creates an unknown-length sequence of transition occurrences.

When combined, scope becomes important. For example:

> many galaxies, each with an independently located beginning

is different from:

> one temporal process repeatedly producing galaxy beginnings.

A formal treatment of repetition scope remains an open requirement.

---


## 4. Persistence Across T-Boundaries

### 4.1 Unchanged information persists

**Status: Derived but necessary**

A temporal transition should not erase every property that is not explicitly changed.

If a component \(X\) is unaffected by a transition, then:

\[
X_{i+1} \equiv X_i
\]

This functions as a frame rule.

However, this should **not** be represented by declaring the entire adjacent S-states identical, because the states differ precisely in whatever changed.

Persistence therefore applies component-wise to unaffected information.

---

## 5. Simultaneous Changes

### 5.1 Unordered collections of changes occur across the same T-boundary

**Status: Established**

If a transition contains an unordered S-list of identity assertions:

```proteus
{
    A = B
    C = D
    E = F
}
```

then every RHS refers to the same previous state and every LHS constrains the same next state:

\[
A_{i+1} \equiv B_i
\]

\[
C_{i+1} \equiv D_i
\]

\[
E_{i+1} \equiv F_i
\]

This is important for operations such as rotating a Rubik-style cube face.

For example, in:

```proteus
face.edges.n = face.edges.w
face.edges.e = face.edges.n
```

the second RHS refers to the **old** `face.edges.n`, not the newly assigned value.

Thus simultaneous declarative updates behave as intended.

---

## 6. Sequential Composition

### 6.1 Adjacent temporal segments share a boundary S-state

**Status: Reconstructed; exact original wording not yet recovered**

The likely underlying rule is:

> The exit S-context of one T-segment is identical to the entry S-context of the next T-segment.

Conceptually:

```text
segment A:  SA0 -> SA1 -> SA2
                             ||
segment B:                  SB0 -> SB1 -> SB2
```

with:

\[
SA_2 \equiv SB_0
\]

This is preferable to saying simply “the last item of one T-list equals the first item of the next,” because T-lists may contain transition descriptions rather than only explicit S-state items.

What is shared is the **boundary S-context**.

---

## 7. Nested / Injected T-Lists

### 7.1 A nested T-list inherits an entry state from the surrounding context

**Status: Strongly supported by prior Proteus examples**

A nested or injected T-list should begin from the state immediately preceding its insertion.

For example:

```proteus
@verb = {T
    change1
    change2
    change3
}
```

When inserted after host state \(H_i\):

- the nested T-list's entry state is identified with \(H_i\);
- its internal changes produce a sequence of new states;
- its exit state becomes the state from which the enclosing T-list continues.

This principle appears in earlier notes describing lower-level continuous transitions as:

1. a starting state inherited from the prior state,
2. zero or more intermediate states,
3. a desired final state.

---

## 8. Repetition in T-Lists

### 8.1 Repetition repeats temporal transitions

**Status: Established in prior examples**

For example:

```proteus
*4+{T
    A = A + 1
| ...}
```

represents four successive transitions.

Conceptually:

```text
S0 -> S1 -> S2 -> S3 -> S4
```

Each repetition's entry state is identified with the previous repetition's exit state.

Thus a T-list ListSpec behaves similarly to a declarative loop.

---


## 8.6 T-list ListSpecs can constrain state-to-state relations directly

**Status: Clarified 2026-09-11**

A T-list may contain ordinary S-descriptions at successive positions while its ListSpec constrains how each state may relate to `%prev`.

Example with no transition restriction beyond the state domain:

```proteus
{T
    *10+_
|
    5 8 2 5
}
```

This describes a system whose successive states are:

```text
5 -> 8 -> 2 -> 5
```

The values may appear unrelated because the ListSpec constrains only the state form/domain, not the transition relation.

A constrained example:

```proteus
{T
    *10+_ = *_+[%prev+1 %prev-1]
|
    3 4 5 4 7
}
```

Here every state after the entry state must be identical to either:

```text
%prev + 1
```

or:

```text
%prev - 1
```

Therefore:

```text
3 -> 4 -> 5 -> 4
```

is coherent, but:

```text
4 -> 7
```

is contradictory because neither permitted relation produces `7`.

This gives a very important general rule:

> A T-list ListSpec may constrain the admissible relation between each S-description and the preceding S-context.

A T-list therefore need not explicitly contain named verbs at every transition. The transition relation can be supplied by the ListSpec and satisfied by successive S-descriptions.

---

## 8.7 Missing S-descriptions may be solved from transition constraints

**Status: Established by example**

Consider:

```proteus
{T
    *10+_ = *_+[%prev+1 %prev-1]
|
    3 4 5 4 _ _ 7
}
```

The unknown states are not arbitrary.

Starting from the known `4`, there are three transitions before reaching the known `7`:

```text
4 -> _ -> _ -> 7
```

Since each transition must be `+1` or `-1`, the only possible state sequence is:

```text
4 -> 5 -> 6 -> 7
```

Therefore the two missing S-descriptions are uniquely determined:

```text
5, 6
```

The transition relation also forces all three transitions in that suffix to be `+1`.

This demonstrates that unknown states inside a T-list can be solved by normalization/search from:

- a prior known state;
- a later known state;
- the number of temporal positions between them;
- the transition constraints imposed by the ListSpec.

This is stronger than merely saying that `...` or `_` denotes missing information. Missing temporal states can be logical unknowns subject to constraint propagation.

---

## 8.8 State descriptions occupy positions; transition relations connect positions

**Status: Clarified**

An S-description in a T-list describes **the state at the temporal position where it occurs**.

For example:

```proteus
{T
    *10+_
|
    5 8 2 5
}
```

contains four state positions:

```text
position 0 : 5
position 1 : 8
position 2 : 2
position 3 : 5
```

A transition constraint determines which pairs of adjacent positions are legal.

Thus it is useful to distinguish:

```text
state at position i
```

from:

```text
relation connecting position i-1 to position i
```

A named verb may explicitly describe that relation, but a T-list ListSpec may also supply the relation implicitly.

This suggests that T-semantics should not require every temporal position to contain an explicit transition object. Rather:

- S-descriptions constrain positions;
- transition descriptions constrain edges between positions;
- ListSpecs may constrain either or both.

---

## 8.9 Explicit verbs and implicit transition constraints should normalize compatibly

**Status: Design goal**

The following two kinds of representation should, where semantically equivalent, converge on the same transition relation:

### Explicit verb

```proteus
#increment = {T
    a = %prev.a + 1
}
```

### ListSpec constraint

```proteus
{T
    state = %prev.state + 1
|
    ...
}
```

A named verb is useful when the transition has reusable structure or semantic meaning.

A ListSpec relation is useful when the transition space itself defines the legal evolution of the T-list.

The engine should not treat these as fundamentally different kinds of causation. Both are declarative constraints between adjacent S-contexts.


## 9. Levels of Temporal Description

### 9.1 High-level S-states may contain lower-level T-spans

**Status: Established conceptually**

An S-state means “unchanged at this level,” not “unchanged at every lower level.”

Example:

At a high level:

```text
switch = ON
```

may be one S-state.

At a lower level, the switch may contain a T-list describing:

- lever motion,
- contact motion,
- electrical transitions,
- mechanical settling,
- etc.

Thus:

> A high-level S-state may contain one or more lower-level T-lists.

---

### 9.2 High-level states correspond to lower-level temporal spans

**Status: Core design principle**

A state at a higher level may be identified with, or mapped to, a sequence of lower-level states.

Conceptually:

```text
High-level state H_i
        |
        | view / identity mapping
        v
Low-level span L[a:b]
```

A high-level transition:

```text
H0 -> H1
```

may correspond to:

```text
L0 -> L1 -> L2 -> ... -> Ln
```

The important identities are likely at the span boundaries:

\[
view_L(H_0) \equiv L_0
\]

\[
view_L(H_1) \equiv L_n
\]

The intermediate states need not themselves correspond to valid high-level states.

This matches continuous-state examples such as rotating a cube face through intermediate angles while the higher-level model records only quarter-turn states.

---

## 10. Parent/Child vs Higher/Lower Level

### 10.1 These are distinct relations

**Status: Current conclusion**

There are at least two independent axes:

#### A. Parent / child relation

This relates **different systems/items**.

Example:

```text
watch
  |
  +-- gear
```

A gear is part of the watch.

#### B. Higher-level / lower-level relation

This relates **different temporal resolutions or descriptions of the same system/item**.

Example:

```text
gear at coarse resolution
        |
        v
gear at fine mechanical resolution
```

These relations often interact, but they should not be identified.

A watch state may contain a lower-level span of a gear, which makes the two axes appear together in one representation.

---

## 11. Parent/Child Temporal Alignment

### 11.1 Rooting a child T-list into a parent provides temporal constraints

**Status: Open / under active design**

Example:

```text
watch
  |
  +-- gear
```

If the gear is rooted in the watch, relations in the watch may align the gear's T-list relative to:

- other gears,
- watch state transitions,
- parent state transitions,
- clock or metric structure.

If the parent has many existing relations, these may strongly constrain the child's temporal phase.

If the parent is relatively sparse, such as `%W`, rooting alone may leave much of the child's temporal placement unresolved.

Further assertions may then be needed to synchronize the child with other items.

---

### 11.2 Parent boundaries may partition child timelines

**Status: Candidate rule**

A state boundary at a higher-level or parent context may induce a partition in a rooted child T-list.

However:

> A temporal partition does not necessarily imply a state change in the child.

A child may have the same state on both sides of a parent-level boundary.

This distinction between **segmentation** and **state transition** is important.

---


## 11.9 Rooting is containment in a parent/world T-structure

**Status: Corrected 2026-09-11**

Rooting should retain its literal Proteus meaning:

> A rooted item/T-list is contained by the parent/world model.

The previous proposal to replace this with a separate `%root` placement-view was too weak. Temporal placement is a consequence of the containment relation; it is not a substitute for it.

For an item `me`:

```proteus
me.%asTList
```

denotes the T-list describing `me` over its lifetime.

If:

```proteus
*_+[ &%W@[start:end] ]
```

means "one of the T-lists contained in this selected World span," then the child-centric rooting assertion:

```proteus
me.%asTList = *_+[ &%W@[start:end] ]
```

is structurally attractive.

The RHS selects a **contained child T-list**, not the World span itself.

Therefore strict identity is preserved:

```text
me's lifetime T-list
    ≡
one child T-list contained by %W
```

We are not asserting:

```text
me's lifetime state-content ≡ all World state-content
```

---

## 11.10 No universal T-list is required

**Status: Current conclusion**

There is no need to introduce a special "universal T-list containing this T-list."

The parent/world already supplies that containment context.

For example:

```text
me.%asTList
    is contained in
%W
```

or:

```text
Buffy.%asTList
    is contained in
Buffyverse
```

The same containment mechanism therefore supports:

- the physical World;
- fictional worlds;
- simulations;
- hypothetical models;
- belief worlds.

Root identity is simply ordinary containment/identity in the appropriate parent T-context.

---

## 11.11 Whole-life rooting and beginning placement are separate constraints

**Status: Important distinction**

For a long-lived object, the entire lifetime should not be forced inside an epoch merely because its beginning lies there.

Therefore distinguish:

### A. Root the whole lifetime in the World

Conceptually:

```proteus
me.%asTList = *_+[ &%W ]
```

or whatever exact syntax selects one child T-list contained in `%W`.

### B. Constrain the first lifetime state to the desired World epoch

Conceptually, from the World side:

```proteus
%W@epoch = {T
    ...
    &me.%asTList@[0]
    ...
}
```

This says that the first state/position of `me`'s already-rooted lifetime occurs somewhere in the selected epoch.

Thus:

```text
whole child T-list -> rooted in World
first child T-item -> located within epoch
```

These two assertions together say:

> `me` exists as a World-contained temporal object, and its beginning occurs during this epoch.

The exact reference syntax for the inverse child-side form still needs testing.

---

## 11.12 Parent containment supplies temporal alignment

**Status: Working rule**

Once a child T-list is contained by a parent T-list, the parent/child relation provides the framework in which their temporal states can align.

Additional identities may determine:

- exact start;
- exact end;
- scale/resolution;
- phase relative to siblings;
- intermediate state correspondence.

Containment therefore supplies the **root context**, while other assertions refine synchronization.

This matches the earlier principle:

> Rooting does not necessarily imply full synchronization.

But rooting **does** imply parent containment.

---

## 11.13 Child-centric and World-centric rooting are inverse views of one identity

**Status: Design goal**

The following should be equivalent views of the same underlying containment:

### Child-centric

```proteus
me.%asTList = oneOfContainedTList(%W)
```

### World-centric

```text
%W contains &me.%asTList
```

Similarly, constraining the beginning may be stated from either side:

```text
me's first T-item is one of the child/state items rooted in epoch
```

or:

```proteus
%W@epoch = {T
    ...
    &me.%asTList@[0]
    ...
}
```

The engine should derive either direction through identity/search rather than storing independent facts.



## 11.14 Prefer hierarchical rooting over direct `%W` rooting

**Status: Clarified 2026-09-11**

Most temporal objects should not need to be rooted directly into `%W`.

Instead, root an item into the nearest natural containing system whose T-list is already rooted.

Conceptually:

```text
planet
  -> solar-system
      -> galaxy
          -> %W
```

or:

```text
person
  -> sufficiently broad containing physical system
      -> ...
          -> galaxy
              -> %W
```

The exact intermediate hierarchy depends on the ontology and on what parent can coherently contain the child's relevant lifetime/span.

### Direct `%W` roots

Direct rooting into `%W` should be reserved primarily for top-level objects for which there is no useful already-rooted parent in the current model.

Galaxies are a natural early example:

```text
galaxy
   -> %W
```

Later, if galaxy clusters, superclusters, cosmic filaments, or other containing structures are modeled, galaxies could instead be rooted through those structures where appropriate.

Thus this is a modeling preference, not a hard claim that galaxies are the only possible direct children of `%W`.

---

## 11.15 Rooting should propagate transitively through containment

**Status: Core requirement**

If:

```text
A is rooted in B
B is rooted in C
```

then `A` is indirectly rooted in `C`.

Conceptually:

```text
A.%asTList
    contained in
B.%asTList
    contained in
C.%asTList
```

should allow the engine to derive the corresponding temporal relation between `A` and `C` through ordinary containment/identity reasoning.

Therefore:

```text
planet -> solar-system -> galaxy -> %W
```

is sufficient to situate the planet in World time without an additional direct planet-to-World assertion.

This avoids duplicate root facts and gives sibling synchronization a natural path through their nearest common rooted ancestor.

---

## 11.16 Choose a parent broad enough to contain the relevant child span

**Status: Important modeling constraint**

The immediate root parent must actually contain the child for the temporal span being asserted.

A too-narrow parent can fail when the child leaves that system.

For example, a person's complete lifetime should not be rooted to a house if the person only lives there temporarily.

A narrower relationship may still be represented over a subspan:

```text
person@[a:b] rooted/contained in house@[c:d]
```

while the person's full lifetime is rooted through a broader system.

Thus distinguish:

- **lifetime/root parent** — broad enough to contain the full T-span being rooted;
- **temporary containment relations** — apply only to selected child/parent T-spans.

This rule is especially important for moving objects.

---

## 11.17 Direct World queries should traverse root chains

**Status: Design goal**

A World query should not require every matching object to be a direct child of `%W`.

For example:

> Find planets existing during epoch E.

may follow:

```text
%W
 -> galaxies
    -> solar-systems
       -> planets
```

and use inherited temporal alignment to determine which planet T-spans intersect the selected epoch.

Likewise, querying a planet for its World time should climb the same root chain in reverse.

This keeps the knowledge base hierarchical while preserving World-level discoverability.



## 11.18 Rooting may be best expressed as temporal projection of a contained child

**Status: Promising reference-semantic direction 2026-09-11**

The expression:

```proteus
*_+[ &%W@[start:end] ]
```

selects items/T-lists at the temporal level of `%W@[start:end]`.

With the current World definition:

```proteus
#world = *(20000+_) +{T million-year | ...}
```

those direct T-items are `million-year` items.

Therefore this expression cannot directly select an arbitrary child lifetime T-list such as a galaxy's lifetime.

### The missing operation is not another containing T-list

What is needed is closer to a **projection of one contained item through a parent timeline**.

Conceptually:

```text
project child X through parent T-list P
```

means:

> At each relevant temporal position of `P`, select the sub-infon/state corresponding to `X`; the resulting ordered sequence is `X`'s T-list as contained in `P`.

For a child `me` contained in `%W`:

```text
%W states:
    W0 { ... &me0 ... }
    W1 { ... &me1 ... }
    W2 { ... &me2 ... }
    ...

temporal projection of `me` through %W:
    me0, me1, me2, ...
```

Then:

```text
me.%asTList
    ≡
projection-of(me through %W)
```

is a strict identity between two views of the **same child states**.

This does not identify `me` with the World states themselves.

---

## 11.19 Rooting remains literal World containment

**Status: Core requirement**

The projection view is only a reference convenience.

The underlying facts are still:

```text
W0 contains me0
W1 contains me1
W2 contains me2
...
```

Thus rooting still means:

> `%W` contains the child through the relevant temporal span.

The child's T-list is the ordered sequence obtained by following those contained child states through the parent T-list.

This is particularly compatible with the Proteus identity philosophy because no separate temporal-placement object is introduced.

---

## 11.20 Candidate reference syntax for temporal projection

**Status: Syntax intentionally unresolved**

If the existing reference system can naturally lift a child/subitem reference across a T-list, a syntax might conceptually resemble:

```text
%W.<me>
```

or:

```text
%W.&me
```

or another existing reference form.

The exact punctuation should **not** be invented until the current reference grammar is examined.

The required semantics are:

```text
P[span].(child-reference)
```

returns:

> the T-list made from the referenced child/subitem across the corresponding parent temporal span.

Then one could conceptually write:

```text
me.%asTList = projection(%W, me)
```

or, over a known lifetime span:

```text
me.%asTList = projection(%W@[birth:end], me)
```

### Important: projection can also assert containment

Because identity is bidirectional, equating:

```text
me.%asTList
```

with the projection of `me` through `%W` should constrain `%W` to contain the corresponding `me` states even when those states were not previously materialized.

Thus this can be both:

- a way to retrieve an already-rooted child timeline;
- a way to root the child timeline into the parent.

---

## 11.21 Projection must be level-aware

**Status: Required**

A parent World view may be indexed in coarse units such as million-years while the child may change at a much finer resolution.

Therefore temporal projection cannot simply mean:

> take one child state from each direct parent list item.

Instead it must respect the T-level/refinement identities already discussed.

A child state may correspond to:

- part of one parent item;
- one whole parent item;
- several parent items;
- a finer nested T-span inside a parent item.

Thus projection is fundamentally a **reference traversal through aligned T-levels**, not a same-index zip operation.

This is exactly the class of remaining problems expected to live in reference semantics.

---

## 11.22 Galaxy use case under temporal projection

**Status: Conceptual target**

For a galaxy `G`:

```text
G.%asTList
    ≡
projection(G through %W)
```

means that the galaxy's lifetime states are the galaxy sub-infons contained through the World timeline.

To say its beginning lies in the early star-formation epoch, constrain the first local state:

```text
G.%asTList@[0]
```

to be one of the projected `G` states whose containing World position lies inside:

```proteus
%W@[-13400:-13300]
```

The exact reference syntax remains to be designed/tested, but the semantic target is now clearer:

```text
first state of G
    ↕ same child state
G as contained in one World temporal position inside epoch
```

No selection of a million-year T-item as the galaxy lifetime is involved.



## 11.23 One rooted child part establishes coherent alignment of the whole child T-list

**Status: Core T-list semantic rule clarified 2026-09-11**

If even one temporal part/state of a child T-list is contained in a parent T-list, then the child T-list is not merely an unrelated list whose one item happens to occur in the parent.

Instead:

> The child T-list and parent T-list must participate in one coherent temporal ordering.

Conceptually, if:

```text
child C_i is contained in parent P_j
```

then all other temporal states/spans of the same child T-list must be placeable consistently on the same parent temporal line.

This is a semantic consequence of T-containment/rooting.

### Order preservation

If:

```text
C_a precedes C_b
```

in the child T-list, and both are resolved into the parent, then their parent placements must preserve that order:

```text
parent(C_a) precedes-or-meets parent(C_b)
```

They may not reverse order or independently jump to unrelated parent timelines.

Thus a rooted child timeline has a monotone temporal embedding in its parent timeline.

### Continuity / no arbitrary jumps

The correspondence must also be temporally continuous.

If a child span:

```text
C@[a:b]
```

is rooted across a parent temporal region, intermediate child positions must correspond to the intervening parent temporal region according to whatever refinement/level identities are known.

A child timeline cannot disappear from one parent time and reappear at an unrelated parent time while still being treated as one continuously rooted T-list, unless the model explicitly represents a discontinuity at another semantic level.

### What one anchor does and does not determine

One known containment such as:

```text
C_i contained in P_j
```

is enough to establish:

- shared temporal root/context;
- temporal orientation;
- order-preserving coherence between the lists;
- the requirement that further mappings be mutually consistent.

It does **not** by itself necessarily determine:

- exact duration of child states;
- exact parent indices of all child states;
- temporal scale/resolution ratio;
- exact phase at finer levels.

Those remain unknown until constrained by additional identities, durations, metrics, parent/child relations, sibling synchronization, or other levels.

Therefore one anchor establishes a **coherent mapping family**, not necessarily a fully solved mapping.

### Terminology: linear coherence

In these notes, "linearly cohere" should be understood primarily as:

> the child and parent share one ordered temporal line with a continuous, order-preserving correspondence.

It should not automatically mean a mathematical affine map such as:

\[
p = ac+b
\]

with constant scale `a`.

A constant affine relation may follow when the two T-levels have fixed uniform units, but arbitrary T-lists need not have uniform metric spacing.

---

## 11.24 Temporal projection follows from T-containment coherence

**Status: Consequence**

The previously discussed "projection of a child through a parent T-list" need not be a separate primitive if T-list containment already carries this coherence rule.

Once any child state establishes that:

```text
child T-list C
```

is rooted in:

```text
parent T-list P
```

then references may traverse that established coherent relation.

Conceptually:

```text
follow C through P
```

is simply a view/search over the already-required parent/child temporal correspondence.

Thus the engine need not create a new projection object.

It only needs to:

1. know that `C` is temporally rooted in `P`;
2. preserve order/continuity of their T-correspondence;
3. resolve as much of that correspondence as current identities permit.

This strengthens the case that the missing behavior belongs in the semantics of T-list containment plus reference traversal.

---

## 11.25 Parent and child temporal cuts must be mutually compatible

**Status: Derived requirement**

Because rooted T-lists share one coherent temporal line, a resolved temporal boundary in one list constrains where the other list may be cut/viewed.

For example, if a parent boundary falls inside one coarse child state, then the child can be viewed as two temporal subspans on either side of that parent boundary without asserting a child-level state change.

Conversely, a child transition occurring within one coarse parent state refines that parent state's temporal span without necessarily creating a new parent-level state.

Thus:

- **temporal boundaries propagate as possible cuts/views;**
- **state transitions remain level-relative.**

This allows parent and child T-lists to line up without requiring identical state granularity.



## 11.26 Parent T-list vs selected parent T-span

**Status: Clarified 2026-09-11**

Do not confuse the full parent T-list with a selected temporal span of that parent.

For the galaxy example:

```proteus
%W
```

is the parent/root T-list.

An epoch such as:

```proteus
%W@[-13400:-13300]
```

is only a selected T-span/view of `%W`.

Thus, if a galaxy begins during that epoch, the intended fact is:

> The first temporal state/position of the galaxy is contained somewhere in this selected span of `%W`.

This does **not** mean that the galaxy's whole lifetime is contained in the epoch.

Conceptually:

```text
%W timeline:
------------------------------------------------------------

selected epoch:
             [----------------]

galaxy lifetime:
                    [--------------------------------------]

galaxy start:
                    ^
                    |
          lies inside selected epoch
```

The epoch constrains the **location of the start anchor**.

The parent/root remains `%W`.

---

## 11.27 A child may begin in a parent span and extend beyond it

**Status: Core requirement**

If:

```text
G@[0] is contained somewhere in %W@epoch
```

then the rest of `G`'s T-list may continue beyond the end of `epoch`.

Therefore avoid formulations that identify:

```text
G.%asTList
```

with:

```text
%W@epoch
```

or otherwise require the whole child timeline to fit inside the selected span.

The selected span is only a search/constraint region for one or more child temporal positions.

This is essential for statements such as:

- a galaxy begins during an early star-formation epoch;
- a person is born during a decade but lives beyond it;
- a war begins during one year and ends in another;
- a species originates during one geological period and persists into later periods.

---

## 11.28 Rooting may follow from one contained child temporal position

**Status: Working hypothesis**

If a child T-position such as:

```proteus
G@[0]
```

is identified as a child contained in some S-position inside `%W`, then this may be sufficient to establish that `G`'s T-list is rooted in `%W`.

The selected epoch only narrows where that first containment can occur.

Whether any additional explicit "coherence" assertion is required should be determined by the identity/reference semantics and engine behavior rather than stipulated prematurely.

Preferred principle:

> State the necessary identities/containments; derive temporal coherence from T-list semantics if possible.

This avoids adding a separate coherence relation unless concrete tests prove that one is required.


## 12. Sibling Temporal Relations

### 12.1 Siblings may be aligned through shared parent relations

**Status: Conceptual**

Sibling items rooted in the same parent gain some common temporal context.

Example:

```text
       watch
      /     \
   gear1   gear2
```

Their exact phase need not be determined solely from siblinghood.

Additional identities or relations may synchronize them more strongly.

Examples:

- meshing gear teeth,
- common drive shaft,
- synchronized clock edges,
- shared parent-state constraints.

---

## 13. Time Units and Physical Time

### 13.1 T-lists are not inherently measured in time units

**Status: Established conceptually**

A T-list may simply represent a sequence of state transitions.

For example:

```text
G@[0], G@[1], G@[2], ...
```

need not mean seconds, milliseconds, or any other unit.

Time units can later be used to measure or index temporal spans.

---

### 13.2 World can be viewed at different temporal resolutions

**Status: Current model**

At the finest physical interpretation, `%W` may be considered a sparse T-list whose leaves correspond to Planck-scale S-states.

In practice, this is far too detailed for most purposes.

Therefore `%W` is commonly viewed/indexed using coarser units such as:

- seconds,
- years,
- million-years,
- billion-years.

These units should be treated as useful temporal views / metrics rather than as the intrinsic semantic resolution of every T-list.

---

## 14. Referring to an Item Through Time

### 14.1 Required semantic distinctions

**Status: Open syntax; semantic need is clear**

Proteus needs distinct ways to refer to:

1. **An item in general**
2. **The item over its lifetime / complete T-list**
3. **The item at a particular time or state**
4. **A temporal span of the item**
5. **Relative temporal positions**
   - `%prev`
   - `%next`
   - `now`
   - possibly beginning / end

Conceptually:

```text
G              persistent item identity
G@life         G over its lifetime / full T-item
G@i            a particular temporal state
G@[a:b]        a temporal span
```

Exact syntax remains open.

---

## 15. Relative Temporal References

### 15.1 `%tprev` and `%tnext`

**Status: Candidate rule**

`%tprev` and `%tnext` are relative to the current T-level. `%prev` and `%next`
remain structural previous/next-item references in ordered lists.

If the current context is:

```text
G at coarse level, state 7
```

then:

```text
%tprev
```

means the previous coarse-level state, not the previous lower-level microstate.

Changing temporal level changes adjacency.

---

### 15.2 `now`

**Status: Open**

`now` differs from `%prev` and `%next`.

`now` is expected to come from a temporal context/root rather than from intrinsic indexing.

Conceptually:

```text
G@now
```

means:

> Find the state/span of `G` whose rooted temporal extent contains the current context's `now`.

If `G` is insufficiently rooted, the answer may remain unresolved rather than contradictory.

---


## 15.3 Reference traversal is the remaining semantic stress test

**Status: Working conclusion 2026-09-11**

The current temporal model appears sufficient in principle.

If additional T-specific semantics are missing, they are most likely to appear when resolving references across temporal structure rather than in the basic meaning of T-lists themselves.

Important traversal cases include:

1. **Within one T-list**
   - current item;
   - `%prev`;
   - `%next`;
   - arbitrary earlier/later slices.

2. **Upward**
   - child state/span to parent state/span;
   - lower temporal level to higher temporal level;
   - nested T-list to its enclosing T-context.

3. **Downward**
   - parent state/span to child state/span;
   - higher-level state to lower-level temporal refinement;
   - enclosing T-context into a nested T-process.

4. **Sideways**
   - one child T-list to a sibling T-list;
   - one subsystem to another through a shared parent;
   - corresponding sibling states at the "same time."

5. **Across roots**
   - local T-list to `%W`;
   - local T-list to a fictional/simulated world;
   - child root through parent root;
   - detection/prevention of accidental cross-world references.

The current expectation is that sibling references at the same time can be resolved by:

1. moving to a sufficiently shared temporal parent/context;
2. preserving the current temporal position/constraints;
3. moving down into the sibling;
4. using identities and rooting constraints to determine the sibling state or span.

No special "same-time sibling" primitive should be added unless this proves insufficient.

---

## 15.4 Prefer derived inference over additional temporal primitives

**Status: Design principle**

As long as temporal facts are specified in terms of identities, the engine should infer consequences through ordinary consolidation, normalization, and search rather than through a growing set of special temporal inference rules.

Examples include inferring:

- missing S-state values;
- which transition/verb must have occurred;
- possible verbs consistent with two known endpoint states;
- temporal position of a child relative to a parent;
- sibling state at a shared parent time;
- missing intermediate states;
- whether two independently stated temporal descriptions refer to the same occurrence.

For example, if a T-list permits only a constrained set of verbs and two successive states are known, the engine may infer which verb(s) can connect them.

Likewise, if:

```text
state A
...
state B
```

is constrained by a finite transition vocabulary, search may infer one or more compatible missing verb sequences.

This should emerge from substitution of identicals plus search over unresolved alternatives, not from a special-purpose "infer verb" facility.

---

## 15.5 Minimal-semantics policy

**Status: Current policy**

Do not add a new T-specific semantic rule merely because an inference is useful.

First attempt to derive the behavior from:

- ordinary list size semantics;
- ordinary ListSpec inheritance/refinement;
- identity/substitution;
- T-ordering;
- `%prev` / `%next`;
- T-context identity interpretation;
- sequence continuity;
- temporal rooting;
- parent/child and level mappings.

Add a new primitive only if a concrete reference or normalization case cannot be represented coherently using these mechanisms.

This keeps T-lists close to their intended character:

> ordinary lists with a small number of temporal semantic modifications.


## 16. Occurrence Identity vs State-Content Identity

### 16.1 Repeated states must not collapse temporal occurrences

**Status: Important derived requirement**

Cyclic systems may return to the same state content:

```text
switch@7  = ON
switch@12 = ON
```

The two occurrences may have identical state descriptions without being the same temporal occurrence.

Thus:

```text
content(switch@7) ≡ content(switch@12)
```

must **not** imply:

```text
switch@7 ≡ switch@12
```

This matters for:

- oscillators,
- gears,
- wheels,
- clocks,
- cyclic mechanisms,
- repeated digital states.

Temporal occurrence identity must therefore include temporal position/context, not merely S-content.

---

## 17. Beginning and End of T-Items

### 17.1 Beginning / end likely concern entry and exit states

**Status: Open**

To say that an object begins at a certain time should likely constrain the beginning of its lifetime T-list, not merely assert that its first named state exists somewhere in a parent interval.

For example, merely asserting:

```text
galaxy@[0] occurs somewhere in epoch E
```

does not necessarily prove that the **beginning boundary** of that state lies in `E`.

This distinction matters when a single state spans a long interval.

A more exact lifetime/boundary semantics remains to be specified.

---


## 17.2 Boundary-null hypothesis

**Status: Promising; implementation details not yet fully verified**

There is a strong algebraic basis for representing the beginning of an infon or the boundary between adjacent infons with a one-state / null infon.

The canonical finite-state theory defines:

```text
*1+0
```

(or equivalently `*1+_`, since only one value is possible)

as the unique one-state **state**, while explicitly allowing multiple identity-distinct infons to carry that same null state.

That distinction is useful:

> A boundary marker can carry no state information while still having its own identity.

This makes a null infon a natural candidate for representing temporal boundaries.

### Beginning of a T-list

If every infon/T-list has an addressable leading null:

```text
N0  S0  S1  S2 ...
^
start boundary
```

then the beginning of a child lifetime can be referenced without pretending that its first S-state is an instantaneous point.

For a galaxy `G`:

```text
G.start-null
```

could be contained in an S-state of `%W` somewhere inside the formation epoch.

Conceptually:

```text
one World S-state in epoch
    contains
the leading null/boundary of G.%asTList
```

This would say precisely:

> the galaxy begins at this World temporal position.

It would not imply that:

- the whole first galaxy S-state fits inside that epoch position;
- the galaxy ends in the epoch;
- the galaxy lifetime has the same temporal granularity as `%W`.

### Boundaries between adjacent T-items

The same null-boundary idea can represent the cut between:

```text
S_i | S_{i+1}
```

without adding information to either state.

A useful conceptual representation is:

```text
... S_i  N_{i+1}  S_{i+1} ...
           ^
        shared cut
```

Whether Proteus should materialize such boundary infons explicitly or derive them as views remains an implementation question.

### Important identity property

Null-state equality is not enough.

Two boundaries can both have state `(1,0)` while remaining identity-distinct.

Therefore the engine must preserve:

```text
same null state
```

separately from:

```text
same boundary infon
```

This is necessary so that different starts/boundaries do not collapse merely because all null infons have one possible state.

---

## 17.3 Rooting a start via a boundary null

**Status: Candidate solution to the galaxy-start problem**

If the leading null of a T-list is first-class/addressable, then rooting the beginning of a galaxy becomes conceptually simple:

```text
start-null(G.%asTList)
    is contained in
one World S-state inside %W@[epoch]
```

This is preferable to identifying:

```text
G@[0]
```

with a World position because `G@[0]` may itself represent a nonzero temporal span.

The boundary null represents the instantaneous cut immediately before the first S-state.

Thus the desired relation is:

```text
World S-state contains child-start-boundary
```

while ordinary T-list containment/identity can determine how the child's later states relate to the World timeline.

### Repeated galaxies

For repeated galaxies, each galaxy instance has its own identity-distinct start-null:

```text
G0.start-null
G1.start-null
G2.start-null
...
```

Each can independently be contained in some World S-state in the selected epoch.

This gives the desired scope naturally:

```text
for each galaxy G:
    one World S-state in epoch contains G.start-null
```

without forcing the galaxy lifetime to fit inside the epoch.

---

## 17.4 Implementation verification still required

**Status: Open**

Recent Codex work reportedly introduced or normalized a leading `*1+_` at the beginning of infons.

Available evidence supports the underlying algebra:

- `(1,0)` is the null state;
- empty ordered composition folds to the null state;
- identity-distinct infons may share that null state.

However, the currently accessible source subset does **not yet establish** that the new leading `*1+_` is:

- retained as a first-class infon rather than synthesized/erased during normalization;
- addressable through the current reference/index syntax;
- identity-distinct per boundary;
- available at both starts and internal cuts;
- usable as a contained sub-infon in another S-list.

These implementation properties must be checked before relying on the boundary-null syntax.



## 17.5 Prefer boundary views over raw singlet-access notation

**Status: Proposed 2026-09-11**

Although temporal boundaries may ultimately correspond algebraically to the null infon `*1+0`, ordinary Proteus notation should probably **not** expose raw singlet addressing merely to reference those boundaries.

Doing so would couple high-level temporal modeling to the lowest-level infon factorization machinery and could force broad decisions about:

- singlet indexing;
- singlet identity;
- factorization views;
- substitution through singlet references;
- normalization of raw singlet paths.

The temporal problem only requires a stable reference to a **cut in an ordered list**.

A boundary/cut view can provide that without exposing the underlying singlet representation.

---

## 17.6 Empty slices as boundary references

**Status: Promising proposal**

If ordered spans use half-open interval semantics, then an empty span:

```proteus
L@[i:i]
```

naturally denotes the cut at position `i`.

Examples:

```proteus
L@[0:0]
```

is the boundary before the first item.

```proteus
L@[i:i]
```

is the boundary between items `i-1` and `i`.

```proteus
L@[n:n]
```

is the boundary after the last item of a known-size list of length `n`.

For a T-list:

```proteus
G.%asTList@[0:0]
```

would denote the beginning of `G`'s lifetime without selecting `G@[0]` itself.

### Algebraic value

An empty ordered span has no variable state content.

Its pure infon value can therefore be:

```proteus
*1+0
```

(or equivalently `*1+_`).

This provides the desired algebraic connection to the null infon.

### Identity must remain contextual

Although every empty slice has the same one-state value, different cuts must remain identity-distinct as views:

```text
L@[0:0]  !=ident  L@[1:1]
```

even though:

```text
value(L@[0:0]) = *1+0
value(L@[1:1]) = *1+0
```

The view must therefore retain at least:

- source T-list / POV;
- cut position.

It must not be collapsed to a naked global null object during reference traversal.

This matches the distinction between:

- same null state;
- same boundary identity.

---

## 17.7 Optional convenience syntax can be sugar

**Status: Design option**

If empty-slice notation proves semantically correct but verbose, convenience references can be defined as sugar rather than new primitives:

```text
L.begin   := L@[0:0]
L.%cut(i) := L@[i:i]
L.end     := L@[size:size]   // when the end is known
```

For T-lists:

```proteus
G.%asTList.%start
```

would simply be a readable alias for:

```proteus
G.%asTList@[0:0]
```

This avoids introducing a separate temporal-boundary object model.

---

## 17.8 Galaxy rooting with a start cut

**Status: Candidate semantic target**

Using an empty-slice boundary view, the start of a galaxy is:

```proteus
G.%asTList@[0:0]
```

The desired formation assertion becomes:

> This boundary/cut is contained in one World S-context occurring somewhere inside the selected epoch.

Conceptually:

```text
some World S-context within %W@[epoch]
    contains
&G.%asTList@[0:0]
```

This states only the beginning.

It does not require:

- `G@[0]` to fit in one World S-context;
- the lifetime to fit inside the epoch;
- the end to lie in the epoch.

Once this containment roots the T-list in `%W`, ordinary T-list identities/reference traversal can constrain the rest of the child timeline.

---

## 17.9 Why empty-slice notation is preferable to raw singlet notation

**Status: Current recommendation**

Advantages:

1. It uses ordinary ordered-list/span semantics.
2. It works for T-lists without making boundaries a separate temporal primitive.
3. It has a natural algebraic normalization to `*1+0`.
4. It preserves contextual identity through the source list and cut position.
5. It can generalize to beginnings, endings, and internal cuts.
6. It avoids exposing arbitrary singlet/factorization internals to application-level models.
7. `%start` / `%end` can later be added purely as readable aliases.

Raw singlet notation may still be useful in the theory/debug layer, but it is not required for World-rooting syntax if empty slices work.



## 17.10 `.begin` and `.end` as explicit boundary references

**Status: Strong proposal 2026-09-11**

Add explicit ordered-list boundary references:

```proteus
L.begin
L.end
```

These denote the null/cut views immediately before the first item and immediately after the last item of `L`.

They are semantic sugar for empty slices:

```proteus
L.begin  := L@[0:0]
L.end    := L@[size:size]
```

when the end position is known.

For a subspan:

```proteus
L@[a:b].begin
L@[a:b].end
```

the boundaries correspond to the cuts at `a` and `b`.

### Boundary identity across concatenation

This makes sequence composition explicit.

If T-list `B` immediately follows T-list `A`, then:

```proteus
A.end = B.begin
```

means that both sequences share the same temporal cut.

Conceptually:

```text
A:
    A0  A1  A2
              |
              |  A.end = B.begin
              |
B:
              B0  B1  B2
```

The identity is between **boundary infons/views**, not between:

```text
A's last S-state
```

and:

```text
B's first S-state
```

Those neighboring S-states may be different.

This is important for verbs and transitions because the change may occur exactly across the shared cut.

---

## 17.11 Boundary identity gives precise sequence composition semantics

**Status: Proposed core rule**

Rather than saying vaguely:

> the last item of one T-list is identical to the first item of the next,

use:

```proteus
first.end = second.begin
```

This is more precise because:

1. the last S-state of the first list may differ from the first S-state of the second;
2. the shared object is the temporal boundary, not either state;
3. both lists remain independently addressable;
4. nested T-lists can compose without duplicating endpoint states;
5. the same rule applies across abstraction levels.

Thus the sequence rule becomes:

> Two adjacent T-spans are temporally contiguous when the end boundary of the first is identical to the begin boundary of the second.

---

## 17.12 `.begin` / `.end` on spans

**Status: Important consequence**

For any selected T-span:

```proteus
S = L@[a:b]
```

we have:

```proteus
S.begin = L@[a:a]
S.end   = L@[b:b]
```

This allows neighboring spans to share boundaries naturally:

```proteus
L@[a:b].end = L@[b:c].begin
```

without implying that the two spans overlap in S-items.

This is especially useful if span semantics are half-open:

```text
[a,b)
[b,c)
```

The cut at `b` belongs as a boundary to both spans while no S-item is duplicated.

---

## 17.13 Nested T-list insertion can use boundary identities

**Status: Derived design**

If a nested T-list is inserted into a host T-list, temporal continuity can be stated with boundary identity.

Conceptually:

```text
host-before.end = nested.begin
nested.end      = host-after.begin
```

This may make explicit some sequence relations that are otherwise implicit in T-list syntax.

The notation does not require the user to write these identities in ordinary cases; T-list composition semantics may generate them automatically.

However, exposing `.begin` and `.end` gives the language a precise way to:

- state unusual alignments explicitly;
- query them;
- constrain partial spans;
- prove that two temporal descriptions share a cut.

---

## 17.14 Galaxy rooting via `.begin`

**Status: Candidate direct-root formulation**

For a galaxy `G`, its lifetime beginning is:

```proteus
G.%asTList.begin
```

To state that the galaxy begins during an epoch, the goal is:

> some World S-context within the epoch contains `&G.%asTList.begin`.

Conceptually:

```proteus
%W@[start:end] = {T
    ...
    { ... &G.%asTList.begin ... }
    ...
}
```

Exact syntax for selecting/adding to one S-context in the epoch remains to be settled.

The key semantic point is now precise:

- `%W` is the parent T-list;
- `%W@[start:end]` is only the constraining epoch;
- `G.%asTList.begin` is the child boundary being rooted;
- `G.%asTList.end` may occur far outside the epoch.

---

## 17.15 `.begin` / `.end` should remain ordinary identity targets

**Status: Design principle**

Do not give boundary references special procedural behavior.

They should participate in ordinary Proteus identity, containment, consolidation, and search like other infon views.

Examples:

```proteus
A.end = B.begin
```

```proteus
event.begin = process@[3:7].begin
```

```proteus
person.%asTList.begin = birth-event.end
```

If the engine can derive further timing consequences from those identities, it should do so through normal substitution and T-list reference semantics.



## 17.16 `.begin` is more canonical across factor/views than `.end`

**Status: Important correction 2026-09-11**

The apparent symmetry between `.begin` and `.end` breaks when a list/infon is viewed through quotient/factor structure and phase.

At phase 0, quotient/factor views share the same starting state/origin.

Thus `.begin` has a particularly natural cross-view interpretation:

```text
factor-view.begin
quotient-view.begin
whole-view.begin
```

may identify at the shared phase-0 origin when the corresponding factorization/view relation is in phase 0.

By contrast, the local terminal position of one factor/view does **not** generally coincide with the terminal position of the whole product/view.

If the geometrical picture is a factor grid, the lower-right corner is reached only in the final phase.

Therefore:

> `.end` is a local endpoint of a particular ordered view, not automatically a globally shared endpoint across quotient/factor or higher/lower-level views.

---

## 17.17 Do not define `.end` as "the lower-right corner" universally

**Status: Core clarification**

For an ordered list/view `L`:

```proteus
L.end
```

should mean structurally:

> the cut immediately after the last item of **this ordered view**.

It should not be defined algebraically as:

> the lower-right corner of every factorization/product representation of `L`.

The latter is only correct when the relevant view is in the phase that actually terminates at that corner.

This preserves a simple local meaning for `.end` while allowing its identities to other views to depend on phase.

---

## 17.18 Boundary identities across concatenation remain safe

**Status: Retained rule**

The correction above does **not** invalidate:

```proteus
A.end = B.begin
```

when `A` and `B` are explicitly adjacent spans in the same temporal ordering.

That identity follows from sequence composition itself:

```text
A then B
```

shares one cut.

The phase problem arises when trying to infer endpoint identities **across different views/factorizations/levels**, not when two local spans are explicitly concatenated.

Therefore distinguish:

### Local sequence adjacency

```proteus
A.end = B.begin
```

Safe when composition says `B` immediately follows `A`.

### Cross-view endpoint identity

```proteus
coarse.end = fine.end
factor.end = whole.end
quotient.end = whole.end
```

Not generally valid without additional phase/alignment information.

---

## 17.19 Cross-level temporal mapping should not assume shared `.end`

**Status: Correction to earlier notes**

Earlier notes suggested that a coarse state/span and its finer refinement might be characterized by both:

```text
coarse.begin = fine.begin
coarse.end   = fine.end
```

The first identity may be canonical at the appropriate phase/root alignment.

The second needs qualification.

If the lower-level view traverses factor/quotient phases, the endpoint corresponding to the coarse boundary may occur only at a particular phase.

Therefore a cross-level mapping must derive the relevant terminal cut from:

- the factorization/view relation;
- phase;
- span extent;
- any additional identities fixing alignment.

Do not impose endpoint equality merely because one description refines another.

---

## 17.20 Phase-sensitive endpoint mapping

**Status: Open formalization**

A useful conceptual distinction is:

```text
L.end
```

= terminal cut of local view `L`

versus:

```text
map-to-parent(L.end)
```

= the corresponding cut in another view/parent, which may depend on phase.

For a factorized system:

```text
phase 0:
    beginnings align

intermediate phases:
    local factor endpoints map to internal cuts/corners of the whole

last phase:
    the final local endpoint may coincide with the whole lower-right/final endpoint
```

Exact formulas should come from the established quotient/factor phase algebra rather than be stipulated in the T-list layer.

This is another reason boundary references should remain ordinary identity targets: the algebra can determine when two boundary references are actually identical.



## 17.21 `.begin` roots a lifetime; it need not mark the onset of a category

**Status: Clarified 2026-09-11**

For top-level objects such as galaxies, `.begin` is useful for inserting/rooting a lifetime T-list into a parent temporal span.

However, this should not be interpreted too strongly as:

> At this exact instant, the object suddenly became a galaxy.

The temporal beginning of an identity-bearing system and the time at which a semantic description such as `galaxy` becomes accurate are different questions.

A more realistic model may contain one persisting object/system whose descriptions evolve:

```text
diffuse matter / gas structure
    -> proto-galaxy
        -> increasingly galaxy-like structure
            -> mature galaxy
```

The underlying object's identity can persist through these changing descriptions.

Thus:

```proteus
G.%asTList.begin
```

may root the history of the evolving system in `%W`, while later states satisfy progressively stronger structural descriptions.

---

## 17.22 Category membership may emerge gradually from accumulating structure

**Status: Modeling principle**

Proteus should avoid introducing an artificial precise boundary merely because a natural-language category has a convenient name.

For a developing galaxy, structural facts may accumulate gradually:

- gas becomes gravitationally associated;
- stars form;
- a persistent bound structure develops;
- rotation / morphology becomes more stable;
- additional properties satisfying a richer galaxy model appear.

As those identities become available, the system can match a galaxy description increasingly well or satisfy progressively more specific models.

If the existing term becomes misleading at earlier stages, define an intermediate concept such as:

```proteus
#proto-galaxy = { ... }
```

rather than forcing a sharp `galaxy.begin` event into reality.

This keeps ontology tied to modeled information rather than linguistic convenience.

---

## 17.23 Object lifetime and classification spans should remain distinct

**Status: Important requirement**

Do not automatically equate:

```text
begin(object lifetime)
```

with:

```text
begin(object satisfies type X)
```

For one persisting system `G`, we may have:

```text
G lifetime:
    [----------------------------------------------------]

proto-galaxy description:
          [----------------------]

galaxy description:
                         [-------------------------------]
```

The same identity-bearing object can satisfy different descriptions over overlapping or successive spans.

This is especially important for naturally emergent objects whose category boundaries are conventional or resolution-dependent.

---

## 17.24 Initial World bootstrap may use coarse beginnings without freezing ontology

**Status: Practical implementation guidance**

The early knowledge base may initially assert something coarse such as:

> many top-level galactic systems have beginnings somewhere in the early star-formation epoch.

This is acceptable as bootstrap knowledge.

Later, richer models can replace/refine that assertion by:

- extending the rooted object's history earlier;
- introducing proto-galaxy states;
- adding lower-level physical structure;
- deriving when stronger galaxy descriptions become satisfied;
- leaving category boundaries partially unknown where the evidence/model does not justify precision.

Therefore the early `.begin` assertion should be treated as refinable knowledge, not a permanent metaphysical commitment to an instantaneous creation event.


## 18. Sparse World Knowledge

### 18.1 Lack of a fact must not imply absence

**Status: Required**

In a sparse World knowledge base:

```text
G exists during span S
```

must not imply that `G` does not exist outside `S` unless a beginning or ending constraint has actually been asserted.

The World model should support partial temporal knowledge without closed-world assumptions.

---


## 18.3 Ellipsis means unspecified members under the containing list constraints

**Status: Clarified / corrected 2026-09-11**

`...` does **not** by itself specify either:

- the type/form of the omitted items, or
- how many omitted items there are.

Those facts come from the containing list's:

1. **size**, and
2. **ListSpec**.

The ellipsis means, approximately:

> The members required here by the containing list exist, but their explicit descriptions are omitted/unknown.

### Fixed-size example

Suppose the containing list is constrained to strings:

```proteus
{$ |
    'hi'
    &*4+{ ... }
    'greetings'
}
```

The inserted sublist has size 4.

Because its members occur in the outer list whose ListSpec is `$`, the four unspecified members are strings.

Conceptually:

```text
'hi'
<string?>
<string?>
<string?>
<string?>
'greetings'
```

The `...` inside the fixed-size `*4+{...}` does **not** mean an indefinite number of members. The size `*4+` determines that there are exactly four.

---

## 18.4 Nested lists can further constrain inherited members

**Status: Established by example**

An inserted list may add constraints beyond those inherited from the surrounding ListSpec.

For example:

```proteus
&*4+{
    *3+$ |
    ...
}
```

means:

> Four unspecified items, each satisfying the inner ListSpec `*3+$`.

If `$` denotes a string and `*3+$` denotes a three-character string, then this represents four missing strings, each exactly three characters long.

Thus constraints can compose hierarchically:

```text
outer ListSpec:
    item is a string

inner inserted list:
    exactly 4 items

inner ListSpec:
    each item is a 3-character string
```

The inner ListSpec **narrows** the possible members; it need not replace all information inherited from the outer context.

This is an important general rule:

> A nested/inserted list inherits applicable constraints from its containing list and may add further constraints of its own.

---

## 18.5 Cardinality comes from list structure, not from `...`

**Status: Corrected rule**

Examples:

```proteus
*4+{ X | ... }
```

means exactly four members satisfying `X`.

```proteus
*_+{ X | ... }
```

means an unknown number of members satisfying `X`.

```proteus
*(4+_)+{ X | ... }
```

means at least four members satisfying `X`, assuming the usual size semantics.

Therefore the same textual ellipsis:

```proteus
...
```

can denote very different amounts of omitted material depending on the containing infon's size.

This applies to both S-lists and T-lists.

---

## 18.6 Relation between `_` and `...`

**Status: Clarified**

`_` and `...` represent different kinds of under-specification.

### `_`

```proteus
_
```

is one unknown infon/value at a position where one item is being described.

### `...`

```proteus
...
```

leaves the members of the surrounding list unspecified, with their number and structure determined by the surrounding size/ListSpec constraints.

Thus:

```proteus
*2+{ _ | ... }
```

contains exactly two unknown items because the size is 2.

And:

```proteus
*_+{ _ | ... }
```

contains an unknown number of unknown items because the size is unknown.

The ellipsis is therefore best understood as **omitted member specification**, not as an independent variable-length operator.

---

## 18.7 Temporal ellipsis can be refined by inserting a constrained T-sublist

**Status: Established / clarified**

In a T-list, an unknown temporal region may be refined by inserting an explicit T-sublist.

For example, the previously discussed equivalence:

```proteus
...
```

can be refined as:

```proteus
&*2+{T
    _
| ...}
```

when the surrounding temporal structure permits the insertion.

The important point is now more precise:

- the inserted sublist contributes **exactly two** temporal positions because its size is `*2+`;
- the two members are unknown because the ListSpec is `_`;
- the inner `...` merely leaves those two members unspecified;
- the enclosing temporal remainder may still remain unspecified outside the inserted span.

So the useful principle is not that ellipsis is intrinsically recursive, but:

> An unspecified region of a list may be refined by identifying part of it with a more constrained sublist, without requiring the rest of the unspecified region to be explicitly materialized.

This is exactly the mechanism needed for gradually adding information to a sparse T-list.

---

## 18.8 Constraint propagation through omitted members

**Status: Core principle**

When omitted members are later exposed or constrained, they inherit all applicable ListSpecs and surrounding identities.

For example:

```proteus
{T
    *10+_ = *_+[%prev+1 %prev-1]
|
    3
    &*2+{T _ | ...}
    7
}
```

the inserted two-item T-sublist contributes exactly two unknown states between the surrounding states.

Those unknown states are still subject to the enclosing T-list's transition relation:

```text
current = %prev + 1
or
current = %prev - 1
```

Thus the normalizer may solve their values from the endpoints and the inherited transition constraints.

This provides a general sparse-modeling mechanism:

1. leave members omitted;
2. identify a region with a sublist of known size;
3. optionally give that sublist a more restrictive ListSpec;
4. allow inherited and local constraints to determine the omitted members.


## 19. Repeated Quantification

### 19.1 Independent repeated instances require independent temporal placement

**Status: Open but central**

For assertions such as:

> Many galaxies began during the early star-formation epoch.

The intended structure is approximately:

\[
\forall G_i \in Population,\quad \exists t_i \in Epoch
\]

such that each galaxy's beginning is rooted at its own independently constrained temporal location.

This must not accidentally become:

\[
\exists t \in Epoch,\quad \forall G_i
\]

which would place all galaxy beginnings at the same unknown time.

Each repetition should therefore introduce:

1. a distinct galaxy instance unless later identities unify instances;
2. an independently unknown temporal placement unless later identities unify placements.

---

### 19.2 World-centric and child-centric formulations should normalize to the same structure

**Status: Design goal**

There should ideally be two natural ways to state the same fact.

#### World / epoch perspective

> During this epoch, many galaxy beginnings occur.

#### Galaxy perspective

> There are many galaxies, each of which begins somewhere during this epoch.

These should be two views on the same underlying identity structure, not two unrelated mechanisms.

---


## 11.7 Minimal rooting pattern: anchor one child T-position in a parent T-context

**Status: Candidate semantic formulation to test in the engine**

A child T-list does not need to be fully synchronized with a parent T-list in order to be rooted.

The minimal root can be a single shared temporal position.

Assuming zero-based T indexing, conceptually:

```proteus
%W@t = {
    ...
    &G@[0]
    ...
}
```

or, when the exact World position is only known to lie somewhere in an epoch:

```proteus
%W@epoch = {T
    ...
    &G@[0]
    ...
}
```

The intended meaning is:

> The temporal position occupied by `G@[0]` is identified with some temporal position in the selected World T-context.

This supplies a **temporal anchor** for `G`.

### What one anchor does not imply

A single anchor does not necessarily determine:

- the duration of `G@[0]`;
- the scale of every subsequent `G` state;
- the relative phase of all later child transitions;
- a one-to-one mapping between child T-indices and parent T-indices.

Those facts may be inferred from:

- another temporal anchor;
- known duration/metric relations;
- parent/child structure;
- sibling relations;
- lower/higher-level mappings;
- physical or logical constraints.

Thus rooting and full synchronization are distinct.

---

## 11.8 Rooting a beginning vs merely observing a state

**Status: Candidate rule**

If `G@[0]` is the first S-item in `G`'s lifetime T-list, then anchoring that T-position into a parent T-context can represent the beginning of `G`, provided T-position identity aligns the corresponding entry position.

This is stronger than merely saying that the content of `G@[0]` is observed during a parent span.

The intended temporal assertion is:

```text
child temporal position 0
    ≡
some parent temporal position
```

rather than merely:

```text
state-content(G@[0]) is present somewhere in the parent
```

This distinction should be tested explicitly in the reference/normalization semantics.

---

## 19.4 World-centric repeated galaxy beginnings

**Status: Revised candidate formulation**

A tempting formulation is:

```proteus
%W@epoch = {T
    ...
    &*_+{T
        galaxy@[0]
    | ...}
    ...
}
```

However, ordinary list semantics make this suspicious.

An inserted T-sublist normally constrains a **contiguous temporal subspan** of the enclosing T-list. Therefore the expression above risks saying that successive positions in one subspan are galaxy beginnings, rather than saying that an unknown number of independently placed galaxy beginnings occur somewhere throughout the epoch.

That is stronger than intended.

### Better structural target: repeat the whole placement assertion

For one galaxy `G`, the desired World-centric assertion is:

```proteus
%W@epoch = {T
    ...
    &G@[0]
    ...
}
```

This means:

> There is some temporal position in this epoch at which `G@[0]` is rooted.

For many galaxies, what must be repeated is **that complete assertion**, including its independent ellipses / placement freedom.

Conceptually:

```text
repeat unknown-many times:
    %W@epoch = {T ... &G_i@[0] ...}
```

Each repetition gets:

- a fresh galaxy instance `G_i`;
- its own independently unresolved placement inside the epoch.

This directly represents the desired scope:

\[
\forall G_i,\ \exists t_i \in Epoch
\]

rather than accidentally creating one contiguous sequence of formation events.

### Candidate World-overlay shape

Proteus should ideally permit an unordered/repeated collection of T-constraints overlaying the same selected epoch, conceptually similar to:

```proteus
%W@epoch = {
    &*_+{
        {T ... &galaxy@[0] ...}
    | ...}
}
```

or equivalently some cleaner syntax in which the repeated item's ListSpec is:

```proteus
{T ... &galaxy@[0] ...}
```

The exact surface syntax needs parser/reference testing.

The important semantic requirement is:

> Each repetition is a separate T-description of the same epoch, not the next item of one shared T-description.

This is analogous to applying multiple independent constraints to one infon.

---

## 19.5 Galaxy-centric formulation

**Status: Design goal; exact reference syntax still to be chosen**

From the galaxy/population point of view, the same fact is:

> There is an unknown-sized population of galaxies, and for each galaxy its first T-position is rooted somewhere in `%W@epoch`.

Conceptually:

```text
*_+{
    G:galaxy
    with constraint:
        %W@epoch = {T ... &G@[0] ...}
| ...}
```

The placement assertion is inside the scope of the repeated galaxy, so every galaxy receives an independent unresolved World position.

A reusable relation can express this without adding new T semantics:

```proteus
#begins-in child parent-span = {
    parent-span = {T
        ...
        &child@[0]
        ...
    }
}
```

Then the population description becomes conceptually:

```text
*_+{
    galaxy constrained by begins-in(%W@epoch)
| ...}
```

The exact way to reference the current repeated galaxy instance is a reference-language issue.

---

## 19.6 World-centric and galaxy-centric forms must normalize to the same identities

**Status: Core design requirement**

The two viewpoints should not create different facts.

For repeated galaxies `G_i`, both forms should normalize to a set of constraints equivalent to:

```text
%W@epoch = {T ... &G_0@[0] ...}
%W@epoch = {T ... &G_1@[0] ...}
%W@epoch = {T ... &G_2@[0] ...}
...
```

where every ellipsis placement is independently unresolved unless additional identities relate them.

This gives the desired logical form:

\[
\forall G_i \in Population,\ \exists t_i \in Epoch:
    root(G_i@[0], W[t_i])
\]

### Same-time formation remains possible

Independent placement does **not** imply distinct times.

Two galaxies may later be inferred or explicitly asserted to have the same temporal anchor.

What must be avoided is accidentally forcing all repeated galaxies to share one unknown anchor.

### Ordering is inferred, not asserted by quantification

The population assertion does not need to pre-order galaxy formation events.

Once individual roots become known, their temporal ordering follows from their positions in the parent T-context.

This is preferable to encoding the population itself as one T-list of formation events, which would introduce ordering/contiguity structure that the original knowledge claim does not contain.


## 19.7 Immediate tests for the galaxy/rooting semantics

**Status: Proposed tests**

Before extending syntax further, test the following progressively:

1. **One exact anchor**
   ```text
   root G@[0] at W@t
   ```

2. **One uncertain anchor in a span**
   ```text
   root G@[0] somewhere in W@[a:b]
   ```

3. **Two children with independent anchors**
   ```text
   G1@[0] somewhere in epoch
   G2@[0] somewhere in epoch
   ```
   Confirm that no identity forces the same temporal position.

4. **Two simultaneous children**
   Explicitly constrain:
   ```text
   G1@[0] and G2@[0] at the same World position
   ```
   Confirm that simultaneity is representable without merging the galaxy identities.

5. **Repeated World-centric overlay**
   Repeat independent constraints of the form:
   ```proteus
   %W@epoch = {T ... &G_i@[0] ...}
   ```
   Verify that each repetition has an independently unresolved placement.

6. **Galaxy-centric equivalent**
   Repeat galaxy instances, each constrained by the same `begins-in` relation.

7. **Round-trip inference**
   - From World view, find galaxies beginning in epoch.
   - From a galaxy, find the World epoch/time containing its beginning.
   - From both endpoints plus transition constraints, infer missing temporal details where possible.

These tests should reveal whether the remaining difficulty is:

- reference syntax;
- current search/normalization implementation;
- or genuinely missing T semantics.



## 19.8 Repeated galaxy rooting can be expressed with a ListSpec

**Status: Promising formulation 2026-09-11**

Repeated quantification does not require a special quantifier if the repeated items are ordinary constraint descriptions generated from a ListSpec.

The essential repeated pattern is:

```proteus
{ ... &G.%asTList.begin ... }
```

meaning:

> an S-description containing the beginning boundary of one galaxy lifetime.

The important scoping requirement is that each repetition instantiate its own galaxy `G` and its own temporal placement.

### Do not make this the ListSpec of `%W` itself

If:

```proteus
{ ... &G.%asTList.begin ... }
```

were used directly as the ListSpec of the selected World T-list, it would mean that **every World T-item/state** in the epoch satisfies that description.

That is too strong.

The intended claim is only:

> an unknown number of independent World-state constraints, at independently unresolved positions in the epoch, each contain one galaxy beginning.

Therefore the repeated list should be a collection of **independent overlays/descriptions of the epoch**, not the ordinary sequence of World T-items.

### Semantic target

Conceptually:

```text
repeat unknown-many times:
    choose fresh galaxy G
    choose some World S-state in epoch
    constrain that S-state by:
        { ... &G.%asTList.begin ... }
```

Equivalently, each repetition can be regarded as a T-description of the same epoch:

```proteus
%W@epoch = {T
    ...
    { ... &G.%asTList.begin ... }
    ...
}
```

and the repeated ListSpec generates an unknown number of such independent constraints.

The exact compact syntax remains a parser/reference question.

### Freshness

For the ListSpec approach to encode the desired quantifier scope, each instantiation must provide:

- a fresh galaxy identity `G_i`;
- an independent unresolved World position for `G_i.%asTList.begin`.

Later identities may equate two positions, allowing simultaneous beginnings, but repetition must not force that equality.

This yields:

\[
\forall G_i,\ \exists t_i \in epoch
\]

without a separate first-order quantifier mechanism.


## 20. Current Galaxy / World Example

Current kernel definitions include approximately:

```proteus
#star = {T ...}
#planet = {T ...}
#solar-system = {T star, planets, ...}
#galaxy = *(100000000000000+_)+{T solar-system| ...}

#world = *(20000+_) +{T million-year | ...}
```

These are early stubs and should not yet be treated as final ontology.

Current experimental statement:

```proteus
%W@[-13400 : -13300] = {
    T
    ...
    *_+{galaxy@[1] | ...}
    ...
}
```

The intended meaning is roughly:

> Many galaxies begin at independently unknown positions within this World epoch.

The exact formulation remains unresolved because it depends on:

- how child T-lists root into parent T-lists;
- how beginnings are represented;
- how repeated instances receive independent temporal positions;
- how a World-centric formulation maps to a galaxy-centric formulation.

---

## 21. Provisional Core Rules

The current temporal semantics can be summarized compactly as:

```text
S identity:
    LHS[i] ≡ RHS[i]

T identity:
    LHS[i+1] ≡ RHS[i]

Persistence:
    unchanged[i+1] ≡ unchanged[i]

Simultaneous transition:
    all RHS read S[i]
    all LHS constrain S[i+1]

Sequential composition:
    exit(Ta) ≡ entry(Tb)

Temporal refinement:
    coarse entry aligns with fine entry
    coarse exit aligns with fine exit

T-list specialization:
    ordinary list structure + temporal interpretation
    %prev/%next recover much local behavior
    T-context additionally carries implicit temporal continuity and rootability

Temporal root:
    determines which world/model supplies the temporal coordinate context
    no cross-world synchronization without explicit/inherited identity

Hierarchical rooting:
    prefer the nearest already-rooted containing system
    direct `%W` roots are mainly for top-level objects
    root/containment relations propagate transitively upward

T-containment coherence:
    one rooted child temporal part establishes a shared parent temporal line
    the whole child T-list must map continuously and order-preservingly into the parent
    one anchor need not determine exact scale, duration, or fine phase



Parent/child rooting:
    constrains temporal correspondence without equating parent and child

Boundary references:
    L.begin and L.end denote identity-distinct null/cut views
    adjacent T-spans compose by first.end = second.begin
    boundary identity does not imply adjacent S-state identity

Ellipsis / omitted members:
    ... leaves list members unspecified
    size determines how many members exist
    ListSpec determines what those members may be
    nested &lists may expose/refine a subspan with additional constraints

Repeated temporal quantification:
    repeated items should receive independent temporal placements
```

These should be treated as a working semantic skeleton, not a final formal specification.

---

# Open Questions

The following should be resolved next.


## Q0. State-item vs transition-item classification

Formalize how the engine distinguishes or infers:

- an S-description of a state;
- a T-description of a transition;
- a named transition / verb;
- a nested T-process;
- a ListSpec that constrains admissible transitions.

Determine whether this distinction is encoded directly in infon structure or emerges entirely from T-context and identity resolution.


## Q0.5. First-position semantics under `%prev`-dependent ListSpecs

When a T-list ListSpec contains `%prev`, define what constrains the first explicit S-description.

Possibilities include:

- the first state is an initializer and is exempt from the `%prev` relation;
- an implicit entry state exists before the first explicit position;
- the enclosing/rooting context supplies `%prev`;
- the first state remains underconstrained until an entry context is known.

This must be explicit because examples such as:

```proteus
{T *10+_ = *_+[%prev+1 %prev-1] | 3 4 5}
```

require a precise interpretation of whether `3` is an initial state or itself the result of a transition.


## Q1. Exact semantics of T-list entry and exit

Clarify the semantics of:

- an explicit first S-state used to initialize a system;
- a T-description/verb used as the first sequence element;
- where `%prev` comes from when no explicit first S-state is written;
- a first transition identity;
- `...` at the beginning or end of a T-list;
- nested `&{T ...}`;
- injection of one T-list into another;
- whether entry/exit states are explicit infons or implicit views.

## Q2. Exact boundary semantics

Determine whether Proteus needs explicit temporal-boundary infons, or whether boundaries are entirely implicit in adjacent S-contexts and their identities.

Current preference: keep boundaries implicit unless explicit objects prove necessary.


## Q2.5. Representation of temporal root / world context

Determine whether temporal root is represented as:

- an ordinary identity path through parent T-lists;
- an explicit field/property of a T-view;
- a POV relationship;
- an inferred relation with no dedicated stored field.

Requirements:

- local `%prev/%next` must work before external rooting;
- multiple worlds must remain independent;
- rooting into `%W`, a fictional world, a simulation, or a belief-model must use the same mechanism;
- root constraints should propagate transitively without inventing precision;
- changing/viewing temporal level must not silently change world root.



## Q2.75. Containment/root reference syntax

The semantic direction is now clearer: rooting means containment in the parent/world T-structure.

Test exact syntax for:

```text
item.%asTList = one contained T-list of %W
%W contains &item.%asTList
%W@epoch contains &item.%asTList@[0]
```

Questions:

- What is the cleanest existing expression for "one child T-list contained in this parent T-span"?
- Does `*_+[&%W@[start:end]]` already have exactly that meaning?
- When a child T-list crosses the boundary of a selected parent slice, is it considered "in" that slice, or must it be fully contained?
- Can a first child T-item be located in an epoch without constraining the whole child lifetime to that epoch?
- Can reverse identity search enumerate all rooted child T-lists or first states contained in a selected World span?



## Q2.9. Hierarchical-root traversal tests

Test:

- galaxy -> `%W`;
- solar-system -> galaxy -> `%W`;
- planet -> solar-system -> galaxy -> `%W`;
- child span temporarily contained in a narrower parent span;
- a child whose full lifetime exceeds the narrower parent's containment span;
- reverse World queries that find indirectly rooted descendants;
- sibling synchronization through the nearest common rooted ancestor.

Verify that no redundant direct `%W` identity is required when a complete containment/root chain already exists.



## Q2.95. Temporal-projection reference tests

Determine whether the existing Proteus reference language can express:

> Follow this child/subitem through a parent T-list and return the resulting child T-list.

Test:

- fixed child reference through a parent T-list;
- sparse child existence;
- child beginning/end inside the parent;
- child state spanning several parent units;
- child changes finer than the current parent unit;
- traversal through nested parent time units;
- reverse identity: equating a child T-list with a projection causes the parent states to contain the child;
- sibling projection through a shared parent.

If existing references cannot express this cleanly, add the smallest possible reference semantic rather than another T-list primitive.



## Q2.97. Linear-coherence tests

Test that one child/parent temporal anchor establishes coherent alignment constraints for the entire child T-list.

Cases:

- one exact child state anchored to one parent state;
- later child state constrained to an earlier parent position: must contradict;
- earlier child state constrained to a later parent position: must contradict;
- unknown intermediate child states: remain unresolved but confined to the intervening parent span;
- child transition inside one parent S-state: valid refinement;
- parent boundary inside one child S-state: valid temporal cut without forcing a child state change;
- two anchors that imply inconsistent temporal order: contradiction;
- two anchors that determine scale/phase: infer intervening placements;
- one anchor only: preserve coherence without inventing exact scale.


## Q3. Parent/child T-list alignment

Define exactly how:

```text
parent T-span
```

relates to:

```text
child T-span
```

and what identities necessarily follow between their S-level views.

## Q4. Higher/lower-level temporal mapping

Formalize:

> A higher-level state corresponds to a lower-level span.

Determine how identities map:

- high-level state entry,
- high-level state exit,
- lower-level intermediate states,
- repeated identical high-level states.

## Q5. Temporal cuts without state changes

Decide how a T-list records a temporal partition where the S-content is unchanged across the partition.

This is required for:

- parent synchronization,
- clocks,
- metric boundaries,
- slicing,
- external event alignment.

## Q6. Lifetime semantics

Define exact references for:

- item in general,
- complete lifetime,
- beginning,
- end,
- open-ended lifetime,
- item existing during a span.

## Q7. `%prev`, `%next`, and `now`

Define:

- level-relative adjacency,
- context inheritance,
- unresolved cases,
- behavior inside nested T-lists.


## Q7.5. Ellipsis and partially exposed spans

The basic rule is now clearer:

- `...` does not itself determine cardinality;
- enclosing size determines cardinality;
- enclosing ListSpec determines member constraints;
- inserted/nested lists may make a previously unspecified region more explicit.

Remaining questions:

- exactly how an `&` sublist is matched against an unspecified region;
- whether its placement is existentially searched when not otherwise anchored;
- how overlapping refinements of the same omitted region consolidate;
- how fixed-size and unknown-size parent lists constrain inserted sublists.



## Q7.75. Reference-traversal test matrix

Before adding further T semantics, construct tests for references that traverse:

- `%prev` / `%next` inside a T-list;
- nested T-list entry/exit;
- child -> parent;
- parent -> child;
- higher level -> lower level;
- lower level -> higher level;
- sibling -> sibling at the same temporal position;
- sibling -> sibling with incomplete phase information;
- local timeline -> `%W`;
- local timeline -> fictional/simulated world;
- root inheritance through multiple parent levels;
- attempts to cross unrelated temporal roots.

For each case, record whether the expected result follows solely from identities and existing T semantics.

Any failure should first be classified as:

1. missing identity/model information;
2. insufficient search/normalization implementation;
3. genuinely missing T semantics.

Only category 3 should motivate a new temporal rule.



## Q7.9. Empty-slice boundary tests

Before adding raw singlet-access notation, test whether ordered empty slices can serve as first-class boundary views.

Required cases:

```proteus
L@[0:0]
L@[1:1]
L@[n:n]
```

Verify:

- their pure state normalizes to `*1+0`;
- different cuts remain identity-distinct;
- the view retains source-list and cut-position provenance;
- an empty slice can participate in containment identities;
- a T-list start cut can be contained in a parent/world S-context;
- concatenation/subspan operations preserve the expected cut identities;
- negative indexing does not create ambiguous boundary aliases;
- the end cut works for known-size lists and remains representable when size is only partially known.

Only if these fail for principled reasons should raw singlet addressing be considered for the temporal API.



## Q7.95. `.begin` / `.end` tests

Test:

```proteus
L.begin
L.end
L@[a:b].begin
L@[a:b].end
```

Verify:

- `L.begin` corresponds to the cut before the first item;
- `L.end` corresponds to the cut after the last item;
- adjacent spans satisfy:
  ```proteus
  L@[a:b].end = L@[b:c].begin
  ```
- boundary identity does not force neighboring S-state identity;
- nested T-list composition can share boundaries without duplicating states;
- different list boundaries may have the same null state while remaining identity-distinct;
- `.end` remains meaningful when list size is partially known, or is explicitly unresolved if not.



## Q7.97. Phase-sensitive `.end` tests

Test `.begin` / `.end` under quotient/factor views:

- phase-0 factor and quotient beginnings align as expected;
- local factor `.end` in phase 0 maps to the correct internal parent cut, not automatically parent `.end`;
- intermediate phases map local endpoints to the correct parent cuts/corners;
- only the appropriate final phase maps the relevant local endpoint to the whole final/lower-right boundary;
- explicit concatenation `A.end = B.begin` remains valid independent of unrelated factor phase;
- cross-level refinements do not acquire endpoint identity unless phase/extent constraints justify it.


## Q8. Repeated quantification

Formalize how repetition introduces fresh:

- object identities,
- temporal placement variables,
- roots / phase constraints.

Then prove or test equivalence of World-centric and child-centric formulations.

---

# Source Notes

## Historical Proteus rule recovered

From *The World as Information: Part 7*:

- In a T-list, the RHS of an identity refers to the previous value.
- This makes identity assertions behave like declarative assignments.
- Unordered sets of changes act simultaneously.
- Ordered T-lists represent sequential actions.
- Repeated T-list ListSpecs represent repeated temporal changes.
- Nested lower-level transitions may begin from the prior state, contain intermediate states, and terminate in a final state.

These prior statements should be treated as stronger evidence than later speculative reconstruction.

---

# Revision Log

## 2026-09-11

### Later 2026-09-11 clarification

Added:
- repeated galaxy starts can be generated by a ListSpec whose repeated constraint contains `&G.%asTList.begin`;
- warning: making that description the ListSpec of `%W` itself would incorrectly require every World state in the epoch to contain a galaxy beginning;
- the repeated collection should represent independent overlays/constraints on the same epoch, with fresh galaxy identity and fresh temporal placement per instantiation;
- distinguished lifetime `.begin` from the onset of a semantic category such as `galaxy`;
- top-level rooting may use a coarse beginning while later ontology extends the same object's history into proto-galaxy states;
- natural categories can emerge gradually as more structural identities satisfy richer descriptions;
- bootstrap World assertions should remain refinable rather than freezing an artificial instantaneous category boundary;
- corrected `.end` semantics for quotient/factor phase: `.end` is local to a view and is not automatically the whole lower-right/final boundary;
- `.begin` retains privileged phase-0 alignment across corresponding factor/quotient views;
- `A.end = B.begin` remains valid for explicit local sequence adjacency;
- cross-view/cross-level endpoint identities must be derived from phase and extent rather than assumed;
- `.begin` / `.end` as explicit boundary references, equivalent to empty-slice cuts;
- exact sequence composition rule: `A.end = B.begin`;
- clarified that boundary identity is not identity of adjacent S-states;
- neighboring half-open T-spans can share a cut without overlapping items;
- galaxy lifetime start can be referenced directly as `G.%asTList.begin`;
- recommended boundary views instead of exposing raw singlet addressing at the application language level;
- proposed `L@[i:i]` as an empty-slice/cut reference whose pure value is `*1+0` but whose identity retains source-list and cut position;
- optional `%start`, `%cut(i)`, `%end` syntax can be sugar for empty slices;
- galaxy beginning can be represented by containing `G.%asTList@[0:0]` in a World S-context inside the epoch;
- boundary-null hypothesis: identity-distinct `*1+_`/null infons can represent temporal starts/cuts without adding state information;
- candidate rooting rule: contain the child's leading null in a World S-state inside the desired epoch;
- explicit warning that current accessible code has not yet verified the recent Codex leading-null implementation is first-class/addressable;
- distinguished the full parent T-list (`%W`) from a selected parent T-span/epoch;
- an epoch constrains where a child start lies but need not contain the child lifetime or end;
- softened explicit coherence requirements: prefer deriving coherence from T-list identities/reference semantics rather than adding a separate coherence relation;
- core T-containment rule: one rooted child temporal part establishes a coherent shared temporal line for the entire child T-list;
- temporal correspondence is continuous and order-preserving, while exact scale/phase may remain unknown;
- temporal projection becomes a derived reference view rather than a separate primitive;
- parent and child temporal cuts must be mutually compatible without forcing state transitions across levels;
- identified flaw in `*_+[&%W@[start:end]]`: it selects World-level `million-year` items, not arbitrary child lifetime T-lists;
- proposed temporal projection: follow one contained child through a parent T-list to obtain the child's T-list;
- projection preserves literal parent containment and strict identity because it returns child states, not parent states;
- projection must be level-aware rather than a same-index mapping;
- hierarchical rooting preference: root most items into the nearest natural already-rooted parent rather than directly into `%W`;
- galaxies as natural early direct `%W` roots, while allowing later higher cosmic containers;
- transitive propagation of root/temporal context through containment chains;
- requirement that a lifetime root parent be broad enough to contain the relevant child span;
- World-level queries should traverse indirect root chains rather than require direct `%W` children;
- correction: rooting is literal parent/world containment, not merely a temporal placement projection;
- child lifetime T-list may be identical to one child T-list contained in `%W`, preserving strict identity because the RHS selects the child rather than the World span;
- whole-life rooting and first-state-in-epoch are separate constraints;
- removed the tentative `%root` abstraction as unnecessary unless reference traversal later proves it useful;
- revised rooting model: do not identify an object's lifetime T-list with World-content spans;
- proposed `%root` as a reference/view from a local T-position/span to its placement in the root temporal context;
- beginning anchor candidate: `me.%asTList@[0].%root = oneOf(%W@[start:end])`;
- no need for a universal/global T-list; the same root-reference mechanism supports `%W`, fictional worlds, simulations, and other temporal contexts;
- corrected repeated-galaxy formulation: repeating a contiguous T-sublist is too strong; repeat the entire independently placed T-constraint instead;
- World-centric repeated quantification should overlay multiple independent T-descriptions of the same epoch;
- temporal ordering of galaxy beginnings should be inferred from resolved roots rather than asserted by the population quantifier;
- minimal temporal rooting as a single child-to-parent T-position anchor rather than automatic full synchronization;
- distinction between rooting a first T-position and merely observing its S-content;
- candidate World-centric galaxy syntax using an injected repeated T-sublist;
- galaxy-centric `begins-in` relation as a reference-language view of the same identity;
- explicit quantifier-scope requirement: each repeated galaxy gets an independently constrained temporal anchor;
- concrete test sequence for exact, uncertain, simultaneous, and repeated roots;
- reference traversal as the remaining stress test for missing temporal semantics;
- same-time sibling lookup should derive through shared temporal context/rooting where possible;
- missing verbs and intermediate transitions should be inferred by ordinary identity/search rather than special temporal inference machinery;
- minimal-semantics policy: add T primitives only after ruling out missing model identities and implementation/search limitations;
- T-lists as specialized ordinary lists rather than wholly separate structures;
- `%prev/%next` as sufficient for much local temporal behavior but not for clean rooting semantics;
- temporal root/world context as a distinct semantic requirement;
- support for independent worlds/models such as `%W`, simulations, beliefs, and fictional universes;
- no automatic temporal synchronization across different roots;
- `now` as root-relative rather than globally absolute;
- corrected ellipsis semantics: `...` omits member descriptions; containing size supplies cardinality and ListSpec supplies member constraints;
- fixed-size `&` sublists can expose/refine a known number of previously unspecified members;
- nested ListSpecs may further constrain inherited item types (e.g. four strings, each three characters);
- state-to-state ListSpec constraints using `%prev`;
- S-descriptions as occupants of temporal positions;
- transition relations as constraints between adjacent positions;
- deduction of missing temporal states from endpoint and transition constraints;
- compatibility goal between explicit verbs and implicit ListSpec transition relations;

- state descriptions vs transition descriptions / verbs;
- `%prev` as the input context of transition identities;
- T-descriptions as possible first sequence elements when an entry context is available or unknown;
- repeated transitions injected with `&`;
- T-list ListSpecs as constraints on admissible verbs/transitions;
- coexistence of transition constraints at different modeling levels;
- `...` as missing specification constrained by surrounding identities, not arbitrary change;
- distinction between repeated objects and repeated transitions.
- recursive ellipsis expansion: `... ≡ _ _ ...`;
- lazy materialization of unknown temporal positions;
- propagation of ListSpec transition constraints through positions exposed from `...`;

Initial reconstruction of temporal semantics, including:

- S-context vs T-context identity;
- previous-state RHS rule;
- temporal persistence;
- simultaneous vs sequential transitions;
- nested T-list entry/exit concept;
- repetition;
- higher/lower temporal levels;
- parent/child distinction;
- rooting and sibling synchronization;
- temporal references;
- sparse World behavior;
- repeated quantification;
- current World/galaxy problem.




# Immediate Implementation Target

## A. Implement `.begin`

**Priority:** First

`.begin` should denote the identity-distinct null/cut immediately before the first item of an ordered list or T-list.

Conceptually:

```proteus
L.begin
```

is equivalent to:

```proteus
L@[0:0]
```

Its pure state may normalize to:

```proteus
*1+0
```

but its identity must retain the source list and cut position.

Required behavior:

- `L.begin` is addressable even when `L`'s later states are unknown.
- Two different lists have identity-distinct `.begin` boundaries.
- `.begin` can participate in ordinary identity and containment.
- `.begin` of a child T-list can be inserted into an S-state of a parent T-list.
- `.begin` survives normalization/reference traversal as a view, even if the underlying null-state representation is optimized away.
- No general singlet-access syntax is required.

Primary rooting use:

```text
some S-state of %W inside epoch
    contains
&G.%asTList.begin
```

This roots the galaxy lifetime in `%W` while constraining only its beginning.

---

## B. Establish the minimal T-list identity/navigation rules

**Priority:** Immediately after `.begin`

The goal is to make T-list behavior derive from ordinary identity plus a small number of T-specific reference rules.

### B1. Local temporal navigation

Support and verify:

```proteus
%prev
%next
```

with level-relative semantics.

Inside T-context:

```proteus
A = B
```

uses the previous S-context for RHS temporal references.

### B2. Sequence adjacency

Adjacent T-spans share a boundary:

```proteus
A.end = B.begin
```

when `B` immediately follows `A`.

`.end` phase semantics remain a later theory issue; only local adjacency should be relied on initially.

### B3. Parent/child insertion

A child temporal boundary/state can be contained in a parent S-state:

```text
parent S-state contains &child.begin
```

or later:

```text
parent S-state contains &child-state
```

Containment plus T semantics should establish the appropriate temporal root/context.

Do not add a separate `root()` operator unless identities prove insufficient.

### B4. Parent-to-child navigation

Given a parent T-context and a child reference, the engine should be able to find the child state/span corresponding to the current parent temporal position when enough identities are known.

### B5. Child-to-parent navigation

Given a child state/span rooted in a parent, the engine should be able to navigate/find the corresponding parent temporal position/span.

### B6. Sibling-at-same-time navigation

Given two children rooted in a shared parent:

```text
child A -> parent <- child B
```

navigation from A's current temporal context to B should proceed through the shared parent identities.

No special "same time" primitive should be introduced unless this fails.

### B7. Higher/lower-level traversal

A higher-level S-state may contain/refine into a lower-level T-span.

Navigation must allow:

```text
higher state -> lower T-span
lower state/span -> containing higher state
```

while preserving the distinction between:

- temporal cuts,
- state transitions,
- resolution/level.

### B8. Sparse population

The engine must be able to insert/refine child boundaries or states into sparse parent T-lists without materializing the whole parent timeline.

For World population this means:

```text
select epoch
find/create one S-position in that span
insert &G.begin
```

while leaving surrounding World states unspecified.

### B9. Reverse search

Because identities are bidirectional, once children are rooted the engine should support queries such as:

```text
find all child T-lists whose .begin is contained in this epoch
```

This is required for World-centric queries after child-centric population.

---

## C. Tests before adding more temporal semantics

Implement small tests in this order:

1. `L.begin` exists and is identity-distinct from another list's `.begin`.
2. `L.begin` has null/one-state value semantics.
3. A parent S-list can contain `&L.begin`.
4. Child -> parent lookup succeeds from that identity.
5. Parent -> child reverse lookup succeeds.
6. Two child begins can be inserted at independent positions in the same parent span.
7. Two child begins can be explicitly identified with the same parent position.
8. A child later state cannot resolve to a parent position earlier than its `.begin`.
9. Sibling lookup through a shared parent succeeds.
10. A lower-level T-span can be navigated from a higher-level S-state and back.
11. Sparse parent spans remain sparse after insertion.
12. Repetition/ListSpec can generate many independently rooted child `.begin` constraints.

Only after failures in these tests should additional T-specific semantics be considered.
