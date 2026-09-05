# Negative indexing and time-span implementation guide

Status: implementation draft under revision. It is subordinate to
`theory/negative-index-time-design.md` and to the intersection-first review in
`theory/end-relative-intersection-plan.md`. Do not implement this guide's
separate coordinate-planner path; end-relative selection must first be
represented and proven as a low-level intersection operation.

Source basis: local branch `proteus3b` at
`bce565d3c7ba042cf615457f155ea508271b875a`, including the uncommitted
`ViewMap` and retained-POV lifetime changes.

## Goal and completion boundary

Implement one path that can:

1. select an end-relative item;
2. select an end-relative typed or fixed-length span;
3. read and write the selected span through exact source mappings;
4. materialize only the necessary part of sparse time;
5. retain a stable selected span under a name;
6. resolve a calendar name such as the 20th Century to a stable span; and
7. relate a thing/event/state to an instant or span without shifting the time
   sequence.

The work is not complete when `#-1:hour` merely prints the right value. It is
complete only when selection provenance, nested traversal, sparse scarcity,
source writes, lifetime, named-span stability, and temporal relations all pass
their acceptance tests.

## Semantic inputs

Implementation must begin with the accepted values of these inputs from the
semantic design:

```text
negative_span_anchor       = start-boundary | end-boundary
interior_time_insert       = materialize-only | shifting-allowed
named_historical_span      = stable | live-relative
default_century_convention = ordinal-1901 | digit-1900
temporal_content_authority = relation | slice-contents | canonical-plus-index
```

The current draft assumes:

```text
start-boundary
materialize-only
stable
not yet selected
canonical-plus-index
```

Do not bury a different choice in a parser special case.

## Current architecture and gaps

### Parser and path representation

`infonIO.dog::parseInfonView()` already parses both forms into `PartPath`:

```text
source#index
source#index:spec
```

`PartPath.path` contains the index expression and `PartPath.spec` contains the
optional span specification. No grammar change is required for
`source#-1:hour` if `hour` already parses as an infon.

`ParentMemberReasoner::resolveParts()` currently has two separate paths:

- a negative numeric index without `spec` calls
  `infonView.getEndRelativePOV()`;
- any numeric index with `spec` constructs a forward skip plus marked
  intersection, without giving inverted/negative indexes end-relative span
  semantics.

The second path is the first semantic gap. It must delegate to a shared index
planner rather than treating the inverted numeric value as an ordinary prefix
count.

### End-relative item state

`infonView` currently owns:

- `endRelativeExtentIsFinal()`;
- `getEndRelativePOV()`;
- retained pending references; and
- resolution when a list closes.

This mechanism correctly captures “wait until the end is final,” but it only
returns one item. It also performs structural lookup directly from
`infonView`, so it cannot express a selected span's source context and local
origin by itself.

Keep the existing item behavior during migration. Move coordinate calculation
behind a common planner only after parity tests exist.

### Intersection selection

An `emIntersection` with `ipGetMarked` is the canonical non-sugar slice
selection. `IntersectionReasoner::selectFunctionResult()` returns the marked
result, while `ParentMemberReasoner` maps some marked matches to RHS source
POVs.

The current mapping is incomplete:

- ordinary marked composite spans do not always become the returned source-
  backed view;
- sparse exposed heads have special mapping logic;
- a synthetic marked list wrapper maps only its first wrapper POV through
  `outerPOV`; and
- selection installation preserves retained POVs but does not build missing
  per-child mappings.

Do not add another special case for negative time. Generalize marked-span
selection so the selected result is a source-backed view regardless of whether
the source is concrete, nested, typed, or sparse.

### Ordered spans

`ModelManager.measureOrderedSpanUnit()` and
`getOrderedSpanConverter()` reduce fixed units to a common base count.
`OrderedSpanReasoner` can split compatible sparse spans and construct typed
views over concrete siblings.

The current view builders use `outerPOV` on direct children. The new
`ViewMap` exists but is not yet populated by these builders. Composite views
therefore lack an explicit traversal context/origin, and the general write
path cannot identify every exact source child.

### Writes

`ReferenceConsolidationStrategy.copyIdentity()` writes through a source-backed
POV only when:

```text
accessMode == aRefTo
outerPOV != NULL
outerPOV.pItem === pov.pItem
```

That is sufficient for one directly selected item. It is not sufficient for a
synthetic composite span whose children correspond to multiple source slots.
A multi-item mapped write needs a validation plan followed by one commit.

### Lifetime

`infonView.retainedPOVs` owns temporary POVs used by end-relative references
and intersection selections. `IntersectionReasoner.installSelectedResult()`
already transfers those keep-alives when replacing an intersection wrapper.

Every new mapping must use this existing ownership path. Do not rely on object
layout, padding, a non-owning `pParent`, or creation-site retention that is
lost when the selected result replaces its wrapper.

## Representation contract

### `ViewMap`

Keep the approved structure small:

```codedog
struct ViewMap {
    our POV: sourcePov
    our POV: startPov
}
```

The fields mean:

```text
sourcePov  the source traversal context and outer stopping boundary
startPov   the exact source item corresponding to local item 0
```

Rules:

1. Both fields are `NULL` on a non-derived POV.
2. A resolved derived scalar sets both to its exact source POV. For a scalar,
   source context and local item 0 may be the same POV.
3. A derived span head sets `sourcePov` to the source context and `startPov` to
   its first exact source item.
4. Every writable derived child sets `sourcePov` to the same applicable source
   context and `startPov` to its own exact source item.
5. If a child is itself a composite unit view, apply rule 3 recursively.
6. `infonView.infSize` or the selected span specification supplies local
   extent. Do not add source-global indexes to `infonView`.
7. `sourcePov` is not semantic-identity evidence. It is traversal/provenance.

`outerPOV` continues to describe an existing direct wrapper/alternative
relationship. New code must not make `outerPOV` and `ViewMap` competing sources
of truth for span mapping.

### Mapping helper

Add one helper owned by `POV`, with a name that reveals both operands:

```codedog
void: mapViewToSource(our POV: sourceContextPOV,
        our POV: sourceStartPOV)
```

Preconditions:

```text
self is a derived view POV
sourceContextPOV != NULL
sourceStartPOV != NULL
sourceStartPOV is reachable within sourceContextPOV's logical traversal
```

Effects:

```text
self.viewMap.sourcePov <- sourceContextPOV
self.viewMap.startPov  <- sourceStartPOV
self.accessMode        <- aRefTo
```

The helper must not alter `pItem`, `pParent`, source list topology, or semantic
identity. If the context/start POV could otherwise be temporary, the selected
result must retain it through its owning `infonView`.

### Coordinate request and resolution plans

Do not store a negative coordinate in a selected `infonView`. Use temporary
reasoning records:

```text
IndexRequest {
    kind                  item | span
    raw_index             signed integer
    origin                from-start | from-end
    span_spec             optional infon
    requested_unit        optional WordUse
}

SpanResolutionPlan {
    status                resolved | wait-for-final-extent | rejected
    source_context_pov
    source_extent_in_unit
    start_boundary
    span_length_in_unit
    base_unit_type
    start_base_offset
    base_count
    source_start_pov
    sparse_split_steps[]
    intersection_request
    diagnostic
}
```

These records are plans, not model state. They may be structs near the ordered-
span reasoner or a new narrowly scoped index/span reasoner. Avoid adding more
branch state to `infonView`.

### Stable named span reference

A historical name must retain a stable reference, not merely the printed span
contents or the original negative expression:

```text
NamedSpanReference {
    name / word binding
    calendar_view_id, if calendar-derived
    source_context_ref
    source_start_ref
    extent and unit
    source-boundary evidence or revision
    retained mapping graph
}
```

The first implementation may realize this with existing infon/POV handles and
`retainedPOVs`; it does not need persistent numeric IDs immediately. The
observable contract is that later source extension does not move an already
resolved historical name.

Do not implement a name by deep-copying the selected values. Reads and writes
through the name must continue to address the original source locations.

## Coordinate algorithms

### Parse an index request

In `ParentMemberReasoner::resolveParts()`:

```text
if pathMode != idxField:
    use existing field behavior

require path is a literal integer for this implementation stage

if spec == NULL:
    kind = item
else:
    kind = span

if path is inverted/negative:
    origin = from-end
    raw_index = -absolute_value
else:
    origin = from-start
    raw_index = value
```

Preserve non-literal numeric behavior as pending/general reasoning. Do not
convert unknown arithmetic into zero.

### Resolve an item coordinate

For source extent `E`:

```text
positive p: require p > 0; item_index = p - 1
negative k: require k < 0; item_index = E + k
```

Require `0 <= item_index < E`.

For negative requests, return `wait-for-final-extent` unless the selected
source/view boundary is final. Existing open-list pending semantics must remain
unchanged.

### Resolve a span coordinate

Under the proposed start-boundary rule:

```text
nonnegative i: start_boundary = i
negative i:    start_boundary = E + i
end_boundary = start_boundary + span_length
```

Require:

```text
0 <= start_boundary <= E
start_boundary < end_boundary
end_boundary <= E
```

A zero-length span is rejected in the first implementation. Supporting an
empty boundary view later requires an explicit use case and representation.

### Determine the coordinate unit

Resolution must identify one unit before applying the signed number:

1. If `span_spec` has an ordered unit type, use that unit for the index and
   span length.
2. Otherwise use the source's logical item unit.
3. Reduce source extent and span length through
   `getOrderedSpanConverter()` only when the conversion is exact.
4. Reject incompatible base units or non-integral conversions.
5. Do not treat variable calendar units such as arbitrary months as fixed
   base-count units. Calendar boundary logic resolves those separately.

This rule is essential for expressions such as `day#-1:hour`: `-1` means one
hour, not one underlying second.

### Locate a logical boundary

Add a boundary navigator that can cross:

- direct concrete siblings;
- transparent nested subitems;
- compatible sparse ordered spans; and
- recursive fixed-unit views.

Conceptual interface:

```text
locateBoundary(sourceContextPOV, offset, unit) ->
    resolved source POV / sparse split plan / pending / rejected
```

Required behavior:

1. Start at the source context's logical boundary 0.
2. Subtract whole sparse span measures arithmetically.
3. Descend into a nested sublist when the target falls inside it.
4. Move back to the enclosing source traversal when a nested sublist ends.
5. Stop when the source context boundary is reached; never continue into an
   unrelated outer or following list.
6. If the boundary falls inside a sparse span, plan a split at that boundary.
7. Return the exact source POV at the boundary after the split/materialization
   plan is committed.

Do not build this from `siblingAtOffset()` alone; that method only counts
direct concrete siblings and cannot satisfy nested or sparse traversal.

### Build the canonical intersection

After coordinate resolution, lower to the existing semantic form:

```text
logical prefix of start_boundary units
marked selected span of span_length units
optional logical suffix
```

The implementation may construct the same `emIntersection` graph currently
used by positive `#index:spec`. It must set `ipGetMarked` and keep a direct
reference to the marked request POV.

The prefix and suffix are constraints, not materialized lists. A sparse prefix
must remain a sparse typed count.

Do not add negative-span behavior directly to
`IntersectionReasoner.selectFunctionResult()`. That reasoner should receive an
ordinary resolved intersection and return the marked source-backed result.

## View construction

### Direct item view

When a selected unit and source unit have equal base measures:

```text
view.pItem = sourceStart.pItem
view.outerPOV = sourceStart            // compatibility during migration
view.mapViewToSource(sourceContext, sourceStart)
```

Writing through the view must target `viewMap.startPov` once the generalized
source-backed predicate is enabled.

### Flat span view

Generalize `POV.makeConcreteSiblingSpanView()` to receive the source context.
Map:

- the returned span head to `(sourceContext, firstSourcePOV)`; and
- each appended child to `(sourceContext, exactSourceChildPOV)`.

The local child list contains local POV wrappers and remains zero-based. It
must not share or rewrite source sibling links.

### Recursive typed span view

Generalize `OrderedSpanReasoner.makeOrderedSpanUnitView()` similarly. For each
recursive child:

```text
child source start = exact source boundary for that child
child source context = enclosing selected source context
```

The synthetic parent relationship describes the local typed view. `ViewMap`
describes the source relationship. Do not substitute one for the other.

### Stop condition

Iteration through a view stops at the first of:

1. the view's local declared extent;
2. the source context's traversal boundary; or
3. a pending/unavailable source boundary.

The iterator must not stop merely because `startPov.pParent` ends; it may need
to rise through nested source parents until it reaches the source context.

## Marked selection propagation

Replace the sparse-only marked-result mapping rule with a general correspondence
rule:

```text
when an LHS marked request POV matches an RHS source/view POV,
record the matched RHS POV as the marked result mapping
```

The rule must work for:

- scalar items;
- marked transparent sublists;
- concrete flat spans;
- recursive typed spans; and
- sparse exposed or split spans.

Type compatibility may help identify a candidate, but type equality alone is
not enough when multiple same-typed source positions are possible. The mapping
must come from the actual traversal correspondence that satisfied the
intersection.

`IntersectionReasoner.selectFunctionResult()` then returns either:

- the exact mapped scalar; or
- the mapped composite view whose children each carry exact mappings.

`installSelectedResult()` must continue transferring `retainedPOVs` before the
wrapper is replaced.

## Source-backed predicate and selection return

Generalize source-backed detection:

```text
direct legacy source-backed:
    accessMode == aRefTo && outerPOV != NULL &&
    outerPOV.pItem === pItem

mapped source-backed:
    accessMode == aRefTo &&
    viewMap.sourcePov != NULL && viewMap.startPov != NULL
```

During migration, either form may be accepted. Newly created span views must
always populate `ViewMap`.

`WorldManager.selectReturnedInfonViewFromPOV()` should preserve the mapping on
the returned LHS POV. It must not promote a composite span's `sourcePov` into
`outerPOV` as if the whole span were one source item.

For a scalar mapped view, `outerPOV` compatibility may remain temporarily. For
a composite view, mapped write/navigation must use `ViewMap`.

## Sparse materialization transaction

Current `splitSparseSpanPrefix()` mutates immediately. End-relative span
selection and mapped write need an explicit plan so invalid RHS shape or a
second boundary failure cannot leave half a split in the source.

Plan records:

```text
SparseSplitStep {
    original_span_pov
    original_count
    prefix_count
    selected_count
    suffix_count
    unit_type
}
```

Validation:

1. Confirm the original POV is still the expected sparse span.
2. Confirm count and unit type have not changed.
3. Confirm all counts are nonnegative and sum to the original count.
4. Confirm the selected span lies within the source/view boundary.
5. Confirm all later write-pair validation before committing splits.

Commit:

1. Split the prefix boundary if needed.
2. Split the end boundary if needed.
3. Materialize only the selected unit heads required for the operation.
4. Set exact parent/source mappings.
5. Preserve total count and following concrete/sparse nodes.

If the normalizer cannot yet provide rollback, build all replacement infons
and POVs detached, validate them, then splice the replacement segment in one
executor action.

## Atomic mapped-span writes

### Write plan

Create a plan before mutating source state:

```text
MappedSpanWritePlan {
    selected_view
    source_context
    pairs[]
    sparse_split_steps[]
    retained_dependencies[]
}

MappedWritePair {
    target_source_pov
    rhs_pov
    preserved_target_type
    preserved_target_list_spec
    preserved_target_item_mode
}
```

### Validation walk

Walk the selected view and RHS together:

1. Both nodes must be scalar or both must be compatible lists/views.
2. List extents and unit structures must align exactly or have an exact ordered-
   span conversion.
3. Every writable selected leaf must have non-NULL `sourcePov` and `startPov`.
4. `startPov` must still resolve within `sourcePov`.
5. No target may escape the selected half-open span.
6. Reject conflicting duplicate writes to the same source slot unless their
   RHS assertions are semantically identical under ordinary merge rules.
7. Validate all sparse split steps.

No source value is modified during this walk.

### Commit walk

After successful validation:

1. Commit required sparse materialization.
2. Re-resolve planned target handles if a split replaced topology.
3. Apply ordinary scalar/list merge semantics to each exact source target.
4. Preserve target slot metadata intentionally, using the existing
   `preserveLHSSlotMetadata()` policy as the starting point.
5. Mark affected normalization/index state dirty.
6. Publish the result only after all pairs succeed.

Do not merely call `copyAsTypeTo()` on synthetic local children without first
resolving their `viewMap.startPov`; doing so changes the view but not time.

### Assignment versus temporal-content addition

These are separate operations:

```text
span = replacement
```

performs a mapped assignment/refinement after exact shape validation.

```text
span.contents = {... thing ...}
thing.during = span
```

adds temporal content or relations. It does not replace the time-unit identity
or insert another time unit.

## Pending and stable end-relative references

### Pending request

If the source/view extent is not final:

- retain the request POV in the source owner's `retainedPOVs`;
- store the signed offset and span specification in request/reasoning state;
- do not create a guessed source mapping;
- do not mutate the source; and
- resume when the relevant source boundary becomes final.

An open source that is intended to grow forever cannot yield a stable “last”
span without an observation boundary or snapshot. Report that as pending, not
as an empty or current-last result.

### Resolved stable span

On resolution:

- compute boundaries once against the final/snapshot extent;
- build and retain exact `ViewMap` mappings;
- clear pending status; and
- preserve the resolved span when it is bound to a historical name.

Extending another outer timeline must not cause resolution against the wrong
boundary. The source context is the selected `sourcePov`, not whichever parent
is currently easiest to reach.

### Relative phrase

“Last year” or “previous century” should remain an expression that is evaluated
against an explicit source revision/observation boundary. Each evaluation
returns a stable span. Do not make a historical name retain a live negative
offset by accident.

## Calendar and named spans

### Boundary provider

Fixed ordered-unit conversion is not enough for calendar systems. Introduce a
calendar-facing boundary operation conceptually equivalent to:

```text
resolveCalendarSpan(calendarView, labelOrUnit, ordinal, era) ->
    source context + stable start/end boundaries + evidence
```

Responsibilities:

- choose chronology, era, and year-zero rules;
- translate a human label such as `20th-Century` to boundaries;
- reject an ambiguous unqualified label when no default applies;
- return half-open stable boundaries; and
- record enough calendar provenance to print/explain the result.

The generic span layer then builds the mapped source view. Calendar code must
not duplicate traversal, sparse splitting, or write behavior.

### 20th Century acceptance example

After the default convention is selected, define one exact expected mapping.
For the ordinal Gregorian/CE convention:

```text
label: 20th Century
printed inclusive years: 1901 through 2000
runtime boundaries: start-of-1901 through start-of-2001
duration in calendar years: 100
```

For a digit-based convention, use 1900 through 1999 and boundaries at 1900 and
2000. Tests must name the convention when checking both.

### Naming

The name-binding operation must retain the stable mapped span. If ordinary word
definition dereferencing currently copies the meaning and loses POV mappings,
do not force named spans through that path. Add or reuse a reference-preserving
binding mechanism with explicit selection provenance.

The design does not require final surface syntax in the first implementation
checkpoint. A low-level named-reference API or explicit relation is acceptable
for testing as long as the semantic object is stable and source-backed.

## Things in time

### Canonical temporal assertion

Under the proposed canonical-plus-index policy, store one authoritative
relation on the thing/event/state:

```text
thing.at = instant
thing.during = span
thing.overlaps = span
thing.starts-at = boundary
thing.ends-at = boundary
```

Maintain time-slice `contents` as a derived/search index containing references
to the same thing identity. Do not deep-copy the thing into each slice.

### Relation validation

- `at` requires one instant or one explicitly chosen smallest slot.
- `during` requires the thing extent to be contained by the referenced span.
- `overlaps` requires a non-empty half-open intersection.
- `starts-at` and `ends-at` require boundary references.
- A relation to a pending relative span remains pending.
- A relation to an ambiguous calendar label is rejected with a calendar
  diagnostic.

### Query behavior

Queries over a named span should use boundary relations and indexes. They must
not scan or materialize every source unit in a billion-year sparse span merely
to determine whether a thing overlaps it.

## Diagnostics

Provide distinct diagnostics for:

```text
INDEX_ZERO_ITEM
INDEX_BEFORE_SOURCE
SPAN_START_AFTER_SOURCE
SPAN_END_AFTER_SOURCE
SPAN_ZERO_LENGTH
SOURCE_EXTENT_NOT_FINAL
SOURCE_VIEW_BOUNDARY_LOST
SPAN_UNIT_UNKNOWN
SPAN_UNIT_INCOMPATIBLE
SPAN_UNIT_NONINTEGRAL
SPAN_MAPPING_MISSING
SPAN_MAPPING_STALE
SPAN_WRITE_SHAPE_MISMATCH
SPAN_WRITE_DUPLICATE_TARGET
CALENDAR_VIEW_REQUIRED
CALENDAR_LABEL_AMBIGUOUS
CALENDAR_BOUNDARY_UNRESOLVED
TIMELINE_INTERIOR_SHIFT_FORBIDDEN
```

Pending is not rejection. In particular, `SOURCE_EXTENT_NOT_FINAL` should
produce a wait state when more source information can arrive.

## Implementation sequence and gates

### Checkpoint 0: freeze current behavior and decisions

Documentation:

- approve/edit D1–D5;
- record exact proposed syntax examples; and
- re-establish the current dynamic and compiled test counts separately.

Tests that must still pass:

- positive item indexing and writes;
- current negative item reads/writes;
- negative lookup across a nested subitem;
- open-list negative reference resolution;
- positive slice sugar and low-level intersection equivalence;
- ordered-unit time tests; and
- `range/select1` with lifetime validation.

### Checkpoint 1: mapping and navigation, no new syntax

- add `mapViewToSource()`;
- populate mappings in direct, flat, and recursive view builders;
- add source-context-bounded boundary navigation;
- generalize source-backed detection; and
- retain all source mapping POVs through selected-result replacement.

Gate: current tests remain at their re-established baseline, focused lifetime
tests pass under Valgrind or equivalent, and no new feature test is enabled.

### Checkpoint 2: non-sugar negative span reads

- add index/span resolution plans;
- resolve negative start boundaries;
- lower to canonical intersections;
- generalize marked result mapping; and
- enable low-level read tests before sugar.

Gate: concrete, nested, typed, cross-nesting, open/pending, and sparse-scarcity
read tests pass.

### Checkpoint 3: non-sugar mapped writes

- implement sparse materialization planning;
- implement atomic mapped-span write planning/commit;
- preserve slot/type metadata; and
- enable the existing pending sparse/concrete/sparse span case as a required
  passing test.

Gate: read-after-write through both source and view agrees; rejected writes
leave byte-for-byte equivalent normalized source output; scarcity tests do not
expand large spans.

### Checkpoint 4: negative slice sugar

- route `#negative:spec` through the same planner;
- compare parsed/lowered graphs with the non-sugar form; and
- add printer round-trip tests.

Gate: sugar and low-level forms produce equivalent normalized output, source
mapping, writes, diagnostics, and pending behavior.

### Checkpoint 5: named and calendar spans

- add stable named-span binding;
- add calendar boundary provider;
- implement the selected 20th Century convention; and
- test stability after source extension.

Gate: a named 20th Century reads/writes the intended years, carries calendar
provenance, and does not drift.

### Checkpoint 6: things in time

- implement canonical temporal relations;
- add/update contents indexes using references;
- implement containment/overlap queries; and
- verify one long event is not copied into every time unit.

Gate: queries by thing and by span agree before and after sparse
materialization, source extension, and named-span reuse.

## Required test matrix

### Existing compatibility tests

```text
write/writeByIdx
write/negativeLast
write/openNegativeLast
read/readByIdx
read/negativeLast
read/negativeSecondLast
read/negativeAcrossSubitem
slice/select
slice/sugar
range/select1
unit/endRelative/resolveOnClose
all timeTests.tst cases
```

### New concrete-span reads

```text
read last one-item span
read last hour from seconds
read last hour from minutes
read two-unit span starting two units from end
positive and negative expressions selecting the same span
span starting at local boundary 0
span ending exactly at source extent
too-negative start
span extending beyond source end
zero-length span
```

### Nested traversal

```text
start inside nested sublist and end inside it
start inside nested sublist and end in following outer sibling
start in outer sibling and end inside nested sublist
nested start with enclosing source context boundary
same structure embedded in a larger unrelated list; selection must not escape
```

### Sparse scarcity

```text
last item in billion-unit span
last multi-item span in billion-unit span
start and end both inside one sparse span
span crossing sparse/concrete/sparse boundaries
span at first/last sparse boundary
incompatible sparse units remain unchanged
```

`sparse/pending/writeSpanAcrossSparseConcreteSparse` becomes a required
feature test rather than an accepted baseline failure once Checkpoint 3 begins.

### Writes and atomicity

```text
write one-item mapped span
write composite typed span
write nested child through selected span
write same span through stable name
RHS too short
RHS too long
incompatible RHS unit
second target mapping missing
duplicate mapped target conflict
all rejected cases leave source unchanged
```

### Pending and lifetime

```text
negative span on open source stays pending
pending span resolves when source closes
write queued before close reaches resolved source
selected result survives intersection-wrapper replacement
named span survives original query cleanup
Valgrind reports no invalid parent/source mapping access
```

### Calendar names

```text
qualified 20th Century under each supported convention
unqualified label uses only the approved default
ambiguous label without default is diagnosed
BCE/CE boundary follows the calendar's year-zero rule
named historical span remains fixed after source extension
relative last-century expression re-evaluates at a new observation boundary
```

### Things in time

```text
thing at instant
event during named century
event overlaps two adjacent named spans correctly under half-open boundaries
event starts/ends on exact boundary
query span contents returns same thing identity
long event is referenced, not copied per year
```

## Validation commands and reporting

From the repository root:

```sh
codeDog Proteus.Lib.dog
```

From `LocalBuild`:

```sh
./TestProteus -t ../timeTests.tst:
./TestProteus -t ../sparseTests.tst:
./TestProteus -t ../defaultTests.tst:read/negativeLast
./TestProteus -t ../defaultTests.tst:read/negativeSecondLast
./TestProteus -t ../defaultTests.tst:read/negativeAcrossSubitem
./TestProteus -t ../defaultTests.tst:write/negativeLast
./TestProteus -t ../defaultTests.tst:slice/select
./TestProteus -t ../defaultTests.tst:slice/sugar
./TestProteus -t ../defaultTests.tst:range/select1
./TestProteus -T 3 -t ../defaultTests.tst:
./TestProteus
```

Report dynamic-file and compiled-test counts separately. The dynamic selector
does not prove the compiled tests ran. Record known baseline failures by exact
test name and do not relabel a new feature failure as baseline.

Also run:

```sh
git diff --check
```

Use Valgrind or an equivalent memory checker for nested selection and
intersection-wrapper replacement. A passing printed result is insufficient if
the selected view reaches freed POV parents.

## Code review checklist

- No intrinsic source-global or negative index was added to `infonView`.
- `ViewMap` remains the only new POV span-origin relation; the old standalone
  source field is not restored.
- Every derived writable leaf has an exact `startPov`.
- Every view traversal has an explicit `sourcePov` stopping context.
- Composite spans are not promoted to a scalar `outerPOV`.
- Marked source mapping comes from actual traversal correspondence.
- Pending extent is distinct from rejection.
- Sparse split steps preserve total extent and following nodes.
- Multi-item write validates before mutation.
- Historical names hold resolved mappings, not live negative offsets.
- Calendar rules do not leak into generic list indexing.
- Temporal content does not shift the timeline or duplicate thing identity.
- Ownership is transferred through `retainedPOVs` at replacement boundaries.
- Positive item/slice semantics and streaming waits remain unchanged.
