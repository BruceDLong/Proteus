# Slipstream shared engine

Build the engine from the Proteus4 checkout:

```sh
cd /home/bruce/devl/Proteus4
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

For edits confined to the native graph headers, the existing generated source
can also be rebuilt directly:

```sh
cd /home/bruce/devl/Proteus4/SharedBuild
codeDog -b proteusGraph
```

Rebuild Slipstream when a public declaration, inline method/constructor, native
public type layout, or compiler ABI changes. CodeDog gives generated interfaces
an ABI symbol and requests eager binding so a stale generated interface fails
at load/link time. This guard covers generated declarations, not changes to
external native headers such as `GraphSearch.h`; those remain the developer's
responsibility.

## Boundary

CodeDog's `ExportInterface='true'` emits public declarations and a manifest from
the same language model as the shared implementation. Slipstream's
`LinkedInterface` imports those declarations, leaving the language metadata
available to CodeDog while omitting the imported C++ implementations and globals.
The `.dog.proxy` still supplies the language metadata. It no longer causes the
engine to be compiled into the GUI executable.

POVs and infonViews remain the live model. No graph copy, serialization, or
second data store is introduced. `GraphNormalizer.withRead`, `withUpdate`, and
`revision` keep GUI locking/publication calls in the engine library. Runtime
destruction is also compiled there so host-created Agents do not embed scheduler
shutdown code. Engine initialization and shared globals also have one owner. `ProteusCore` declares
its existing host callbacks in `virtualMethods` so standalone library generation
does not depend on seeing Slipstream's subclasses.

The library links with `--no-undefined`. Imported class declarations are checked
against the manifest; a client cannot silently extend an exported class's layout.
CodeDog currently supports one linked interface per Linux C++ build.

## Focused verification

```sh
cd /home/bruce/devl/Proteus4
g++ -std=c++17 -O2 -pthread -ISharedBuild graph/test_shared_library.cpp \
    -LSharedBuild -Wl,-rpath,"$PWD/SharedBuild" -lproteusGraph \
    -lgmpxx -lgmp $(pkg-config --cflags --libs icu-uc icu-io) \
    -o /tmp/proteusGraph-shared-test
/tmp/proteusGraph-shared-test
ldd -r SharedBuild/libproteusGraph.so
```

The integration test uses only the public header. It covers scalar normalization,
retention of the caller's POV/view, a host write waking an existing subscription,
temporal preservation, callback dispatch from library to host, and shutdown.
CodeDog's `Tests/test_shared_library.py` separately builds a library and client,
changes only the library implementation, verifies the unchanged client sees the
change, and verifies that changing the generated layout requires a client rebuild.
