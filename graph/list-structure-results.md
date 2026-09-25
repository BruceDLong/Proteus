# Shared list-structure access

This is the first consolidation toward reasoning by language constructs.
Four consumers now share one structural walk and one layout builder:

- Strict correspondence advances the walk one graph edge at a time.
- Finite definition application projects the same positions into instance slots.
- RHS representation checks inspect the same layouts without writes.
- Whole-list grouping uses the same source spans and extent equations.

The walk implements list membership, embedded descent, and list boundaries.
The builder records each ordinary position as one item, each open tail as one
symbolic gap, and each list size as an equation over its positions. An ordinary
nested list keeps its identity and counts as one member. Embedded boundaries
retain their owning POVs. Ancestor infon identity detects cycles while allowing
shared storage at separate sibling positions.

Existing semantic eligibility checks remain with the consumers. The walk does
not decide whether a definition applies or whether strict/loose correspondence
holds. Source and template data remain read-only; all existing priced Sets,
canonical occurrence identities, evidence invalidation, and subscriptions stay
in place. Stored inspection and published stream inspection differ explicitly:
the latter stops before unfinished input, while the former exposes the stored
member to its consumer's checks.

The old independent layout walks were removed from DefinitionPlan,
RHSRepresentation, GroupedListPlan, and the correspondence dispatcher. The two
specialized published-neighbour accessors were also removed. This reduces
independent algorithms; overall production line count is approximately unchanged
because the shared cursor, projection, and incremental interface are explicit.

## Validation

- Full CodeDog generation and C++ build pass.
- Native list-structure tests: 14/14, with AddressSanitizer, UndefinedBehaviorSanitizer,
  and LeakSanitizer completing successfully outside the restricted sandbox.
- Existing span-equation tests: 200/200 under the same sanitizers.
- Focused graph/parser tests: 401/401.
- Existing selected language fixtures: 38/38, plus the original grouping fixture 1/1.
- Standalone streaming harness rebuilt: all 33/33 checks pass across supported cases 1, 2, 4, 5, 6, and 8.
- `git diff --check` passes. No original test inputs or expectations were changed.

Native checks can be built from the repository root:

```sh
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -g graph/test_list_structure.cpp -o /tmp/p4-list-structure-tests
/tmp/p4-list-structure-tests
```

## Remaining work

This change consolidates structural access. Existing finite planners still
collect their layouts synchronously inside callbacks; exposing that collection
as scheduler-owned resumable work remains further consolidation. Definition
compilation, ordinary member-normalization walks, and intersection selection
retain their distinct walks and responsibilities.

The grouping planner and semantic eligibility predicates still exist. The next
step is to use this shared structure when unifying extent acquisition and
correspondence requirements. No new grouping form, syntax support, or speculative
transaction mechanism is introduced here. No full legacy suite is claimed.
