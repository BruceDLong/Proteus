# Buffered parser publication and Rubix startup

The shared sequence chart does not run on the current Rubix display path. This
change instead reduces repeated normalization during model loading.

## Finding

An instrumented Slipstream startup at `7b98398` recorded the following across
its active searches before the cube first became visibly colored:

- 220,854 graph visits and 154,882 task resets.
- 4,892 graph publications.
- 460,842 catalogue lookups and 118,194 type-certificate freshness checks.
- About 7.0 seconds inside catalogue lookup, certificate validation and
  definition-plan construction, excluding time spent in other instrumented
  functions. The complete instrumented startup took 32.4 seconds.

The parser held the graph lock while changing the shared infonView, then
released it at every input operation, including nonblocking reads from closed
buffers. Each release flushed any pending parse milestones. Those publications
invalidated definition evidence and restarted active searches on intermediate
versions of the same document.

## Change

`ProteusParser::inputMayBlock()` checks input closure under the buffer mutex.
Closed input can return either text or EOF without awaiting a producer. Its
read operations can therefore share a publication. Open input retains an
immediate release around the entire operation: a scan can require text beyond
its starting position.

`InputPause` accepts that nonblocking guarantee. `Publication` requests a yield
at the next input operation after a two-millisecond interval. Pending changes
are published before every yield and at parser completion. This interval is a
fairness target between operations, not a hard deadline on the duration of a
single scan. Error callbacks still release the lock immediately.

Notifications, subscriptions, graph dependencies, and definition requirements
remain enabled. The batching applies to any closed input buffer and has no
knowledge of Rubix structure, definitions, or rendering.

## Measurement

| Pair | Baseline visible color | Batched visible color | Baseline / batched visits |
| --- | ---: | ---: | ---: |
| 1 | 31.07 s | 26.84 s | 215,048 / 192,145 |
| 2 | 32.44 s | 25.54 s | 211,977 / 180,807 |
| 3 | 32.28 s | 25.73 s | 205,070 / 196,274 |

Median visible-color time fell from **32.28 s to 25.73 s** (20.3%).
Median first-hue availability fell from **11.17 s to 6.62 s**.
Median graph visits fell from **211,977 to 192,145** (9.4%).
Sequence/chart visits remained zero in every run.

Timing runs use private 1100x800 Xvfb displays, fresh per-run assets and XDG
state, and the real Slipstream cube and Kanban. Runs are sequential and alternate
baseline/change; no test or compilation workload runs alongside them. The
baseline is `7b98398`, including the sequence chart. The changed app uses the
same generated app code and assets with the changed parser methods copied
verbatim from regenerated Proteus4 C++; it includes the changed graph header.
The accepted generated-code diff contains only those parser changes and the new
helper declaration. Both apps carry the same three visit counters and first-hue
measurement. Neither app changes the scene or display demand policy.

The visible-color milestone is sampled screenshots, with roughly 1.1 seconds
between samples. First known hue is an earlier data-availability milestone.
Neither means every live normalization subscription has ended. Screenshots were
visually inspected to confirm a colored cube and populated Kanban.

Local raw evidence, build logs, scripts, manifests and screenshots are under
`/tmp/p4-cube-chart-bench/`. Final runs are named `batch-control-1..3` and
`batch-accepted-1..3`. Profiling evidence is `profile-1/result.json` and
`profile-labels.json`; preliminary experiments are separate from the final runs.

## Validation and next limit

- Regenerated and built with `codeDog Proteus.Lib.dog`.
- 1,299/1,299 graph checks across the 59 focused groups passed. This includes
  the new closed-buffer reply test and existing open/nested streams, split reply
  markers, cancellation and parse errors.
- 60/60 native graph scenarios passed, including a new test that a nonblocking
  input producer yields after its publication interval and wakes a subscriber.
- 13/13 stream-insertion checks and 14/14 legacy `parse/` language tests passed.
- Actual colored cube and populated Kanban confirmed in private-display captures.

The full older default suite was not rerun for this change. The parser/runtime
passed the focused lifecycle and semantic checks above.

The remaining work includes broad invalidation after semantic Sets and repeated
definition/field validation across searches. Per-fact dependencies or better
reuse should be evaluated against this actual path. The parser change reduces
how often an intermediate model is published; it does not make those later
searches independent of unrelated writes.
