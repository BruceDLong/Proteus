# Signed scalar payloads (2026-09-29)

The blocker was in the engine, not the cube's location/orientation model.
An unsized negative literal used a positive magnitude plus `inverted`, and the
definition fact reader rejected that representation. Thus even the known field
in `{#pose={xRot:0 xLoc:-1.1} pose.xRot}` could not be obtained.

Literal numeric minus now belongs to the signed rational `FlexNum` payload.
`inverted` remains available for reciprocal sizes (`/`) and symbolic/non-numeric
inversion. The printer emits the payload sign without a second plus sign.
The definition reader accepts literal numeric facts in `fromContext` mode and
reads them without modifying declaration bodies. Ordinary negative coordinates
remain negative while their extent is unknown.

## Preserved size behavior

This follows Proteus3 `WorldManager.rangeRules` and
`ConcatReasoner.joinOnRight`, not a new bounds or wrapping policy:

- A negative displacement with a known extent is translated once by adding that
  extent: `*10-1` becomes `*10+9`.
- `fromContext` operands are added; other numeric operands compose radices.
  `(*10+9 +2)` remains `*10+11`; no modulo operation is introduced.
- There is no new general rejection of out-of-extent numeric literals.
- Signed arithmetic now uses the actual negative operand; `(*10+1 -2)` yields
  a numeric payload of -1. This fixes the old magnitude-only subtraction bug.

`numericExtentApplied` records that a payload has already been interpreted
against a known extent. It is carried through fact combination and publication,
including copies, so an already translated negative result is not translated
again on replay. It is separate from the sign, `sizeMode`, and `isNormed`.
Facts acquired with a new size use the same projection for checking and writing.
Comparisons that need an unknown extent remain pending rather than proving a
contradiction. No graph scheduler or cube-specific rule was added.

The historical permissive finite-value policy is intentionally retained, even
for out-of-extent results. This change does not define a new serialization or
bounds policy for those cases.

## Remaining boundaries

The temporary probe `*10+_ == -1` still stays symbolic: the range matcher does
not yet use the target's extent to interpret that unsized negative input.
Ordinary equality and definition-supplied extents are covered by this change.
Another range-consistency probe, `_ = (*10+9 +2)`, produces payload 11 but then
rejects through range-expression provenance; `_ = *10+11` succeeds. The negative
computed counterpart has the same issue. The range reader treats the retained
unknown target as a bounded range after learning its size. Resolving that
inconsistency requires a deliberate bounds/provenance decision; this patch
leaves it visible and does not treat it as a successful copy. The size-mode tests
check composition and replay; literal-copy tests separately check preservation
of the one-time extent translation.
The four original-state cube queries remain unresolved after the isolated
negative-definition blocker is repaired. Verb application and preservation
across modeled changes have not been implemented by this numeric change.
See [the cube exercise](cubie-transition-exercise.md).

## Validation

- Full CodeDog regeneration and C++ build passed.
- `unit/graphSignedNumber/`: **5/5**. Checks the signed parser payload, retained
  reciprocal-size flag, raw literal round trips, one-time extent translation,
  replay/copy stability, later size publication, contradiction on explicit
  re-normalization, size-mode composition, and read-only declaration bodies.
- `graph/numeric-literal-tests.tst`: **16/16**, including negative definition
  queries, definition-supplied size, matching signed equality, negative slices
  and end-relative item selection.
- Complete default inventory: **1603/1643 passed**, with exactly the same 40
  failure names as `/tmp/p4-force-rule-full.log` (the preceding 1598/1638
  baseline). All **1257 registered graph tests passed**. Final log:
  `/tmp/p4-signed-full-final.log`. No legacy expectation was changed.
- The separate cube exercise is **4/8**; its four full-model queries remain
  unresolved. No successful verb, transition, or GUI behavior is claimed.

The generated-code sign operation was also checked: CodeDog's FlexNum unary
minus mutates its operand, so the shared extent projection uses GMP arithmetic
on a value copy rather than negating the extent in a comparison.
