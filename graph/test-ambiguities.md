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
