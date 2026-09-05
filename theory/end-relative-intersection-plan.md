# End-relative selection through intersections

Status: approved design and staged implementation record. The engine currently
implements the R10 boundary described below; the remaining deferrals are
called out explicitly.

Source basis: local branch `proteus3b` through the R10 changes described here.

Related drafts:

- `theory/negative-index-time-design.md`
- `theory/negative-index-time-implementation-guide.md`

This document corrects one architectural assumption in the implementation
guide: end-relative selection must not be resolved by a separate index planner
before an intersection is built. The canonical semantic operation must itself
be an intersection. `#n` only constructs that operation.

## Question being answered

How should Proteus express and execute an intersection whose selected item or
span is positioned relative to the end of an ordered source, while preserving:

- ordinary intersection semantics;
- typed ordered-unit conversion;
- transparent nested traversal;
- sparse spans;
- streaming and final-boundary behavior;
- exact source mappings and writes; and
- locally zero-based `infonView` indexing?

The immediate failures are negative typed item selections such as:

```proteus
hour:{second| 1 2 3 4 5 6}#-1
day:{second| 1 2 3 4 5 6 7 8 9 10 11 12}#-1
%W.day#-1#-1#-1 = 80
```

The design must also cover explicit span selection and mapped span writes. A
fix that only makes these three shapes pass is incomplete.

## Established facts

### `#n` currently has two execution paths

`infonIO.dog` parses `#` into a `PartPath`. The semantic split is later in
`ParentMemberReasoner.resolveParts()`:

- a nonnegative numeric item index constructs an `emIntersection`;
- a negative literal item index directly calls `getEndRelativePOV()`; and
- an index with a span specification constructs a forward skip plus a marked
  `emIntersection`, regardless of whether the number was negative.

The direct negative path is the defect. It bypasses the machinery that infers
that an hour contains minutes, builds ordered-span traversal windows, splits
sparse spans, records marked matches, and returns source-backed results.

### The four new failures confirm the bypass

The positive typed cases enter `rsIntersect` and return typed minute/hour
views. Their negative equivalents never enter `rsIntersect`; the part lookup
turns into `?`. The nested negative write therefore has no usable source
correspondence.

### Existing low-level intersection syntax cannot yet express end alignment

Forward forms work:

```proteus
[& *3+{...} <&*4+{...}>] <~ source
```

Probes using an open prefix followed by a marked typed span did not select the
suffix:

```proteus
[&{...} <&*1+{minute| ...}>] <~ hour:{second| 1 2 3 4 5 6}
```

They returned `UNDEFINED`; moving the open part after the mark did not supply
end alignment either. This is not merely missing `#` lowering. The
intersection system lacks a constraint saying that a pattern boundary must
coincide with the selected source/view boundary.

### Source end is not the same as the last direct child

The source may be:

- a flat concrete list;
- a typed container stored in smaller units;
- transparent nested sublists;
- a sparse counted span;
- a view whose local item zero begins inside a nested source; or
- an open/streamed list whose end is not final yet.

The applicable boundary is the selected traversal context, not an unrelated
outer list. `POV.viewMap.sourcePov` supplies that context and
`POV.viewMap.startPov` supplies the exact source origin of a derived view.

## Invariants

1. `#n` is syntax sugar. After lowering, no matcher, navigator, selector, or
   write path may need to know that `#` was used.
2. The outer object remains an `emIntersection`; its inner value remains the
   literal pattern. Do not introduce a second negative-index evaluation mode.
3. `intersectPos` answers only which satisfied pattern item is returned:
   first, last, or marked. It must not also encode source alignment.
4. End alignment is a relation between an intersection pattern boundary and
   its source traversal boundary.
5. `infonView` remains locally zero-based. Signed indexes and source-global
   coordinates are transient request information, not intrinsic indexes.
6. End-relative resolution uses the end of the current source/view. It cannot
   escape `viewMap.sourcePov`, even when `viewMap.startPov` is nested.
7. The selected result is source-backed. Every writable local child maps to
   the exact source POV it represents.
8. A non-final end causes the intersection to wait. It must not guess the
   current end or silently produce `UNDEFINED`.
9. Typed conversions must be exact. No fractional unit is rounded.
10. Sparse work is proportional to crossed structural/span boundaries, not
    the numeric distance from the start of time.
11. Selection is read-only with respect to source meaning. If traversal needs
    sparse exposure, mutation must be separately planned and committed.
12. A composite mapped write validates the whole operation before changing
    any source item.

## Separate semantic dimensions

The following properties must remain independent in the model:

| Dimension | Examples | Purpose |
| --- | --- | --- |
| result selection | first, last, marked | chooses the returned match |
| source alignment | current/default, source-end | constrains where the pattern is positioned |
| traversal order | forward | determines logical item order |
| source context | `viewMap.sourcePov` | defines the boundary that cannot be escaped |
| local origin | `viewMap.startPov` | maps local item zero to the source |
| extent/finality | exact, pending, incompatible | determines whether alignment can resolve |

In particular, an end-aligned match does not imply reverse output order. The
selected span still reads and writes in ordinary forward order.

## Candidate designs

### A. Keep direct `getEndRelativePOV()` lookup

The current path can find scalar positions in closed, untyped lists. It cannot
represent a typed span, a marked composite selection, or a mapping for every
child in a writeable view. Extending it would create a second intersection
engine under another name.

Decision: reject as the canonical design. The helper may remain temporarily
for parity during migration, then should cease to implement `#-n`.

### B. Convert a negative index to a positive prefix before intersection

This computes `extent - offset`, then constructs today's forward intersection.
It superficially reuses the matcher, but puts semantic resolution in the path
resolver and requires it to duplicate typed-unit, sparse, nesting, finality,
and diagnostic logic.

Decision: reject as a representation. The same calculation may be used
*inside the intersection engine* as an execution plan after an end-aligned
intersection has been recognized.

### C. Give an open prefix greedy/backtracking behavior

This would try to make `[&{...} <span>]` choose a suffix without any explicit
boundary relation. It changes the meaning and search policy of existing open
patterns, introduces ambiguity when several matches exist, and can create
unbounded backtracking on streaming or sparse sources.

Decision: reject as the default meaning of an open prefix.

### D. Traverse and match the whole pattern in reverse

A reverse iterator could begin at the source end and match a reversed pattern.
It is general, but requires reverse versions of transparent traversal, sparse
splitting/exposure, ordered-unit windows, wait states, marked correspondence,
and candidate selection. It also risks accidentally reversing the returned
span.

Decision: keep as a future option only if forward normalization proves
insufficient.

### E. Add an explicit source-end boundary constraint to intersections

The intersection records that its logical end boundary must coincide with the
end of its source traversal context. Once the boundary is final, the
intersection reasoner measures the fixed suffix, locates its forward start,
and runs the existing forward matching pipeline from there.

This is generic intersection functionality. Negative `#` sugar constructs the
same object that a future low-level intersection notation will construct.

Decision: recommended for detailed design, subject to the representation and
semantics gates below.

## Recommended semantic representation

The semantic object should be expressible independently of concrete parser
syntax as:

```text
Intersection {
    pattern: ordered literal pattern
    result: first | last | marked
    boundaryConstraint: none | patternEndEqualsSourceEnd
    source: source/view constraint
}
```

`boundaryConstraint` is a placeholder name, not an approved field name. Two
concrete representations remain to compare:

1. a small mode on the outer intersection, meaningful only for
   `emIntersection`; or
2. an explicit zero-width source-boundary term in the inner pattern.

### Outer mode

Advantages:

- boundary alignment is plainly separate from consumed pattern items;
- no zero-width item is exposed to ordinary size/count/traversal code;
- the reasoner can validate a measurable suffix before scheduling work; and
- the default value preserves every existing intersection.

Costs:

- copying, printing, debugging, and parser construction must preserve it; and
- a low-level textual notation is required so `#` is not its only producer.

### Zero-width boundary term

Advantages:

- the constraint is structurally visible in the pattern; and
- more boundary predicates could later compose with patterns.

Costs:

- current list items consume extent, so count, iteration, merge, and generated
  rules would need a new zero-width category;
- nested zero-width terms complicate candidate scheduling and last-item logic;
  and
- a boundary term is not a model item and should not become selectable.

Provisional recommendation: use an outer intersection boundary mode and give
it an explicit low-level parse/print form. Do not choose the spelling until the
AST behavior is approved. This is less invasive than teaching every list path
about a zero-width pseudo-item.

## Canonical patterns

All index sugar should construct a marked intersection term. That gives item
and span selection the same correspondence path and makes the selected source
position explicit. The four canonical lowerings are:

| Sugar | Prefix before mark | Marked term | Suffix after mark | Boundary |
| --- | ---: | --- | ---: | --- |
| `#p`, `p > 0` | `p - 1` items | one logical item | none | current/default |
| `#i:span`, `i >= 0` | `i` units | `span` | none | current/default |
| `#-k`, `k > 0` | none | one logical item | `k - 1` items | source-end |
| `#-k:span` | none | `span` | semantic-dependent | source-end |

This deliberately preserves the existing distinction: positive item syntax is
one-based, while positive slice syntax uses a zero-based boundary count.
`#0` is invalid as an item, but `#0:span` begins at boundary zero.

The negative pattern is an end-aligned suffix in normal forward order. The
marked term is followed by enough fixed suffix constraint to place it relative
to the end.

For an item request `source#-k`, where `k > 0`:

```text
pattern extent = k logical units
marked target  = first logical unit in the pattern
trailing extent = k - 1 logical units
boundary constraint = pattern end equals source end
```

Thus `#-1` has a one-unit pattern and no trailing term; `#-2` has a marked
one-unit target followed by one unit. The entire pattern matches forward.

For `source#-k:span`, under the start-boundary proposal in the companion
semantic draft:

```text
selected extent = measure(span)
trailing extent = k - selected extent
require trailing extent >= 0
boundary constraint = pattern end equals source end
```

This exactly means “the span begins `k` units before the end.” If the project
instead chooses an end-boundary interpretation for negative slices, only this
lowering formula changes; the intersection boundary mechanism does not.

No concrete Proteus notation is asserted here. Conceptually, the low-level
form for `#-2` is:

```text
end-aligned-intersection [ <target> one-unit-suffix ] <~ source
```

## Measurement and alignment plan

An end-aligned intersection cannot run until its fixed suffix has an exact
measure and the applicable source boundary is final.

### 1. Identify source context

Resolve the RHS source POV used by the intersection. If it is already a
derived view, use its `viewMap.sourcePov` as the outer traversal boundary and
its local origin/extent as a narrower window. Do not climb through an unrelated
parent merely to find a convenient closed list.

### 2. Determine logical unit

- An explicit marked span supplies its ordered unit.
- An implicit item request uses the logical child unit of the typed source,
  exactly as positive `#n` currently does.
- An untyped source uses its ordinary logical item unit.
- Mixed fixed units reduce through `getOrderedSpanConverter()` to a common
  base unit.
- Variable calendar units require a calendar boundary mapper and are outside
  this first engine stage.

### 3. Measure the anchored pattern

Add an intersection-pattern measurement result with at least:

```text
exact(base unit, count)
pending(reason)
incompatible(diagnostic)
unsupportedVariablePattern(diagnostic)
```

The first implementation accepts only an exactly measurable anchored suffix:
scalars, fixed counted spans, and fixed ordered-unit definitions. An arbitrary
open or alternative-dependent suffix is not silently guessed.

### 4. Establish final source extent

Finality belongs to the selected traversal context. A typed definition with an
open pattern does not make a closed concrete instance open. Conversely, a
stream whose current items are known but whose tail remains unfinished has no
stable end.

An end-aligned intersection on a non-final context stays pending and retains
the intersection/source POVs through existing ownership mechanisms. It does
not create a guessed source mapping.

### 5. Locate the forward start boundary

Compute:

```text
startBaseOffset = sourceBaseExtent - anchoredPatternBaseExtent
```

Require a nonnegative, exactly aligned result. Locate it by walking structural
segments and subtracting their measured extents. A sparse counted span is
crossed arithmetically. Transparent nesting is entered and exited within the
same source context. A derived source starts at its `viewMap.startPov` and
cannot cross its local extent or `viewMap.sourcePov` boundary.

This phase returns a traversal plan. It must not mutate a sparse source merely
to discover a read position.

### 6. Run ordinary forward intersection processing

Start the existing correspondence/matching pipeline at the located boundary,
limit it to the source/view end, and match the pattern in forward order.
Ordered-span windows continue to build typed minute/hour/day views. Ordinary
constraint mismatches reject the intersection; they do not trigger a search
at earlier positions because the end constraint fixed the alignment.

## Result correspondence and `ViewMap`

The matcher must record actual traversal correspondence, not reconstruct it
later from type equality.

For every satisfied LHS pattern POV, record the RHS POV/window that satisfied
it. For the marked result:

- a scalar maps to its exact source POV;
- a flat span head maps to `(sourceContext, firstSourcePOV)`;
- every flat child maps to its exact source child;
- a recursive typed span applies the same rule at each level; and
- a sparse view maps to planned/symbolic source boundaries until a write
  requires materialization.

The selected `infonView` remains a local, zero-based view. `ViewMap` belongs on
its POVs:

```text
viewMap.sourcePov = traversal context and stopping boundary
viewMap.startPov  = exact source item corresponding to local item 0
```

The current sparse-only marked mapping cases should be replaced by this
general correspondence rule. Type compatibility may validate a match, but it
cannot identify which of several same-typed source positions was matched.

`IntersectionReasoner.installSelectedResult()` must continue transferring
`retainedPOVs` when it replaces the wrapper.

## Writes

### Scalar and chained writes

Once an end-aligned selection returns an exact mapped scalar, ordinary copy or
merge semantics must target `viewMap.startPov`. The chain
`%W.day#-1#-1#-1=80` should therefore work because each selected typed view
preserves the next source context/origin.

### Composite span writes

A span write is not repeated assignment to synthetic view children. Before
mutation, build a plan containing every local-to-source leaf pair and any
required sparse boundary materializations.

Validation must establish:

- exact extent and ordered-unit compatibility;
- exact, still-reachable source mappings;
- no target outside the selected half-open span;
- compatible RHS structure; and
- no conflicting duplicate source target.

Only after all validation succeeds may sparse boundaries be materialized and
ordinary source merges be committed. If detached replacement construction
cannot guarantee all-or-nothing publication, multi-item writes must remain a
later checkpoint rather than claiming atomicity prematurely.

## Sparse sources

Read selection should use a symbolic traversal plan:

```text
sparse segment + local base offset + requested extent
```

It should not expand or permanently split a sparse segment just to return a
view. A write may need to materialize the target boundaries. That operation
must preserve:

```text
prefix count + selected count + suffix count = original count
```

and preserve the coordinates of every following source location.

Existing `splitSparseSpanPrefix()` mutates immediately. It is suitable only as
a commit primitive after a separate validation plan exists, not as a locator.

## Streaming and wait behavior

An end-relative expression needs an observation boundary:

- closed finite source: resolve now;
- open source that later closes: wait, then resolve once;
- intentionally live/unbounded source: remain relative/pending until an
  explicit snapshot or observation boundary is supplied.

The wait state belongs to intersection processing. The old virtual scalar
created by `getEndRelativePOV()` must not become the state carrier for typed or
composite end-aligned requests.

The implementation must distinguish:

- waiting for more items within a currently open list;
- waiting for the selected source boundary to become final; and
- a final source whose anchored pattern is out of range or incompatible.

Only the last case is a terminal mismatch/diagnostic.

## Case matrix

Every row must be covered by either an acceptance test or an explicit staged
deferral before implementation is called complete.

| Axis | Cases |
| --- | --- |
| request | item, one-unit span, multi-unit span |
| origin | positive/start-relative, negative/end-relative |
| source shape | flat, transparent nested, typed nested, view-of-view |
| storage unit | same unit, finer unit, coarser nested unit |
| source extent | closed, closes later, permanently live/snapshot |
| representation | concrete, sparse, mixed concrete+sparse |
| result | read scalar, read composite, chained selection |
| mutation | scalar write, nested scalar write, whole-span write |
| validity | zero, out of range, incompatible unit, nonintegral conversion |
| matching | satisfied, content mismatch, alternatives, same-typed positions |
| lifetime | immediate use, retained/named result, wrapper replacement |

Important concrete cases include:

- `#-1`, `#-2`, and `#-extent` on a flat closed list;
- an out-of-range `#-(extent+1)`;
- a negative selection whose local origin is in a nested sublist but whose
  suffix crosses back into its enclosing source context;
- a view-of-view whose end is the inner view end, not the ultimate source end;
- day stored in seconds selecting its final hour;
- day stored in minutes selecting its final hour;
- exact and nonexact unit conversions;
- a billion-unit sparse prefix with a selected suffix;
- an open source before and after closure;
- several same-typed source units, proving correspondence comes from the
  actual matched position;
- a marked multi-item span read and mapped write; and
- source extension after a resolved stable selection, proving that the
  resolved view does not drift.

## Implementation checkpoints

Each checkpoint ends with tests and review. Do not combine all checkpoints in
one change.

### Checkpoint 0: settle the two blocking semantics

Approve or revise:

1. whether `#-k:span` names the span start `k` units before the end; and
2. whether the low-level representation is an outer intersection boundary
   mode or a zero-width boundary term.

The century convention, temporal-relation authority, and historical naming
syntax do not block the generic intersection work and should remain separate.

### Checkpoint 1: represent and round-trip an anchored intersection

- add the chosen semantic representation;
- add a low-level parse/print form independent of `#`;
- preserve it through intersection construction and copying;
- expose it in diagnostic graph/string output; and
- prove that default intersections round-trip unchanged.

No `#-n` lowering changes yet.

### Checkpoint 2: measure and wait without selecting

- measure fixed anchored patterns through ordered-unit definitions;
- identify the correct source/view context and finality;
- produce resolved, pending, incompatible, and out-of-range plans; and
- add flat, typed, nested, sparse, and open-source plan tests.

The source remains unmodified.

### Checkpoint 3: execute low-level anchored intersections

- locate the forward start boundary;
- reuse ordinary forward ordered-span matching;
- limit traversal to the selected source/view boundary;
- record exact LHS-to-RHS correspondence; and
- pass low-level read tests without using `#`.

This is the proof that the feature belongs to intersections.

### Checkpoint 4: return fully mapped views

- populate `ViewMap` for scalar, flat, and recursive typed views;
- replace sparse-only marked-result special cases with correspondence-based
  mapping;
- preserve retained POV lifetimes across selected-result installation; and
- test view-of-view and nested traversal stop conditions.

### Checkpoint 5: lower `#` sugar

- make positive and negative item/span forms call one intersection builder;
- remove the direct negative `getEndRelativePOV()` branch from `#` handling;
- use the approved negative-span formula; and
- prove sugar/non-sugar equivalence from normalized behavior, not just output.

The existing positive `#n` and positive slice suites must remain green.

Implemented through R10 for negative item indexes, single-unit typed spans,
fixed repeated typed spans, and finer-unit traversal across concrete nested
source children. Compact `#` lowering and the transparent low-level
intersection form share the same composite mapping and write behavior, even
when a selected span crosses an outer child boundary. The resulting
`infonView` remains locally zero-based; its root `ViewMap` retains the outer
source as the traversal boundary and the first nested unit as its origin.

Finer positions implied only by a coarse sparse count remain a later
extension. Without concrete nested children, the engine rejects the finer
coordinate instead of materializing or inventing those positions. Variable
calendar-unit conversion also remains out of scope.

### Checkpoint 6: writes

- generalize source-backed scalar detection to `ViewMap`;
- pass chained negative scalar writes;
- add validated local-to-source pair planning for composite spans;
- add sparse materialization only at commit; and
- prove failed composite writes leave the source unchanged.

### Checkpoint 7: regression and scarcity validation

- run all 20 time tests, then the complete `LocalBuild/TestProteus` suite;
- add open/close lifetime tests and crash-protected variants;
- instrument sparse work to show structural rather than numeric scaling;
- inspect generated `LocalBuild/TestProteus.cpp` if generated dispatch or
  copied fields changed; and
- run `git diff --check`.

## Likely code ownership

The exact names may change, but responsibilities should remain narrow:

- `infonIO.dog`: low-level anchor syntax, parse, and print only;
- `Proteus.Lib.dog`: intersection representation/default/copy support only;
- `ParentMemberReasoner`: lower every `#` form into a canonical intersection;
- `IntersectionReasoner`: own boundary-plan lifecycle, waiting, and result
  selection/installation;
- `OrderedSpanReasoner`: exact unit measurement and forward traversal windows;
- POV/list navigation: locate boundaries within a supplied traversal context;
- propagation correspondence: record the actual matched source POVs;
- reference consolidation: validate and commit mapped writes; and
- debug/tests: make anchor, context, origin, and wait reason observable.

Do not put end-relative semantics in the parser, printer,
`selectFunctionResult()`, or a source-backed write predicate alone. Those are
construction, presentation, final selection, and execution boundaries; none
has enough information to own the operation.

## Diagnostics

The plan must yield distinct messages/states for:

- source end not final;
- anchored pattern larger than the source/view;
- incompatible ordered units;
- nonintegral unit conversion;
- variable or unmeasurable anchored suffix;
- selected source mapping no longer reachable; and
- composite write shape mismatch.

A normal pending end-relative expression must not be printed as `UNDEFINED`.
An actual final mismatch must not remain pending forever.

## Implemented direction

Candidate E is implemented: an ordinary `emIntersection` carries the
source-end request as a negative size and resolves it into a forward traversal
window. `#-n` lowers to that representation rather than owning separate
navigation semantics.

The settled implementation choices are:

1. parenthesized negative intersection size is the low-level textual form;
2. fixed anchored patterns are supported, while variable-length anchored
   suffixes remain deferred rather than introducing backtracking; and
3. composite span writes are validated and committed atomically.
