# Existing language baseline — 2026-09-23

Run from the private Proteus4 fork:

```sh
python3 graph/run_language_baseline.py
```

This runs 38 fixtures read directly from `defaultTests.tst` through
`GraphNormalizer`, using the same infon printer and expected output strings as
the ordinary tests. The selected parser fixtures also go through normalization.
There is no fallback to `Agent.normalize()`.

## Observed results

- **9/38 output comparisons match; 29/38 fail.** The runner correctly exits 1.
- All 38 handles settle without a failed status; **23 remain symbolic**.
- All 18 scalar targets show the expected value and size information before
  their retained RHSs. All 18 fail the complete text comparison because those
  RHSs are still printed. Three bare-number cases also have an unspecified size
  and therefore remain symbolic. No expected output was relaxed to accept this.
- The 9 matching list outputs are the 8 selected parser fixtures plus
  `noRHS/dots/simple`. All remain symbolic. The parser already provides these
  representations; this is not evidence that the graph normalizes their
  children or validates list relationships.
- The other 11 cases expose list identities, nested child work, open-list
  materialization, computed size, or list concatenation that is not implemented.
  `merge/list/empty` also retains its printed RHS.
- The focused graph suite remains **20/20**, including the added assertion that
  one compatible RHS supplies both missing size and value in one Set batch.
- CodeDog regeneration and the C++ build pass. One ordinary scalar test passes
  through its existing test mode, confirming that graph mode is opt-in.

The graph currently does not publish its settlement into the legacy
`infonView.isNormed` printer flag. This accounts for the redundant scalar RHS
rendering. A future result/settlement contract must preserve unresolved work;
simply hiding every RHS when a handle settles would conceal unsupported or
symbolic constraints.

## Next implementation order

1. Normalize physical list children, including nested lists and embedded spans.
   An ordinary nested list contributes one parent item; an embedded span can
   contribute its extent. These relationships must not be conflated.
2. Resolve and validate list sizes from the available evidence. Inner `~size`
   and outer `*size+` syntax already share the same `infSize` slot. Preserve the
   distinction between counted, given, context-dependent, and computed sizes.
3. Apply compatible list RHS information, keeping size and value together when
   both are available. Propagate constraints to corresponding child POVs.
4. Keep open and sparse forms symbolic when necessary, with subscriptions and
   lazy materialization instead of eagerly expanding all occurrences.
5. Address intersections after this list foundation is established.

Result presentation/settlement needs to be addressed alongside this work so
that completed scalar and child work can render cleanly without discarding
retained constraints. These are remaining tasks, not behavior added by this
baseline checkpoint.

## Per-case evidence

`Symbolic` records the handle flag; `Match` compares the complete printed
result with the original expectation. The size column is the root size:
`unknown` ignores the numeric payload in an unknown state.

| Original fixture | Actual result | Expected result | Size | Symbolic | Match |
|---|---|---|---|---|---|
| `merge/num/typed1` | `*16+10 = *16+10` | `*16+10` | 16 | no | no |
| `merge/num/literalEqual` | `10 = 10` | `10` | unknown | yes | no |
| `merge/num/literalToUnknown` | `10 = _` | `10` | unknown | yes | no |
| `merge/num/unknownFromLiteral` | `10 = 10` | `10` | unknown | yes | no |
| `merge/num/typed2` | `*16+9 = *16+9` | `*16+9` | 16 | no | no |
| `merge/num/typed3` | `*16+10 = *16+10` | `*16+10` | 16 | no | no |
| `merge/num/typed4` | `*16+9 = *16+9` | `*16+9` | 16 | no | no |
| `merge/num/typed5` | `*16+8 = *16+8` | `*16+8` | 16 | no | no |
| `merge/num/typed6` | `*16+10 = *16+_` | `*16+10` | 16 | no | no |
| `merge/num/typed7` | `*16+9 = *_+9` | `*16+9` | 16 | no | no |
| `merge/str/typed1` | `*5+'Hello' = *5+'Hello'` | `*5+'Hello'` | 5 | no | no |
| `merge/str/literalEqual` | `'Hello' = 'Hello'` | `'Hello'` | 5 | no | no |
| `merge/str/literalToUnknown` | `'Hello' = $` | `'Hello'` | 5 | no | no |
| `merge/str/unknownFromLiteral` | `'Hello' = 'Hello'` | `'Hello'` | 5 | no | no |
| `merge/str/typed2` | `'Hello' = 'Hello'` | `'Hello'` | 5 | no | no |
| `merge/str/typed3` | `'Hello' = 'Hello'` | `'Hello'` | 5 | no | no |
| `merge/str/typed4` | `'Hello' = 'Hello'` | `'Hello'` | 5 | no | no |
| `merge/str/typed5` | `'Hello' = *5+$` | `'Hello'` | 5 | no | no |
| `parser/list/simple` | `{'A' 12 {1 2 3}}` | `{'A' 12 {1 2 3}}` | 3 | yes | yes |
| `parser/list/empty` | `{}` | `{}` | 0 | yes | yes |
| `parser/list/unknown` | `{1 ... 3}` | `{1 ... 3}` | unknown | yes | yes |
| `parser/list/sized1` | `{~2\| 'A' 12}` | `{~2\| 'A' 12}` | 2 | yes | yes |
| `parser/list/sized2` | `{~2\| 'A' 12}` | `{~2\| 'A' 12}` | 2 | yes | yes |
| `parser/list/sizedNested` | `{1 {~2\| 3 4}}` | `{1 {~2\| 3 4}}` | 2 | yes | yes |
| `parser/list/partial` | `{{ ... } {1 ... } {1 ... 5} {}}` | `{{ ... } {1 ... } {1 ... 5} {}}` | 4 | yes | yes |
| `parser/list/embedded` | `{1 2 &{3 4 5} 6 7}` | `{1 2 &{3 4 5} 6 7}` | 7 | yes | yes |
| `merge/list/empty` | `{} = {}` | `{}` | 0 | yes | no |
| `merge/list/nums` | `{_ _ _} = {2 3 4}` | `{2 3 4}` | 3 | yes | no |
| `merge/list/multiID` | `{_ _ _} = {_ 2 _} = {1 _ _} = {_ _ 3}` | `{1 2 3}` | 3 | yes | no |
| `merge/list/N_dots3` | `{ ... } = {1 2 3}` | `{1 2 3}` | 3 | yes | no |
| `merge/list/dots` | `{ ... } = {2 3 4}` | `{2 3 4}` | unknown | yes | no |
| `trav/list/withEmpty` | `{{1 2} {} {_ = 3} _ = 4}` | `{{1 2} {} {3} 4}` | 4 | yes | no |
| `noRHS/dots/simple` | `{ ... }` | `{ ... }` | unknown | yes | yes |
| `noRHS/dots/nestedOut` | `{{ ... } _ = 5}` | `{{ ... } 5}` | 2 | yes | no |
| `noRHS/dots/nestedIn` | `{1 _ = 2 3 ... }` | `{1 2 3 ... }` | unknown | yes | no |
| `parse/innerID` | `{1 {{2 _ = 3}}}` | `{1 {{2 3}}}` | 2 | yes | no |
| `merge/calcSize` | `*(10)+5` | `*10+5` | expression (state 1) | yes | no |
| `concat/simpleList` | `({1 2} {} {3 4})` | `{1 2 3 4}` | unknown | yes | no |
