# Closed list RHS correspondence — 2026-09-24

Closed, ordered, untyped physical lists now propagate RHS information through
identities between corresponding members. The graph entry point remains opt-in;
no legacy agenda normalizer is used by these tests.

## Behavior

A required correspondence goal carries the two existing parent POVs. Its paired
walk must reach both ends, accept every physical position, and validate declared
sizes before publishing anything. A matching prefix followed by an unsupported
span or unequal length therefore creates no member identities.

The Set uses the existing parent-size copy primitive and records bidirectional
member identities in the existing registry, tagged `iekListCorrespondence`.
Physical lists and next/prev links remain intact. Ordinary child goals then use
the same scalar compatibility checks and size/value Sets as before. This covers
multiple RHSs, separate field sources, transitive constraints, nested lists,
conflicts, subscriptions, cancellation, and later completion invalidation.

The generic scheduler gained only an optional opaque second query endpoint,
which participates in query identity. The correspondence goal and structural
checks live in the Proteus adapter. No wrapper graph nodes were introduced.

Examples verified through the actual parser:

| Input | Outcome |
|---|---|
| `{*_+_ *_+_} = {*10+7 *10+8}` | `{*10+7 *10+8}`, fully concrete and marked `isNormed` |
| `{*_+_ *_+_} = {*10+7 *_+_} = {*_+_ *10+8}` | Same fully normalized result, with both RHSs contributing |
| `{{*_+_} {}} = {{*10+7} {}}` | `{{*10+7} {}}`, including normalized nested members |
| `*_+{*_+_} = {*10+7}` | RHS supplies parent size 1 and the child's size/value |
| `{*10+7} = {*10+8}` | Failed handle, retained conflicting constraints |
| `{*10+_ *10+_} = {*10+7}` | Symbolic, with no partial correspondence published |
| `{*10+_ &{*10+_}} = {*10+7 *10+8}` | Symbolic; embedded alignment remains unsupported |

RHS work lists are retained. Successful `isNormed` flags allow the existing
printer to hide them. Derived identities remain live, so later information on a
source member can wake an unresolved target; later conflicts invalidate flags
and fail a new normalization pass. Repeated searches do not duplicate identities.

## Validation

- CodeDog regeneration and C++ build: **passed** (`Marker: Build Successful`).
- Graph integration: **62/62 passed**, including 20 new correspondence scenarios.
- Native graph runtime: **22/22 passed** under AddressSanitizer,
  UndefinedBehaviorSanitizer, and LeakSanitizer.
- Unchanged language fixtures: **25/38 output matches**, up from 24/38.
  All 38 settle without failed handles; 21 remain symbolic.
- `git diff --check`: passed.
- Full legacy suite: not run for this milestone.

Commands:

```sh
codeDog Proteus.Lib.dog
# From LocalBuild:
./TestProteus -t unit/graphFoundation/
# From the fork root:
g++ -std=c++17 -Wall -Wextra -Werror -pthread \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  graph/test_graph_search.cpp -o /tmp/proteus4-correspondence-native
ASAN_OPTIONS=detect_leaks=1 /tmp/proteus4-correspondence-native
python3 graph/run_language_baseline.py
```

The baseline runner exits 1 for the 13 expected mismatches. Original inputs and
expected strings were not edited. The integration test harness's printed assertion
summary was checked rather than relying on its exit code.

`merge/list/empty` is the new output match. Two more fixtures now have resolved
member values, while their unknown numeric sizes keep them symbolic:

| Fixture | Current printed result | Expected |
|---|---|---|
| `merge/list/nums` | `{2 3 4} = {2 3 4}` | `{2 3 4}` |
| `merge/list/multiID` | `{1 2 3} = {1 2 3} = {1 2 3} = {1 2 3}` | `{1 2 3}` |

The remaining failures are those two cases, three bare-number scalar cases,
four earlier child-rendering cases, two open-list materialization cases,
computed size, and concatenation.

## Boundaries and next work

Correspondence currently requires closed ordered physical lists whose matched
positions are ordinary members. Nested list values work; flattening embedded
spans, open tails, sparse or template alignment, and unordered matching remain
symbolic. Existing physical-span normalization and lazy sparse indexed access
continue to work independently.

The validated finite identity batch is atomic under the context lock.
Cancellation can interrupt its graph traversal or enter after publication.
Conservative restarts and registry scans are still foundation implementations,
not large-list performance optimizations. Derived identities assume stable
positions in closed lists; structural rewriting and retraction need their own
invalidation design.

The next list milestone is correspondence across embedded/open spans. Computed
sizes, concatenation, redundant RHS removal, and intersections remain separate
work.
