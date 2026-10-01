# One-cubie transition exercise

The first change uses the actual cube model: edge cubie 10, at
`location:{xLoc:-1.1, yLoc:0, zLoc:-1.1}`, changes `xRot` from `0` to `0.5`.
This is the small orientation edit already present in `Resources/userKB.pr`'s
`solvedState2`; it is not yet a physical face turn.

`Resources/TestFiles/rubixTransition.pr` imports the existing Foundation and cube
models and places `solvedState` before an unknown second state. These public
model assets exist in this workspace but are not tracked by the Proteus4 Git
repository. The exercise requires them in the runtime assets directory.

## Required transition observations

- Later selected cubie xRot is 0.5; earlier xRot remains 0.
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

The second state remains `?` until this transition is implemented. The rule
applies to a modeled change; this fixture does not invent a transition between
arbitrary disconnected partial descriptions. Concrete verb syntax, dependency
tracking, and how tentative preservation responds to later evidence remain
implementation/design work.

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
negative-definition regressions, and original-state cube reads. It does not yet
test successful verb application or claim preserved after-state values.

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

The current fixture intentionally leaves its second state unknown while that
behavior is implemented. This checkpoint establishes original-state access and
mapping regressions; it does not implement automatic preservation, verb
application, or the partial description of `solvedState2`.

Validation for this groundwork: source build succeeded; temporal graph tests
**51/51**, cube groundwork **8/8**, and `git diff --check` passed. Logs are
`/tmp/p4-temporal-return-build.log`, `/tmp/p4-temporal-return-51.log`, and
`/tmp/p4-temporal-return-cube-final.log`. The complete legacy suite and GUI were
not exercised; no engine implementation or production cube asset changed.
