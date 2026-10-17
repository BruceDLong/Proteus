# TCursor supplied-work milestone — 2026-10-14

TCursor uses the regular WorldManager solver to compute the next item's supplied work, then retains unaffected values from a stable previous view. It has one reusable current infon, real occurrence context, and a staged destination. No transition-authorization subsystem or parser qualifier change is introduced. In particular, `:` still permits a partial match from the start.

## Execution and ownership

The common internal `solve()` entry separates source, destination and occurrence context. Public normalize uses the same source and destination. A cursor step supplies the next occurrence and an isolated output destination, then uses the existing queue and publication transaction. When given work drains, that same queue applies one internal retention relationship through existing field/span correspondence. The previous value is read-only throughout the step.

The first milestone handles wrkList work and definitions, including their expression prerequisites. It stops additional ancestor-source discovery after that supplied work is exhausted. It has no tree-depth cutoff. Unrelated retained subtrees are borrowed descriptions; no descendant frames are created merely to carry them. A bare initial literal list is a seed description, not a claim that its child recipes have been fully evaluated.

TCursor shares adjacent navigation with ordinary relative-reference lookup. `%prev` uses the saved cursor value for the matching prior occurrence, so all changed fields see the same inputs. A pending step retains its exact request and waiting link; another movement call settles that step rather than advancing again. Agent cancellation and ordinary contradictory constraints prevent publication. `writeBack()` explicitly publishes the available current description. The shared copy helper gives a published root its own immediate POV wrappers and correct parent context while retaining immutable descendant payloads.

Successive steps retire the previous internal handle when it is still last in the dedicated cursor session. Pure movement therefore retains one internal request. Explicit write-back and other session requests remain Agent-tracked; caller-retained handles remain valid. A checked scalar copied into a snapshot retains its completion witness; reading it does not replay its earlier temporal recipe. Lists and unresolved descriptions keep their partial status where appropriate.

## Behavioral checks

The fresh `unit/worldManager/tCursor` suite covers:

- Direct numeric transitions and work supplied by definitions, with unknown numeric size preserved.
- Consecutive changes and changes interspersed with unchanged occurrences.
- Two-field permutation from one prior snapshot, nested record updates and fixed-length ordered correspondence.
- Unchanged scalar and material-subtree retention, source-model immutability and stable cursor storage.
- Existing `=:` partial-match composition, conflicting new assertions and `%?` independence at field and compound scope.
- Parser waits on headers and RHSs, unrelated arrivals, same-handle continuation and Agent cancellation.
- Explicit write-back, correct stored parent context and a written result surviving subsequent movement.
- Immediate backward navigation and proved initial boundaries.

The scalar suite's old `5 = %?` unsupported placeholder is replaced by assertions for the supported value, retained independence marker, immutable RHS and repeated completion. No protected language fixture or expected output is changed.

Two ownership regressions were reproduced before fixing them. First, carrying an unchanged named value discarded its completion witness; a later named rule replayed its recipe and reported a false cyclic prerequisite. Retaining the exact scalar proof fixes both the result and the work bound. Second, write-back initially left immediate stored members parented to the cursor. Sharing the root-description copy operation fixes the stored occurrence context without traversing untouched material.

## Work measurements

For the following rows, sizes are 8 / 32 / 128. Temporal counts exclude initialization and include every subsequent step.

| Family | Frames | Relationships |
| --- | --- | --- |
| Consecutive numeric increment rules | 49 / 217 / 889 | 28 / 124 / 508 |
| Consecutive named retention rules | 35 / 155 / 635 | 14 / 62 / 254 |
| Numeric rules alternating with unchanged items | 34 / 142 / 574 | 16 / 64 / 256 |
| Named rules alternating with unchanged items | 26 / 110 / 446 | 8 / 32 / 128 |
| Unused material descendants | 9 / 9 / 9 | 3 / 3 / 3 |
| Fields at the changed record level | 15 / 39 / 135 | 3 / 3 / 3 |

The cursor session retains one internal request across these chains. The named transition test computes an increment from an initial anonymous number; repeated named arithmetic still reaches the existing regular-engine `Nominal expression needs type rules` boundary. The scaling test therefore separately exercises named `%prev` retention and anonymous numeric arithmetic, rather than introducing a cursor arithmetic workaround.

These results establish linear work for the measured fixed-layout families. They do not establish constant-time wide-record output, general sparse alignment, arbitrary definition solving or history reconstruction. Owned root wrappers require work proportional to that immediate output level; borrowed descendant payloads are not copied or scanned.

## Remaining integration boundaries

Ancestor listSpec application to cursor steps and general primary-level materialization are subsequent work. Immediate backward movement can reuse the saved previous view; further movement uses available destination evidence, without reconstructing arbitrary accumulated history or inventing inverse rules. An unavailable new neighbour is reported separately from a proved boundary; subscribing to creation of a new neighbour is not implemented. Existing supplied headers and RHSs use ordinary parser subscriptions.

The real-model diagnostic loads copied rat/foundation/cube definitions successfully, then reports `Record meaning still has unresolved relationships` during initial cursor filling. It publishes no cube view and therefore cannot yet demonstrate state 2. This is an integration boundary, not evidence that the cube renders.

The earlier viewer experiment used discarded materialization assumptions. Its four-file patch is saved as `/tmp/p4-tcursor/slipstream-integration-wip.patch`; the four viewer files were restored byte-for-byte to their pre-experiment contents, preserving the user's existing edits. The restored viewer regenerates successfully in 74.27 seconds. Live user/cube resources match their pre-task hashes. No new GUI-rendering success is claimed.

## Final validation

Both maintained CodeDog targets regenerate successfully: shared library **143.80 s**, test executable **207.89 s**. The independent client linked against the regenerated library passes **173/173 cursor checks**. The final executable passes **all 57 WorldManager suites** and reports **24 failures / 535 registrations** in the full run: the same sixteen defaults and eight historical compiled failures, with unchanged failing names. All fifteen protected `.tst` files are byte-for-byte identical to HEAD; unrelated `myTests.tst` remains untracked and untouched. The test manifest is valid JSON and `git diff --check` passes.

The complete-run log SHA-256 is `64c298b7c991af8eba9a8c8cbcf27cfde64372a02c08de665644241bc46c18b8`. Machine-readable validation is saved in `/tmp/p4-tcursor/validation.json`.

Evidence is under `/tmp/p4-tcursor`: the final library and test build logs, focused engine and full-suite logs, shared-library client results, and the cube diagnostic. Generated-C++ experiments were used to reproduce ownership bugs; CodeDog is the maintained implementation.
