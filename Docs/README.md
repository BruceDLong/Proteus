# Proteus normalization

This is the build and testing guide. See [Design Docs](<Design Docs/README.md>)
for architecture and semantic notes, [Historical](<Design Docs/Historical/README.md>)
for earlier design notes and recovery of removed reports, [shared-library.md](shared-library.md)
for Slipstream integration, and [test-ambiguities.md](test-ambiguities.md) for
unresolved fixture questions recorded by the retired audit helper.

The Proteus Engine on `main` routes Agent normalization
and TestProteus through `WorldManager.dog`. Agent owns sessions; normalize returns
a `WorldRequest` handle. The old GraphNormalizer is not linked into this engine.

The maintained solver is CodeDog. The accepted contract and implementation limits
are in [the consolidated design](<Design Docs/normalization-design.md>); choices and reasons
are in [the decision log](<Design Docs/normalization-design-decisions.md>).

## Build and verify

Use CodeDog commit `e0dae8e` or later. From the repository root:

```sh
codeDog Proteus.Lib.dog
cd LocalBuild
./TestProteus -T 3
```

The first command generates `LocalBuild/TestProteus`; `-T 3` enables per-case
crash/timeout protection. From `LocalBuild`, select a fixture or a fixture group
with the suffix after the colon:

```sh
./TestProteus -t defaultTests.tst:parser/list/simple -T 3
./TestProteus -t defaultTests.tst:parser/list -T 3
```

Use `./TestProteus -t defaultTests.tst:` for all default language fixtures.
The retained `.tst` suites live in `Tests/`. Bare filenames, including `@include`
names, are searched in `../Tests` first, then the working directory, then its
parent. Explicit relative and absolute paths are used as supplied. The `.tst`
extension may be omitted.

For a generated-C++ experiment, `codeDog -b TestProteus` rebuilds from
`LocalBuild`; finish maintained changes by regenerating from CodeDog source.

Build the shared engine separately with `codeDog BuildProteusGraph.dog` from the
repository root. See [shared-library.md](shared-library.md) for consumer rebuild
requirements and [Resources/README.md](../Resources/README.md) for optional
application-world setup. Default tests use tracked assets.

The executable identifies WorldManager at startup. Check the printed failure
summary: the inherited harness can exit zero on test failures.

The active suite contains 83 compiled CodeDog checks and 332 default language
cases. All 16 tracked `.tst` files, including print-mode fixtures, are preserved.
Expected outputs were not adjusted to match current failures. The last full
validation, on 2026-10-17 after adding cursor readiness
([D089](<Design Docs/normalization-design-decisions.md#d089-require-selected-cursor-fields-before-presenting-a-temporal-state-2026-10-17>)),
reported **391 passes and 24 failures out of 415 tests**. All eleven new checks
passed; the failure names and diagnostics matched the 24/404 pre-change run.
The earlier D087 validation also passed ten print-mode fixtures and retained
the separate fixture suites' results.

Under D072, 140 registrations backed by verbatim C++ and 14 standalone drivers
for the retired Proteus4 engine were removed. This includes the newer
WorldManager C++ test suites. The remaining `<% ... %>` delimiters in
`ProteusTests.dog` quote CodeDog macro expansions, not native implementations.
Subsequent cleanup converted the remaining production native function bodies to
CodeDog. Future tests belong in ordinary CodeDog or `.tst` fixtures.

ProteusTests.dog lists all 83 compiled checks, including eleven cursor-readiness
checks. The graph inventories, audit scripts
and diagnostic assets were removed from the checkout after the fixtures moved
to Tests. The last inventory and retirement manifest remain available with:

```sh
git show 5dbbe10:graph/engine-test-migration.json
git show 5dbbe10:graph/retired-normalizer-tests.json
```

The previous large archived inventory is available with
`git show bd7078b:graph/engine-test-migration.json`.
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
./TestProteus -t print-mode-tests.tst:
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

Proteus3 remains on branch `proteus3b`. The imported redesign checkpoint remains
on `codex/normalizer-redesign`; `main` contains its history and canonical-path
integration. The annotated `Proteus4` tag records that promotion. This checkout
owns its Git data and does not depend on either fork's former checkout directory.
The old solver and tests remain available in ancestor commit `b52322a`:

```sh
git show b52322a:ProteusTests.dog
git show b52322a:AGENTS.md
```

The redesign immediately before test cleanup is commit `bd7078b`. Dated reports
from graph/ are retained in Git history; the
[Historical index](<Design Docs/Historical/README.md>) gives recovery commands.
Their implementation paths and pass counts describe those checkpoints; use this
guide and the latest decisions for current build and test instructions.
