# Intersection result selection — 2026-09-24

First, last, and marked selection now use goals over existing inner POVs.
The selection walk checks list extent evidence and follows embedded spans without
normalizing unrelated member values. A last-item goal waits for evidence of the
end; a marked item can resolve while an unrelated tail remains open. An explicit
extent can prove that an open tail has no remaining positions.

The Set publishes an identity to the selected POV. Ordinary scalar Sets or
list adoption then acquire its fields. The original inner expression, selection
kind, and selected POV remain attached for revalidation. Later conflicting
information invalidates normalization rather than silently replacing the result.
Empty open spans, cycles, unsupported alternatives, and excluded costly routes
remain symbolic. Subscription resumption and cancellation are covered.

The parser now clears the outer size's literal flag when moving the expression
into `innr`. The original extent belongs to the inner list; the selected result's
outer size starts unknown and any actual outer size constraint is preserved.

Validation after full CodeDog regeneration and C++ build:

- Graph integration: **155/155 passed**, including 22 selection scenarios.
- Original expression fixtures: **9/9 exact output matches**.
- Original selection/adoption fixtures: **6/11 exact output matches**. All
  selected values appear, but five retain an RHS while bare numeric sizes remain
  unknown: `merge/directAny/list`, `getLastItem/intID`, `getLastItem/listID`,
  `getLastItem/intIDQ`, and `getLastItem/listIDQ`. For example, `5 = 5` is still
  printed instead of `5`. Expected outputs were not changed.
- The unchanged 38-fixture graph baseline improves from **25/38 to 27/38** exact
  output matches. The two newly matching cases are `merge/calcSize` and
  `concat/simpleList`; the other 11 remain mismatches with retained RHSs.
- Focused legacy parser/selection checks: **10/10 passed** (`getLastItem/` and
  `marked/getNotLast`). The full legacy suite was not run.
- `git diff --check` passed. No generic scheduler changes were made; native
  scheduler sanitizers were not rerun for these adapter/GoalSpec changes.

Matching output alone does not imply concrete normalization. Four expression
fixtures and nine selection/adoption fixtures still settle symbolically.

Run from the fork root:

```sh
codeDog Proteus.Lib.dog
(cd LocalBuild && ./TestProteus -t unit/graphFoundation/)
python3 graph/run_language_baseline.py --fixtures graph/expressions-language.txt
python3 graph/run_language_baseline.py --fixtures graph/selection-language.txt
python3 graph/run_language_baseline.py
(cd LocalBuild && ./TestProteus -t ../defaultTests.tst:getLastItem/)
(cd LocalBuild && ./TestProteus -t ../defaultTests.tst:marked/getNotLast)
```

Alternative filtering, function arguments, templates, positional selectors, and
embedded-string traversal remain deferred. The public `Agent.normalize()` entry
point still uses the legacy engine; these results exercise the opt-in
`GraphNormalizer` API without falling back to agenda processing.
