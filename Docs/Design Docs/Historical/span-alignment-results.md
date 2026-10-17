# Embedded/open list spans — 2026-09-24

The graph normalizer now aligns ordered physical list relationships across
embedded list spans and open tails. Existing POVs remain the graph nodes; no
legacy agenda normalization is invoked. The old closed-only paired walk and
its temporary evidence class have been removed.

## Planning and execution

1. The Proteus adapter walks both existing layouts, including embedded owners
   and their physical members. The generic query holds opaque return points so
   the same physical member can be visited in different embedded contexts.
2. The correspondence GoalSpec collects loaded positions, symbolic tail gaps,
   and interval-size constraints. `SpanAlignment.h` plans identities and finite
   fills without inspecting or mutating a POV. Unique gap widths are inferred;
   ambiguous partitions are not enumerated.
3. The GoalSpec proposes a Set whose work includes new identities and members.
   The generic frontier ranks this work before execution. Excluding the Set
   leaves the physical list untouched. A revision discards stale queued plans.
4. The adapter publishes the batch under the context lock. New physical members
   get independent infonViews and registry identities to their sources. Ordinary
   child goals then acquire their size/value, expose further edges, and establish
   `isNormed` when all required work is concrete and validated.

A known loaded prefix can extend an open tail without closing it. A gap closes
only when its end is proved. Billion-item extents stay compressed: the tests
create either zero members or the single member already loaded on the other
side. Known extents use exact integer conversion, including beyond 2^53; values
outside the supported signed 64-bit range stay symbolic.

The full supported layout is checked before a plan is published. Unsupported
later spans or inconsistent declared extents therefore produce no prefix
identities. Supported ambiguity can still yield anchored prefix/suffix identities
while the middle remains symbolic. Existing subscriptions resume after new RHS
information arrives; cancellation stops that session's future work.

Examples verified through the actual parser and POV representation:

| Input | Result |
|---|---|
| `{*10+_ &{*10+_ *10+_} *10+_} = {*10+1 *10+2 *10+3 *10+4}` | Values 1–4 resolve through the embedded list; root is normed |
| `{*10+1 ... *10+4} = {*10+1 *10+2 *10+3 *10+4}` | The middle gap becomes the closed embedded list `{*10+2 *10+3}` |
| `{...} = {*10+1 *10+2 ...}` | Two independent members are created; the tail remains open |
| `{*10+_ ... ... *10+_} = {*10+1 *10+2 *10+3 *10+4}` | Endpoints resolve to 1 and 4; ambiguous gaps remain open |
| `*1000000000+{...} = {*10+7 ...}` | One loaded member is copied; the remaining extent stays symbolic |
| `{...} = {{*10+7} *10+8}` | A new nested list view resolves via identity, without aliasing source member links |

## Validation

- Full CodeDog regeneration and C++ build: passed (`Marker: Build Successful`).
- Parser/POV graph integration: **90/90 passed**, up from 62 scenarios.
- Generic graph runtime: **24/24 passed** under AddressSanitizer,
  UndefinedBehaviorSanitizer, and LeakSanitizer.
- Native span planner: **200/200 passed** under the same sanitizers.
- Unchanged 38-fixture language baseline: **25/38 exact output matches**;
  all settle without failed handles, and 21 remain symbolic.
- Four additional original span fixtures (`merge/list/N_dots1`, `N_dots2`,
  `dotsC`, `misc`): **0/4 exact output matches**. Their requested member values
  now appear, but unresolved numeric sizes/open tails keep the result symbolic
  and the retained RHSs visible. These are not counted as passing tests.
- `git diff --check`: passed. Full legacy suite not rerun for this milestone.

The 38-fixture count is unchanged, but two previously unresolved list outputs
now contain their complete values: `merge/list/N_dots3` prints
`{1 2 3} = {1 2 3}`, and `merge/list/dots` prints
`{2 3 4} = {2 3 4}`. The expected strings omit the RHS. Bare numeric members still
have unknown size, so the graph correctly withholds `isNormed`; this milestone
does not change that policy or discard constraints to hide the difference.

The additional nested-gap test uses an interior `&{...}`. The existing parser
folds an unknown trailing embedded tail into its parent, so that spelling does
not preserve a separate nested node to test.

Commands from the fork root (run the integration executable from `LocalBuild`):

```sh
codeDog Proteus.Lib.dog
(cd LocalBuild && ./TestProteus -t unit/graphFoundation/)
g++ -std=c++17 -Wall -Wextra -Werror -pthread \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  graph/test_graph_search.cpp -o /tmp/proteus4-spans-native
ASAN_OPTIONS=detect_leaks=1 /tmp/proteus4-spans-native
g++ -std=c++17 -Wall -Wextra -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  graph/test_span_alignment.cpp -o /tmp/proteus4-span-planner
ASAN_OPTIONS=detect_leaks=1 /tmp/proteus4-span-planner
python3 graph/run_language_baseline.py
```

The baseline runner exits 1 for its 13 output mismatches. The integration
harness can return zero despite failed assertions; its printed summary was
checked. Original language inputs and expected strings were not edited.

## Remaining boundaries

Embedded-string correspondence, template/repetition alignment, computed sizes,
concatenation, and unordered matching remain outside this implementation.
Sparse indexed access remains independently supported. Work lists and original
constraints remain attached. Structural reordering/retraction and large-list
performance optimization are separate work; this implementation uses monotone
tail extension/closure, conservative revision restarts, and registry scans.
