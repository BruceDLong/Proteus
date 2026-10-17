# Slipstream shared engine

Use CodeDog commit `e0dae8e` or later, which includes the shared-interface,
input-lock, checked-parser, file-mode, shared-ownership, and exception bindings.
Build the engine from the canonical Proteus checkout:

```sh
cd /home/bruce/devl/Proteus
codeDog BuildProteusGraph.dog
```

This produces `SharedBuild/libproteusGraph.so`, `proteusGraph.hpp`, and
`proteusGraph.interface.json`. The standalone test build remains
`codeDog Proteus.Lib.dog` and is independent of this target.

Build Slipstream once against that interface:

```sh
cd /home/bruce/devl/Slipstream
codeDog Slipstream.dog
```

After changing engine implementations, rebuild `BuildProteusGraph.dog` and
restart Slipstream. There is no need to regenerate, compile, or relink
Slipstream when the public interface stays compatible. The generated executable
has a RUNPATH to this checkout's `SharedBuild` directory; it does not require
`LD_LIBRARY_PATH`. This is a local development setup, not an installed package
or live replacement of an engine in a running process.

Regenerate with CodeDog after changing maintained source. Rebuild Slipstream
when a public declaration, inline method, type layout or compiler ABI changes.
CodeDog gives generated interfaces an ABI symbol and requests eager binding so
a stale generated interface fails at load/link time.

## Boundary

CodeDog's `ExportInterface='true'` emits public declarations and a manifest from
the same language model as the shared implementation. Slipstream's
`LinkedInterface` imports those declarations, leaving the language metadata
available to CodeDog while omitting the imported C++ implementations and globals.
The `.dog.proxy` still supplies the language metadata. It no longer causes the
engine to be compiled into the GUI executable.

POVs and infonViews remain the live model. No graph copy, serialization or
second data store is introduced. WorldManager owns reasoning and Agent owns its
sessions. Engine initialization and shared globals have one owner. ProteusCore
declares its existing host callbacks in virtualMethods so standalone library
generation does not depend on seeing Slipstream's subclasses.

The library links with `--no-undefined`. Imported class declarations are checked
against the manifest; a client cannot silently extend an exported class's layout.
CodeDog currently supports one linked interface per Linux C++ build.

## Verification

The obsolete `graph/test_shared_library.cpp` driver depended on GraphNormalizer
and was retired under D072. Current test instructions are in [README.md](README.md).
For shared-library changes, rebuild the engine and validate the linked consumer's
actual behavior. Passing the language suite alone does not establish GUI behavior.

CodeDog's `Tests/test_shared_library.py` separately tests the generator's exported
interface and ABI behavior.
