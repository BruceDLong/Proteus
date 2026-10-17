# Fixed Rubix verbs and the TCursor viewer

The public model now supports:

```proteus
{T
    solved-rubix-turn-state
    turn-red-face-clockwise
    turn-blue-face-counterclockwise
}
```

Both directions exist for red, orange, white, yellow, blue and green. Clockwise
is viewed from outside the face named by its center color. Each verb describes
nine changed positions. The cursor retains the other positions and the cube's
overall pose. These are ordinary model definitions; no cube dispatch was added
to WorldManager and no path-cost planner was introduced.

## Representation and implementation

`World/tools/generate_rubix_verbs.py` reuses the existing authored cube geometry
and emits `public/Things/rubixVerbs.pr`. The original cube and old two-state turn
relation files remain unchanged, including their preexisting local edits.
Named positions denote places; occupants carry sticker descriptions. A frame
index denotes one of the 24 cube orientations, never a cubie identity. Per-turn
permutations compose those frames without approximate Euler-angle addition or
numeric wrapping.

The rendered parts and faces are derived from the inventory. Face views address
the existing ordered display positions; no redundant slot metadata is added to
cubie descriptions. This preserves the existing surface-color self references.
A preliminary redundant metadata field exposed an unresolved candidate in
ordinary type search, so it was removed rather than broadening search semantics
for this feature. All six actual drawing surfaces are tested below.

The maintained engine change is thirteen added lines and three removed lines:

- Temporal relative references climb structural ancestors to the nearest T-level.
- Computed indices inherit navigation and template context.
- Published nominal qualifications remain readable before the occurrence's own
  meaning is demanded; an obsolete scalar-milestone guard was removed.
- A closed previous record cannot prohibit fields supplied by the new state.
  Retention is distinct from assertion. The new state's explicit extent is still
  checked against the result.

D069 records these choices and their limits. The synthetic regressions are part
of `unit/worldManager/tCursor` and do not load live world or user files.

## Validation

CodeDog regenerated the shared engine and test executable successfully. Final
build times were 167.41 seconds for the library and 232.04 seconds for tests.
Slipstream's CodeDog build succeeded in 93.79 seconds; the final generated asset
copy was refreshed from the tested public model and its SHA-256 matched.

The full TestProteus run reports **24 failures / 535 registrations**, with the
same failure names as the preceding baseline: sixteen default-language cases
and eight historical compiled cases. All 57 WorldManager suites pass, including
**273/273 TCursor checks**. All fifteen protected language fixtures are unchanged.
The new regressions exercise nested prior-state references, contextual indices,
record growth, repeated qualified projection reads, immutable source descriptions,
unrelated deferred work and bounded per-step work.

Run the model integration checks with:

```sh
cd /home/bruce/devl/World
python3 tools/generate_rubix_verbs.py --check
python3 tests/check_rubix_verbs.py --engine ../Proteus4-redesign
```

Using the regenerated shared library, these validate all twelve verbs over
**206 turns**, every inverse and four-turn identity, mixed-face sequences of
8/32/128 steps, 78 projected cubies and **468 drawing surfaces**. Physical positions
and complete orientation frames are checked with an independent rotation oracle.
The tests also check unchanged source states/definitions, stable cursor storage,
one retained request and preservation of the overall zLoc:3.5. Mixed sequences
use 536–554 frames per step for all three lengths; there is no temporal-history
replay. This fixed-size result is not a general bound for arbitrary inference.

The actual Slipstream GUI was run against the final shared library on a task-owned
virtual display. Screenshots confirm colored solved, red-turn and blue-turn views.
The Next, Next and Previous controls return success through TCursor. The existing
preview rotation remains active; screen orientation is not itself the state oracle.
The two preexisting world-loading NRMERR messages remain, with no DISPLAY failures
in this run. The cube path and world-loading completeness are separate claims.

Detailed logs, checksums and screenshots are in `/tmp/rubix-verbs/`, including
`full-final.log`, `model-final.log`, `validation.json`, `navigation.log` and
`state1.png`/`state2.png`/`state3.png`/`previous.png`.

## Remaining scope

These are fixed explicit verbs, not parameterized verbs or ancestor temporal
listSpec execution. Cursor backward navigation still has its existing immediate
snapshot behavior; inverse face-turn verbs are available but no general inverse
solver or history reconstruction was added. Turn interpolation/animation between
states is not implemented. The demo source and README edits remain alongside the
preexisting Slipstream migration changes rather than committing unrelated work.
