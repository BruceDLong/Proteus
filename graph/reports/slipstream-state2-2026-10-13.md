# Slipstream state 2 engine probe — 2026-10-13

The current cube model does not yet supply all fields needed to render state 2 through the replacement engine. This probe uses the maintained shared library and a standalone public-interface client; it is not a Slipstream GUI run or an asynchronous-startup parity test.

## Inputs and method

The diagnostic copies the actual foundation/color definitions, the full cube definitions, and the two-state `rubix-cube` expression from Slipstream's current `userKB.pr`. The kernel's `rat`/`reel` declarations are supplied separately. The input files are unmodified; the extracted cube expression retains its `rubix-state` listSpec and missing `zLoc`. It does not load the unrelated kanban or all other kernel models. Permanent behavioral tests use small synthetic definitions rather than these changing assets.

Temporary evidence is in `/tmp/p4-slipstream-state2`: `probe.cpp`, `manifest.json`, input snapshots, original logs, the isolated generated-C++ experiments, and `verified-*.log`. The client initializes ProteusCore, parses and registers each definition document, then calls Agent.normalize on the target POV. Member requests connect their actual ancestor POV path. Measurements below count the target request only, after definition registration.

Snapshot hashes:

- `/home/bruce/devl/Slipstream/Resources/public/Informatics/Foundation.pr`: `bce67e9942bf6c3c95aff1dc0fb5b146593392f667db3cd4ce7b38e14c2bb178`
- `/home/bruce/devl/Slipstream/Resources/public/Things/rubixCube.pr`: `315f7e7d1905680919cfcc1f29b468ce6ddd68c4a7d862ec2f220c6883a999dc`
- `/home/bruce/devl/Slipstream/Resources/userKB.pr`: `122c23e6b316e435fcc0f50f89619264c5c38bf2e0ee2cb51db552665939d2ad`

## Reproduced blocker and accepted repair

A direct request for the second state initially rejected `front-color=red` with `Nominal types do not have a proved identity` (31 frames, 31 relationships). The user explicitly approved accepting asserted names after checking compatible definitions. D061 records the implementation and its distinction from type-predicate matching.

Relaxing only the name check was insufficient: it could drop the RHS's record constraints during scalar deferred comparison. The missing target definition is now a local prerequisite of the identity: the frame retains the resolved RHS, checks that meaning, and resumes ordinary field correspondence. A broader definition-first ordering was rejected because it regressed replacement and temporal-context cases. Synthetic cases retain hue 0 for red and reject red/blue conflicts without publication. Unknown saturation/brightness values remain unknown; no defaults are invented.

## Final target requests

| Request | Outcome | Frames / relationships | Observed result |
| --- | --- | --- | --- |
| Whole actual two-state expression | Partial | 2051 / 1515 | Both written states retained; state 2 zLoc unknown |
| Second state directly | Partial | 1451 / 1118 | `Record meaning still has unresolved relationships`; no publication |
| Second state's written zLoc directly | Partial | 3 / 2 | `zLoc:*_+_`; no implicit previous-state source supplied |
| `solved-rubix-state.parts.center-cubes@[1].thing.location.zLoc` | Complete | 53 / 19 | `zLoc:1.1` |
| `solved-rubix-state.parts.center-cubes@[1].front-color.hue` | Unsupported | 50 / 17 | `Correspondence needs a list layout` |
| Actual two-state expression followed by `@[2].thing.location.zLoc` | Unsupported | 3 / 0 | `Named list layout needs template correspondence` |

The complete known-position query took about 0.8 ms; the partial direct state request took about 15 ms on this machine. The cube definition document parsed in roughly 18–20 ms. These are diagnostic observations, not rendering benchmarks or evidence that unresolved normalization is fast enough.

The isolated temporary trace finds unresolved `%.parts...` selections in face occupants. Whole-list normalization now demands the named state constraints through their asserted template relationships, but still cannot complete the face selections. Its partial run took about 24 ms; the returned written states do not establish a fully materialized cube. The earlier experimental global definition-first ordering left those constraints undemanded (52 frames / 51 relationships) and was not retained.

## Temporal control

`{T 3.5 _}` and the corresponding nested-location example remain partial. Explicit templates `{T _=%tprev|3.5 _}` and `{T {location:{zLoc:_}}=%tprev|{location:{zLoc:3.5}} {location:{zLoc:_}}}` complete. D060 established contextual template application, not the implicit preservation relationship used by this real cube.

## Work bounds and next integration work

Repeated fixed-width `front-color=red` assertions use 55/175/655 frames and 44/140/524 relationships for 8/32/128 occurrences. This checks linear work for the repaired semantic family. It does not establish a general bound for face searches or the entire cube.

Next, specify how a demanded member receives the relevant enclosing definition and listSpec constraints through the common correspondence walker. Compare whole-list, direct-state, and direct-coordinate requests analytically before implementation; whole spans need one shared traversal rather than per-cublet scans from the start. Then specify implicit temporal preservation as a demanded corresponding RHS, honoring supplied changes and `%?`. Validate each rule with synthetic positive, conflicting, partial and streaming cases plus work counters. These are separate from changing Slipstream's host interface.

Slipstream's build source and existing executable still target `/home/bruce/devl/Proteus4/SharedBuild`. Its display bridge also calls the old `graphNormalizer` interface. No application files or world assets were changed by this investigation. Linkage migration and visible colors/poses validation remain pending until the demanded engine fields work.

## Validation

Both CodeDog builds succeed. The independent library client passes the focused named-query, nominal-evidence (111 checks), plural-list-type, scalar-replacement and temporal-template suites. The complete retained runner reports 24 failures / 533 tests, preserving the baseline failure names and diagnostics. Compatible named assertion expectations were updated to the user-approved rule; protected `.tst` files and unrelated user tests are unchanged. Logs: `library-build-final.log`, `test-build-final.log`, `focused-final.log`, and `full-final.log` in the diagnostic directory.


## D063 follow-up

The next ancestor-requirement cycle is recorded in `member-requirements-2026-10-13.md`. It extends member requests through containing definitions and structured ancestor listSpecs, with a separate constraint boundary for borrowed sources. The literal Proteus `main` reference does not establish the implicit temporal preservation proposed above; the user gives historical behavior precedence, and no implicit temporal rule is implemented in D063. The earlier phrase “honoring supplied changes” does not grant ordinary supplied values an override under D062's conditional preservation rule.
