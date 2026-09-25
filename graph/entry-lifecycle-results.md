# Agent entry point and stream lifecycle — 2026-09-24

`Agent.normalize(pov)` now returns a graph normalization handle. Each Agent owns
one persistent graph context using its existing identity registry. The former
Agenda construction and processing in this entry point, the `normalizeOneLevel`
flag manipulation, and the threaded normalizer's initial sleep/wait loop are
removed. There is no fallback into AItems.

Synchronous query, load, and test callers use `normalizeAndWait()`. This helper
retains unresolved searches so their subscriptions survive the call, including
searches already restarted by a concurrent publication. It records a failed
handle in the Agent's error fields. `Agent.close()` cancels remaining searches
and joins the graph worker; `ProteusCore.deInit()` calls it.

Streaming parsing uses a graph publication lease. Parser mutations hold the
context lock, input-buffer operations release it, and parse milestones publish
revisions. Searches can visit completed children in a growing literal list;
unfinished parser-owned fields and indexed materialization stay deferred.
A stream retains its handle and waits for input completion as well as graph
quiescence. Cancellation/failure releases that wait. Both streaming and
parse-first loader paths use the same lifetime mechanism, with idempotent joins.
Stream shutdown cancels normalization and closes input before joining producers.

Reply delivery is separate from goal execution order. Settlement claims reply
markers; after all settled markers are claimed, it captures result text under
the graph lock and delivers callbacks outside it in input-marker order. This
preserves nested reply order and removes consumed markers from containing-list
printouts. Origin IDs and item positions are preserved. Callbacks may publish
information, but must not wait for the graph worker or close it. Re-normalization
does not resend consumed replies. Symbolic settled results can also reply.

Validation:

- Full CodeDog regeneration and C++ builds passed for `Proteus.Lib.dog` and
  `streamReplyTest.dog`.
- Public entry/lifecycle integration: **16/16 passed**. Coverage includes handle
  traces, ordinary queries, shared registry identity, retained subscriptions,
  shutdown, conflicts, exactly-once ordered/nested replies, incremental parser
  input, split reply tokens, cancellation, parse failure, buffered/streaming
  loaders, and deferred indexed access while input is incomplete.
- Existing graph semantic integration: **155/155 passed**.
- Generic runtime: **28/28 passed** under AddressSanitizer,
  UndefinedBehaviorSanitizer, and LeakSanitizer. New cases cover publication
  during input suspension, EOF versus quiescence, cancellation, and completion
  effects that reenter the publication API. LeakSanitizer required execution
  outside the sandbox's ptrace environment. No ThreadSanitizer result is claimed.
- Existing streaming harness selections `-f 1`, `2`, `4`, `5`, and `8`:
  **29/29 checks passed**. These exercise scalar/list replies, nested boundaries,
  and split reply tokens. The typed-path and streaming-RHS correspondence cases
  were not claimed as migrated.
- Existing language baselines now run through the public Agent entry:
  **27/38** general, **9/9** expression, and **6/11** selection exact output
  matches, unchanged from the preceding graph-only checkpoint. Known retained
  RHS/unknown numeric-size differences remain visible.
- Parser checks: four original parse-only numeric fixtures and five error/
  location fixtures passed. The broader `parse/` language group was **4/14**:
  its other ten tests invoke normalization or definitions and remain mismatches
  (fraction reduction, string/template parsing, grouping, inner RHS rendering,
  and the defined language). The full legacy suite was not run.
- `git diff --check` passed. The original Proteus checkout was not changed.

Remaining boundary: this is a lifecycle migration, not complete semantic parity.
Relationships to a still-parsing RHS remain subscribed until it finishes;
incremental RHS correspondence, definitions/types, paths/cursors, and general
parsing-language rules still need migration. Legacy AItem declarations remain
compiled for those facilities and reference tests, but ordinary normalization
uses only the graph engine.

Run from the fork root:

```sh
codeDog Proteus.Lib.dog
(cd LocalBuild && ./TestProteus -t unit/graphEntry/)
(cd LocalBuild && ./TestProteus -t unit/graphFoundation/)
python3 graph/run_language_baseline.py
python3 graph/run_language_baseline.py --fixtures graph/expressions-language.txt
python3 graph/run_language_baseline.py --fixtures graph/selection-language.txt
codeDog streamReplyTest.dog
(cd LinuxBuild && ./streamReplyTest -f 1)
```
