# isNormed results — 2026-09-24

The graph now marks a fully known supported scalar `isNormed` after its entire
search settles without deferred work. The normal printer therefore suppresses
its retained RHSs. No RHS is deleted and no test expectation is changed.

## Validation

- CodeDog regeneration and C++ build: passed.
- Existing language output comparisons: **24/38 pass**, up from **9/38**.
- The 15 newly passing cases are the seven `merge/num/typed` cases and all
  eight selected string cases. Their size and value were already correct;
  publishing completion removes the redundant RHS text.
- The nine earlier list output matches remain symbolic; they do not establish
  that list/member normalization is implemented.
- Focused graph tests: **24/24 pass**. These check positive scalar completion,
  contradictions, unsupported constraints, cost-excluded work despite known
  fields, incomplete list members, and invalidation after an RHS gains
  conflicting information even after the completed handle has been released.
- Native graph scheduler: **16/16 pass** under AddressSanitizer,
  UndefinedBehaviorSanitizer, and LeakSanitizer.

The change adds a generic root-goal completion notification and a publication
invalidation hook. The Proteus adapter owns the `isNormed` decision. Completion
metadata does not create a graph revision. Publications clear tracked flags
conservatively across the context; weak references keep completed targets
invalidatable without keeping those POVs alive.

List normalization remains next. Knowing a list's extent or literal container
is insufficient to mark it: its required members must be accounted for and
normalized. The current implementation therefore only marks supported scalars.

## Remaining output mismatches

Three bare-number cases retain an unspecified size and stay unmarked. The
remaining eleven cases need list identities, child normalization, list extent
handling, computed-size evaluation, or concatenation.

| Original fixture | Actual | Expected |
|---|---|---|
| `merge/num/literalEqual` | `10 = 10` | `10` |
| `merge/num/literalToUnknown` | `10 = _` | `10` |
| `merge/num/unknownFromLiteral` | `10 = 10` | `10` |
| `merge/list/empty` | `{} = {}` | `{}` |
| `merge/list/nums` | `{_ _ _} = {2 3 4}` | `{2 3 4}` |
| `merge/list/multiID` | `{_ _ _} = {_ 2 _} = {1 _ _} = {_ _ 3}` | `{1 2 3}` |
| `merge/list/N_dots3` | `{ ... } = {1 2 3}` | `{1 2 3}` |
| `merge/list/dots` | `{ ... } = {2 3 4}` | `{2 3 4}` |
| `trav/list/withEmpty` | `{{1 2} {} {_ = 3} _ = 4}` | `{{1 2} {} {3} 4}` |
| `noRHS/dots/nestedOut` | `{{ ... } _ = 5}` | `{{ ... } 5}` |
| `noRHS/dots/nestedIn` | `{1 _ = 2 3 ... }` | `{1 2 3 ... }` |
| `parse/innerID` | `{1 {{2 _ = 3}}}` | `{1 {{2 3}}}` |
| `merge/calcSize` | `*(10)+5` | `*10+5` |
| `concat/simpleList` | `({1 2} {} {3 4})` | `{1 2 3 4}` |
