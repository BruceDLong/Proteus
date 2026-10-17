# Computed sizes and concatenation — 2026-09-24

The graph API now evaluates concatenations in a POV's size or value slot using
existing operand POVs. Numeric concatenation uses following operands' sizes as
radices, with the existing additive `fromContext` rule. Strings append in order;
ordered physical list chunks contribute independent member views. New member
creation is priced before publication. The original expression stays reachable
for subsequent validation, subscription updates, and conflicts.

Verified examples include `*(10)+5` → `*10+5`, `(*8+4 *4+3)` → `*32+19`,
nested strings and numbers, and `({1 2} {} {3 4})` → `{1 2 3 4}`. An open final
chunk publishes only its loaded prefix; a later RHS can supply its end and more
members. A computed billion-item size exposes one sparse occurrence without
expanding the list. Invalid negative sizes and mixed operand types stay symbolic.

Validation:

- Full CodeDog regeneration and C++ build passed.
- Graph integration: **122/122 passed**, including 17 expression scenarios.
- Unchanged original expression fixtures: **9/9 exact output matches**.
  Four remain symbolic: negative size, and three cases with bare numeric members
  whose sizes remain unknown. Matching text does not imply full normalization.
- `git diff --check` passed. Full legacy suite not run.

Run from the fork root:

```sh
codeDog Proteus.Lib.dog
(cd LocalBuild && ./TestProteus -t unit/graphFoundation/)
python3 graph/run_language_baseline.py --fixtures graph/expressions-language.txt
```

General expression evaluation, mixed scalar/list concatenation, template chunks,
and concatenation across multiple open chunks remain deferred. The native
scheduler did not change; field-expression semantics belong to the adapter and
its GoalSpec.
