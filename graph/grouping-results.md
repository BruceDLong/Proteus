# Fixed-width list grouping

The original `parse/grouping1` fixture now passes unchanged:

```proteus
{{_ _}|...} == {1 2 3 4 5 6 7 8}
// {{1 2} {3 4} {5 6} {7 8}}
```

The result has size **4**, and each pair has size **2**. The RHS remains size
**8**. No identity joins those two differently sized containers. Only their
corresponding scalar members receive persistent identity edges.

## Architecture

The adapter recognizes whole loose work (`==`) and advertises a grouping goal.
It still rejects unrelated loose semantics. The goal waits for the ordinary
definition walk to expose a repeated finite group requirement. A separate
`GroupedListPlan` validates the source layout, divisibility, declared extents,
existing prefixes, prior occurrence references, and every group's known fields.
It reuses `ProteusDefinitionPlan` for member construction and acquires available
size, overlay, and value together. All instance allocations and writes wait for the
manager's priced Set. A conflicting later group cannot cause earlier writes
from that same grouping batch.

A prior sparse lookup's canonical POV becomes the physical group POV. Lookup
through a second POV sharing the container's infon storage is also preserved.
Distinct occurrence POVs for the same index conservatively defer until a joint
alias plan can preserve all constraints. No whole-container identity or copies
back into shared definitions are introduced.

Template application now checks loaded instance prefixes even when their tail
is open. It visits only loaded/demanded members, including a one-member prefix
of a billion-item list. A derived extent smaller than that prefix is rejected
before closing the tail or installing the template.

Output uses revision-scoped evidence that the template and RHS are represented.
Both semantic relationships remain stored. Unknown numeric sizes still produce
symbolic handles; fully known members permit `isNormed`. Later values, changed
templates, and contradictions invalidate the evidence and remain reachable.

## Validation

Commands from the repository root:

```sh
codeDog Proteus.Lib.dog
python3 graph/run_language_baseline.py --fixtures graph/grouping-language.txt
python3 graph/run_language_baseline.py
```

Focused grouping checks run from `LocalBuild` with
`./TestProteus -t unit/graphGrouping/`.

After a full CodeDog build and final C++ header rebuild:

- 401/401 focused graph and parser checks pass, including 27 grouping checks.
- 38/38 existing selected original language fixtures pass.
- The unchanged original `parse/grouping1` fixture passes (1/1).
- The original `parse/grouping2` fixture still fails its expected output (1/1),
  settling symbolically without a normalization error; its boundary is below.

The standalone harness was rebuilt with `codeDog streamReplyTest.dog`.
Its supported cases (`./streamReplyTest -f 1`, `2`, `4`, `5`, `6`, and `8`, from
`LinuxBuild`) pass all 33/33 checks. Cases 3 and 7 are outside this checkpoint.

## Remaining boundaries

The size-defined grouping prerequisite below is now handled by the subsequent
[virtual-shape checkpoint](virtual-shape-results.md).

`parse/grouping2` specifies pair width using `*2+{...}` rather than explicit
member positions. Its next prerequisite is deriving a finite member layout from
that size without blindly materializing large symbolic extents. It is not
implemented by this change.

Also deferred: variable-width groups and lookahead, string segmentation,
`==:` prefix matching, nested group patterns, unordered/root-constrained source
lists, prefixed definition bodies, size-only grouping, general virtual-tail
indexing after a loaded prefix, and joint mapping of distinct references at one
logical index. The generic graph scheduler and existing strict span-alignment
algorithm are unchanged. No full legacy-suite or sanitizer run is claimed.

All work remains local to the private Proteus4 fork; the original Proteus
checkout is untouched.
