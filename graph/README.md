# Proteus4 normalization replacement

The Proteus Engine on `codex/normalizer-redesign` now routes Agent normalization
and TestProteus through `WorldManager.dog`. Agent owns sessions; normalize returns
a `WorldRequest` handle. The old GraphNormalizer is not linked into this engine.

The maintained solver is CodeDog. It supports context-free scalar acquisition,
the documented scalar matching cases, and continuation when a particular parser
input finishes. Lists, definitions, general expressions, temporal rules and
effects report unsupported. The accepted contract and implementation limits are
in [the consolidated design](normalization-design.md); choices and reasons are in
[the decision log](normalization-design-decisions.md).

## Build and verify

From `/home/bruce/devl/Proteus4-redesign`:

```sh
codeDog Proteus.Lib.dog
cd LocalBuild
./TestProteus -t unit/worldManager/
./TestProteus -t defaultTests.tst:
```

The normal executable identifies WorldManager at startup. Scalar/session and
parser/stream lifecycle checks live in this suite; there is no separate required
WorldManager executable. Use `-T 3` when per-case crash/timeout protection is useful.
Check the printed failure summary: the inherited harness can exit zero on failure.

Language fixtures and active failures are preserved except for explicitly
recorded user-approved corrections. Under D057, the 1,384 unported placeholder
registrations are archived rather than run as failures. The
[migration inventory](engine-test-migration.json) distinguishes 198 active compiled
registrations from historical assertion references. The 332 default cases,
historical parser/model checks, already ported behaviors and 52 WorldManager
suites stay active. Add fresh behavioral tests as each semantic area is developed;
the archive is a source of edge cases, not a mandatory porting backlog.
The inventory also preserves retired runtime and standalone native-test references.
`UNSUPPORTED` results in active tests remain failures; retirement does not establish
that a previously untested behavior works.

## Printer modes

`toString(infon)` keeps the current compressed output. Select a mode with
`toString(infon, ipmCompressed)`, `ipmExpanded`, or `ipmDebug`.
`toStringColor` and `InfonViewPrinter.printInfonView` also accept a final mode
parameter, defaulting to compressed. Debug includes the existing size and
retained-template diagnostics. Expanded currently uses compressed output; cursor
expansion will be implemented separately. Printing does not normalize the model.

Dynamic `.tst` rows accept an optional seventh column:

```text
name, display, input, expected, execution-mode, world-file, print-mode
```

`print-mode` is `compressed`, `expanded`, or `debug`. Omitted or blank means
compressed. It applies to parse, norm, multi, world, and graph result checks;
leave the sixth column empty when no world file is needed. Existing fixtures
and expectations remain unchanged. Run the focused fixtures from `LocalBuild`:

```sh
./TestProteus -t ../graph/print-mode-tests.tst:
```

## Handles and live input

`Agent.normalize()` uses its default session; callers can also create explicit
Agent sessions and use `normalizeInSession()`. A handle exposes `status()`,
`complete()`, `result()`, `failed()`, `error()` and `cancel()`. Status distinguishes
ready, complete, partial, waiting, rejected, unsupported, cancelled and error.

Runnable work drains synchronously. `waitForCompletion()` waits for currently
executing work to leave the model lock and returns even if status is waiting;
it does not wait for a live input stream to close. The loader owns the input and
parser thread lifetime. Agent closure cancels its remaining requests. Completed
handles retain their result and status after closure.

Parser reads that may block release the model lock. A completed infon wakes only
requests registered on that infon, preserving their frames and relationship
cursors. Readiness checks and wait registration use the same lock. General link
subscriptions, independent routes during a wait, cyclic dependencies, and streamed
list prefixes remain later work. Sharing deductions across sessions is deferred.

## Reference implementations

Proteus3 remains in `/home/bruce/devl/Proteus`. The original Proteus4 checkout in
`/home/bruce/devl/Proteus4` and commit `b52322a` retain the previous solver and test
assertions. Use them for historical comparisons. Reports elsewhere in this folder
describe that checkpoint unless they explicitly identify the replacement; their
old implementation paths and pass counts do not describe this engine.

## Test selection for the normalizer redesign

This section records pruning before the engine cutover; current execution coverage is described above.

The language fixtures are the protected baseline. All 15 `.tst` files remain
byte-for-byte unchanged: `defaultTests.tst` contains 271 direct cases and includes
61 more from the sparse, temporal, and world suites. The first approximately
200 original cases are therefore preserved without an arbitrary numeric cutoff.
All 58 compiled cases still present from before the graph experiment are retained.

On 2026-10-11, 76 implementation-specific compiled cases and three standalone
scheduler drivers were retired. That pruning left 1,530 compiled cases before the engine cutover.
The [retirement manifest](retired-normalizer-tests.json) records every removed
name, its reason, protected fixture hashes, and the recovery checkpoint.
Retired tests and native drivers remain recoverable from commit `b52322a`.
Their source files are removed, rather than merely disabled in the test list.

The removed checks prescribe cache epochs, proof identity, internal requirement
DAG layouts, validation-cache calls, or the old beginPass/found/SetCommand and
GraphManager/PersistentTraversal protocols. Useful branches of mixed groups are
retained: contextual and sparse aliases, independent instances, late meanings,
contradictory new assertions, completion, and released-handle lifetime.
Language, list/span, numeric-range, parser, streaming/reply, temporal preservation,
and integration assertions remain available for migration. Performance properties such as bounded sparse
access and avoiding repeated work still matter; old scheduler counters and cache
layouts are not requirements on the replacement.

The remaining graph-facing tests are semantic regression material, not a promise
that their current helper API or every status assertion will survive the redesign.
Port useful checks through WorldManager as its interface is defined. In particular,
D02F permits unknown numeric size at completion and D03 limits list expansion;
older status/depth expectations must not silently override those decisions.
Preserve failures with clear semantics; do not delete them just to improve pass counts.
Historical validation reports may mention retired files or earlier counts.

Validation after pruning: CodeDog generation and the C++ build succeeded. The
332-case default language run retained exactly the same 32 failure names as the
reference binary. The 75 selected compiled cases retained the same eight failures;
all 17 surviving cases from the edited mixed groups passed. All 15 language
fixture hashes and the retained registrations/case IDs were checked unchanged.
Other retained compiled and standalone suites were not rerun. No engine source
was changed, and no failing language expectation was weakened.
