# Logical list navigation

Embedded spans contribute positions to their containing list. Ordinary nested
lists contribute one position. Relative and indexed queries now share that
interpretation through the list structure helpers.

- `nextListPosition` walks either direction; `adjacentListPosition` also supports
  a standalone occurrence whose containing POV is not supplied.
- Graph queries retain existing owner POVs as traversal context. They can leave
  an embedded span but stop at the containing ordinary list. No permanent parent
  index or second adjacency graph is built.
- `listBoundary` measures the stored prefix and its embedded extents. Its finite
  inspection work is charged to the route. A known extent can prove a zero-width
  open tail or skip a virtual span without visiting its positions.
- Unknown embedded extents and open ends expose size/value refinements. Searches
  may resolve inside an available prefix without requiring the rest of the span.
  When the needed boundary remains unknown, the handle stays subscribed.
- Sparse positions still use the registry's canonical indexed materialization.
  Skipping a billion-position span creates no occurrences; selecting within it
  creates only the demanded occurrence.
- Positive given intersection extents now select the prefix endpoint through
  indexed lookup: `*3+['Cat' 'Hat' 'Bat' 'Dog']` selects `'Bat'`. A given prefix
  length is not treated as a claim that the stored body ends at that position.

Relative bindings retain their enclosing context and reuse existing stream
provenance checks. Shared contextual definition/template instantiation and
temporal navigation remain separate work. Finite boundary inspection is still
synchronous, like the existing finite layout planners.


## Validation

- Full CodeDog regeneration and C++ build passed; the final native-only adjustment
  was rebuilt from the generated target.
- Focused graph/parser checks: 508/508, including 34 new navigation checks and
  all 33 relative-position checks. Coverage includes both traversal directions,
  empty/nested spans, ordinary list boundaries, delayed extents, cancellation,
  cycles, cost exclusion, sparse occurrence reuse and stream reinterpretation.
- Shared list-structure tests: 18/18 with `-Wall -Wextra -Werror`.
- Rebuilt standalone streaming harness: 33/33 supported checks (1, 2, 4, 5, 6, 8).
- The default language suite improved from 135/270 to 138/270, without losing
  any previously passing fixture. See the full
  [older language inventory](language-compatibility-results.md) for all six suites.

Indexed access advertises size and value refinement before requiring a directly
navigable list. This preserves computed-size sparse lookup. Relative lookup can
also participate while a named container's order is still unknown; requiring
completed type application first would deadlock when that application itself
needs the relative value. Explicitly unordered containers remain deferred, and
streaming bindings retain the enclosing ordered-body evidence.

The former relative test for an unsupported embedded span now uses an unknown
open end; a finite embedded span is supported. Original language fixtures remain
unchanged. No full legacy-unit, sanitizer or GUI-integration pass is claimed.

Run the focused suites from `LocalBuild` with:

```sh
./TestProteus -t unit/graphNavigation/
./TestProteus -t unit/graphRelative/
```
