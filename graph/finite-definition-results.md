# Finite list definitions — 2026-09-25

After registering `{#pair={*10+1 *10+2}}`, normalizing `pair` produces
`pair:{*10+1 *10+2}` with `infSize = 2` and normalized independent members.
An ordinary nested list counts as one member. Embedded list spans contribute
their flattened positions. Member scalar sizes do not enter the list count.

Supported cases include empty lists, nested lists, embedded list spans, alias
chains, named scalar members, and compatible list RHSs in a definition. Existing
instance members are filled in place; uniquely determined open instance tails
can acquire their missing finite members. Unknown scalar requirements remain
unknown and subscribed until additional information arrives. Instantiating
`{#pair={*10+_ *10+_}}` twice can therefore produce different member values
without changing either the other instance or the definition.

The application has three stages:

1. The existing definition graph walk visits body POVs, aliases, RHSs, and
   physical members. Postorder leave edges compile their requirements. Cycles,
   missing meanings, and unsupported bodies cannot reach application.
2. `DefinitionPlan.h` checks those requirements against the instance. It reuses
   the existing `SpanAlignment.h` planner, including its unique-gap reasoning.
   The plan contains only pending writes, member allocations, and certificate
   bindings. There is no persistent replacement graph or copied definition
   infon tree. Ambiguous layouts stay symbolic without enumerating partitions.
3. The graph manager prices the plan before its Set executes. The Set fills
   available size/value/overlay fields together, establishes orderedness, and
   creates only absent instance members. An incompatible later member rejects
   this batch before its earlier writes. Previously completed Sets from other
   goals are not rolled back.

Shared definition bodies remain read-only. No application adds definition RHSs
or bidirectional identities to them. The normalizer still checks an instance's
existing work constraints. Created members preserve their named types, while
existing type labels are retained. Completion still requires every member to
normalize; a known list extent alone cannot set `isNormed`.

Validation certificates include definition bindings, scalar facts, body RHSs,
and member POVs. Parent and member certificates share their evidence. Repeated
normalization reuses the same members and performs no additional application
Set. Unrelated revisions preserve certificates. Copied checked lists transfer
evidence only after their destination has the required contents, including
when the destination originally had no value. A size-only copy cannot transfer
proof of missing members. New constraints, modified bodies, or replaced member
POVs trigger validation again. Equivalent type conjunctions are canonicalized
for evidence reuse regardless of their application order.

Order is explicit once established by an ordered definition or copied ordered
list. This avoids leaving a correctly populated named list symbolic merely
because its original order flag was unknown. Evidence checks and finite plan
construction still inspect data; no constant-time performance claim is made.
Cancellation takes effect between graph steps and atomic Set batches.

Validation:

- Full CodeDog generation and C++ builds passed for `Proteus.Lib.dog` and
  `streamReplyTest.dog`.
- `unit/graphListDefinitions/`: **41/41 passed**. This includes nested/embedded
  lists, independent occurrences, partial results and subscriptions, late
  definitions/member types, cycles, atomic conflict rejection, size and overlay
  conflicts, ambiguous tails, cost exclusion, cancellation, named-member
  evidence reuse, copies into populated and unknown destinations, partial-copy
  boundaries, modified/replaced definition members, public queries, and real
  streaming definition/use input. A billion-item template remains unexpanded.
- Scalar definition regressions: **36/36 passed**. The former finite-list
  deferral fixture now specifically tests an open definition body.
- Existing graph foundation: **155/155 passed**; entry/lifecycle: **16/16**;
  growing RHS streams: **19/19**; parser fixtures: **53/53**.
- Original `word/deref` language fixture: **1/1 passed**.
- Standalone streaming selections `1, 2, 4, 5, 6, 8`: **33/33 checks passed**.
- `git diff --check` passed.

After building, reproduce the focused checks from `LocalBuild`:

```sh
./TestProteus -t unit/graphListDefinitions/
./TestProteus -t unit/graphDefinitions/
./TestProteus -t unit/graphFoundation/
./TestProteus -t unit/graphEntry/
./TestProteus -t unit/graphStream/
./TestProteus -t parser/
./TestProteus -t word/deref
```

The full legacy suite and sanitizers were not rerun. The generic graph runtime
and span-alignment algorithm were unchanged. This is a private local Proteus4
change; the original Proteus checkout was not modified.

Remaining scope: closed physical ordered definition bodies are supported.
Open/sparse templates, unordered record matching, contextual paths, computed
definition expressions, and registry relationships inside shared bodies stay
deferred. A billion-item template stays compressed. Multiple writable positions
sharing the same underlying list need a joint plan and remain deferred. Lazy
member templates and record/field matching are the next implementation stages.
