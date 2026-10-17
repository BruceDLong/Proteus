# Physical list normalization — 2026-09-24

This checkpoint adds physical-member traversal and required subgoals to the
private Proteus4 graph normalizer. It does not redirect the legacy entry point.

## Behavior added

- Normalize existing members, including ordinary nested lists and embedded
  list/string spans, through graph subgoals.
- Count an ordinary nested value as one item; count an embedded span by its
  nonnegative integer extent. Reject inverted/undefined extent evidence.
- Infer an unknown counted size through a GoalSpec-generated Set. Child size
  Sets publish revisions and make previously unavailable parent counts usable.
- Preserve explicit `fromGiven` unknown sizes and size presentation metadata.
  A declared size disagreement remains symbolic without overwriting evidence.
- Mark a closed list `isNormed` only after its extent agrees and all required
  members complete. A healthy child can be marked while a sibling or parent
  stays symbolic. Successful child results use the existing printer.
- Resolve available children of an open list without closing its tail. Leave
  unsupported templates and sparse regions symbolic without expanding them.
- Subscribe and resume when a missing child's RHS arrives; cancellation stops
  the parent session and all its subgoals.

The generic manager now records required goal dependencies and per-goal
unresolved work. It settles dependencies from children to parents, handles
shared subgoals, leaves dependency cycles symbolic, and retries a shared goal
if a cheaper route becomes available. It still has no knowledge of POV fields
or Proteus list semantics.

## Validation

- CodeDog regeneration and C++ build: **passed** (`Marker: Build Successful`).
- Actual POV integration tests: **42/42 passed** (previously 24).
- Native graph runtime: **22/22 passed** (previously 16), with AddressSanitizer,
  UndefinedBehaviorSanitizer, and LeakSanitizer enabled.
- Unchanged language-fixture comparison: **24/38 match**, **14 mismatches**.
  All 38 handles settle without failure; 22 retain symbolic obligations.
- `git diff --check`: passed.
- Full legacy suite: not run for this bounded milestone.

Commands used:

```sh
codeDog Proteus.Lib.dog
# From LocalBuild:
./TestProteus -t unit/graphFoundation/
# From the fork root:
g++ -std=c++17 -Wall -Wextra -Werror -pthread \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  graph/test_graph_search.cpp -o /tmp/proteus4-lists-native
ASAN_OPTIONS=detect_leaks=1 /tmp/proteus4-lists-native
python3 graph/run_language_baseline.py
```

The baseline runner exits 1 for the known mismatches. Its original inputs and
expected outputs have not been changed. The test harness itself may exit zero
on assertion failures, so its printed summary was checked.

## What the unchanged baseline count means

`parser/list/empty` now finishes without symbolic obligations. Four existing
list cases have newly resolved child values but still fail exact printing:

| Fixture | Current result | Expected |
|---|---|---|
| `trav/list/withEmpty` | `{{1 2} {} {3 = 3} 4 = 4}` | `{{1 2} {} {3} 4}` |
| `noRHS/dots/nestedOut` | `{{ ... } 5 = 5}` | `{{ ... } 5}` |
| `noRHS/dots/nestedIn` | `{1 2 = 2 3 ... }` | `{1 2 3 ... }` |
| `parse/innerID` | `{1 {{2 3 = 3}}}` | `{1 {{2 3}}}` |

Those bare numeric children retain unknown size and therefore are not marked
`isNormed`; their RHSs stay visible. The focused `closedList` case supplies
both size and value and verifies the final printed result `{*10+7 'ok'}`.

The other remaining mismatches are three bare-number scalar cases, five
list-to-list cases (`empty`, `nums`, `multiID`, `N_dots3`, `dots`), computed size,
and concatenation. Matching parser output alone still does not prove complete
normalization of bare-number or open lists.

## Next slice

List-to-list member correspondence and RHS propagation come next. Computed
sizes, concatenation, template normalization, nested indexed traversal,
intersections, and stream-parser integration remain separate work. Redundant
RHS removal is also not implemented; constraints are retained.
