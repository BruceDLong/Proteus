# Proteus Temporal Rooting: Terminology and Existence Techniques

## Purpose

This note captures the terminology and techniques developed while discussing how the Proteus engine can represent objects that exist in time.

The immediate goal is to get Proteus to the point where it can serve as the foundation of a knowledge / education tracking system. The recent engine work allows T-lists to use negative indexes, including a world T-list whose origin can be astronomical year 0. The next major semantic milestone is representing and reasoning about the existence of objects over time.

This document distinguishes:

- concepts that appear settled,
- useful terminology introduced during the discussion,
- several ways of asserting temporal existence,
- and syntax that is still provisional.

---

# 1. Core Proteus terminology

## 1.1 Infon

An **infon** is a Proteus information object.

Example:

```proteus
*5+3
```

This infon has five possible states and is currently in state `3`.

---

## 1.2 State

A **state** is an actual or possible value of an infon at an instant.

For example:

```proteus
*5+3
```

has five possible states but is in state `3`.

A number such as `5`, when used as an index into a T-list, is therefore **not** a state. It is an index or position in that T-list.

---

## 1.3 Index / position

An **index** or **position** identifies an infon within a T-list.

If a T-list is indexed relative to astronomical year 0, then negative indexes naturally refer to times before year 0.

For example, conceptually:

```text
%w[-1]
%w[0]
%w[1]
```

refer to infons at three positions in the world T-list.

---

# 2. S-infons and T-infons

## 2.1 S-infon / S-list

An **S-infon** is an infon representing a slice that has no state changes **at the level at which that infon is defined**.

An S-infon can therefore be written as a regular list.

Example:

```proteus
{
    ...
    bike
    ...
}
```

The important qualification is:

> “No state changes” applies only at the infon’s defined level.

An object contained in an S-infon may itself contain T-structure and may change internally.

---

## 2.2 T-infon / T-list

A **T-infon** or **T-list** is a temporally varying infon.

A T-list consists of S-infons and/or references to other T-infons.

Conceptually:

```text
T-list
    S-infon
    S-infon
    &T-infon
    S-infon
    ...
```

The exact syntax and normalization rules determine how these appear in Proteus.

---

## 2.3 S and T are level-relative

A bike can be continuously present in a world slice while changing internally.

For example:

```text
world slice:
    { ... bike ... }
```

may be an S-infon at the world level even while:

```text
bike:
    color changes
    position changes
    owner changes
    condition changes
```

are represented by T-structure inside the bike.

This distinction is essential:

> Persistent object existence must not be confused with persistent object state.

---

# 3. Structural descent and external association

Proteus already distinguishes two kinds of navigation.

## 3.1 `.` — structural descent

A dot expression searches for sub-infons.

Examples:

```proteus
bike.frame
bike.frontWheel.tire
```

These refer to infons structurally within the bike.

---

## 3.2 `^` — external association

Caret relations represent associated items that are not sub-infons.

Examples:

```proteus
bike^owner
Bob^birthday
```

A `^` relation implies or accesses a larger super-system containing the related items in some relation.

Example:

> Alice rode the bike.

At the relevant time, there is a system containing Alice and the bike in a riding relation. During that interval, something such as:

```proteus
bike^rider
```

may apply.

For the immediate rooting work, `^` relations probably do not need special handling. Their consequences should largely follow through ordinary identity and substitution.

---

# 4. Temporal existence as containment

The central semantic idea is:

> An object exists in a world at a time when an appropriate infon from the object’s T-list occurs as a sub-infon of the world infon at that time.

For an interval, the corresponding spans align point-by-point.

A useful discussion notation is:

```text
WorldSpan contains ObjectSpan
```

or mathematically:

```text
W[a:a+n] ⊃T O[b:b+n]
```

where `⊃T` is only explanatory notation, not currently proposed as a new Proteus operator.

Semantically this means:

```text
for every k in the span:
    O[b+k] is contained in W[a+k]
```

So T-containment is a lifting of ordinary S-infon containment across aligned positions.

---

# 5. Rooting

## 5.1 Root

An object is **rooted** in a host T-list when the available information establishes a temporal connection between some part of the object's T-list and that host T-list.

Typical hosts include:

```text
%w
@buffyWorld
```

A root does not require that the object's complete temporal extent be known.

---

## 5.2 Host T-list

The **host T-list** is the temporal model in which existence is being considered.

Examples:

```text
%w
@buffyWorld
```

An object can be rooted in one host without being rooted in another.

---

## 5.3 Anchor

An **anchor** is a known temporal correspondence between part of an object's T-list and part of a host T-list.

Conceptually:

```text
object local position  <->  host position
```

Example:

```text
bike[?] <-> %w[2005]
```

The bike-local position may be unknown.

---

## 5.4 Presence anchor

A **presence anchor** establishes that some S-infon of the object exists at a particular host time without saying that this time is the beginning or end of the object's extent.

Conceptually:

```text
bike[unknown-index] <-> %w[2005]
```

This is the natural result of a statement like:

> There is a bike in 2005.

or:

> I have a bike.

when the utterance is rooted at a particular time.

---

## 5.5 Start anchor and end anchor

A **start anchor** identifies the beginning of an object's T-list with a host time.

An **end anchor** identifies the ending boundary of an object's T-list with a host time.

These are stronger than a presence anchor.

Examples of increasing knowledge:

```text
"I have a bike."
    presence anchor known
    start unknown
    end unknown

"I got a new bike in January 2020."
    likely start anchor known
    end unknown

"I had a bike from January 2020 through May 2024."
    start and end constrained
```

---

## 5.6 Extent

The **extent** of an object is the complete span of its T-list.

The extent may be:

- fully known,
- partly known,
- or entirely unknown except for one or more presence anchors.

An object does not need a known start or known end in order to be rooted.

---

# 6. Rooted worlds and fictional worlds

Rooting is world-relative.

Suppose a fictional bike exists in a Buffy story:

```text
@buffyWorld
@buffyBike
```

If the bike is temporally contained in `@buffyWorld`, then it is rooted in that world.

It need not be rooted in `%w`.

The actual world `%w` may instead contain a story that describes `@buffyWorld`.

Thus:

```text
@buffyBike rooted in @buffyWorld
```

does not imply:

```text
@buffyBike rooted in %w
```

This allows Proteus to represent actual, fictional, hypothetical, simulated, or otherwise contextual worlds without needing primitive tags such as “real” or “fictional.” The important fact is which host T-list the object is rooted in.

---

# 7. Three main ways to assert existence

Three important techniques emerged.

They may establish related temporal facts without necessarily producing the same stored world model, because Proteus is intentionally lazy.

---

## 7.1 Internal / normalized world model

The most explicit representation places the object directly inside the world model.

Conceptually:

```proteus
%world = {
    ...
    {
        ...
        bike
        ...
    }
    ...
}
```

At the relevant temporal level, a world S-infon contains an S-infon of the bike.

This is an explicit or materialized representation of presence.

---

## 7.2 External model assertion

A second technique asserts containment from outside the world's stored definition.

Provisional discussion notation:

```text
%world#2005 contains bike#_
```

or a span-to-span form.

This can establish a root without requiring that the entire `%world` representation be rewritten immediately.

The object side may identify:

- the entire object T-list,
- part of its T-list,
- or one S-infon from its T-list.

Different forms communicate different amounts of alignment information.

### Weak form

```text
%world#2005 contains bike#_
```

Meaning:

- the bike is present in `%world` at 2005,
- some bike S-infon participates,
- the bike-local index is unknown,
- the bike's start is unknown,
- the bike's end is unknown.

### Stronger form

Conceptually:

```text
%world#2005 contains bike#0
```

Meaning:

- bike local index 0 aligns with world 2005.

This supplies a start alignment if `bike#0` is known to be the first temporal position.

### Span form

Conceptually:

```text
world-span contains bike-span
```

Meaning that corresponding positions are contained point-for-point.

---

## 7.3 Introduction

**Introduction** is expected to be by far the most common way objects enter the knowledge model.

Example:

> I have a bike.

The phrase introduces a bike without requiring explicit world normalization.

The bike may remain under the speaker's local model:

```text
%me
    assertion
        bike
```

If `%me` is rooted in `%w` at the utterance time, the assertion can establish that the bike is also rooted in `%w` at that time.

Proteus need not immediately materialize:

```text
%w[t] = { ... bike ... }
```

unless some later reasoning task requires it.

This is a crucial lazy-modeling principle.

---

# 8. Introduction

## 8.1 Introduction is a general semantic operation

Introduction is not merely a linguistic feature of English.

The English article:

> a

is one common way of requesting introduction.

For example:

> a bike

introduces an otherwise unidentified infon constrained to satisfy `bike`.

Conceptually:

```text
introduce fresh b such that b is a bike
```

---

## 8.2 Introduction of sub-infons

When an object is introduced, its required structural sub-infons may also be introduced.

For a bike, this can include:

```text
bike.frame
bike.frontWheel
bike.rearWheel
bike.handlebars
...
```

Because `.` refers to structural sub-infons, these can naturally arise while satisfying the structure of the introduced bike.

Thus adding one object may introduce a whole graph of required subordinate infons.

This is expected to be a major behavior of knowledge construction in Proteus.

---

## 8.3 Introduction does not require eager world materialization

If:

> I have a bike.

is asserted in a model rooted in `%w`, the bike can remain locally represented.

The engine only needs to be able to prove a root path when required.

This avoids exploding every implied fact into the global world model.

---

# 9. Direct roots, derived roots, and materialization

Because Proteus is lazy, it is useful to distinguish how a root is known.

## 9.1 Direct root

A **direct root** is explicitly asserted by temporal containment or already materialized containment.

Example:

```text
world-span contains bike-span
```

---

## 9.2 Derived root

A **derived root** can be proven through other rooted structures and implications.

Example:

```text
%me is rooted in %w
"I have a bike" is rooted in %me at time t
therefore the bike can be proven rooted in %w at t
```

The bike need not yet appear explicitly in the `%w` structure.

---

## 9.3 Root path

A **root path** is the chain of identities, containments, introductions, or implications by which an object can be proven rooted in a host.

Example:

```text
bike
  ^
"I have a bike"
  ^
%me at t
  ^
%w
```

---

## 9.4 Materialization

**Materialization** means constructing a more explicit representation in a host model because a query or reasoning operation requires it.

Proteus should not materialize all logically possible containment eagerly.

A useful principle is:

> Rooting is a provable relationship, not necessarily a stored relationship.

---

# 10. Bare presence: “There is a bike”

The remaining primitive case is an object existing at a time with no other asserted relation.

Example:

> There is a bike.

or:

> There was a bike in 2005.

The semantic content should be only:

> Some S-infon of a bike is present as a sub-infon of the host S-infon at that time.

No ownership, riding, location, or other super-system relation needs to be asserted.

Useful terminology:

- **presence** — an object's S-infon occurs in a host S-infon;
- **presence assertion** — an assertion of that fact;
- **presence anchor** — the resulting temporal correspondence.

Conceptually:

```text
bike[unknown local index]
        |
        v
%w[2005]
```

This is enough to root the bike in `%w`.

---

# 11. Important unresolved syntax question

The semantic idea of bare presence is clearer than the final Proteus syntax.

A candidate discussed was:

```proteus
%w#2005 = { ... bike ... }
```

but this raises an important question:

> What exactly does `%w#2005` evaluate to?

Two possibilities must be distinguished.

## Possibility A: point selection

`%w#2005` means:

> the S-infon at index 2005.

Then an S-list RHS is natural:

```proteus
%w#2005 = { ... bike#_ ... }
```

where `bike#_` means some S-infon from the bike's T-list.

## Possibility B: one-position T-slice

`%w#2005` means or produces:

> a one-position slice of `%w`, which is still formally a T-infon.

Then the correct relationship between the T-level RHS and an S-list representation must be made explicit.

This connects with an existing Proteus rule:

> A T-slice containing only one state at its defined level can be written as a regular list.

The unresolved question is whether this is only a representational equivalence or whether the selection syntax itself should return an S-infon.

---

# 12. `bike` versus `&bike` inside a world slice

Another syntax question remains open.

If the RHS is an S-list describing a world infon, should presence look like:

```proteus
{ ... bike ... }
```

or:

```proteus
{ ... &bike ... }
```

The answer depends on the precise existing semantics of `&` when a T-infon participates inside another T-list or S-list.

The important semantic requirement is independent of syntax:

> At a single host time, the world contains an appropriate S-infon of the same bike, not a frozen copy of the bike's entire T-list.

This should be resolved by reference to the engine's current `&T` and substitution semantics rather than by inventing a special existence operator.

---

# 13. Point selection versus span selection

A potentially useful distinction is:

```text
point selection -> S-infon
span selection  -> T-infon
```

However, this is still provisional until mapped exactly onto current Proteus syntax.

The deeper semantic distinction should remain even if the implementation uses a different representation:

- an infon at one temporal position,
- versus a slice containing one or more temporal positions.

A one-position T-slice may still have an S-list representation.

---

# 14. Existence over a span

The long-term target is to represent statements such as:

> The bike exists from t1 to t2.

This should not require a special `exists(t1,t2)` predicate.

Instead, it should fall out of aligned temporal containment.

Conceptually:

```text
host span:
    t1 ... t2

contains

bike span:
    i ... i+n
```

The bike-local index `i` may be known or unknown.

If the complete bike T-list is known and its start is anchored, the host extent can be derived.

If only an interior presence anchor is known, the full extent may remain unknown.

---

# 15. Example: known extent and start

Suppose a bike's T-list is known to be five years long and its beginning is anchored to January 2020.

Conceptually:

```text
bike size = 5 years
bike[0] <-> world[January 2020]
```

Then:

```text
bike local index    host time
0                   Jan 2020
1                   Jan 2021
2                   Jan 2022
3                   Jan 2023
4                   Jan 2024
```

and the ending boundary is January 2025.

This is stronger information than merely knowing:

> The bike was present in January 2020.

---

# 16. Example: unknown extent

For:

> I have a bike.

at utterance time `tu`, the natural temporal information is:

```text
start anchor      unknown
presence anchor   known at tu
end anchor        unknown
```

Conceptually:

```text
bike T-list
... ---- [ ? ] ---- ...
          |
          v
        %w[tu]
```

The known anchor need not correspond to bike index 0.

This is expected to be extremely common.

---

# 17. Example: fictional world

Suppose:

> In the Buffy universe, Buffy got a bike in 1995.

The bike can be rooted in:

```text
@buffyWorld
```

without being rooted in:

```text
%w
```

The actual world may contain the Buffy story, which in turn describes `@buffyWorld`.

This cleanly separates:

```text
story exists in %w
```

from:

```text
bike exists in @buffyWorld
```

---

# 18. Suggested engine test progression

The immediate tests should focus on small semantic primitives rather than complete natural-language inference.

## Test 1: internal presence

Build a world T-list in which one selected world infon explicitly contains a bike infon.

Verify that the bike can be recognized as rooted in that host.

---

## Test 2: external bare presence

Express the equivalent of:

```text
%w at 2005 contains some S-infon of bike
```

without requiring:

- the bike's start,
- the bike's end,
- or the bike's complete size.

Verify that this establishes a presence anchor.

---

## Test 3: known local alignment

Assert the equivalent of:

```text
%w at 2005 contains bike local index 0
```

Verify that subsequent aligned indexes can be inferred when the bike's T-structure is known.

---

## Test 4: span containment

Select a host span and a bike span of compatible size.

Verify pointwise containment across the entire span, including negative host indexes and spans crossing year 0.

---

## Test 5: introduction

Introduce a bike inside a rooted assertion.

Verify that the bike can be proven rooted even if it is not eagerly materialized in `%w`.

---

## Test 6: structural sub-infons

Introduce a bike whose model requires sub-infons such as:

```proteus
bike.frame
bike.frontWheel
bike.rearWheel
```

Verify that those structural infons are introduced appropriately and retain temporal consistency with the bike.

---

## Test 7: fictional host

Root one bike in `%w` and another in a fictional world T-list.

Verify that rooting in one host does not imply rooting in the other.

---

# 19. Design principles established in the discussion

1. **State and index are different concepts.**
   A state is a value of an infon; an index is a position in a T-list.

2. **S/T distinctions are level-relative.**
   An S-infon may contain T-infons.

3. **Object existence should emerge from containment and identity, not from a special existence predicate.**

4. **An object can be rooted without knowing its full temporal extent.**

5. **A root may be established by an interior presence anchor.**

6. **Rooting is world-relative.**
   An object can exist in a fictional or hypothetical world without being rooted in `%w`.

7. **Introduction is expected to be the dominant way knowledge objects enter the model.**

8. **Introduction can recursively introduce structural sub-infons.**

9. **Proteus should remain lazy.**
   A derived root need not immediately be materialized in the global world model.

10. **Direct containment, external containment assertions, and introduction are distinct representation techniques.**
    They can establish related existence facts without producing identical stored models.

11. **`^` relations can be handled later through super-system semantics and substitution of identicals.**
    They are not required for the first rooting milestone.

12. **The final single-time syntax is still unresolved.**
    In particular, the roles of point selection, one-position T-slices, `bike` versus `&bike`, and `bike#_` need to be checked against current engine semantics.

---

# 20. Working vocabulary

| Term | Working meaning |
|---|---|
| **state** | An actual or possible value of an infon |
| **index / position** | A location in a T-list |
| **S-infon / S-list** | An infon with no state change at its defined level |
| **T-infon / T-list** | An infon whose defined level varies temporally |
| **host T-list** | The world/context T-list in which existence is considered |
| **host span** | A slice of the host T-list |
| **object T-list** | The object's own temporal infon |
| **object span** | A slice of the object's T-list |
| **presence** | An object S-infon occurs in a host S-infon |
| **root** | A provable temporal connection between an object T-list and a host T-list |
| **anchor** | A known temporal correspondence supporting a root |
| **presence anchor** | An anchor proving presence without identifying an endpoint |
| **start anchor** | An anchor known to identify the beginning of an object's extent |
| **end anchor** | An anchor known to identify the ending boundary |
| **extent** | The full span of an object's T-list |
| **direct root** | A root established by explicit containment/alignment |
| **derived root** | A root provable indirectly through other rooted information |
| **root path** | The reasoning chain by which a derived root is established |
| **materialization** | Explicitly constructing a containment that was previously only derivable |
| **introduction** | Establishing a new existential infon constrained by a description |
| **bare presence** | Presence with no additional relation asserted |
| **T-containment** | Pointwise containment between aligned temporal spans; discussion term only |

---

# 21. Immediate unresolved questions

Before finalizing engine syntax, clarify:

1. What exactly should a single-index temporal selector return?
   - an S-infon,
   - a one-position T-slice,
   - or a view capable of participating as either?

2. When a bike is present at one host time, should the RHS contain:
   - `bike`,
   - `bike#_`,
   - `&bike`,
   - or another existing Proteus expression?

3. How should an unknown bike-local index be represented?
   It must mean “some S-infon of this same bike T-list,” not “an arbitrary bike.”

4. When a span is used, what current Proteus mechanism most naturally expresses pointwise temporal containment?

5. Which facts should be stored versus left derivable through a root path?

6. How should an introduced object's required structural sub-infons inherit or derive their temporal rooting?

These should be answered using existing substitution, span-selection, view, and `&T` semantics wherever possible rather than by adding a special-purpose existence subsystem.
