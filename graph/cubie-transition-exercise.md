# One-cubie transition exercise

The current target is edge cubie 1, at
`location:{xLoc:0, yLoc:-1.1, zLoc:1.1}`: change `xRot` from `0` to `0.3` in
T-item 2. This is a single-cubie orientation edit, not a physical face turn.
Earlier checkpoints below used edge cubie 10 and `0.5`.

`Resources/TestFiles/rubixTransition.pr` imports the Foundation and cube models.
Its second state now retains the parent of one leaf constraint:

```proteus
rubixState.edgeCubes@[1].thing.orientation.xRot=0.3;;;;;
```

The five semicolons return through the five selected descendants to the
`rubixState`. This uses existing retained-path syntax. The matching change in
Slipstream's `Resources/userKB.pr` removes the duplicated `solvedState2` body;
its outer T-item also relies on preservation for the omitted whole-cube `thing`.
The test history contains direct rubixState values; the GUI history wraps them
with that whole-object thing description.

The shared public model is tracked in the separate `/home/bruce/devl/World`
repository. Its center/edge/corner group record is now unordered. Cubies within
each group and the six stickers remain ordered, because their positions matter.
The single-color side representation remains unchanged.

## Required transition observations

- Later selected cubie xRot is 0.3; earlier xRot remains 0.
- Its yRot, zRot, location, and sticker information remain available.
- A neighboring cubie retains its known values.
- Information genuinely unknown in the source is not invented.
- The transition does not identify the complete earlier and later states.
- Re-normalization does not duplicate the transition or alter the earlier state.

The user clarified the preservation rule on 2026-09-29: a change, including
one expressed by a verb, preserves other values unless the applicable model
implies changes to them. Implied changes can propagate through causal chains;
forward pedaling on a functioning bicycle can imply chain and wheel movement,
and typical riding can imply a location change. Preservation must account for
those consequences, not merely the directly written changes.

The partial second-state statement is now an acceptance case. T-list adjacency
supplies preservation, as clarified below; implementing the demand-driven
correspondence and its evidence remains necessary. Inferred changes must retain
precedence over inherited values, without identifying successive occurrences.

## First reproduced blocker

Before applying a verb, querying the original model remained symbolic. The
initial smaller reproducer was:

```proteus
{#pose={xRot:0 xLoc:-1.1} pose.xRot}
```

The known xRot remained `pose.xRot`. Replacing -1.1 with 1.1 resolved it.
The same problem occurred with -1. Both negative cases now resolve.

The original parser represented the minus using `infonState.inverted` and sets an
unsized explicitly signed value to `sizeMode=fromContext`.
`ProteusGraphAccess.plainStructure` rejected that representation for ordinary
fact reads; `typeFacts` was therefore invalid, and `ProteusDefinitions.children`
could not compile the containing definition. `predicateFacts` also excluded
`fromContext` sizes; removing only the inverted-value guard would not suffice. This blocked unrelated known fields.
The agreed repair stores a literal numeric sign directly in `FlexNum`, while
retaining the existing interpretation against a known size. The negative value
remains usable before that size is known. See [signed-number-results.md](signed-number-results.md).

The general issue is exposing usable parts of a definition while other parts
remain symbolic, or faithfully carrying the complemented representation through
application. It must be solved without editing away negative cube coordinates
or treating a partially checked definition as fully validated.

## Running the groundwork

After the usual build/resource staging, from `LocalBuild`:

```sh
./TestProteus -t ../rubixTransitionTests.tst:
```

The suite is separate from the default inventory. It includes positive controls,
negative-definition regressions, original-state reads, and explicit acceptance
checks for the changed rotation, unchanged sibling rotation, location, sticker,
and original state after reading the second. Unimplemented behavior remains a
failure; these expectations are not skips or adjusted to incomplete output.

## Initial result (2026-09-29)

Using the existing executable with isolated temporary runtime assets: **2 pass /
6 fail**. The positive-coordinate control and contradictory temporal-equality
check pass. Both complemented-coordinate definition checks and all four full
cube reads fail. The full model may expose further issues after the isolated
definition blocker is addressed; these results do not attribute every unresolved
route exclusively to that blocker.

Log: `/tmp/p4-cube-transition-groundwork.log`. No engine code changed, no rebuild
was needed, and no GUI or complete legacy-suite validation was performed.

## After the signed-number repair

The isolated negative fractional and integer definition tests now pass. The
exercise is **4 pass / 4 fail**, compared with its initial 2 pass / 6 fail.
All four full cube reads still remain symbolic; successful transitions are not
claimed. Log: `/tmp/p4-signed-cube-final.log`.
The Foundation location and orientation definitions and cube coordinates were
not changed. See [signed-number-results.md](signed-number-results.md) for the
representation change, sizing compatibility, and focused validation.

## Located temporal cuts and full-model loading diagnosis

See [temporal-cut-results.md](temporal-cut-results.md) for explicit history
mapping, unchanged S-values at an aligned cut, and the real stored cube-data
exercise. This is a graph-API transition test, not completed verb application.

At that checkpoint, the four unresolved full-world reads had two separate
blockers: `%INSERT` had not been migrated to graph dispatch, and direct loading
exposed whole-definition compilation waiting on the unrelated contextual
`centerCube` template. Earlier
cube fixture failures did not by themselves prove that the imported definitions
were loaded. The small signed-number regressions remain valid independent checks.


## Return to temporal work (2026-10-01)

Stream lock refinement is deferred until heavier stream use provides evidence.
The next semantic objective remains a partial change with preserved unrelated
knowledge and consequences obtained through ordinary model relationships.

Re-running the untouched cube groundwork still produced 4/8 passes, but the
old import and definition diagnosis was no longer the first failure. Inspection
of the live world and the search steps showed:

1. Both imports loaded, their definitions were registered, and the initial cube
   already contained the written orientations and locations.
2. `%W.cubeTransition` stopped at the first unnamed imported list. In an ordered
   field search, an untyped earlier member cannot silently be excluded.
3. Naming the import container exposed the history, but indexed access still
   waited for its order. `T` specifies temporal composition independently of
   order; the previously undefined `cubeTransition` type supplied no order.

The fixture now names the import container `libraries` and defines
`#cubeTransition = {T ? ?}`. This states its intended two-position ordered shape
without supplying either state's values. No engine matching rule or cube asset
was changed. The four queries now reach the original known values. Their expected
printing retains the established open oneOf form, for example
`*_+[xRot:0 ... ]`, because the path crosses unordered partial descriptions.
The known value is tested without claiming that those descriptions are closed.

Three added temporal regressions establish the next step's graph groundwork:

- A chain of history identities, including a cycle, reaches the same before and
  after occurrences without identifying those occurrences. Repeating the query
  adds no identities.
- A missing later value stays subscribed through that chain; supplying the value
  resolves the existing handle and leaves the earlier value unchanged.
- Two independent histories can have equal initial values and the same aligned
  cut while retaining different later values. Neither coincidence nor boundary
  alignment is a history mapping.

These are history-identity cases, not a general causal-rule evaluator. More
complex modeled relationships must expose their own constraints and demands.

## Agreed state succession and next implementation boundary

The user clarified on 2026-10-01 that T-list adjacency itself supplies
preservation. In `{T {a:0, b:7} {a:1}}`, the second state retains `b:7` unless
an applicable model implies a change to it. A separate transition operator is
not required. In the cube history, position 1 is the initial state and position
2 is the next state. A reference to a cubelet in state 2 targets that later
occurrence, with an RHS specifying its orientation change. The known initial
state remains intact.

The graph should expose that correspondence from existing T-list relationships
and POVs. It must not duplicate the entire state or identify entire successive
states. A requested later part can obtain unchanged information from the prior
state, while explicit and model-implied changes determine affected parts.
Preservation needs retained derivation evidence, not an unconditional equality
with an old value: discovering an applicable consequence must not manufacture a
contradiction against a default or retract an authored earlier-state fact. The
existing history-mapping goals already supply the explicit mapping case; general
parent/part projection and preservation remain to be implemented.

At that checkpoint the fixture left its second state unknown. It established
original-state access and mapping regressions; it did not implement automatic
preservation or verb application. The newer fixture described above makes the
partial second state and its acceptance observations explicit.

Validation for this groundwork: source build succeeded; temporal graph tests
**51/51**, cube groundwork **8/8**, and `git diff --check` passed. Logs are
`/tmp/p4-temporal-return-build.log`, `/tmp/p4-temporal-return-51.log`, and
`/tmp/p4-temporal-return-cube-final.log`. The complete legacy suite and GUI were
not exercised; no engine implementation or production cube asset changed.

## Unordered model and single-target path groundwork

Early definition exposure now includes written unordered fields, even though the
record's total membership is unknown. Parser-pending bodies, explicit unfinished
syntax, member templates and embedded root spans retain their existing goals.
This fallback is only for unfinished compilation: it cannot bypass a compiled
plan whose member correspondence is ambiguous. It never grants a type certificate.

The path-write clarification and its regressions are documented in
[path-write-results.md](path-write-results.md). There is no cube-specific edge or
normalizer branch.

The GUI `<` and `>` buttons select physical T-items through `TimeView`. The
final isolated run exercised first -> second -> first. The first state displays
colored cubies and returns after switching back. The second displays colored,
overlapping geometry: inherited cubie locations and other rotations remain
unknown. Captures are under `/tmp/p4-cube-state-ui-final/`. This confirms control
behavior and the initial rendering, not completed temporal preservation. The
user's live window was not controlled.

The one-world batch of 11 query assertions passes eight. The selected second
X-rotation is 0.3, the requested sticker hue is 160, all initial-state reads pass,
and reading the second state leaves the original X-rotation at 0. The three
failures are second-state Y-rotation, location Z, and a neighboring cubie's
X-rotation. They remain acceptance failures for preservation, not changed
expectations. The standalone 15-case file was not run to completion; the batch
avoids reloading and normalizing the full model for every query. Log:
`/tmp/p4-cube-transition-verified-batch.log` (rerun against the final source and
model). The initial-state-only fixture also passed
five independent position/orientation queries.

The unknown-description question was settled on 2026-10-02 with a separate
`%?` marker: `b:%?` explicitly withholds preservation, while ordinary `b:?`
retains its ordinary infon meaning. The marker permits later constraints to
supply any compatible value, including the earlier value. No new `!%` behavior
is requested. Automatic preservation is not implemented by this checkpoint.

Final source verification: Proteus4 and Slipstream builds succeeded; all **1,332
graph tests across 62 suites** passed, including the new unordered-definition
case and eight path-write regressions. Four parsing checks cover the actual
shared cube model, Slipstream knowledge base, transition fixture, and retained
path syntax. The complete legacy suite was not run for this checkpoint. Logs:
`/tmp/p4-cube-verified-build.log`, `/tmp/p4-cube-verified-audit/results.json`,
`/tmp/p4-cube-model-final-parse.log`, and
`/tmp/p4-cube-slipstream-final-build.log`.


## Guarded adjacency preservation (2026-10-02)

The unchanged one-world batch now passes **all 11 query assertions**. In state 2,
the selected first edge cubie has X-rotation 0.3, preserved Y-rotation 0, and
preserved location Z 1.1. The neighboring cubie's X-rotation remains 0. The
sticker hue remains 160, and rereading state 1 still gives X-rotation 0.
The three previous acceptance failures are resolved without changing the world
model or expected query strings. The standalone 15-case file was not rerun;
this batch loads the same actual cube once and checks the 11 retained observations.

These results use the general routes in `TemporalPreservation.h`: previous
occurrence navigation, finite ordered/nominal part correspondence, current
constraint guards, and ordinary scalar Sets. No cube-specific solver branch was
added. The `%?` behavior and scope are documented in
[temporal-context-results.md](temporal-context-results.md).

Performance is still a limitation. The successful batch took **330.45 seconds**
with peak RSS **196,964 KiB** on this run; source regeneration overlapped its
early portion, so this is an integration observation, not a controlled benchmark.
The run now resolves parts that the previous checkpoint left unknown. Earlier
150/180-second bounded attempts did not finish. Preservation Sets now publish
specific changed objects and shared state slots instead of a broad graph change,
but the full model still performs substantial normalization work. Do not treat
passing values as proof of interactive rendering speed.

Evidence: `/tmp/p4-preservation-final-cube.log` and
`/tmp/p4-cube-transition-batch.tst`. The harness reports this as one passing test
containing 11 observations. No Slipstream GUI run was made for this increment.


## Cube timing after shared-definition helper fix (2026-10-02)

The same one-world batch passes all **11 observations** with both helper
implementations. The changed X-rotation is 0.3; the preserved Y-rotation,
location, neighboring cubie, and sticker values still resolve, and rereading
state 1 returns X-rotation 0.

| Implementation | Elapsed time | Peak RSS | Observations |
| --- | ---: | ---: | ---: |
| Previous helpers (`7a354e3`) | 312.34 s | 193,376 KiB | 11/11 |
| DAG-aware helpers (`68196a0`) | 326.43 s | 198,336 KiB | 11/11 |

This pair shows **no cube speedup**: the updated run was 4.5% slower and used
4,960 KiB more peak RSS. These are single sequential runs, updated first and
baseline second; the small difference does not distinguish helper bookkeeping
cost from the engine's known scheduling variation. The dramatic improvement on
deep shared definition alternatives does not establish an improvement for this
world-model workload. Repeated definition-plan construction and broad definition
notifications remain candidates from the earlier cube profile.

The isolated baseline uses the current generated program with only
`graph/DefinitionPlan.h` restored to `7a354e3`. Compiler flags, model assets,
empty session file, test catalogue, and query assertions match. Neither measured
run overlaps a build or uses profiling counters. Both include loading and
normalizing one world plus its observation queries. Incomplete attempts made
while aligning the isolated test catalogue were excluded.

Evidence: `/tmp/p4-helper-cube-after.log`, `/tmp/p4-helper-cube-before.log`,
`/tmp/p4-helper-cube-comparison.json`, and the unchanged
`/tmp/p4-cube-transition-batch.tst`. Model/binary hashes are recorded in
`/tmp/p4-helper-cube-before/manifest.json`. No engine or model source changed
in this check. GUI frame rate and the standalone 15-case fixture were not tested.


## Cube timing after scoped definition publication (2026-10-02)

Ordinary definition plans now notify their instance destinations and shared state
readers. Plans with potential registry effects retain the broad notification.
The notification change and its regression coverage are described in
[the traversal report](traversal-profile-results.md#scoped-definition-publication-2026-10-02).

| Definition publication | Elapsed time | Peak RSS | Observations |
| --- | ---: | ---: | ---: |
| Previous model-wide publication | 324.90 s | 178,708 KiB | 11/11 |
| Scoped instance publication | 311.71 s | 158,964 KiB | 11/11 |

This is **4.1% less time and 11.0% less peak memory in one pair**. The cube still
takes roughly five minutes for this batch. The timing difference is modest and
may include scheduling variation; it does not establish a large cube speedup.
Both versions resolve the changed X-rotation to 0.3, preserve the other checked
fields and neighboring cubie, and return the original first-state X-rotation 0.

The isolated control uses the same generated tests and headers with only
`DefinitionApplications.h` restored to the pre-change version. Thus both contain
the DAG-aware definition helpers. Compiler flags, model assets, empty session
files, catalogue, and 11-observation fixture match. Runs were sequential, scoped
first and control second, with no diagnostic counters or concurrent builds.
The measurement includes loading/normalizing one world and its queries; the
harness reports the 11-observation batch as **0 failures / 1 test**.

Artifacts: `/tmp/p4-definition-publication-cube/{after,before}.log`,
`{after,before}.time`, `results.json`, and `manifest.json`. The unchanged fixture
is `/tmp/p4-cube-transition-batch.tst`. All 1,388 registered graph tests passed
separately. GUI frame rate and the full legacy suite were not measured here.
