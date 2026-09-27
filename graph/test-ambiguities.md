# Ambiguous existing tests

The user authorized recording and skipping tests whose intended semantics are
ambiguous while completing the remaining compatibility work. Skipped tests are
reported separately from passes. Missing implementations are not a reason to skip.
Inputs and expected outputs remain unchanged. The audit checks each skipped
fixture's hash so a later test edit cannot silently inherit an obsolete exception.
Use `--include-ambiguous` to run these tests too.

## `defaultTests.tst:merge/dots/align4b`

Input: `{1 4 ... 5 _} = {... 4 5 6}`.
Expected: `{1 4 &{4 5 6 ... } 5 6}`.

The explicit `4` on the right can align with the left's existing `4`, or with a
position inside its open span. The current Proteus3 executable produces the
expected text, but that alone does not establish which placement the language
requires. The general span equations do not prove a unique placement.

Question for later discussion: preserve alternatives symbolically, choose one
placement, or wait for additional evidence? The first-parse policy for ambiguous
grammars does not automatically decide arbitrary equality placement.

No context/T-list tests are skipped merely because those systems are being
redesigned. Add an entry only after identifying a concrete semantic question.

## User-world lookup past a known untyped value

`defaultTests.tst:user/getUser`, `user/getUser3`, `user/getNth`, and `user/setNth`
load `Resources/user.pr` or `Resources/user_3.pr`. Both worlds start with `76543`
before the typed `bruceLong` member. All four queries first select `.bruceLong`.

Proteus3 skips that numeric member. The graph's nominal matcher currently returns
unknown when a candidate has no type word, preserving the established rule that
an unresolved earlier ordered candidate cannot be skipped. A concrete value does
not inherently establish whether that object can later acquire the requested type.

Question: should a known untyped literal count as a proved nominal nonmatch,
while an unknown value still waits for type evidence? This is distinct from the
settled rule that matching structure alone does not establish type identity.
The skips cover this first ambiguous lookup; they do not claim the later nested
selection or forced-write operations already work. A focused test retains the
current pending behavior for `{76543 b:*10+2}.b` through a world source.

## `defaultTests.tst:slice/simpleSugar`

Input: `{1 2 3 4 5 6 7}@[3:5]`. Expected: `5`.

The original lowerer describes a zero-based prefix count followed by the spec.
That puts the spec against `4`, not `5`. Proteus3 returns `5`, but also returns
`99` for `@[3:99]` and `@[0:99]`, and `5` for the out-of-range `@[99:5]`.
Those probes all passed on the current Proteus3 binary; the original fixture
therefore does not establish that a source position was selected. The graph
currently checks the selected occurrence and yields an empty oneOf on mismatch.

Question: should a scalar spec constrain the item after the boundary, search
onward, or denote a different operation? What should a proved nonmatch return?
Keep the original fixture unchanged and skip it while the contract is clarified.
Reference probes: `/tmp/p4-scalar-slice-reference.tst` and `.log`.

## `defaultTests.tst:temporal/noSListFallback`

Input: `{1 %tprev 3}`. Expected: `{1 UNDEFINED 3}`.

The prohibition on treating an S-list predecessor as a temporal predecessor is
clear and remains required. The unresolved issue is finality: other existing
fixtures explicitly allow an ordinary list to acquire `T` meaning through a later
identity. Absence of that marker today is therefore not necessarily proof that
no temporal context can become available. The graph currently leaves the cursor
symbolic and subscribed instead of borrowing the S-list predecessor.

Question: what evidence makes missing temporal context permanently `UNDEFINED`?
Could a later T identity reopen that result, or should it remain subscribed until
there is explicit negative/boundary evidence? Skip the old output assertion while
retaining the no-structural-fallback rule. This is not a skip of temporal neighbor
or boundary operations generally; their unambiguous fixtures still count as failures.

## `defaultTests.tst:word/findPlural`

Input: `{123, foot, {foot|foot1,foot2}}.feet`.
Old expected value: `feet:{foot1, foot2}`.

Plural inference and lookup now find the inner collection. Under the established
open-unordered membership rule, the unbounded outer description can contain more
matching collections. The actual result is therefore
`*_+[feet:{foot| foot1, foot2} ... ]`. The inner `foot|` remains meaningful: its
local template also constrains unlisted members. It is not a fully consolidated
closed collection.

The pending fixture choice is whether to add an explicit outer size of three
for a definite lookup, or to retain the original input and expect an open oneOf.
Preserve the original fixture while that choice is pending. The separate
`unit/graphWordForms/lookup` case uses the bounded input and expects
`feet:{foot| foot1, foot2}`. This skip is about the old result contract, not a
missing plural-lookup implementation.
