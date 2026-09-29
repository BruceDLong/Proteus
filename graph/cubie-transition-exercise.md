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
