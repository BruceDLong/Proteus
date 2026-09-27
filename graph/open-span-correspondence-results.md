# Correspondence between open spans

The shared span planner now recognizes containment between two open layouts.
After matching their anchored ends, one remaining span can contain the other
layout's remaining sequence, including its unresolved inner spans. For example:

```proteus
{1 ... 5} = {_ 2 ... 4 _}
```

The result preserves the unknown middle:

```proteus
*_+{1 &{2 ... 4} 5}
```

The planner records a splice; the existing correspondence goal publishes it with
ordinary member creation and identity relations. New members retain source type
identity and order metadata. Unknown source spans remain
embedded spans. It never enumerates candidate lengths or materializes each item
of a sparse span. A billion-position source stays compressed.

The outer physical tail can close while the nested unknown span stays open.
Normal size/value goals and subscriptions follow the retained identities, so
later input fills the same result; contradictions still fail. Replaying the
normalization does not append duplicate spans or identities. Route costs include
member creation, and cancellation stops later propagation.

RHS representation proofs now inspect explicit positions in open layouts using
the same alignment plan. A bare unknown source gap adds no facts, but written
positions and bounded subspans still need evidence. Hiding a redundant RHS does
not remove its graph relationship or claim the value is fully normalized.

## Boundaries

A referenced gap must denote its complete owner. Taking only an owner's trailing
remainder needs an offset view and is still deferred. Crossing unresolved spans,
repetition-template splicing, parser-owned tails, and general ambiguous overlap
are not assigned guessed layouts. Definition planning explicitly defers splices
rather than treating them as finite positional fills.

The original `merge/dots/align2a` and `align2b` assertions retain their legacy
embedded-empty-span spelling. Two parser round-trip checks confirm that both
spellings print as the current canonical form. The exact original assertions
remain reported as failures; their expected outputs are unchanged.

## Validation

- Source regeneration: `Marker: Build Successful`.
- 890/890 focused checks, including 13 open-span cases and two parser round-trip
  checks. Coverage includes contained and nested spans, both equality directions,
  multiple inner gaps, finite-prefix acquisition, late values and contradictions,
  cancellation, crossing-span deferral, compressed billion-position spans,
  route-cost limits, endpoint conflicts, source type preservation, and replay.
- Standalone shared span planner: 272/272 cases passed.
- Unchanged original default suite: 192/270 passing, with no prior pass lost,
  crash, or timeout. `merge/list/dotsC` is recovered. The legacy spelling
  assertions described above remain counted as failures.

Logs: `/tmp/p4-composition-source-build.log`,
`/tmp/p4-composition-final-summary.log`, and
`/tmp/p4-composition-final-default/results.json`.
