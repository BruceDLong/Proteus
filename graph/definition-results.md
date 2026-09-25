# Definitions and scalar type applications — 2026-09-24

This first slice registers definitions and applies unambiguous context-free
numeric/string definitions and aliases through the graph normalizer.

For example, after registering `{#seven=*10+7}`, normalizing `seven` obtains its
numeric overlay, size 10, and value 7. `{#digit=*10+_}` constrains `digit:7`
without manufacturing a value for `digit` alone. Alias chains and scalar body
RHSs combine their constraints before the application Set executes. Conflicting
known fields fail instead of being overwritten. Explicit sizes survive normal
printing and reparsing, including when size and value came from separate body
constraints.

The catalogue belongs to the graph context used by the Agent. Normalizing a
word declaration publishes its existing body and allows subscribers to resume;
re-registering that same declaration is idempotent. Definitions can follow
uses in a loaded list or arrive through a stream. Distinct declarations with
the same name stay ambiguous. Alias cycles stay symbolic without generating
an unbounded chain of copied RHSs. A registered declaration counts as one item
in its containing list; registration does not require evaluating its body.

The implementation is divided into three parts:

- `GraphFoundation.Lib.dog` reads the existing infon representation and applies
  scalar facts. `infonIO.dog` initializes a bare named use's size as unknown,
  because that parser branch skips ordinary scalar initialization.
- `graph/DefinitionApplications.h` owns name bindings, scalar requirement
  certificates, and the type GoalSpec. Certificates record definition facts,
  alias bindings, and body RHS witnesses. They are separate from completion
  flags and survive unrelated graph revisions.
- `graph/ProteusGraph.h` exposes declaration publication and definition walks
  through ordinary graph demands, edges, and required subgoals. Cost filtering,
  cancellation, subscriptions, and revision restarts use the unchanged generic
  graph runtime.

Shared definition bodies remain read-only. No definition RHSs are appended to
scalar uses. Already-checked infons reuse their certificates, including through
another POV. A compatible copied result can inherit the same certificate when
all required facts are present at its destination. A size-only copy cannot
inherit proof of a value it did not copy. New type requirements, changed bodies,
new ambiguity, and conflicting target RHSs are still checked.

This reuse currently applies only to context-free scalar constraints. `%prev`,
context-derived numeric sizes, list/record definitions, sparse member templates,
usage tags, morphological lookup, general alternative-meaning search, and
contextual bindings remain symbolic or unsupported. The catalogue does not
populate the legacy global ModelManager. Structured object validation such as
location/orientation or hue/brightness is the next-stage consumer of the same
reuse principle; it is not claimed as implemented here. Certificate witnesses
are checked on reuse, so avoiding expansion is not a constant-time promise.

Validation:

- Full CodeDog generation and C++ builds passed for `Proteus.Lib.dog` and
  `streamReplyTest.dog`.
- `unit/graphDefinitions/`: **36/36 passed**. Coverage includes scalar and
  string meanings, aliases, forward uses, independent occurrences, read-only
  bodies, repeat registration/normalization, shared-infon POVs, copied
  certificates, partial-copy boundaries, added types, changed definitions,
  unrelated revisions, ambiguity, conflicts, cycles, subscriptions,
  cancellation, cost exclusion, context isolation, public queries, real
  streaming definition/use input, and explicit-size rendering round trips.
- Existing graph foundation: **155/155 passed**; entry/lifecycle: **16/16**;
  growing RHS streams: **19/19**.
- Original parser fixtures (`parser/`): **53/53 passed**.
- Exact public graph/printer probes: **2/2 passed**, producing
  `{#hex = *16+10 hex:*16+10}` and `{#hello = 'hi' hello:'hi'}`.
- Standalone streaming selections `1, 2, 4, 5, 6, 8`: **33/33 checks passed**.
- `git diff --check` passed.

Reproduce the focused suite after building:

```sh
cd LocalBuild
./TestProteus -t unit/graphDefinitions/
./TestProteus -t unit/graphFoundation/
./TestProteus -t unit/graphEntry/
./TestProteus -t unit/graphStream/
./TestProteus -t parser/
```

The full legacy suite and ThreadSanitizer were not run. No original Proteus
checkout files were changed and no remote was added to the private fork.
