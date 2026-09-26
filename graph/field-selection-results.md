# Scalar field selection through the graph

This records the initial scalar checkpoint. The examples below use explicit
aliases to follow the subsequently corrected nominal contract. See
[structured-field-results.md](structured-field-results.md) for optional
definitions, finite structures, and replaceable field-result links.

Single-step dot queries now share a goal with
`GraphNormalizer.findField(sourcePOV, predicatePOV)`. Both use existing logical
members and read-only type meanings. The direct API returns a handle whose
`result()` is the selected source POV or a symbolic oneOf. Dot syntax publishes
the result on its working POV and then uses ordinary normalization.

```proteus
{#want=*10+_ #left=want #right=want}
{left:*10+1 right:*10+2}.want
// left:*10+1

{left:*10+1, right:*10+2}.want
// *_+[left:*10+1 right:*10+2]

{2 3 'Cat' 'Hat'}.$
// 'Cat'
```

## One match rule, two result policies

`ScalarPredicate.h` contains the three-state comparison previously used by
sequence matching. Both consumers now use it. A known contradiction excludes a
candidate; missing required facts remain unknown. Definitions supply constraints
without changing their shared bodies or inserting application RHSs.

`FieldSelection.h` advances through `ListStructure.h`. Ordered lookup cannot
advance past an unknown ordinary member until a refinement or later publication
classifies it. Unordered lookup can inspect subsequent members but does not
publish its result until relevant candidates and the end have been classified.
Embedded spans retain their logical boundaries. A known zero-width span can be
crossed; an unresolved/sparse nonempty span stays visible as missing work without
being expanded. An unordered span inside an ordered source remains deferred.

An ordered result is the original selected POV. Multiple unordered results use
the existing unknown-size intersection representation. Its alternative POVs
refer to the source infonViews and have explicit source identities. This is a
symbolic choice, not a concatenation or a choice based on whichever route is
cheapest. Further oneOf solving remains separate work. A single unordered match
reduces to that member.

Candidate refinements keep their containing-list context, including embedded
owners, so relative queries do not silently acquire a different context. Cost,
publication, subscription, and cancellation remain in the unchanged generic
scheduler. Result creation happens in a priced Set, after the match proof.

## Dot-path integration and retained evidence

Dot preparation makes small source/predicate views over the existing data. It
does not clone list members or shared definitions. The working result retains
the original expression for unresolved rendering, source/predicate provenance,
and the accepted candidate witnesses. Replay reuses the result and its identities.
Scalar results use ordinary identities and type-evidence inheritance; a oneOf
retains its symbolic state. Reply flags and source positions follow the working
result, while the retained expression does not reintroduce a claimed reply marker.

This first lowering admits a single dot read. Chained paths, semicolon-retained
parents, writes, and other path kinds remain untouched rather than losing their
continuations. A dot expression on an RHS is exposed as an ordinary refinement.

## Boundaries

The matcher currently admits scalar constraints and unique context-free scalar
definitions. Structured-member predicates, contextual/overloaded meanings,
source-list constraints/types, nonempty virtual spans, parser-owned unfinished
lists, and early publication of partial unordered alternatives remain pending.
Missing or ambiguous definitions and no proved result leave a subscribed search.
Selection does not fabricate a missing scalar size or mark a symbolic choice
`isNormed`.

The user resolved this question: separate member-type definitions are optional.
They then retracted structural type equivalence. `.wheel` must not select a pedal
based only on matching structure. The linked structured-field checkpoint records
both corrections and the current limitations.

## Validation

The final source was regenerated with `codeDog Proteus.Lib.dog`; the build
reported `Marker: Build Successful`. Temporary generated-C++ experiments were
replaced by this regeneration before running the final checks.

- Field goal tests: 22/22.
- Dot-path tests: 13/13, including deferred continuations, expression RHSs,
  subscriptions, cancellation, replay, and reply metadata.
- Existing focused matching, list, definition, representation, navigation,
  entry/stream, parser, and numeric checks: 611/611.
- Total focused checks: 646/646.
- Original default fixtures: 161/270 pass; 109 remain failing assertions.
  No crashes, timeouts, incomplete summaries, or previous passes lost.

The two recovered fixtures are `read/readStrA` and `temporal/sBeginField`.
The latter exercises ordinary field selection, not temporal boundary reasoning;
its bare numeric size remains unknown. Original expectations were not changed.

Replay checks include scalar meaning facts that have not been copied into input
members. Alternative identities sharing an exact infonView add no new matching
obligation. The result and identities are reused without mutating those members
or adding redundant type RHSs.

Final logs: `/tmp/p4-field-final-regenerated-build.log`,
`/tmp/p4-field-verified-focused.json`, and
`/tmp/p4-field-verified-default/results.json`. The portable compatibility
inventory is [language-compatibility-status.json](language-compatibility-status.json).
No full legacy-unit, sanitizer, or GUI pass is claimed.
