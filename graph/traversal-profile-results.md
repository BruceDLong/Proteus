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
