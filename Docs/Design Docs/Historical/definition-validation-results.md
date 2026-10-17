# Shared definition validation and Set evidence

2026-10-01. Baseline: `c744b54` (buffered parser publication), on the private
`codex/proteus4` branch.

## What was repeated

An instrumented real Slipstream Rubix startup made 122,376 certificate-freshness
calls and 84,946 physical source-witness checks. Of the latter, 63,526 revisited
the same source POV within the same validation epoch. Catalogue lookup ran
448,241 times and declaration-registration verification 446,126 times.

The profile attributed about 1.23 seconds to witness checks, 1.22 seconds to
replaying catalogue dependencies, and 1.66 seconds to catalogue lookup including
registration verification. These are profiling observations, not independent
end-to-end speedup estimates.

## Changes

- Certificates compiled from a shared definition now share immutable source
  snapshots. A snapshot is checked once per validation epoch, with its observed
  dependencies replayed on reuse. Different historical snapshots of the same
  POV remain distinct: a new valid snapshot cannot rehabilitate an old proof.
- Catalogue resolution caches unique, absent, and ambiguous answers by name
  for the epoch. Every lookup still observes that name's dependency. Direct
  declaration publication also invalidates the cache.
- Cached certificate validation exposes a shared dependency proof, including
  dependencies discovered through list-header aliases. A late definition still
  wakes consumers of a previously absent name, including completed results
  whose original search handle has been released.
- `SetResult::retainEvidence` lets an atomic Set retain checks it has established
  after its publication invalidates older evidence. The graph runs this callback
  under its context lock, before the next search step. It cannot mutate model
  data or start another search.
- Scalar-copy and span-correspondence Sets forward that evidence. Inheritance
  still verifies supporting source/destination data at the copy boundary, then
  retains the successful context-free destination/member checks. Reusing B's
  meaning when A is Set from B no longer immediately loses that data check to
  the Set's own publication.

A's additional type words and constraints remain separate obligations. A
size-only copy cannot certify missing contents. Contextual expressions still
use their bound instance and completion evidence. The retained checks do not
set the complete object's `isNormed` flag. If a Set changes a definition used by
its own certificate, that obsolete meaning is rejected before retention.

This keeps the existing graph, POV storage, and definition semantics. No cube
shape, field name, or test-specific route was added. Later model publications
still conservatively clear instance checks; retaining them through unrelated
later writes needs more precise read/write dependencies. Some compound alias
producers still retain meaning certificates without forwarding data-check
receipts. Copy-boundary source and destination plan construction also remains.

## Real application measurement

Three control/candidate pairs were run sequentially in alternating order, each
in a fresh private Xvfb display and fresh XDG directories. Both versions use
identical generated Slipstream source and assets, including the real Rubix and
Kanban models. The candidate substitutes the changed graph headers. Both carry
identical temporary counters; the candidate additionally counts retained data
bindings. Instrumentation is outside the repository.

| Run | Control visible colors | Changed visible colors | Graph visits, control / changed |
| --- | ---: | ---: | ---: |
| 1 | 24.21 s | 22.79 s | 187,008 / 190,313 |
| 2 | 26.42 s | 22.94 s | 195,305 / 190,040 |
| 3 | 24.84 s | 21.24 s | 187,206 / 187,641 |

Median visible-color time: **24.84 → 22.79 seconds (8.3% reduction)**.
Median first known hue: **6.43 → 6.10 seconds**.

| Work at the visible-color milestone, median | Control | Changed |
| --- | ---: | ---: |
| Physical source-snapshot validations | 82,167 | 41,233 |
| Catalogue rescans | 544,955 | 4,808 |
| Definition-plan constructions | 14,305 | 14,096 |
| Data bindings retained through producing Set | 0 | 140 |
| Graph visits | 187,206 | 190,040 |

Source-snapshot validations fell by **49.8%**, and catalogue rescans by **99.1%**.
The graph visit count is essentially unchanged/slightly higher; this step saves
validation work within the traversal. It does not eliminate general search
reconciliation or claim that all definition applications are unnecessary.

The milestone is the first visibly colored cube, sampled roughly once per
second, not completion of all normalization or subscriptions. Captures from the
candidate runs were inspected: the cube has visible colored faces and Kanban
contains its cards. Animation and sampling make individual times variable.
The regular Slipstream executable and the user's running app were not replaced.

Reproduction artifacts are under `/tmp/p4-cube-chart-bench/`:

- `definition-control/` and `definition-transfer/`: isolated build sources/binaries.
- `definition-final-control-1..3/` and `definition-final-transfer-1..3/`:
  `result.json`, app logs, and private-display captures.
- `run-definitions.py`: benchmark runner.
- `definition-transfer-manifest.json`: source hashes and counter labels.
- `definitions-profile-1/result.json` and `definitions-profile-labels.json`:
  the diagnostic profile preceding these changes.

## Validation

- Regenerated with `codeDog Proteus.Lib.dog`; final header edits rebuilt with
  the generated SCons target.
- **1,309/1,309** checks across all 60 focused graph groups passed.
- **61/61** native graph-runtime scenarios passed, including evidence retained
  after its producing publication and invalidated by a later edit.
- **13/13** stream-insertion checks and **14/14** legacy `parse/` checks passed.
- Ten new definition cases cover shared-source changes, instance isolation,
  late meanings, distinct snapshots, subscriptions from cache hits, catalogue
  ambiguity, scalar/list copies, extra destination requirements, and later
  changes to a copied definition.
- `git diff --check` passed.

Focused test logs and counts are in
`/tmp/p4-cube-chart-bench/final-definition-tests/`. The full older default
language suite was not rerun for this optimization.
