Proteus4 normalizer replacement
------------------------------

Work in `/home/bruce/devl/Proteus4-redesign`, branch `codex/normalizer-redesign`.
The user approved replacing the solver and moving it into the normal Proteus
Engine early. This is now a standalone repository. Proteus3 remains in
`/home/bruce/devl/Proteus`. The failed Proteus4 checkout and its local branch
were removed after cleanup; checkpoint `b52322a867d2be2c1fda186929b05dc0c70e8be4`
remains in this branch's history. Use `git show <commit>:<path>` for historical
behavior; do not restore an old solver fallback.

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
uniquely determined embedded span extents, retained ancestor listSpecs across open
embedded spans and local parser-completion continuation listed in the design
status. General sparse alignment, definition, intersection, temporal and effect
semantics remain unsupported. Stream ownership and shutdown use the new path; general streamed list propagation and other link arrivals remain later work.
An unsupported result must never be replaced with output from the old solver.

Keep the existing .tst language fixtures and their expectations. The 71 compiled
checks use ordinary CodeDog: the 58 historical parser/model checks, five string
behavior checks, incomplete-RHS parsing, four parser input-lock regressions,
two identity-range checks and one parser-completion ordering check.
D072 retires all verbatim C++ test
bodies and the standalone drivers for the failed graph engine, including the
later WorldManager C++ suites. Do not restore them or add native C++ assertions
inside `<% ... %>` blocks. CodeDog macro bodies use that delimiter too, but expand
to ordinary CodeDog and remain supported.

Add focused tests in .tst fixtures or ordinary CodeDog as behavior is developed.
Do not change expectations to match incomplete behavior. Preserve known active
failures. `graph/engine-test-migration.json` inventories the active compiled tests;
`graph/retired-normalizer-tests.json` records retirements and recovery commits.
The former archived inventory is available in Git, not a porting backlog.
Production native bindings were not changed in the test cleanup.

Build and verification
----------------------

Build with `codeDog Proteus.Lib.dog`. Run from LocalBuild:

    ./TestProteus
    ./TestProteus -t defaultTests.tst:

The startup banner identifies the engine. Check the printed failure count; the
inherited harness can exit zero on test failures. Add `-T 3` for per-case timeout
protection. Generated C++ can be rebuilt with `codeDog -b TestProteus`, but finish
with a CodeDog regeneration. Native scalar/session suites were retired under D072.

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
