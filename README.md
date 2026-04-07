# Proteus

**Proteus** is an information-theoretic programming language and inference engine built around the concept of *infons* -- structured information units that can represent numbers, strings, lists, and complex nested data. Proteus aims to bridge programming and natural language by combining a formal data model with natural language understanding capabilities.

## Key Features

- **Infon-based data model** -- Programs are expressed as structured information ("infons") that support numbers, strings, lists, typed fields, and nested structures.
- **Natural language integration** -- Includes an English language translator (`xlators/xlator_en.dog`) for parsing and processing natural language constructs.
- **Agenda-based inference engine** -- Resolves relationships between infons through an agenda-driven normalization and merging process.
- **Model and vocabulary management** -- Define and look up typed words and their meanings via the built-in model manager.
- **Infon viewer** -- A standalone viewer application for inspecting infon structures.

## Prerequisites

Proteus source files are written in [CodeDog](https://github.com/BruceDLong/CodeDog) (`.dog` files), which compiles to C++. To build Proteus you will need:

- **[CodeDog](https://github.com/BruceDLong/CodeDog)** -- the CodeDog compiler
- **GNU C++ toolchain** -- GCC/G++ on Linux (the primary supported platform)
- **Python 3** -- for `ruleMgr.py` and related tooling

## Building

The default build configuration targets Linux with the GNU C++ toolchain. From the project root:

```bash
codedog Proteus.Lib.dog
```

The build line in `Proteus.Lib.dog` is:

```
LinuxTestBuild: Platform='Linux' Lang='CPP' LangVersion='GNU' testMode='makeTests'
```

## Running Tests

Proteus includes a CodeDog test suite and a C++ test harness:

```bash
# Run the CodeDog test suite (built via the LinuxTestBuild config)
./Proteus --test

# Compile and run the C++ test harness
g++ -o infonTest infonTest.cpp && ./infonTest
```

## Project Structure

| Path | Description |
|---|---|
| `Proteus.Lib.dog` | Main engine library and entry point |
| `infonIO.dog` | Infon input/output, parsing, and serialization |
| `infonList.dog` | Infon list data structures and operations |
| `ModelManager.dog` | Model and vocabulary management |
| `Functions.dog` | Built-in functions |
| `debugSystems.dog` | Debugging and diagnostic systems |
| `clip.dog` | Clipboard and utility operations |
| `timeAccess.dog` | Time access utilities |
| `infonViewer.dog` | Standalone infon viewer application |
| `DB_workAround.dog` | Database v1 workarounds (string utilities) |
| `testInflect.dog` | Inflection testing for the English translator |
| `xlators/xlator_en.dog` | English language translator |
| `ProteusTests.dog` | Test suite |
| `ProteusDBServer.dog` | Database server component |
| `ruleMgr.py` | Rule management (Python) |
| `infonTest.cpp` | Infon C++ test harness |
| `Examples/` | Example Proteus programs |
| `Resources/` | Sample `.pr` files and a web interface (`web/`) |
| `theory/` | Experimental and theoretical work |

## Example

Here is a Tic-tac-toe game written in Proteus syntax (`Examples/Tic-tac-toe.pr`):

```proteus
def tic_tac_toe: {

    def playerSymbol: ['X' | 'O']

    def slot: {T [' ' | 'X' | 'O'] | ...}

    def row: *3+{slot| ...}

    def board: *3+{row| ...}

    def move: {column:1..3, row:1..3}

    def player: {name  playerSymbol  moves:{T move| ...}}

    def turn: {player, move, board}

    def winner: [ 'X'  'O'  'Tie']

    *2 + { player |
            {%.name = userInput<:{prompt: "Player X, enter your name"  %.playerSymbol="X"}}
            {%.name = userInput<:{prompt: "Player O, enter your name"  %.playerSymbol="O"}}
    }
    def play: {
        turns: {T turn|
            {playerSymbol:'X' move:player.0.move.0  board:{ *3+{' '  ' '  ' '}|...}}

            #{  {
                playerSymbol= !playerSymbol
                move=player.playerSymbol.move = userInput<:{prompt: (%.name " please enter your move:")}
                board.(move.column).(move.row) = playerSymbol
                }
             | ...}

            {[ %turns.size==9  |  CheckWinningBoard<: board]}
        }
        winner: [{%turns.size=9 %='Tie'}  | turns.last.player]
    }
}

tic_tac_toe.play
```

## Status

Proteus is under active development (version **0.8**). Current work includes:

- Streaming normalization (work in progress)
- Syntax updates (`withEach` loop changes)
- Thread synchronization fixes
- Agenda ordering improvements

**Known issue:** `Proteus.Lib.dog` references `WorldManager.dog` via `#include`, but this file is not present in the repository.

## Authors

- **Bruce Long**
- **KT Lawrence**

## License

All Rights Reserved.

> "This file is part of the Proteus Language suite. All Rights Reserved."

Copyright (c) 2015-2023 Bruce Long
