Proteus4 normalizer replacement
------------------------------

Work in `/home/bruce/devl/Proteus4-redesign`, branch `codex/normalizer-redesign`.
The user approved replacing the solver and moving it into the normal Proteus
Engine early. Proteus3 is in `/home/bruce/devl/Proteus`; the original Proteus4
checkout at `/home/bruce/devl/Proteus4` and checkpoint
`b52322a867d2be2c1fda186929b05dc0c70e8be4` preserve the previous implementation.
Use those folders for historical behavior; do not restore an old solver fallback.

Read the accepted contract in `graph/normalization-design.md`. Record choices,
reasons and alternatives in `graph/normalization-design-decisions.md`. The older
reports describe reference behavior unless they explicitly name the replacement.
Old APIs, scheduler counters, cache epochs and implementation details are not
requirements on the new engine.

Architecture
------------

WorldManager.dog owns the complete replaceable reasoning interface. Agent owns
and tracks sessions. Normalize returns a WorldRequest handle; status and result
remain accessible to the caller. Sharing deductions across sessions is deferred.
WorldManager reads/writes POVs through a virtual graph rather than duplicating
the model. New semantics should compose through relationships and optimizations
need justification. Preserve qualifications, occurrence identity and context.

CodeDog is the maintained source. Generated C++ experiments are allowed for a
concrete problem, but port the solution back and regenerate before acceptance.
Native thread/lock bindings are implementation boundaries, not another solver.
Live goals retain progress and continue at waiting links; no reset/restart API.
Parsing done plus every current wrkList member tried exhausts given work; later
items come from requested links. Meaningful work can remain in the stored list.
Known numeric values can finish with unknown size under D02F. Do not fabricate
a size or subscribe just because it is unknown. Ordinary parentheses usually
preserve meaning; type-changing qualifications such as A==(B=C) still matter.
Planning remains deferred until intersections and definitions are considered.

Scope and tests
---------------

The engine supports the scalar/expression cases, closed ordered-list composition,
finite context-free listSpecs, closed embedded spans, finite open-tail closure,
uniquely determined embedded span extents and local parser-completion continuation
listed in the design status. General sparse alignment, definition, intersection,
temporal and effect semantics remain unsupported. Stream ownership and shutdown use the new
path; general streamed list propagation and other link arrivals remain later work.
An unsupported result must never be replaced with output from the old solver.

All 15 protected .tst files and their expected outputs remain unchanged. Keep the
58 historical parser/model cases. The normal TestProteus executable runs through
WorldManager. `graph/engine-test-migration.json` indexes every retained compiled
test and its original assertion source. Tests awaiting an API port remain named
and report NOT_PORTED as a failure. Port useful assertions without restoring old
solver APIs or changing expectations to match incomplete behavior. Preserve known
failures and distinguish unsupported features from failures of supported behavior.
`graph/retired-normalizer-tests.json` records the earlier test pruning and hashes.

Build and verification
----------------------

Build with `codeDog Proteus.Lib.dog`. Run from LocalBuild:

    ./TestProteus -t unit/worldManager/
    ./TestProteus -t defaultTests.tst:

The startup banner identifies the engine. Check the printed failure count; the
inherited harness can exit zero on test failures. Add `-T 3` for per-case timeout
protection. Generated C++ can be rebuilt with `codeDog -b TestProteus`, but finish
with a CodeDog regeneration. The scalar harness has moved into TestProteus.

Use focused behavioral checks and real language fixtures. Measure relationship
attempts, queue work, allocations and subscriptions, not merely trace messages.
For fixed-width list work, avoid repeated lookup from the start for each member.
Arbitrary inference/ambiguity has no assumed linear bound. Remove obsolete
dependencies as callers migrate. Preserve unrelated work and explain changes,
validation and unsupported boundaries clearly. Slipstream still requires its
own migration and visible rendering validation; engine tests alone do not prove it.

Commit completed major changes after the relevant checks, without waiting for
a separate commit request. Keep commits scoped to the completed work, leave
unrelated local files out, and report any remaining known failures.

Syntax
------

Definitions use #, for example `#minute = *60+{second| ...}`. @ is reserved for
bracketed postfix selection: `items@[3]`, `items@[3:*5+{ ... }]`. The old items#3
syntax is not supported. A reply marker requests normalized output to the caller;
implement its effect through the engine when that milestone is reached.
