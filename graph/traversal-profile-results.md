# Where cube startup traversal multiplies

2026-10-01; engine baseline `926d579`. This is a diagnosis, with no engine
behavior change. Two isolated Slipstream runs used the real Rubix and Kanban
models and a temporary graph-event trace.

## Main finding

The dominant repetition is across publications. The within-pass visited map is
working: in these traces, no accepted `(session, task, query)` visit repeated
within the same revision. The same task revisits its queries after its state is
invalidated and its visited set is cleared.

| Captured work | Run 1 | Run 2 |
| --- | ---: | ---: |
| Accepted graph visits | 195,852 | 191,891 |
| Distinct visits within each task, ignoring revision | 30,831 | 28,895 |
| Repeated visits within the same task | 165,021 (84.3%) | 162,996 (84.9%) |
| Distinct complete query keys across all searches | 11,288 | 11,271 |
| Goal/task instances across searches | 16,491 | 15,240 |
| Distinct goal query keys | 4,113 | 4,114 |
| Searches started | 150 | 150 |

The query key includes POV identity, demand, argument, related POV, owner stack,
and coordinates. Different contexts were not incorrectly counted as identical.
A repeated visit is not automatically unnecessary: real input changes require
revisiting their consumers. These counts locate amplification; they are not a
promise that all visits can be reduced to the distinct-query count.

The sequence/Earley matcher had zero visits in both runs. This workload's
observed growth comes mainly from list normalization, definitions, and paths.
It does not establish an asymptotic bound for other workloads.

## What causes the retries

In run 1, attribution to the publication that dirtied the task was:

| Publication | Repeated visits it triggered |
| --- | ---: |
| Set path selection result | 89,376 |
| Prepare path selection | 27,783 |
| Apply finite list type requirements | 18,159 |
| Start inserted stream | 9,094 |
| Apply repeated member requirements | 5,820 |
| Set list order from type header | 4,892 |
| All other causes | 9,897 |

Path preparation/publication accounted for **71.0%** of repeated visits in run
1 and **71.3%** in run 2. There were only 14 path preparations and 31/32 result
publications. Parser publications caused **7/1 task resets** in the two runs,
respectively, despite 125/120 parser publications. Late parser input is not the
main retry source in this capture after the earlier buffering improvement.

The concrete cube path is the `%.color` expression in `#centerCube`, at
`Resources/public/Things/rubixCube.pr:10` in Slipstream. Its copied expressions
retain origin 19, character position 290. The six center instances perform
self selection, color selection, and filtering of the resulting oneOf.

For example, run 1 revision 435 published one of these filtered color results.
It dirtied **4,057 tasks across 124 searches**. Those included 110 tasks under
the Kanban root as well as language-document normalization. The six analogous
late color publications each woke about 3,900–4,100 tasks. A later finite-list
definition application woke 10,776 tasks across those 124 searches.

These are ordinary path operations with wide notifications, not thousands of
different cube alternatives. Open unordered choices must keep their legitimate
subscriptions; dropping those obligations would change the model semantics.

## Where the repeated work goes

Run 1, grouped by owning goal rather than the kind of an individual edge:

| Owning goal | Visits |
| --- | ---: |
| List contents | 86,966 |
| Size and value | 57,298 |
| Type requirements | 32,089 |
| Find field facts in parent definition | 12,863 |
| All other goals | 6,636 |

The English-word document's list walk restarted 76 times. The world root was
visited 90 times. These are searches that can remain subscribed because some
information is symbolic; a local cube result should not by itself force all of
them to traverse again.

There were also **12,369 unchanged counted-size Sets**. They did not publish
changes, so they are a consequence of repeated list walks rather than the
source of the invalidation cascade.

## Source-level cause

1. `ProteusGoal::found`, `graph/ProteusGraph.h:295`, returns path preparation as
   a boolean Set without a specific change footprint.
2. `ProteusPathGoal::publish`, `graph/PathSelection.h:690`, likewise publishes
   the result with a boolean `true`. The default footprint is broad.
3. `Context::graphChanged`, `graph/GraphSearch.h:309`, invalidates old evidence
   and wakes every nonterminal task for a broad publication.
4. Even many explicitly scoped model writes still include `modelFacts_`.
   `ProteusAccess::observeTask`, `graph/ProteusGraph.h:757`, observes that same
   token for every task, so those writes also have wide effects.
5. `resetTask`, `graph/GraphSearch.h:463`, clears the visited map, required
   children, outgoing demands, and subscriptions before starting another pass.

The runtime already has selective dependency keys and can preserve clean
shared children. The adapter currently gives it insufficiently precise data
for general structure/value/registry reads. Merely replacing a broad footprint
with the existing global model token would not solve the problem. Publishing a
new local token without converting its readers would instead miss required
updates.

## Recommended sequence

1. **Make path reads and writes precise together.** Start with path preparation
   and result publication, using the existing dependency mechanism. Track the
   view binding, inspected payloads, list membership, and relevant relationships
   through the adapter. A path result should wake consumers of that result and
   its affected relationships. Preserve fallback invalidation for unaudited
   operations. Aliases matter: distinct POVs can share an infonView or underlying
   value/size state, while path publication can replace a POV's infonView.
2. **Separate list structure from child completion.** A changed member value
   should re-evaluate its dependent goals and parent completion without making
   a clean list walk rediscover all of its members. A membership/extent change
   must still invalidate that walk. This fits the existing required-subgoal
   graph; it does not require a new cube or list-shape algorithm.
3. **Then share identical work across searches.** Run 1 had 16,491 task instances
   for 4,113 exact goal keys. For example, the same orientation-list goal appeared
   in five searches. The current `taskIndex` is per search. Share proved work
   while preserving distinct owner contexts, costs, cancellation and root
   completion; do not simply combine every caller's lifecycle.

Small cleanup after the main issue: the subgoal factory is called before
`startTask` checks its existing-query index (`GraphSearch.h:700` and `:540`).
Run 1 made 119,843 refinement requests but created only 16,341 non-root tasks:
about 103,502 factory results were immediately discarded in favor of an existing
goal. Checking the index before construction removes that overhead without
changing search semantics, but will not remove the reset cascade.

The first regression should have independent subscribed roots and several
local path results. Publishing a result must wake its actual consumers and
preserve other frontiers and completion evidence. Cover late/absent members,
open oneOfs, alias replacement, and new registry edges. Vary the number of
unrelated roots and local publications to detect the current cross-product
in re-traversal. Then repeat this same visible-cube trace.

## Capture and limitations

Temporary instrumentation logs query identities, new goals, resets, accepted
visits, changed/unchanged Sets, subgoal links, publications and newly dirtied
tasks. Publications are associated with the Set or producer that caused them.
Node labels are shallow identity/type/source metadata; no recursive printer or
normalizer is invoked by the logger. Weak ownership keys distinguish recycled
POV addresses without keeping the model objects alive.

Artifacts are under `/tmp/p4-cube-chart-bench/`:

- `traversal-profile/`: instrumented build from the current headers.
- `traversal-profile-1/` and `traversal-profile-2/`: `traversal.tsv`, `analysis.json`,
  `analysis.txt`, application logs, benchmark counters, and screenshots.
- Run 1 also has `detail.txt` with every path publication and the searches it woke.
- Setup and analysis scripts: `/tmp/p4-traversal-profile.py`,
  `/tmp/p4-traversal-analyze.py`, and `/tmp/p4-traversal-detail.py`.

Both captures reached a visibly colored cube and populated Kanban. The event
files stop at that milestone; buffering omitted the last 53/13 accepted visits
relative to the counters. Tables consistently use the recorded prefix, under
0.03% short of the counted visits. A publication near the milestone can still
have pending work, so wake counts and eventual reset counts differ.

Tracing adds overhead and can change scheduling. The timings from these runs
are not optimization benchmarks. The previous uninstrumented comparison remains
in `definition-validation-results.md`. Engine source and the regular Slipstream
executable were unchanged by this investigation; regression suites were not
rerun for diagnostic instrumentation.


## Shared definition helper fix (2026-10-02)

Baseline `7a354e3`. `DefinitionRequirement::hasExpressions()` and
`sameDefinition()` recursively treated the compiled requirement DAG as a tree.
Repeated references therefore multiplied helper work without increasing the
number of graph visits. Both now use an iterative worklist and a temporary
visited set: requirement nodes for expression detection, requirement pairs for
structural comparison. The sets live for one call, so edits between calls stay
visible. Existing comparison fields and contextual-expression checks are retained.

An uninstrumented before/after probe used the same generated source and build
flags (`-g -std=gnu++17 -fpermissive`, no optimization), with only these helpers
changed. Definitions have the form `#p0='A'`, `#p1=*_+[{p0} p0]`, and so on;
`{pN 'X'} == {'A' 'Y'}` forces consideration of the shared alternatives before
rejecting the suffix. Both versions leave the same symbolic result.

| Probe | Before | After | Graph steps, both versions | Input matches, both versions |
| --- | ---: | ---: | ---: | ---: |
| Nested definitions, depth 18 | 17.484 s | 0.458 s | 6,704 | 37 |
| Nested definitions, depth 20 | 80.835 s | 0.451 s | 8,112 | 41 |

A direct depth-20 DAG probe with 42 distinct requirement objects took 1.038 s
before and 0.170 ms after, returning the same expression/equality answers.
These are single-run measurements. They exclude diagnostic helper counters;
the earlier instrumented timings are not the same benchmark. Temporary probe
sources and results are in `/tmp/p4-helper-baseline/`, `/tmp/p4-helper-fixed/`,
and `/tmp/p4-helper-benchmark-results.json`.

Validation: CodeDog regeneration/build succeeded; all **1,382 distinct registered
graph tests passed**. Three new helper cases cover deeply shared requirements,
all expression-bearing edge kinds, unequal comparison counterparts, expression
source/scope differences, and mutations between calls. The existing language
scaling regression now reaches depth 20. Nested suite selectors caused 113
additional repeat executions; none failed. Full legacy tests and cube startup
were not rerun for this change. Preservation scheduling, model-wide definition
notifications, and step-history retention are unchanged.


## Scoped definition publication (2026-10-02)

`ProteusDefinitionPlan` already identifies the instance slots it can modify.
Ordinary plan application now publishes those destinations through the existing
object-dependency mechanism instead of publishing the shared `modelFacts` token.
The footprint includes the POV/view, shared size/value states, and neighboring
position slots. It is collected before and after application so replaced state
slots, old tail links, and newly allocated members are represented. Embedded
owners come from the existing layout places; no virtual members are enumerated.
The temporary footprint adds no persistent validation cache or adjacency graph.

Operations that can alter registry relationships retain the existing broad
fallback: binding contextual-expression copies, combining a named member
template, and materializing an already registered sparse occurrence. Their
registry effects need explicit dependency keys before they can be narrowed.
Type-certificate publication continues to use its existing evidence keys.

A control executable retains the previous `DefinitionApplications.h` while
using the same generated tests and remaining headers. After declaring
`#seven=*10+7` and `#pair={seven seven}`, four independent scalar/list definition
applications produce the following extra steps in unrelated waiting searches:

| Waiting searches | Previous publication | Scoped publication |
| ---: | ---: | ---: |
| 1 | 12 | 0 |
| 8 | 96 | 0 |
| 32 | 384 | 0 |

Actual incoming RHSs still resolve those waiting searches. Additional regressions
verify a distinct view sharing the modified value state and position-only
subscriptions across tail append and middle insertion. All six new cases pass;
the control fails the four cases that require isolation. The same control passes
the two direct footprint/link checks, which exercise the new plan helper in both
executables. CodeDog regeneration/build succeeds, and all **1,388 distinct graph
tests in 66 groups** pass, including temporal preservation and sparse cases.

Evidence: `/tmp/p4-definition-publication-focused.log`,
`/tmp/p4-definition-publication-control-focused.log`, and
`/tmp/p4-definition-publication-tests/results.json`. The retained pre-change
executable is in `/tmp/p4-definition-publication-baseline/`; the isolated
publication control is in `/tmp/p4-definition-publication-control/`.


The unchanged temporal-cube batch passes all 11 observations in both versions.
The scoped run took **311.71 s / 158,964 KiB peak RSS**, compared with
**324.90 s / 178,708 KiB** for the publication control: 4.1% less elapsed time
and 11.0% less peak RSS in this single sequential pair. This is a modest result;
the timing difference alone does not separate improvement from scheduling
variation. See [the cube exercise](cubie-transition-exercise.md) for the setup.
The independent-subscriber tests establish the eliminated cross-product in work
directly, without depending on wall-clock timing.

A separate remaining candidate is `ProteusDefinitions::prune()`, which still
clears successful instance checks on every data publication. Its contribution
was not independently measured in this change.


## Reusing successful instance checks (2026-10-02)

`ProteusDefinitions::accepts()` now keeps a dependency proof for a successful,
context-free instance check. Both the first validation and subsequent cache hits
expose that proof to readers. It includes the shared size/value states and list
positions actually examined, so a later reader does not lose subscriptions when
the answer comes from the cache.

Scoped object publications preserve these records; publishing a dependency
invalidates its check. Unknown-footprint/model-wide changes, catalogue updates,
and a new top-level normalization retain the conservative cache reset. This
keeps edits made between quiescent normalization calls visible. A Set receipt
without captured reads, and a check made outside an object-tracking scope,
retain their existing publication-scoped lifetime. Pending and contextual
checks are not cached. The atomic Set's source/target rechecks remain direct.

Invalid records are reclaimed in batches when the cache grows, rather than
scanning all cached instances on each publication. The existing cache key still
distinguishes the POV, compiled requirement, and immediate-parts demand.
Records do not survive the next top-level search reset; no separate graph or
list of unsuccessful attempts is introduced.

Validation: CodeDog regeneration/build succeeds and all **1,396 distinct graph
tests in 67 groups** pass. Eight new cases check proof reuse through 32 unrelated
publications, shared descendant value and size edits, list-link changes, broad
reset, pending-to-known refinement, untracked access, and explicit
re-normalization after a raw edit. Shared-state/link cases subscribe a new
reader through a cache hit before changing the data.

The old-cache control fails the five cases requiring consistent dependency
publication on cache hits; pending, untracked, and re-normalization behavior
already work there. Its only API shim accepts the new optional `prune` argument
while preserving the original unconditional reset. The cube timing baseline
uses the untouched executable from `1a7c54c`, not that shim.

Evidence: `/tmp/p4-instance-check-focused.log`,
`/tmp/p4-instance-check-control-focused.log`,
`/tmp/p4-instance-check-tests/results.json`, and
`/tmp/p4-instance-check-baseline/`.


The first implementation collected an indexed dependency set even for
contextual checks that cannot enter this cache. It passed the tests but took
349.64 s / 177,288 KiB on the cube, versus 335.10 s / 156,848 KiB for the
unchanged baseline. The final version keeps contextual checks on their direct
path and collects other reads into a vector, building a proof only on success.
It passes the full 1,396-test graph suite again.

The final cube run passed all 11 observations in **335.75 s / 157,504 KiB**.
Against baseline, that is 0.65 s more elapsed time and 656 KiB more peak RSS:
essentially unchanged in this single-run comparison. **There is no measured
cube speedup from this change.** The reuse regression demonstrates the avoided
repeat checks across unrelated publications, but those checks do not explain
most of this workload's cost. Repeated construction of incomplete definition
plans remains a candidate for a separate profile; it was not measured here.

See [the cube exercise](cubie-transition-exercise.md) and
`/tmp/p4-instance-check-comparison.json`. The slower first executable/header
are retained under `/tmp/p4-instance-check-first/`; the final measurement is
under `/tmp/p4-instance-check-refined-cube/`. GUI performance was not tested.


## Shared conjunctions, scopes, and pending alternatives (2026-10-02)

Baseline `e26d4ff`. Definition compilation still revisited shared subrequirements
in `scopeDefinition()` and copied repeated pairs into separate trees in
`combineDefinitions()`. The helpers now share results within one operation:
source requirements for scoping, ordered requirement pairs for conjunction.
An unchanged scoped result is reused as evidence that this subtree needs no
rebinding in that call. A conjunction retains the original left/right nominal
metadata precedence and validates the same scalar, layout, and member constraints.
It now counts its recursive helper work in the existing route-cost estimate.

Nested alternative validation now shares accepted, rejected, and pending
outcomes for the same POV and requirement during a single plan construction.
Each alternative still checks instance facts with its own local binding scope,
as in the previous independent plan constructor; staged writes are not executed
or shared. A reentrant check remains pending. Scratch is allocated only when
alternatives occur and released before the public constructor returns. A later
plan reads the current instance and definition again. No persistent attempted-POV
table, new graph nodes, or structural type equivalence was introduced.

Operation-count probes use the same generated translation unit and build flags
against the before/after headers. Direct DAG probes share each child twice.
Parsed definitions use `#p0=*10+_`, `#p1={p0 p0}`, and so on; the conjunction
query is `pN=pN`. Parsed alternatives use `#p0=*10+7`,
`#p1=*_+[p0 p0]`, and so on, checked against `*10+_`.
Compilation does not materialize the exponentially many described item positions.

| Probe | Depth | Metric | Before | After |
| --- | ---: | --- | ---: | ---: |
| Direct shared scoping | 20 | Scope visits | 2,097,151 | 41 |
| Direct shared conjunction | 16 | Result requirement objects | 131,071 | 17 |
| Parsed shared definitions | 20 | Scope visits over compilation | 6,291,411 | 1,563 |
| Parsed conjunction | 16 | Result requirement objects | 131,071 | 33 |
| Parsed pending alternatives | 16 | Plan preparation calls | 131,071 | 33 |

Pending alternative probes remain pending and produce zero writes. Sharing is
bounded by distinct subproblems within each operation; this does not claim that
all compilation, matching, or scheduling is linear. Probe sources/results are
under `/tmp/p4-definition-analysis/` and `/tmp/p4-definition-sharing-probe/`.

Validation: CodeDog regeneration/build succeeded and all **1,404 distinct
registered graph tests passed**, including eight new cases for sharing bounds,
scope preservation, changed source requirements, unequal conjunction pairs,
pending-to-known transitions, distinct targets, separate instance writes, and
parsed definition/alternative DAGs. The source bodies remain unchanged by plans.

The same eleven-observation temporal-cube batch ran sequentially against the
saved baseline and updated binaries, without test/build/probe overlap:

| Temporal cube | Baseline | Shared definition work |
| --- | ---: | ---: |
| Elapsed | 323.04 s | 331.28 s |
| Peak RSS | 152,576 KiB | 160,784 KiB |
| Observations | 11/11 | 11/11 |

This pair shows no cube speedup: the updated run took 8.24 s longer and
8,208 KiB more peak RSS. Runtime scheduling remains nondeterministic, so one
pair does not establish a stable difference. These are batch measurements,
not Slipstream frame-rate/startup measurements. The broad
legacy language suite and live GUI were not rerun. Logs, timings, and hashes are
under `/tmp/p4-definition-sharing-cube/`; graph test results are under
`/tmp/p4-definition-sharing-tests/`.

This is the first graph-simplification step: repeated compiled requirements and
equivalent branch checks are consolidated. Per-definition record analysis,
implication-based pruning, and removal of residual instance RHS obligations
remain separate work. Existing model relationships and subscriptions are retained.


## Reusable nominal record analysis (2026-10-02)

Baseline `bd24158`. Record definition validation previously compared every
requirement with every other requirement to establish distinguishability, then
compared each requirement with every supplied instance field. It reread an
instance field's types for every requirement.

`NominalMemberIndex` compiles the three existing positive nominal matching
rules into name-to-slot lookups. Primary words remain distinct from additional
types: sibling `wheel` and `pedal` fields do not become interchangeable merely
because both advertise `part`. Unknown primary words, duplicate candidates, and
one field matching multiple requirements still leave the plan pending. Pairs
retain requirement order, including ordered partial-description checks.

The immutable index belongs to the compiled requirement. Conjunctions adopting
an unordered declaration share its analysis. A linear signature check compares
member count, order, primary words, and additional types before reuse, rebuilding
when any changes. Copies with different layouts acquire their own analysis.
Changing a scalar constraint does not rebuild this purely nominal index, but
the instance is still checked against the changed constraint before any writes.
There is no instance-POV history in this cache; storage is proportional to the
definition's names and slots. Existing identity-first correspondence elsewhere
retains its original matcher.

The probe repeats the same record plan 16 times, independently of successful
instance-check caching. It uses the same generated translation unit/build flags
with before/after headers and counts operations at their source. Separate
regression tests check reuse across 16 different instances.

| Fields | Metric over 16 plans | Before | After |
| ---: | --- | ---: | ---: |
| 64 | Required-field pair comparisons | 32,256 | 0 |
| 64 | Instance type reads | 65,536 | 1,024 |
| 64 | Accounted work visits | 165,392 | 9,488 |
| 256 | Required-field pair comparisons | 522,240 | 0 |
| 256 | Instance type reads | 1,048,576 | 4,096 |
| 256 | Accounted work visits | 2,627,600 | 37,904 |

The updated probe builds one index across all 16 plans at every tested size
(8, 16, 32, 64, 128, and 256 fields). These counts demonstrate removal of the
all-pairs scans for distinct typed fields, not a universal linear-time claim:
map lookups, aliases, and candidate sorting still have their own costs.
Probe sources, logs, and counts are under `/tmp/p4-record-index-probe/`.

Validation: CodeDog regeneration/build succeeded. All **1,411 registered graph
tests in 69 groups passed**, including seven new record-index tests. The new
matcher agrees with the previous record matching contract across 5,329 small
layout pairs involving aliases, shared ancestors, unknowns, omissions, and
duplicates. Other cases cover shared analysis, copied/edited layouts, changed
value constraints, atomic pending/rejected plans, and bounded repeated matching.
Results are in `/tmp/p4-record-index-tests/`.

The unchanged eleven-observation temporal-cube batch ran sequentially against
the saved baseline and rebuilt executable, without other tests, builds, or
probes overlapping either run:

| Temporal cube | Baseline | Indexed records |
| --- | ---: | ---: |
| Elapsed | 325.90 s | 304.55 s |
| Peak RSS | 157,848 KiB | 157,964 KiB |
| Observations | 11/11 | 11/11 |

The measured deltas are -21.35 s and +116 KiB peak RSS.
This is one timing pair, not evidence of a stable runtime difference. The
operation-count improvement is established independently; the cube still takes
several minutes. These are batch measurements, not live Slipstream frame-rate
measurements. The full legacy language suite and GUI were not rerun. Logs,
timings, and source/binary hashes are under `/tmp/p4-record-index-cube/`.

This step prunes impossible nominal candidates using existing definition facts.
It does not add structural type equivalence, implication/theorem-based pruning,
or remove unresolved instance obligations and their subscriptions.


## Phase profile and shared evidence identity (2026-10-03)

Baseline `bc9ddce`. The current eleven-observation cube exercise was profiled
by model load and individual query, with counts for definition planning,
correspondence, and scheduler activity. Diagnostic counters and nested timers
were added only to scratch copies of the generated executable and graph headers.
No per-POV history or instrumentation was added to the production engine.

The largest measured component was `ProteusDefinitions::remember()`:
**1,204,411 calls and
150.31 seconds of exclusive measured time**.
The initial diagnostic batch took 302.03 seconds and passed all eleven
observations. Public plan construction took 91.73 seconds inclusively;
its recursive `prepare` was called 3,510,315 times. Nested inclusive times
overlap and must not be added together. Temporal preservation's entire
`finishPass()` took only 0.53 seconds inclusively, and goal factories 0.09 seconds.
Those were not the appropriate first optimization targets for this workload.

The expensive duplicate-certificate check walked the witnesses of inherited
source evidence and used nested scans to compare catalogue bindings, even when
both certificates shared exactly the same `Evidence` object. Every member
certificate propagated by `applyChanges()`/`inheritChanges()` shares that source
evidence. Repeated checks therefore compared these collections with themselves.

The production change adds one identity shortcut after requirement equality:
shared evidence already establishes equal witnesses and bindings. Current-proof
validation still runs first; different requirements remain distinct. Separately
constructed evidence still uses the existing dependency comparison. This adds
no new cache, metadata, subscriptions, or instance-retention structure.

The updated profile took the shortcut **1,196,195 times**.
`remember()` exclusive time fell to
**3.15 seconds**.
Pending/contextual checks and the atomic Set's source/target validation remain
unchanged; this step removes repeated equality work rather than weakening checks.

After the shortcut, public plan construction is the largest remaining measured
component at 90.14 seconds
inclusively, with 3,497,716 recursive preparations.
The source/target rechecks in `reusable()` account for
72.97 seconds inclusively
within that planning work. Those checks can run after another write in the
same atomic Set; any later reuse must describe that intermediate state.

Diagnostic phase times (not ordinary-build performance figures):

| Phase | Before | After |
| --- | ---: | ---: |
| Model load/normalization | 182.349 s | 95.174 s |
| Five initial-state reads | 6.148 s | 4.948 s |
| First second-state read (changed rotation) | 6.990 s | 3.517 s |
| Further second-state reads | 104.592 s | 45.479 s |
| Recheck original state | 1.716 s | 1.740 s |

The initial-state and transition rows are read requests after loading. Loading
itself normalizes the model and may do work affecting both states. These rows
must not be interpreted as an attribution of every operation to one state.

| Counter | Before | After |
| --- | ---: | ---: |
| `plan.construct` | 22,002 | 21,975 |
| `plan.prepare` | 3,510,315 | 3,497,716 |
| `definitions.remember` | 1,204,411 | 1,197,869 |
| `graph.startRequests` | 44,205 | 44,257 |
| `graph.tasksCreated` | 6,937 | 6,938 |
| `graph.wakeCalls` | 58,991 | 58,426 |
| `graph.newlyDirty` | 18,477 | 18,483 |
| `graph.resetTask` | 33,767 | 33,816 |
| `graph.visits` | 136,493 | 135,339 |
| `preservation.exactComparisons` | 16,639 | 16,569 |
| `preservation.nominalComparisons` | 13,116 | 13,116 |

Counts include repeats across publications. Wake calls include notifications
to already dirty tasks; initial task activation is included in resets.
Scheduling/streaming publication timing can change between executions, so
differences in graph counts do not by themselves establish a scheduling change.
The production edit changes only certificate equality checking.

Validation: CodeDog regeneration/build succeeded and **all 1,415 registered
graph tests passed**. Four focused cases cover shared provenance, different
requirements on shared evidence, equivalent independent evidence, and distinct
missing-name dependencies. They check later definition publication still
invalidates the affected proof.

Ordinary builds were then compared sequentially against the saved baseline,
with the same fixture, assets, and empty session data. No profile, build, or
other test overlapped either timed run:

| Temporal cube | Baseline | Shared evidence shortcut |
| --- | ---: | ---: |
| Elapsed | 291.89 s | 148.28 s |
| Peak RSS | 155,508 KiB | 171,432 KiB |
| Observations | 11/11 | 11/11 |

Elapsed time decreased 49.2% in this pair. One pair is not a stable
performance guarantee; the independent before/after profile identifies the
removed work. Ordinary peak RSS increased 15,924 KiB
(10.2%). The diagnostic pair
went the other way, from 171,672 to 158,932 KiB; these runs establish no consistent
memory improvement. The code adds no retained state. The full legacy language
suite and live Slipstream GUI were not
rerun. All timings are for the batch model/query exercise.

Evidence: `/tmp/p4-phase-profile/` and `/tmp/p4-phase-profile-after/` contain
the diagnostic sources, labels, logs, and per-phase JSON counts.
`/tmp/p4-proof-identity-tests/` contains graph test results;
`/tmp/p4-proof-identity-cube/` contains ordinary timings, logs, and hashes.
The setup/analysis scripts are `/tmp/p4-phase-profile-setup.py`,
`/tmp/p4-phase-profile-after-setup.py`, and their `-analyze.py` counterparts.
Aborted scratch setup runs were excluded; the completed diagnostic runs were
sequential.


## Transfers between shared infonViews (2026-10-03)

Baseline `6e757fb`. The follow-up profile separated source and destination
plans used by `ProteusDefinitions::reusable()`. It found 3,077 of 3,118
transfer checks had the exact same `infonView` at both endpoints. One
`edgecubes` destination/requirement pair was planned 1,229 times, returning
605,897 member bindings and no staged writes. Those counts include repeated
checks across publications, not that many distinct objects.

Certificates are already keyed by `infonView`. The implementation therefore
returns no transfer plan when the two endpoints share that exact view. Both
discovery (`canInherit`) and execution (`inheritChanges`) use this guard;
they no longer rebuild source/target plans or replay child certificates merely
to transfer evidence back into its existing owner. The guard observes both
POVs, so retargeting an alias remains a relevant dependency change.

This is not successful validation or a new acceptance receipt. Ordinary type
goals still validate current data, publish needed child certificates, and
observe contextual completion and demanded sparse occurrences. Distinct views
keep both fresh checks even when their value or size buffers are shared. That
also preserves detection of a mutation earlier in the same atomic Set, before
publication could invalidate a cached check. No cache, retained instance
history, graph adjacency, or cube-specific rule was added.

CodeDog regeneration/build succeeded. All **1,423 registered graph tests
in 71 groups passed**, including eight new `unit/graphViewTransfer/`
cases. They cover bounded alias work and retargeting dependencies, separate
headers sharing a value buffer, partial copies, unpublished source/destination
edits, child-certificate propagation, invalid shared data, independent contextual
instances, and a contradiction in one demanded item of a billion-item sparse
list without enumerating the list. Existing expectations were unchanged.

Ordinary executable comparison, sequential with no overlapping build/test/probe:

| Eleven-observation temporal cube | Baseline | Shared-view guard |
| --- | ---: | ---: |
| Elapsed | 153.23 s | 66.68 s |
| Peak RSS | 156,760 KiB | 115,672 KiB |
| Observations passed | 11/11 | 11/11 |

This pair used **56.5% less elapsed time** and **26.2% less peak
RSS**. It is one paired measurement, not a stable performance guarantee or a
live GUI measurement. The full legacy suite and Slipstream GUI were not rerun.

The separate diagnostic run also passed all eleven observations and took the
identity guard **1,042 times**. Its
counter comparison with the preceding investigation is:

| Diagnostic work | Before | After |
| --- | ---: | ---: |
| Model load/normalization | 94.411 s | 46.149 s |
| Eleven query phases | 52.447 s | 16.789 s |
| Public plan constructions | 20,331 | 10,637 |
| Recursive preparations | 3,449,600 | 638,274 |
| Source transfer plans | 3,118 | 41 |
| Destination transfer plans | 3,099 | 22 |
| Certificate remember calls | 1,179,195 | 5,446 |

Instrumented phase times are separate from the ordinary executable comparison.
Scheduling/publication timing can change counts between runs. Inclusive nested
timers overlap and must not be added together. The before/after diagnostics
use the same counters plus the new guard counter; instrumentation remains in
scratch copies only.

Evidence: `/tmp/p4-view-transfer/baseline/` preserves the prior executable and
headers; `/tmp/p4-view-transfer/cube/` has ordinary logs, times and hashes.
`/tmp/p4-view-transfer/tests/` contains the graph results. Diagnostic sources,
logs and counts are in `/tmp/p4-plan-origin-profile/` (before) and
`/tmp/p4-view-transfer/profile/` (after). Reproduction scripts are
`/tmp/p4-view-transfer-benchmark.py`, `/tmp/p4-view-transfer-checks.py`,
`/tmp/p4-view-transfer-profile-setup.py` and its `-analyze.py` counterpart.


## Compiled contextuality and retained source validation (2026-10-03)

These two changes follow baseline `13d275b` and were implemented and timed
separately. Step 1 is commit `8ce4f7e`; step 2 accompanies this report.

1. The definition compiler records whether a completed requirement contains
   contextual expressions. `hasExpressions()` can read that summary and stop
   at already analyzed children, rather than revisiting shared descendants.
   A copied requirement loses its summary and becomes an editable builder;
   unfinished builders remain uncached. Contextual expressions still execute
   in their instance's graph goals, and changed meanings compile new snapshots.
2. A positive source-certificate check can survive an unrelated scoped
   publication when its existing dependency proof remains valid and all its
   object reads were tracked. Broad invalidation, catalogue changes, and an
   explicit normalization retain the conservative reset. Negative results and
   untracked reads do not acquire cross-publication reuse.

The second change reuses existing dependency groups. It adds a reset counter
and small flags to existing evidence/witness objects, not another cache,
per-POV traversal history, or graph. A proof first checked outside object
tracking must capture tracked reads before serving a tracked consumer. Shared
witnesses have the same rule. Before applying a definition plan or transferring
evidence between distinct views, a fresh check also rereads bindings and source
witnesses: an earlier write in the same Set may not have published yet.

CodeDog regeneration/build succeeded. Step 1 passed all **1,426 graph tests**
after correcting the open-template setup of one newly added unit-conversion
fixture. The final build passed all **1,435 registered graph tests in 73
groups**, with no existing expectation changes. Three contextuality cases
exercise copies, conjunction/scope/unit transformations, and changed meanings.
Nine source-validity cases cover unrelated writes, shared-value edits, list
link changes, explicit resets, untracked reads, tracking upgrades, late
declarations, and unpublished source/binding edits at a Set boundary. The
strengthened tracking-upgrade case failed before the guard correction and
passed afterward. Both an existing proof and another proof sharing its
witnesses must wake when their source changes.

Ordinary executables ran sequentially on the same eleven-observation temporal
cube fixture, with no build, test suite, or diagnostic probe overlapping a
timed run:

| Temporal cube batch | Baseline | Step 1 only | Both changes |
| --- | ---: | ---: | ---: |
| Elapsed | 59.30 s | 55.21 s | 47.81 s |
| Peak RSS | 117,252 KiB | 115,596 KiB | 111,932 KiB |
| Observations passed | 11/11 | 11/11 | 11/11 |

Step 1 reduced elapsed time **6.9%**; adding step 2 reduced it a further
**13.4%**. Together they used **19.4% less time** and **4.5% less peak RSS**
than this sequence's baseline. This is one three-version measurement, not a
stable performance guarantee. The previous report's 66.68 s measurement is a
different run; 59.30 s is the freshly measured baseline used here. The full
legacy language suite and live Slipstream GUI were not rerun.

A separate scratch diagnostic also passed all eleven observations. Compared
with the preceding shared-view diagnostic:

| Diagnostic work | Before | Both changes |
| --- | ---: | ---: |
| Model load/normalization | 46.149 s | 36.629 s |
| Eleven query phases | 16.789 s | 11.938 s |
| Contextuality checks | 165,865 | 174,893 |
| Time in contextuality checks | 4.514 s | 0.159 s |
| Source-witness checks | 226,209 | 60,919 |
| Time in source-witness checks | 8.185 s | 1.981 s |
| Public plan constructions | 10,637 | 11,731 |
| Recursive plan preparations | 638,274 | 648,472 |

The new counters record 62,586 direct contextuality-summary hits and 112,307
builder checks, which can stop at analyzed children. Source-certificate
validation reused its tracked proof across publications 200,444 times; 57,444
witness calls actually reread their snapshots. Thus the savings come from
less work inside repeated checks, not a reduction in plan count. Scheduling
can vary between runs. Inclusive nested timers overlap and must not be added
together; the diagnostic time is separate from the ordinary comparison.

Evidence is under `/tmp/p4-definition-validity/`: `baseline/`, `step1/`, and
`step2/` preserve executables and headers; `cube/` contains ordinary timings,
logs, and hashes; `tests-step1/` and `tests-final/` contain graph results;
`diagnostics/` contains scratch instrumentation, logs, and per-phase counters.
The earlier diagnostic is `/tmp/p4-view-transfer/profile/phases.json`.
Reproduction scripts are `/tmp/p4-definition-validity-benchmark.py`,
`/tmp/p4-definition-validity-tests-final.py`,
`/tmp/p4-definition-validity-profile-setup.py`, and its `-analyze.py`
counterpart. `upgrade-before.log` records the reproduced tracking failure;
the final graph log records its passing result. Instrumentation is not part
of the production sources.


## Retaining completed parts of pending definitions (2026-10-03)

Baseline `0fb1267`. This investigation tested whether the temporal cube was
re-solving information already known, including whether `isNormed` could stop
that work. Only 36 of 6,350 normalization starts in the diagnostic found that
flag set. Whole-infon completion is too coarse here: one unresolved contextual
obligation can leave a definition pending while its other parts are satisfied.
The prior validator rebuilt the complete nested plan on each such attempt.

The accepted change reuses a positive validation for a completed,
context-free subrequirement while the parent remains pending. It uses the
existing instance-check cache and dependency proofs. Parents retain shared
child proof groups, rather than flattened copies of all descendant reads.
The cache still resets on explicit normalization/broad invalidation; a relevant
object or list-link publication invalidates the affected child. A cached value
does not certify an incoming contradictory assertion or a changed definition.

`ProteusDefinitionPlan::validates()` is a read-only entry point. It cannot
return a Set plan or incomplete certificate bindings. Ordinary writable plans
retain the full preparation. Reuse is also excluded for contextual subtrees,
explicit immediate-part validation, untracked reads, deferred allocations, and
after an earlier staged write. No `isNormed` rule, temporal preservation rule,
size/value interpretation, or compiler setting was changed.

Seven new `unit/graphPartialValidation/` cases each run with 16, 64, and 128
known members. Repeated validation with a pending sibling performs two cache
lookups per attempt, independent of the known subtree's width, and stores no
additional entries. Other cases check a shared-value mutation, an appended
list link, resolution of the pending sibling, earlier staged writes, explicit
part policy, and complete bindings when a writable plan is applied.
CodeDog regeneration/build succeeded. All **1,442 registered graph tests in
74 groups pass**, in both the normal build and a separate `-O2` build.
Existing expectations were unchanged.

The diagnostic experiment reduced recursive uncached definition preparations
from **633,470 to 71,315**. There were 41,945 successful subtree reuses and
16,187 stores over the full load plus eleven queries. Time in public plan
construction fell from **16.553 s to 3.243 s**, and total diagnostic time
fell from **47.95 s to 31.90 s**. These counters were collected in scratch
instrumented prototypes; the ordinary final-build comparison below validates
the resulting implementation independently. Nested inclusive timers overlap
and must not be added together.

Final ordinary executable measurements, sequential with no overlapping
build/test/probe, using the same eleven-observation cube fixture:

| Compiler setting | Baseline elapsed | Final elapsed | Baseline peak RSS | Final peak RSS |
| --- | ---: | ---: | ---: | ---: |
| Current normal build, no optimization | 48.61 s | 32.25 s | 119,908 KiB | 112,948 KiB |
| Separate `-O2` build | 10.37 s | 6.09 s | 106,956 KiB | 100,544 KiB |

The algorithm change reduced elapsed time **33.7%** in the normal build
and **41.3%** in the optimized comparison. Each executable passed all eleven
observations: changed rotation, preserved other fields and color, neighboring
cubie, and unchanged original state. This is one sequential measurement per
configuration, not a stable performance guarantee. The optimized final test
executable has lightweight phase timers; the other ordinary executables have
no diagnostic instrumentation.

The optimized final run spent **4.885 s** loading/normalizing the two-state
world and **1.113 s** in the eleven queries. Its five state-2 queries took:

| Query after loading | Seconds |
| --- | ---: |
| Changed first edge cubie's x rotation | 0.098 |
| Preserved y rotation | 0.121 |
| Preserved z location | 0.150 |
| Neighboring cubie's x rotation | 0.196 |
| Preserved sticker hue | 0.234 |

Loading already performs work on both states. These are field-query latencies,
not a measurement of constructing all of state 2 from an already loaded state
1. A complete cold load/normalization is still above one second. The full
legacy language suite and live Slipstream GUI were not rerun. Compiler
optimization was measured separately: CodeDog's current Linux builder emits
`-g -std=gnu++17 -fpermissive` without an optimization flag
(`/home/bruce/devl/CodeDog/buildDog.py`, `LinuxBuilder`); neither that builder
nor Slipstream was modified here.

Other experiments were not incorporated:

- Narrowing path-write publication reduced one event from 4,277 newly dirty
  goals to four. A following broad scalar Set overlapped with it. Narrowing
  both events still gave 48.64 s versus 47.95 s, with no meaningful total-time
  gain. Dirty-notification counts alone overstated the useful work removed.
- Allowing whole contextual checks into the positive cache gave 47.96 s:
  those checks were pending, so they had no completed result to reuse.
- Publishing completion at the preservation guard, with per-task retained
  proofs, passed the graph tests but took 55.26 s and 262,624 KiB peak RSS.
  It did not increase the number of already-marked normalization starts.
- Scanning registry relationships was only 0.219 s in this diagnostic;
  the definition-write footprints were small. Neither was the main cause.

Evidence is under `/tmp/p4-transition-work/`: `profile.log` and `labels.json`
hold the baseline counters; `scoped*`, `context*`, `scoped-both*`, `subchecks*`,
and `settle*` retain the individual experiments. `tests-final/` and
`final-optimized-tests/` contain final graph results. `ordinary/` contains
final timings, hashes, logs, and per-query durations. `final/` retains final
headers, generated source, normal executable, and optimized timed executable.
`optimized-baseline/` retains the unchanged baseline executable and source.
The scripts `/tmp/p4-transition-profile-setup.py`,
`/tmp/p4-transition-subchecks-experiment.py`,
`/tmp/p4-transition-tests-final.py`, `/tmp/p4-transition-final-setup.py`, and
`/tmp/p4-transition-final-benchmark.py` reproduce the main checks.
The optimized compiler command was:

```sh
g++ -std=gnu++17 -fpermissive -g0 -O2 -pthread TestProteus.cpp \
    -lgmp -lgmpxx -licuio -licui18n -licuuc -licudata -o TestProteus-O2
```

## Retained field comparisons (2026-10-05)

Baseline: `7609397`. Field-selection goals now retain individual candidate
comparisons and their dependency proofs across retries. A scoped publication
invalidates the affected comparison; unchanged comparisons reuse their answers.
The existing path performs the comparison and schedules its ordinary required
refinements. No new scheduler goal kind or change to `isNormed` is involved.
A fresh explicit normalization uses a new comparison epoch. Answers belong to
path goals; the context keeps a weak lookup index with batched reclamation.

A single instrumented comparison used the current World definitions and
Slipstream `Resources/userKB.pr`, with `-O1 -g0`, identical profiling hooks, and
no concurrent builds. These are headless normalization timings, not GUI timings.

| Measurement | Baseline | Retained comparisons |
| --- | ---: | ---: |
| Forward turn normalization | 1.839 s | 1.991 s |
| Inverse turn normalization | 6.141 s | 6.409 s |
| State-2 display-data normalization | 149.974 s | 125.006 s |
| State-2 search steps | 2,970,754 | 2,988,575 |
| State-2 task resets | 398,281 | 399,724 |
| Exclusive field-selection expansion time | 52.282 s | 33.538 s |
| Display-process peak RSS | 906,012 KiB | 920,380 KiB |

This saves about 17% on the measured state-2 normalization, with approximately
2% more peak memory, but does not reduce the outer traversal/reset count.
The small raw-turn timings are slightly slower. The independent placement and
axis-permutation oracle passes, and the previous state remains unchanged.
Both display-data drivers still stop at the existing missing-`thing` lookup
on a projected cubie; this change does not establish correct state-2 rendering.

A discarded version made each candidate assessment a separate scheduled goal.
It emitted 28,012 assessment publications in the bounded state-2 run; dependency
reconciliation consumed 71 seconds and normalization exceeded the 150-second
budget. Keeping comparison resumption inside the existing path avoids that
extra scheduler traffic. Direct diagnostic counters verify one affected
comparison and zero unaffected comparisons after an update, with 2, 9, and
33 candidates, for both delayed acceptance and delayed rejection.

Final validation: the optimized CodeDog build succeeds, and 1,505 selected graph
checks pass, including seven new evidence/subscription tests. The older
`unit/graphDescriptive/modelTransition` case remains excluded from this audit;
the complete legacy suite and the live Slipstream GUI were not run.

Detailed evidence and reproduction scripts are under
`/tmp/p4-candidate-subscriptions/`; `direct-profile/` holds the accepted-design
measurements, `comparison.json` records the main counters, and `accepted-audit/`
holds the final production-binary checks. The earlier baseline is under
`/tmp/p4-reprofile/`.
