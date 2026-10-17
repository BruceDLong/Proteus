# Proteus

Proteus is a modeling language and reasoning engine for representing information,
constraints and relationships. Its central data structure, an *infon*, can
represent numbers, strings, lists and nested models. A point of view (POV)
represents an occurrence of an infon in a particular context, allowing the same
information to participate in different models.

The maintained implementation is written in [CodeDog](https://github.com/BruceDLong/CodeDog).
On `main`, `WorldManager.dog` owns normalization and accesses the model through
a virtual graph. `Agent` owns reasoning sessions; normalization returns a
`WorldRequest` whose status and result the caller can inspect. Parsing, stream
loading and printing are separate parts of the library.

The engine is under active development and has known unsupported cases. See
the [build and testing guide](Docs/README.md) for the validation baseline and
the [design documents](<Docs/Design Docs/README.md>) for semantic contracts,
implementation limits and open questions.

## Building

Use CodeDog commit `e0dae8e` or later with its C++ dependencies installed. The
validated build platform is Linux with Python 3 and the GNU C++ toolchain.

From the repository root, build the test executable:

```sh
codeDog Proteus.Lib.dog
```

This generates `LocalBuild/TestProteus`. Build the shared engine used by
Slipstream as a separate target:

```sh
codeDog BuildProteusGraph.dog
```

This generates `SharedBuild/libproteusGraph.so` and its exported C++ interface.
See the [shared-library guide](Docs/shared-library.md) for building and updating
the linked application. The optional sibling World repository is described in
[Resources/README.md](Resources/README.md); default tests use committed fixtures
and do not require it.

## Running tests

From the repository root after building:

```sh
cd LocalBuild
./TestProteus -T 3
```

`-T 3` enables per-case crash/timeout protection. To run one language fixture:

```sh
./TestProteus -t defaultTests.tst:parser/list/simple -T 3
```

Check the printed failure summary: the inherited harness can exit zero even
when tests fail. The [testing guide](Docs/README.md) records known failures,
additional fixture suites and printer-mode checks. Tests belong in ordinary
CodeDog or `.tst` fixtures.

## Project structure

| Path | Purpose |
|---|---|
| `Proteus.Lib.dog` | Infon storage, Agent sessions, ProteusCore and test build entry point |
| `WorldManager.dog` | Reasoning interface, normalization and virtual graph access |
| `infonIO.dog` | Parser, input synchronization and printing |
| `infonList.dog` | POVs, list views and traversal |
| `InfonStreamLoader.dog` | Stream connections and parser/normalizer thread lifetime |
| `ModelManager.dog`, `WordRules.dog` | Vocabulary lookup and word forms |
| `Functions.dog` | Built-in function support |
| `xlators/` | Language-specific vocabulary and inflection support |
| `ProteusTests.dog`, `defaultTests.tst` | Compiled CodeDog checks and default language fixtures |
| `BuildProteusGraph.dog` | Shared-library and exported-interface build |
| `clip.dog` | Command-line interface source |
| `Resources/` | Tracked model/test assets and optional public-world link |
| `graph/` | Additional language fixtures, test inventories and diagnostic tooling/data |
| `Docs/` | Build guides, current design references and historical reports |
| `Examples/`, `theory/` | Older examples and research material; check against current syntax and semantics |

## Documentation

- [Build, testing and request lifecycle](Docs/README.md)
- [Design references and working notes](<Docs/Design Docs/README.md>)
- [Historical reports and superseded plans](<Docs/Design Docs/Historical/README.md>)
- [Shared-library integration](Docs/shared-library.md)
- [Resources and test fixtures](Resources/README.md)

The previous engine remains on `proteus3b`. The `Proteus4` Git tag records the
promotion of the cleaned redesign to `main`; it does not rename runtime version
metadata. See the [reference history](Docs/README.md#reference-implementations)
for earlier implementation checkpoints.

## Authors

- **Bruce Long**
- **KT Lawrence**

## License

All Rights Reserved.

> "This file is part of the "Proteus Language suite" All Rights Reserved."

Copyright (c) 2015-2023 Bruce Long
