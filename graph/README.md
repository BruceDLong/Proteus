# Proteus4 normalization replacement

The Proteus Engine on `codex/normalizer-redesign` now routes Agent normalization
and TestProteus through `WorldManager.dog`. Agent owns sessions; normalize returns
a `WorldRequest` handle. The old GraphNormalizer is not linked into this engine.

The maintained solver is CodeDog. The accepted contract and implementation limits
are in [the consolidated design](normalization-design.md); choices and reasons
are in [the decision log](normalization-design-decisions.md).

## Build and verify

From `/home/bruce/devl/Proteus4-redesign`:

```sh
codeDog Proteus.Lib.dog
cd LocalBuild
./TestProteus -T 3
./TestProteus -t defaultTests.tst:
```

The executable identifies WorldManager at startup. Check the printed failure
summary: the inherited harness can exit zero on test failures.

The active suite contains 64 compiled CodeDog checks and 332 default language
cases. All 16 tracked `.tst` files, including print-mode fixtures, are preserved.
The user's untracked `myTests.tst` is preserved separately. Expected outputs were
not adjusted to match current failures.

Under D072, 140 registrations backed by verbatim C++ and 14 standalone drivers
for the retired Proteus4 engine were removed. This includes the newer
WorldManager C++ test suites. The remaining `<% ... %>` delimiters in
`ProteusTests.dog` quote CodeDog macro expansions, not native implementations.
Production native bindings are unchanged. Future tests belong in ordinary
CodeDog or `.tst` fixtures.

The [active inventory](engine-test-migration.json) lists all 64 compiled checks.
The [retirement manifest](retired-normalizer-tests.json) records removed names,
files, fixture hashes and recovery commits. The previous large archived inventory
is available with `git show bd7078b:graph/engine-test-migration.json`.
Retired checks are not a mandatory porting backlog, and their removal does not
establish that the behaviors they covered work.

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

Proteus3 remains in `/home/bruce/devl/Proteus`. This checkout owns its Git data;
it no longer depends on the removed `/home/bruce/devl/Proteus4` folder.
The old solver and tests remain available in ancestor commit `b52322a`:

```sh
git show b52322a:ProteusTests.dog
git show b52322a:AGENTS.md
```

The redesign immediately before test cleanup is commit `bd7078b`. Reports
elsewhere in this folder describe historical checkpoints unless they explicitly
identify the current replacement. Old implementation paths and pass counts in
those reports are historical evidence, not current build or test instructions.
