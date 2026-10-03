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
