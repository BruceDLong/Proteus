# Negative indexing and time-span design

Status: semantic draft for review. This document does not authorize an engine
implementation yet.

Companion implementation draft:
`theory/negative-index-time-implementation-guide.md`.

Source basis: local branch `proteus3b` at
`bce565d3c7ba042cf615457f155ea508271b875a`, including the uncommitted
`ViewMap` and retained-POV lifetime changes.

## Purpose

Define one coherent meaning for:

- indexing a sequence from its end;
- selecting a span whose start is expressed with a negative index;
- materializing a location in a sparse time sequence without changing later
  time coordinates;
- naming a stable historical span such as the 20th Century;
- referring to a thing at, during, or overlapping a time span; and
- reading and writing through a selected or named span.

The design must preserve the ordinary Proteus mechanisms. A negative-index
span is a selection over an ordered source, not a second time subsystem.
Positive and negative slice sugar must lower to the same intersection and POV
navigation operations used by their non-sugar forms.

## Current behavior that must remain valid

### Item indexing

Ordinary item syntax is one-based:

```proteus
{'Cat' 'Hat' 'Bat' 'Dog'}#3       // 'Bat'
```

Current negative item syntax is end-relative:

```proteus
{'Cat' 'Hat' 'Bat' 'Dog'}#-1      // 'Dog'
{'Cat' 'Hat' 'Bat' 'Dog'}#-2      // 'Bat'
```

The end-relative traversal is structural rather than limited to direct list
children. For example, `{'A' &{'B' 'C'} 'D'}#-2` selects `C`.

For a closed source with flattened extent `E`, the current item-coordinate
rules are:

```text
#p,  p > 0       -> local item index p - 1
#-k, k > 0       -> local item index E - k
#0               -> invalid as an item reference
```

If the source extent is not final, a negative item reference remains pending.
It resolves after the source closes and retains the pending POV so an earlier
read or write is not left pointing at a destroyed temporary object.

### Slice indexing

The existing slice form uses a boundary count, not an item ordinal:

```proteus
{0 1 2 3 4 5 6 7 8 9}#6:*3+{ ... }   // {6 7 8}
```

Its non-sugar form is:

```proteus
[& *6+{...} <&*3+{...}> ] <~ {0 1 2 3 4 5 6 7 8 9}
```

Therefore `#6:span` means “start the selected span after six source items.”
This is intentionally different from item expression `#6`, which means “the
sixth item.” The documentation and parser tests must keep this distinction
visible.

### Ordered-unit views

A typed time unit is currently modeled as a fixed-size ordered span over a
smaller unit. Test definitions use small cardinalities:

```proteus
@second = _
@minute = *3+{second| ...}
@hour   = *2+{minute| ...}
@day    = *2+{hour| ...}
```

The normalizer can view a source represented in seconds as minutes, hours, or
days when the definitions reduce to a common base unit. Sparse spans may be
split or exposed symbolically; traversal must not expand a billion-item prefix
one item at a time.

## Terms

### Source sequence

The ordered infon whose coordinate space is being used. It may contain direct
items, nested transparent sublists, concrete typed spans, or sparse typed
spans.

### Source extent

The number of logical items in the selected unit after transparent nesting and
valid unit conversion. An extent is *final* only when the source boundary is
known not to move for the operation being resolved.

### Item coordinate

A zero-based internal coordinate naming one logical source item. User-facing
ordinary `#p` item syntax remains one-based.

### Boundary coordinate

A zero-based position between items. A source of extent `E` has boundary
coordinates `0` through `E`. Spans use half-open boundaries `[start, end)`.

### Span

A view with a start boundary, an end boundary, and a source traversal context.
The end may be derived from a fixed span definition, an explicit length, or a
named calendar boundary rule.

### Stable span and relative span expression

A stable span has resolved source boundaries. Extending the source later does
not move it. A relative span expression, such as “the last year,” is evaluated
against a specified source revision or observation boundary and may produce a
different stable span on a later evaluation.

These must not be represented as the same kind of long-lived object. In
particular, a historical name such as `20th-Century` must not drift when the
world timeline grows.

## Proposed semantic rules

### 1. Negative span indices name start boundaries

For slice syntax `source#i:spanSpec`, `i` is a boundary displacement:

```text
i >= 0       -> start boundary i
i < 0        -> start boundary E + i
```

The span occupies `[start, start + length(spanSpec))` and is valid only when:

```text
0 <= start <= E
start + length(spanSpec) <= E
```

Examples for a two-hour day:

```proteus
day#0:hour       // first hour:  [0, 1)
day#1:hour       // second hour: [1, 2)
day#-1:hour      // last hour:   [E - 1, E)
```

For a 24-hour day:

```proteus
day#-3:*2+{hour| ...}   // the two hours starting three hours before the end
```

This chooses the start-boundary interpretation rather than making the negative
number identify the selected span's end. It is consistent with current
end-relative item lookup: both `source#-1` and `source#-1:oneItemSpan` begin at
the final logical item.

The unit of `i` is the unit implied by `spanSpec` when a valid ordered-span
conversion exists. An untyped `spanSpec` uses the source's current logical item
unit. A mixed-unit expression that has no exact conversion is unresolved or
rejected; it must not round.

### 2. Negative indexing is relative to a view boundary, not calendar zero

`#-1` always means “relative to the final boundary of this source/view.” It
does **not** mean year 1 BCE and it does not by itself select a calendar era.

Signed calendar labels belong to a calendar mapping. This keeps generic list
navigation independent from whether a calendar has a year zero, how it names
eras, or where its epoch lies.

### 3. Spans are half-open

Every time span uses `[start, end)`. Adjacent spans share a boundary and no
instant belongs to both merely because one span ends when the next begins.

This convention makes fixed-unit composition exact:

```text
first hour  = [0, 60 minutes)
second hour = [60, 120 minutes)
two hours   = [0, 120 minutes)
```

Human calendar names may print inclusive labels such as “1901–2000,” but their
runtime boundaries remain the beginning of 1901 and the beginning of 2001.

### 4. Selection creates a source-backed view

Selecting a span must not copy its source items into an independent list. The
result is locally zero-based and carries its source mapping in POVs:

```text
POV.viewMap.sourcePov  traversal context and stopping boundary
POV.viewMap.startPov   source item corresponding to local item 0
```

`sourcePov == NULL` continues to mean the POV is not derived as a view/span.
`startPov` may be inside a nested sublist while `sourcePov` is an enclosing
source. Traversal may leave that nested list only along the source context's
ordinary logical order, and it must stop at the selected span end or the
`sourcePov` boundary.

No negative coordinate, source-global coordinate, or calendar coordinate is
stored in `infonView`. The selected `infonView` remains an ordinary locally
zero-based list/span.

Every source-backed child in a composite unit view must map to the exact source
POV it represents. Mapping only the composite head is insufficient for writes
to nested members.

### 5. Selection and naming do not create semantic identity

A selection recovers existing source locations and records provenance. It does
not prove that an independently constructed infon is identical to the selected
source. A name may bind to the stable span reference, but content equality is
not identity evidence.

The span mapping must remain alive for as long as the selected or named span
can be read or written. Temporary source/intersection POVs reachable through
non-owning parent links must be transferred through `infonView.retainedPOVs`
when an intersection wrapper is replaced by its selected result.

### 6. “Insert into time” never shifts interior time coordinates

There are three different operations that ordinary list code might call an
insert. The time API and implementation documentation must name them
separately:

1. **Materialize a time slot.** Split an implicit/sparse span into a prefix,
   the addressed slot or subspan, and a suffix. Total source extent and all
   later coordinates are unchanged.
2. **Add temporal content.** Add or assert a thing/event/state in the selected
   slot's `contents`, or add a relation from the thing to a stable span. The
   sequence of time units is unchanged.
3. **Extend the timeline.** Add new time capacity only at an authorized source
   boundary. This changes the source extent and therefore the result of future
   relative expressions.

Inserting a new interior time unit and shifting all later units is not a valid
way to record an event. It would invalidate existing coordinates, names, and
source mappings.

### 7. Writes through spans update source locations

A write through a one-item reference updates or refines the mapped source slot,
as current positive and negative item writes do.

A write through a multi-item span is a pairwise mapped write after both sides
have been aligned to compatible unit structure. It must be atomic at the
semantic-operation level:

- validate source mapping, extent, unit conversion, and RHS shape first;
- plan any sparse splits/materializations without exposing a partial result;
- reject or remain pending without modifying the source if any mapped pair is
  invalid; and
- commit all mapped updates together.

Rebinding a synthetic view child is not sufficient. The executor must target
the mapped source POV. Conversely, a span assignment must not replace the
source container or shift its siblings.

### 8. Sparse time stays sparse

Locating a time inside a million-year or billion-year span should use unit
measure arithmetic and split at only the required boundaries. The normal form
after materializing one location is conceptually:

```text
sparse prefix + materialized selected slot/span + sparse suffix
```

The prefix and suffix may be absent at a boundary. Their counts must add with
the materialized extent to the original extent. Named spans may remain views
over sparse backing; naming alone must not materialize every contained unit.

### 9. Named calendar spans are mappings over time

A name such as `20th-Century` denotes a stable span produced by a particular
calendar view. Its definition needs:

```text
calendar system / chronology
era and epoch rules
start boundary
end boundary
source timeline
source revision or other stable-boundary guarantee
```

The generic ordered-span layer handles the resulting source boundaries. The
calendar layer is responsible for translating the human label to those
boundaries.

This separation is necessary because “20th Century” is ambiguous without a
calendar convention. Under the conventional Gregorian/CE ordinal meaning it
is the half-open span from the start of 1901 through the start of 2001. A
digit-based convention would instead use 1900 through 2000. That choice belongs
in the calendar definition, not in negative-index traversal.

The following is illustrative pseudo-Proteus, not approved syntax:

```proteus
20th-Century = Gregorian-CE.span(century:20)
```

If the calendar maps CE year 1 to source boundary `ceStart`, the conventional
ordinal calculation is:

```text
start = ceStart + (20 - 1) * 100 years
end   = start + 100 years
```

The resulting value is a stable source-backed span. Its local item 0 is the
first year of that century even if the source stores that year inside a nested
or sparse million-year structure.

### 10. Things in time use explicit temporal relations

A thing is not made temporal merely by inserting it between time units. It is
related to a stable instant or span. At minimum the semantic vocabulary must
distinguish:

```text
at            the thing is associated with one instant or smallest selected slot
during        the thing's temporal extent is contained by the referenced span
overlaps      the thing and span share some non-empty temporal extent
starts-at     the thing begins at the referenced boundary
ends-at       the thing ends at the referenced boundary
```

Whether these are stored as relations on the thing, references in a
`year_slice.contents` list, or both is a storage/indexing decision. Their
meaning must be relation-based so one event spanning many years does not need
to be copied into every year and so a named span can be queried without
materializing all of it.

A time-slice `contents` list may be a reverse index or a convenient ownership
location, but membership there must preserve a link to the same thing/event
identity.

## Non-sugar intersection model

The canonical operation is an intersection selection over the source:

```text
prefix constraint + marked selected span + suffix constraint
```

For a known two-hour source, selecting the last hour can be expressed by the
existing forward intersection:

```proteus
[& *1+{hour| ...} <&*1+{hour| ...}>] <~ day
```

For a negative slice expression, resolution proceeds as follows:

1. Determine the source/view traversal context and final extent in the
   requested unit.
2. Convert the negative boundary displacement to `start = E + i`.
3. Validate the requested span end.
4. Construct or normalize the same intersection graph that a forward prefix
   of `start` units and a marked span would use.
5. Select the marked result while preserving a `ViewMap` from every selected
   view POV to its exact source POV.

The prefix is logical. It may remain a sparse count and must not be expanded.
If the extent is not final, steps 2–5 remain pending. If a calendar operation
supplies already-stable source boundaries, it may begin at step 4 without
performing an end-relative calculation.

This is the semantic lowering target. The parser does not need a separate
negative-span runtime once the intersection selector can carry the resolved
source origin and mappings correctly.

## Proposed user-facing examples

These examples state intended meaning; syntax involving named calendar spans
is provisional until the decisions below are approved.

```proteus
// Read the last hour of a day.
day#-1:hour

// Write the last two hours without shifting the day.
day#-2:*2+{hour| ...} = replacement-hours

// Bind a stable historical span obtained from a calendar mapping.
20th-Century = Gregorian-CE.span(century:20)

// Refer to a thing during that span.
event:{during = 20th-Century}

// Add information at a location addressed relative to a stable source end.
timeline#-3:year.contents = {... event ...}
```

The final example must add temporal content or refine the selected year. It
must not insert a new year before the final three years and shift subsequent
coordinates.

## Required implementation invariants

1. `infonView` indexing remains locally zero-based.
2. User-facing item indexing remains one-based; slice offsets remain boundary
   counts.
3. Negative indices are resolved against the selected source/view boundary,
   never against an unrelated outer list.
4. `ViewMap.sourcePov` is `NULL` only for a non-derived POV.
5. `ViewMap.startPov` identifies local item 0, including when it is nested.
6. Derived traversal cannot escape `sourcePov` or the selected half-open span.
7. Every writable derived child maps to its exact source child.
8. Selection and naming preserve source identity but do not create semantic
   identity evidence.
9. Lifetimes of mapping and parent POVs outlive all selected/named views that
   reach them.
10. Sparse selection is proportional to the number of crossed span boundaries,
    not to the numeric distance from the origin.
11. Interior materialization and content insertion do not change timeline
    extent or later coordinates.
12. Multi-item write validation completes before any source mutation becomes
    visible.
13. Calendar-label resolution is separate from generic index arithmetic.
14. Stable named spans do not drift when the source later extends.

## Decisions requiring review

### D1. Negative slice anchor

Proposed: `source#-k:span` starts `k` logical units before the source end.
Thus `#-1:hour` selects the last hour. The alternative is to treat `-k` as the
span's end boundary, which would make `#-1:hour` select the hour immediately
before the last hour and would disagree with negative item lookup.

### D2. Meaning of “insert into time”

Proposed: allow materialization/refinement of an existing implicit slot and
addition to temporal contents; forbid shifting insertion inside an established
timeline. Timeline extension is a separate boundary operation.

### D3. Historical names

Proposed: a named historical span is stable after resolution. Relative phrases
such as “last year” remain expressions evaluated against an explicit source
revision or observation boundary.

### D4. 20th Century convention

Choose whether the default calendar view uses the ordinal convention
1901–2000 or a digit-based 1900–1999 convention. The engine should support
multiple named calendar views, but one convention must own the unqualified
English label.

### D5. Temporal-content authority

Choose whether the authoritative assertion is primarily `thing.during = span`,
primarily membership in `timeSlice.contents`, or one canonical relation with
the other maintained as an index. The design recommends one canonical relation
plus derived indexes so the same thing is not independently copied into many
time slices.

## Implementation-documentation work after semantic approval

Once D1–D5 are settled, the implementation guide must include:

- exact parser AST/lowering rules for positive and negative `#index:spec`;
- a single coordinate-resolution API for item and boundary coordinates;
- traversal pseudocode that uses `sourcePov` and `startPov` across nesting;
- `ViewMap` construction and propagation rules for scalar, flat-span, and
  recursive typed views;
- stable versus pending relative-reference state and lifetime ownership;
- sparse split/materialization transaction steps;
- atomic mapped-span write planning and commit behavior;
- calendar mapping interfaces and named-span storage;
- temporal relation and indexing operations;
- diagnostic rules for non-final extents, out-of-range spans, incompatible
  units, ambiguous calendars, and stale mappings;
- migration stages that preserve ordinary item indexing; and
- parser, unit, dynamic, sparse-scarcity, lifetime, read/write, and calendar
  acceptance tests.
