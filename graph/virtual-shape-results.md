# Finite extents over virtual list positions

A definition can now describe an empty open list body such as `*2+{...}`.
Its positions remain virtual. `DefinitionRequirement::memberLayout()` expresses
explicit members and a virtual tail through the existing `SpanLayout` equations.
Type application and grouping both consume that description; there is no new
solver or grouping rule for a particular spelling.

These original language fixtures now produce the same result:

```proteus
{{_ _}|...} == {1 2 3 4 5 6 7 8}
{*2+{...}|...} == {1 2 3 4 5 6 7 8}
// {{1 2} {3 4} {5 6} {7 8}}
```

A sized homogeneous member constraint also supplies a finite group width:

```proteus
{*2+{*10+_|...}|...} == {*10+1 *10+2 *10+3 *10+4}
```

## Semantics and preservation

- `#row=*1000000000+{...}` establishes size without allocating a billion
  requirements, POVs, or occurrences. Applying the type leaves its value
  symbolic. Size alone does not justify `isNormed`.
- An existing closed instance must have the required extent. A concrete RHS
  can fill missing positions through ordinary strict correspondence. A closed
  definition constraint can combine with the virtual shape in either order.
- The read-only definition walk can acquire an extent from another compatible
  RHS. An initially unknown width leaves grouping symbolic; publication of the
  missing constraint wakes its subscribed search. Cancellation prevents that
  search from resuming. The shared definition body itself remains unchanged.
- Grouping prepares members only for the loaded source it is mapping. Its
  existing atomic Set and cost policy still govern publication. Original group
  POVs and supported canonical occurrence references are reused.
- Definition evidence includes open/closed tail state. Later closure or extent
  changes invalidate earlier proof, even when the member pointers are unchanged.
- A numeric RHS such as `*2+_` remains a numeric constraint and conflicts with a
  list. It is not treated as an untyped request to copy only the size.

## Boundaries

Open definition bodies with an explicit prefix, embedded virtual definition
spans, variable-width/nested grouping, and general indexed access into
untemplated virtual tails remain deferred. A zero-width group cannot establish
how many groups exist and remains symbolic. Fractional and out-of-range virtual
extents are not narrowed into indices.

A finite fill into an already-templated instance remains deferred. In particular,
an already-requested sparse member inside a group must not be replaced by a new
physical member during mapping. This checkpoint preserves that reference and
leaves the unsupported group mapping symbolic. Existing supported group-level
occurrence reuse continues to work.

The finite planner callbacks still run synchronously. This change does not add
resumable planning callbacks, `%prev`, or a general virtual-position iterator.

## Validation

- CodeDog generation and the C++ test build pass.
- New virtual-shape cases: 30/30. These include billion-item shape application,
  billion-wide group mismatch/empty-source checks, late extent and cancellation,
  typed groups, canonical group reuse, demanded inner-member preservation,
  conflicts, repeated normalization, and proof invalidation on body closure.
- Existing focused graph/parser checks: 411/411; combined total 441/441.
- Original selected language fixtures: 38/38 plus both grouping fixtures, 2/2.
- Rebuilt standalone streaming harness: 33/33 supported checks (1, 2, 4, 5, 6, 8).
- `git diff --check` passes.

## Reproducing the checks

```sh
codeDog Proteus.Lib.dog
# From LocalBuild:
./TestProteus -t unit/graphVirtualShape/
# From the repository root:
python3 graph/run_language_baseline.py
python3 graph/run_language_baseline.py --fixtures graph/grouping-language.txt
```

The original fixture inputs and expected strings remain unchanged. No full
legacy-suite or sanitizer pass is claimed for this checkpoint.
