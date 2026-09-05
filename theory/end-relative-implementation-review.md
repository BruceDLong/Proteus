# End-relative intersection implementation review

Status: representation decisions approved on 2026-09-04. R1 characterization,
R2 extent/availability separation, the R3 traversal plan, and R4 accepted-match
correspondence recording are complete. R5 now gives every ordered-span result
one local, recursively source-mapped construction contract. R6 now validates
and commits mapped writes transactionally. R7 now removes the remaining
compatibility inference from result selection and work transfer. No negative
`#` sugar has been added.

Source basis: local branch `proteus3b` through the R7 changes described here.

Related documents:

- `theory/end-relative-intersection-plan.md`
- `theory/negative-index-time-design.md`
- `theory/negative-index-time-implementation-guide.md`

## Executive assessment

The current code is a successful vertical prototype. It proves that a
negative-sized intersection can locate an end-relative source window, run the
ordinary forward intersection machinery, return typed time spans, and write a
selected composite span back to its source.

It should not yet be treated as the final architecture. Several passing paths
depend on conventions reconstructed in later phases rather than facts recorded
at the point where they become known. Those conventions will become difficult
to maintain when negative `#` sugar, open streams, view-of-view selection,
sparse writes, stable named spans, and calendar boundaries are added.

The implementation now has one explicit intersection traversal plan beside the
existing projection path, records accepted correspondence, and constructs
positive, negative, direct, recursive, and sparse ordered-span views under the
same local mapping rules. Mapped assignments now build and validate a complete
operation before publishing any source changes. Result selection now consumes
explicit mapping and work-role facts rather than reconstructing them from field
equality or mapping shape.

## Current verified boundary

The regenerated `LocalBuild/TestProteus` executable currently demonstrates:

- parsing and printing parenthesized negative sizes such as `*(-1)`;
- selection of the final item and earlier end-relative items;
- forward selection across a transparent nested sublist;
- selection of an end-relative multi-item span;
- selection of a final `minute` from seconds;
- selection of a final `hour` from seconds;
- writing a selected final `minute` through to `%W.day`;
- explicit resolved, pending, and rejected traversal plans owned by merge
  AItems;
- retention and parser-revision wake-up of pending end-relative merges;
- exact pending coordinates without guessed source values;
- stable nested and derived-view traversal boundaries;
- rejection plans for out-of-range, incompatible-unit, and nonintegral-unit
  requests;
- direct logical lookup of typed sparse root positions without source
  materialization;
- accepted-match correspondence records for scalar, flat, recursive typed,
  and sparse results;
- complete `ViewMap`s on selected end-relative composites and their writable
  descendants;
- local copies with complete recursive `ViewMap`s for positive, negative,
  direct, recursive typed, and sparse ordered-span views;
- view-of-view mappings whose source context is the immediate source view, so
  traversal cannot escape that boundary;
- separation of mapping provenance from write authority: a complete `ViewMap`
  alone does not permit source mutation;
- recursive mapped-write planning with structure, target reachability,
  compatibility, and duplicate-target validation before mutation;
- coalescing of equivalent duplicate writes and rejection of conflicting
  duplicate writes;
- sparse-write preflight, order-independent descending publication, and
  rejection of invalidated sparse plans before any position is exposed;
- sparse span selection and write-back across sparse/concrete/sparse
  boundaries;
- source completion through general `countSize(true)` reasoning rather than a
  mapped-assignment tail toggle;
- end-relative marked selection without selection-time first-child mapping
  recovery;
- explicit selector-local versus consumer work classification and
  consumer-only result transfer;
- valid mapped scalar behavior when `sourcePov` and `startPov` are equal;
- preservation of the established positive nested write; and
- preservation of `range/select1`.

The ordinary and protected full runs report `8/367`. Relative to the R5
`9/357` checkpoint, all seven compiled mapped-write tests and all three R7
result-selection tests pass, and the former
`sparse/pending/writeSpanAcrossSparseConcreteSparse` baseline failure now
passes. The remaining failures are the four older baseline failures plus the
four intentionally unsupported negative `#` sugar tests. This is a regression
checkpoint, not a claim that the full suite is clean.

## R1 characterization baseline

`endRelativeR1Tests.tst` is intentionally not included by `defaultTests.tst`.
It records the approved contracts without changing the established full-suite
baseline. Run it from `LocalBuild` with:

```sh
./TestProteus -T 3 -t ../endRelativeR1Tests.tst:
```

The current implementation reports `4/10` failures.

Passing contracts:

- a closed concrete list selects its final item;
- an exact sparse extent locates its final logical position even while that
  position's value remains unknown;
- a marked term after the first pattern item returns the correct final item;
- a content-constrained typed match selects the actual final compatible span;
- a short composite RHS rejects before changing any source value; and
- a recursively incompatible RHS rejects before changing any source value.

Recorded gaps after R3:

- out-of-range, incompatible-unit, and nonintegral-unit requests now produce
  rejected traversal plans, but enclosing intersection result installation
  still prints a residual pattern instead of `UNDEFINED`;
- a large sparse suffix avoids expanding the million-item source prefix but
  returns a local 60-position view rather than the requested compressed sparse
  `minute` view.

The test file also lists lifecycle cases that cannot be characterized by a
final normalized string. R2 added compiled assertions for pending open sources,
exact-coordinate/content-pending separation, derived-view stop boundaries, and
stable resolved views after ultimate-source extension. Post-wrapper lifetime
is covered by the retained local-view ownership added in R4. Duplicate-target
behavior is now covered by R6 compiled tests.

## R2 implementation record

R2 replaces the ambiguous `endRelativeExtentIsFinal()` Boolean with three
typed results while leaving merge control flow on the existing projection
path:

- `EndRelativeExtentResult` distinguishes `exact`,
  `unknown-or-unbounded`, and `incompatible` and carries the exact extent;
- `EndRelativeWindowAvailability` independently distinguishes `available`,
  `pending`, `out-of-range`, and `incompatible` for one offset and item count;
  and
- `EndRelativePatternMeasureResult` measures every fixed pattern position in
  source logical units and reports `resolved`, `pending`, invalid request,
  incompatible unit, nonintegral unit, or extent overflow.

The old extent helper's callers now have explicit meanings:

- scalar end-relative references ask only whether an exact ordinal exists;
  a closed `-1` still uses the authoritative concrete tail, while an exact but
  unfinished list uses logical ordinal traversal;
- retained-reference scans run only when the extent is exact;
- end-relative span construction requires the complete requested window to be
  available and retains the POV-local boundary;
- the current mapped-write tail completion check asks only for exact extent;
  that legacy completion action is still scheduled for removal in R6; and
- negative-intersection preparation consumes the complete-pattern measurement
  only when it is resolved. At the R2 checkpoint, pending and rejection statuses
  were intentionally not yet merge lifecycle decisions; R3 gives those states
  an owning traversal plan.

At the R2 checkpoint, the compiled `unit/endRelative/` group passed `11/11`.
It covered closed, pending, open, incompatible, and out-of-range extent/window
states; stable derived-view boundaries; and resolved, pending, incompatible,
and nonintegral complete-pattern measurements. The isolated R1 contract suite
remained `5/10`, and the protected full suite remained at its expected `9/338`
baseline.

## R3 implementation record

R3 introduces `IntersectionTraversalPlan` without replacing the existing
forward matcher or changing `infonView` indexing. The merge AItem receives the
plan before a negative request can be rewritten as a positive local window.
The plan records:

- source-start versus source-end origin;
- exact, unknown, or incompatible source extent;
- resolved, pending, out-of-range, or incompatible window content;
- pattern and source-window logical extents plus traversal unit;
- the source traversal context, source start POV, and local zero-based
  projection; and
- the source/view dependencies needed for the plan's lifetime.

Resolved plans construct the existing local projection and assert that its
`ViewMap.sourcePov` and `ViewMap.startPov` equal the context and start selected
by the planner. Pending plans leave the negative request and original RHS
unchanged. Rejected plans set the merge rejection state without constructing a
projection.

Pending ownership is explicit: `Agenda` retains the merge AItem, compares the
plan's observed parser revision with the current revision, and re-enqueues the
same AItem at merge step 1 when input advances. Any other enqueue transfers the
AItem out of the pending list first, preventing duplicate ownership. A static
normalization with no advancing parser exits without a retry loop.

R3 also distinguishes an intrinsically counted sparse value from whether its
POV is transparent in an outer list. Direct logical indexing can therefore
produce canonical implicit positions for a typed sparse root while outer-list
flattening still requires `isSubItm`. This keeps sparse end-relative lookup
proportional to the selected window and does not materialize source items.

One test-fixture gotcha was confirmed: assigning an integer literal directly
to `FlexNum` generates a C++ conversion that selects the `int` formatting
constructor, so the numeric value remains zero. R3 tests assign through an
explicit `int64` value, matching the normalized engine state. This is a
CodeDog/FlexNum overload issue, not an end-relative semantic exception.

The regenerated build passes `unit/endRelative/` at `20/20`, including nine R3
plan/lifecycle assertions. The isolated R1 suite remains at its approved `5/10`
checkpoint, `timeTests.tst` remains at `4/23` for the unsupported negative `#`
sugar cases, `range/select1` passes, and the protected full suite remains at
`9/347`.

## R4 implementation record

R4 adds `IntersectionCorrespondence` records to the resolved traversal plan.
Each record retains the pattern POV, the local matched POV, and the exact
source context/start pair. The merge AItem still owns the plan; descendant
merge work and subscription continuations borrow that same plan so the record
is available at the merge that actually accepts a POV pair.

`IntersectionReasoner.recordAcceptedCorrespondence()` is the single write
point. It runs only after a merge reaches `msAccept`, maps the accepted pattern
and local POV through `POV.mapViewToSource()`, and recursively supplies mappings
for selected composite descendants. Repeated acceptance handling is harmless:
the plan deduplicates an identical pattern/local pair and retains every POV
needed by the correspondence.

End-relative propagation no longer treats candidate pairing as proof by calling
the ordinary passive-scalar mapping helper. Its marked-result selection also no
longer reconstructs a composite mapping from the first selected child. Ordinary
positive intersections retain both compatibility paths until R5 gives their
views the same local mapped construction contract; this preserves existing
marked-range behavior without mixing it into the new end-relative path.

Five compiled R4 cases cover acceptance timing/deduplication and scalar, flat,
recursive typed, and non-materialized sparse mappings. The regenerated build
passes all 25 compiled `unit/endRelative/` cases. The isolated R1 suite remains
at `5/10`, `timeTests.tst` remains at `4/23`, `range/select1` and
`marked/lookahead` pass, and the protected full suite remains at its approved
`9/352` result.

## R5 implementation record

R5 gives all ordered-span builders the same representation. Each accepts an
explicit source traversal context, constructs a locally owned root and
descendants, and recursively maps those POVs back to their corresponding
source POVs. Positive sibling selection, negative end-relative selection,
direct same-unit selection, recursive typed conversion, and sparse logical
positions no longer choose between shared and copied source items according to
how the source was reached.

View-of-view construction deliberately preserves its immediate boundary. Its
`ViewMap.sourcePov` is the source view, and its `startPov` is the matching local
item inside that view. Following mappings to an ultimate source is a separate
navigation operation. Mapped local copies do not use `outerPOV`; that field
remains available for its existing alternative/validation role.

R5 also separates provenance from permission. A complete `ViewMap` says where
a local result came from, but does not by itself authorize source mutation.
Final selection grants `viewMapWriteEnabled` only when the selected result has
pending consumer work. Reference consolidation follows a mapping only when
that authority is present, so ordinary mapped reads continue to refine and
return their local projections. R7 replaces the former work-list proxy with an
explicit consumer-work classification.

During validation, traversal was found to leave inherited `cstListSpec` work
on a source item immediately before replacing it with a local projection. The
local builder already materializes that represented type shape. It therefore
consumes that specific seeded source work after mapping the local copy, without
moving unrelated work or resetting the source's copy guard. This prevents a
later source-side normalization from replaying work already represented by the
view.

Five compiled R5 cases cover positive flat and recursive views, direct views,
view-of-view boundaries, and non-materialized sparse views. The regenerated
build passes those `5/5`, all `25/25` compiled `unit/endRelative/` cases,
`range/select1`, and `marked/lookahead`. The isolated R1 suite remains at its
approved `5/10` checkpoint, `timeTests.tst` remains at `4/23` for unsupported
negative `#` sugar, and the protected full suite is `9/357`: the five known
baseline failures plus those four sugar cases.

## Foundations worth keeping

### Negative size is the low-level request

For the current approved low-level syntax, a negative intersection size means
that the pattern begins relative to the source end:

```proteus
*(-2)+[<_> _] <~ {1 2 3 4}
```

The pattern still matches forward. Its negative size determines an
end-relative origin; it does not make local indexing negative or reverse the
result.

### `ViewMap` owns view provenance

`POV.viewMap.sourcePov` is the source traversal context and stopping boundary.
`POV.viewMap.startPov` is the source item corresponding to local item zero.
The selected `infonView` remains locally zero-based.

This separation is the correct foundation for nested sources, view-of-view
selection, stable named spans, and writes.

### Implicit logical positions preserve sparse structure

`ImplicitItemMap` gives positions inside a counted sparse span stable POV
identity without physically expanding the source list. Canonical implicit POVs
are retained by the sparse source infon. This is preferable to creating a
different temporary POV on each lookup or materializing every preceding item.

### Selection uses local projections

`POV.makeEndRelativeSpanView()` copies source values into a local projection
for matching. Matching can refine that projection without changing the source.
A later assignment reaches the source only through `ViewMap`.

### Existing lifetime ownership is reused

`IntersectionReasoner.installSelectedResult()` transfers retained POVs from the
intersection wrapper to the selected result. This is a necessary ownership
rule, not an end-relative special case.

## Current execution path

The current path is spread across several layers:

1. `MergeReasoner.processStep1()` asks
   `prepareEndRelativeIntersectionSourceView()` to examine the LHS and RHS.
2. `OrderedSpanReasoner.endRelativePatternSourceItemCount()` converts the
   negative pattern count to a count in source logical items when a typed unit
   such as `minute` or `hour` is requested.
3. `IntersectionReasoner.prepareEndRelativeSourceView()` creates an
   end-relative local source projection, converts the negative pattern size to
   a positive local cardinality, and substitutes the projection as the RHS.
4. Ordinary forward intersection propagation matches against the projection.
5. When a descendant merge accepts, `recordAcceptedCorrespondence()` records
   its exact pattern/local/source relation and populates the selected mapping.
6. `IntersectionReasoner.selectFunctionResult()` returns the already mapped
   marked result. Unified builders supply positive and negative maps directly;
   selection performs no first-child mapping recovery.
7. Final result selection grants explicit mapped-write authority only when an
   explicitly classified consumer work item is present; a passive mapped read
   remains local.
8. `ReferenceConsolidationStrategy` recognizes the authorized mapped result
   and writes scalar or flattened composite values to the mapped source POVs.

The traversal plan now states the end-relative operation and its accepted
correspondences. View construction, work ownership, and mapped-write validation
now have explicit representation boundaries.

## Gotcha register

### G1: resolved by treating every complete `ViewMap` as valid

`WorldManager.carryPassiveScalarSource()` populates `ViewMap` for an ordinary
passive scalar and may set `sourcePov == startPov`. Before R7, other code used
equality or inequality of the two fields to distinguish ordinary identity from
a derived span.

Populating the map is not itself wrong. The former `POV.sourcePOV` field was
passive correspondence metadata, and moving that relationship into `ViewMap`
means an LHS scalar matched to an RHS source scalar is a derived/mapped POV. A
scalar mapping may naturally have the same POV as both its traversal context
and local item zero.

Risks:

- equality is a valid mapping state, especially for a scalar context;
- traversal provenance is being used as semantic-identity evidence; and
- several callers must repeat the same undocumented category test.

Desired invariant:

```text
ViewMap is populated when a POV is derived from or corresponds to a source
POV, including a passive scalar match.
A POV with no derived/source correspondence leaves both fields NULL.
Mapped source-backed behavior tests ViewMap validity, not field equality.
Operation role is represented independently of traversal provenance.
```

R7 removes every semantic comparison between `ViewMap.sourcePov` and
`ViewMap.startPov`. Result selection copies any complete map, and consolidation
priority is selected by `viewMapWriteEnabled` rather than by map shape. An equal
source and start remains an ordinary valid scalar mapping.

### G2: the request is destructively normalized before matching

`prepareEndRelativeSourceView()` changes the negative size to a positive local
cardinality after it creates the source projection. That makes the existing
matcher usable, but the resulting intersection no longer carries its original
end-relative constraint.

Risks:

- retries, diagnostics, and graph inspection cannot see why the source began
  at that position;
- a copied or retained normalized expression can lose the source-end relation;
- pending versus resolved behavior depends on exactly when the mutation occurs;
  and
- it is difficult to assert that the source window still matches the request.

Desired invariant: keep the negative size as request information until a
resolved traversal plan exists. The matcher may consume a positive local
window, but the plan should retain the relation that produced it.

### G3: typed measurement inspects only the first pattern and source items

`endRelativePatternSourceItemCount()` determines the requested unit from the
first pattern POV and measures it against the first source logical item. It
then multiplies that conversion by the absolute negative count.

This is sufficient for the current `*(-1)+[<minute>]` and
`*(-1)+[<hour>]` cases. It is not yet a measurement of an arbitrary anchored
intersection pattern.

Risks include mixed fixed units, a marked term that is not first, alternative
patterns, nonintegral conversions, and view-local source units that differ from
the ultimate source's first item.

Desired invariant: measure the complete fixed anchored pattern in one selected
base unit and return `resolved`, `pending`, or a specific rejection.

### G4: known end position is conflated with content completion

`infonView.endRelativeExtentIsFinal()` now treats any literal list size as a
usable end extent, even when `tailUnfinished` is true. Runtime inspection of
the `%W.day` write case showed a parsed source with `streamState=parseDone`,
`sizeMode=fromCount`, literal size 12, and 12 concrete items, while
`tailUnfinished` was still true after definition-shape inheritance.

The literal size is mathematically enough to locate an end-relative position.
The tail flag instead describes whether more value structure may need to be
constructed or matched. The current helper's name and callers conflate those
two facts.

Risks:

- a caller may treat a known coordinate as proof that selected contents are
  already available;
- a caller may unnecessarily wait for stream closure even when an exact size
  fixes the logical end; and
- source representation artifacts can determine whether a list looks
  complete.

Desired invariant: use two independent results:

```text
extent: exact | unknown-or-unbounded | incompatible
contents at requested window: available | pending | incompatible
```

An exact literal size may establish the end coordinate while contents remain
pending. A live source without an exact size cannot resolve an end-relative
coordinate. Both queries must be about the selected source/view context, not
an unrelated outer list.

### G5: ordered-span construction is now unified

R5 removes the construction split. Positive sibling spans, negative
end-relative spans, direct same-unit spans, recursive typed spans, and sparse
positions all return local copies. The root and every copied descendant have a
complete `ViewMap`; no mapped local copy uses `outerPOV` as provenance.

Every builder receives its source context explicitly. A view built from
another view maps to that immediate view, rather than silently flattening to
the ultimate source. This makes the traversal stop boundary part of the
constructed graph and preserves local zero-based indexing at every layer.

One traversal interaction required an explicit ownership rule. Logical
traversal may seed inherited `cstListSpec` work on a source item before the
local replacement is built. Because the local copy materializes that same type
shape, construction consumes only that represented seed from the source while
leaving persistent source work intact.

### G6: resolved by consuming correspondence recorded before selection

R4 resolves this for end-relative traversal plans, and R5 builders supply the
positive mappings directly. R7 removes the last result-selection recovery
branch.

`IntersectionReasoner.selectFunctionResult()` now accepts only the mapping on
the selected marked POV. A mapped first child cannot cause an unmapped
composite root to acquire inferred provenance.

Risks:

- the first child may not identify the selected composite's full context;
- same-typed positions can be confused if the actual match is not recorded;
- scalar, wrapper, sparse, and composite cases need separate recovery logic;
  and
- result selection now owns source-correspondence semantics.

Desired invariant: when a pattern POV is satisfied by a source POV/window, the
matching/correspondence layer records that exact relation. Selection merely
returns the already mapped marked result.

### G7: resolved with explicit work-record ownership

An ordinary marked range clears selector-local work rather than projecting it
outward. A mapped span must retain outside assignment work. R7 records this
distinction directly on each work POV with `selectorLocalWork`.

Risks:

- the test depends on G1's equality convention;
- a legitimate scalar or narrow view may choose the wrong branch;
- mapping provenance and work ownership are different concepts; and
- another view category could silently inherit selector-local work.

Desired invariant: work records state whether they belong to the selector or
to the expression consuming the selected result. Selection transfers only the
consumer work, independent of source mapping shape.

Parser-created `=` work is consumer work. `<~` feeds, generated path/index
selector sources, propagated matching constraints, intersection alternatives,
and marked list-spec outer feeds are selector-local. Result selection discards
selector-local work already present on the selected value and transfers only
consumer work from the intersection wrapper.

### G8: resolved in R6 by a mapped-write transaction

`MappedSpanWritePlan` now recursively records each local view, resolved source
target, RHS assertion, and target metadata without mutating the source. Planning
rejects incompatible recursive shape, incompatible scalar or ordered-unit
types, targets outside the declared source context, incomplete mappings, and
conflicting duplicate targets. Equivalent duplicate assertions coalesce into
one source write.

Sparse targets remain canonical implicit positions during planning. Commit
preflights every sparse target against the unchanged source and publishes them
in descending logical-index order, independent of pair traversal order. A
stale sparse plan rejects before exposing any position. After publication, all
pairs use the same metadata-preserving scalar copy operation used by direct
source-backed writes.

### G9: resolved in R6 through general completion reasoning

Mapped assignment no longer toggles `tailUnfinished`. After mapped leaf commits,
the existing `countSize(true)` list reasoning is invoked on affected ancestors
and the source context. An exact list can therefore close when its ordinary
size constraints are satisfied, while assignment itself does not declare a
live source complete.

### G10: logical navigation mutates parent metadata during reads

The logical sparse/nested navigation helpers assign `pParent` while locating
or crossing positions. They do not expand the physical list, but navigation is
not entirely observational.

Risks:

- the same POV reused in another traversal context may retain the latest
  parent;
- concurrent or nested traversal can make parent provenance order-dependent;
  and
- a future stable named view may depend on metadata installed by a prior read.

Desired invariant: canonical source POVs have stable structural parents.
Traversal-specific ascent state belongs to the traversal plan or iterator.

### G11: lifetime correctness is distributed

Canonical implicit POVs are retained by their source infon, end-relative views
retain source positions, and selected results inherit the intersection's
retained POVs. These are individually sensible, but there is no single
statement of which object owns an entire selected mapping graph.

Risks:

- a new view builder may omit one retention edge;
- non-owning `pParent` or `outerPOV` links can outlive their owners;
- layout-sensitive crashes can reappear far from the missing edge; and
- retained graphs can grow without a clear release boundary.

Desired invariant: the selected result owns one resolved traversal/mapping
plan or an explicit set of all POVs required by that mapping. Installation
transfers that ownership exactly once.

## Proposed simplification

Introduce one temporary reasoning record, provisionally named
`IntersectionTraversalPlan`. It is not an `infonView` and does not change local
indexing.

Conceptually:

```text
IntersectionTraversalPlan {
    status: not-applicable | pending | resolved | rejected
    origin: source-start | source-end
    extentStatus: exact | unknown-or-unbounded | incompatible
    windowContentStatus: available | pending | incompatible
    sourceContextPov
    sourceStartPov
    sourceLogicalExtent
    patternLogicalExtent
    traversalUnit
    localSourceView
    correspondences[]
    retainedDependencies[]
    diagnostic
}

IntersectionCorrespondence {
    patternPov
    localViewPov
    sourceContextPov
    sourceStartPov
}
```

The plan should be owned by the intersection AItem while work is pending. If a
resolved plan can always be recomputed cheaply and deterministically, only the
retained source dependencies need to persist; that should be demonstrated
rather than assumed.

### Simplified pipeline

1. Recognize a negative-sized intersection without changing its size.
2. Ask one planner to determine the applicable source context, exact extent,
   requested-content availability, unit, measured pattern extent, and forward
   source start.
3. Return `pending` without guessing if the source extent is unknown. If the
   extent is exact but requested contents are unavailable, retain the resolved
   coordinate and wait for those contents.
4. Build one locally zero-based source view as an adapter for the existing
   forward matcher. The plan, rather than the substituted RHS alone, records
   why and where that view exists.
5. As propagation satisfies pattern POVs, record their exact local/source
   correspondences in the plan and populate `ViewMap` through one helper.
6. Return the already mapped marked result. `selectFunctionResult()` performs
   no first-child or type-based mapping recovery.
7. Classify selector-local and consumer work explicitly, then transfer only
   the consumer work to the selected result.
8. For assignment, derive a `MappedSpanWritePlan` from the mapped result,
   validate it completely, and commit it through the established scalar
   source-backed write operation.
9. Let ordinary list normalization re-evaluate completion after mapped merges;
   do not toggle source completion inside the mapped-write operation.

This may add one explicit record initially, but it should remove conditions
from `MergeReasoner`, `selectFunctionResult()`, passive-source propagation,
ordered-span view construction, and reference consolidation.

## Recommended representation decisions

The following decisions were approved on 2026-09-04:

1. **`ViewMap` validity:** `sourcePov != NULL && startPov != NULL` means the POV
   is derived from or corresponds to a source POV. Passive scalar matches may
   populate `ViewMap`; POVs with no source correspondence leave it empty.
2. **Mapped scalar validity:** a mapped scalar remains valid even if
   `sourcePov === startPov`; equality never classifies the mapping.
3. **Extent versus contents:** an exact literal extent fixes the end coordinate
   even if contents remain pending. Track exact extent separately from window
   content availability and structural/stream completion.
4. **Correspondence:** map every selected composite node and every writable
   leaf at the moment of actual matching.
5. **Write semantics:** recursively validate compatible structure and apply
   the established scalar source-backed write operation at mapped targets;
   leaf count alone is insufficient.
6. **Work ownership:** identify selector-local versus consumer work directly,
   not through view provenance.
7. **Source completion:** mapped assignment does not directly close a source.
   Ordinary list normalization may recognize completion after mapped merges
   satisfy all exact positions.

## Safe migration checkpoints

### Checkpoint R0: preserve the working behavior

- Keep the current source changes intact while the redesign is reviewed.
- Retain the current focused tests as behavioral evidence.
- Do not add negative `#` lowering or named-span behavior yet.

### Checkpoint R1: add characterization tests

Before moving code, cover:

- a closed list, an exact-sized list with pending contents, and a live list
  without an exact extent;
- resolution before and after stream closure;
- a view whose end differs from its ultimate source end;
- multiple same-typed candidate spans;
- a negative pattern with the marked term after its first item;
- an out-of-range negative extent;
- a nonintegral or incompatible unit conversion;
- a failed composite write that leaves every source value unchanged;
- duplicate mapped targets;
- a sparse source large enough to prove non-materializing traversal; and
- a selected/named view used after its original intersection wrapper is gone.

### Checkpoint R2: separate extent, availability, and completion

- Complete on 2026-09-04.
- Added separate exact-extent and requested-content availability results without
  changing callers.
- Characterized every former `endRelativeExtentIsFinal()` caller.
- Replaced the helper so a known coordinate is not described as a
  fully final source.
- Distinguished exact-sized pending values from live sources with no exact end.
- Added complete fixed-pattern measurement returning resolved, pending, or a
  specific rejection.

### Checkpoint R3: introduce the traversal plan beside the current path

- Complete on 2026-09-04.
- Built `IntersectionTraversalPlan` from the same negative-sized request while
  retaining the existing projection path.
- Added assertions that plan context, nested or derived start, logical extent,
  and local projection agree.
- Added explicit Agenda ownership and parser-revision wake-up for pending merge
  AItems, including transfer out of pending ownership on any other enqueue.
- Preserved negative requests while pending and locally zero-based projections
  after resolution.

### Checkpoint R4: record correspondence during matching

- Complete on 2026-09-04.
- Added one accepted-match correspondence helper and retained records on the
  owning traversal plan.
- Populated complete `ViewMap`s for scalar, flat, recursive typed, and sparse
  end-relative results.
- Removed first-child mapping recovery from the end-relative selection path
  after equivalence tests passed; retained it for ordinary positive views until
  R5 unifies their construction.
- Preserved `range/select1`, `marked/lookahead`, and the full-suite baseline.

### Checkpoint R5: unify view construction

- Complete on 2026-09-04.
- Made every ordered-span result local and recursively source-mapped.
- Migrated positive and direct ordered-span selectors through the same
  construction contract as end-relative selectors.
- Removed derived-versus-ordinary construction branches after positive,
  recursive, view-of-view, and sparse parity tests passed.
- Kept `ViewMap` as provenance and added separate, selection-time mapped-write
  authority so passive mapped reads cannot change their sources.
- Preserved immediate view boundaries and left `outerPOV` out of mapped local
  provenance.

### Checkpoint R6: make mapped writes transactional

- Complete on 2026-09-05.
- Replaced flattened immediate writes with recursive `MappedSpanWritePlan`
  construction that does not mutate source state.
- Validated recursive structure, scalar and ordered-unit compatibility, source
  context reachability, complete leaf mappings, and duplicate target
  assertions before commit.
- Coalesced equivalent duplicates and rejected conflicts before the first
  source write.
- Preflighted sparse materialization, made publication independent of plan pair
  order, and rejected stale sparse plans before exposing a source position.
- Routed composite leaves and legacy scalar source-backed writes through the
  same metadata-preserving commit operation.
- Removed assignment-driven tail closure and invoked ordinary
  `countSize(true)` reasoning after mapped commits.
- Captured marked sparse correspondence before destructive traversal exposure,
  allowing selection and write-back across sparse/concrete/sparse boundaries
  while preserving exact compatible sparse spans in compressed form.
- Added seven compiled transaction tests; improved the R1 characterization
  suite from `5/10` to `4/10` failures and the full suite from the R5
  `9/357` checkpoint to `8/364`.

### Checkpoint R7: remove compatibility inference

- Complete on 2026-09-05.
- Removed every semantic `sourcePov === startPov` test. Complete-map validity
  and explicit `viewMapWriteEnabled` authority now select the relevant paths.
- Removed first-child mapping recovery from result selection.
- Added `POV.selectorLocalWork`, classified selector construction sites, and
  transferred only consumer work to selected results.
- Confirmed the R5 local-copy/recursive-map construction contract remains the
  sole policy for positive, negative, direct, recursive, and sparse span views.
- Confirmed mapped assignment contains no source-tail closure; R6's ordinary
  `countSize(true)` re-evaluation remains the only post-write completion step.
- Added three compiled result-selection tests covering consumer-only transfer,
  an equal source/start scalar mapping, and rejection of first-child recovery.
- Regenerated and inspected `LocalBuild/TestProteus.cpp`. The focused sparse
  suite passes `21/21`, result-selection tests pass `3/3`, and both ordinary
  and protected full suites match the R6 failure set at `8/367`.

## Completion criteria for the refactor

The implementation is ready for negative `#` sugar only when:

- the negative request remains observable until its traversal plan resolves;
- unknown source extents wait, exact end coordinates resolve deterministically,
  and unavailable window contents remain pending;
- source/view boundaries are explicit and view-of-view traversal cannot escape;
- actual match correspondence supplies every selected mapping;
- `selectFunctionResult()` only selects and installs an already mapped result;
- no semantic decision depends on `sourcePov === startPov`;
- failed composite writes cannot partially mutate the source;
- mapped writes do not directly alter source completion state;
- sparse reads remain proportional to structural boundaries rather than the
  numeric distance from the beginning; and
- the focused negative intersection tests, existing positive time tests,
  `range/select1`, generated build, `git diff --check`, and full protected suite
  all match their approved expectations.

## Scope beyond this review

This review does not settle calendar naming conventions, the precise meaning
of the 20th Century, temporal relation authority, historical-span persistence,
or negative `#` syntax. Those remain later semantic stages. The traversal and
mapping invariants above should be stable enough that those stages do not need
another positional engine.
