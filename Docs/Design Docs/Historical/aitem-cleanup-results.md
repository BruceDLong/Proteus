# Remove the AItem engine

Ordinary and streaming normalization already ran through `Agent.normalize()` and
its persistent graph context. This cleanup removes the inactive second engine.
`GraphSearch.h`, graph goals, cost selection and language semantics are unchanged.

## Removed

- `AItem`, `Agenda`, propagation records, transient intersection traversal plans,
  dispatch/lifecycle records, and their factory and completion bookkeeping.
- `AgendaEngine.dog`, `AItemLifecycle.dog`, `Reasoner.dog`, `WorldManager.dog`,
  `WorldManagerRules.dog`, `Diagnostics.dog`, and `debugSystems.dog`.
- `ruleMgr.py`, which generated the deleted AItem merge-dispatch matrix.
- The Agent's old WorldManager member and unused word-dereference convenience
  methods whose only callers belonged to the old engine.
- The old deferred-work side list, its lifecycle helpers and unused completion
  flags. The semantic `wrkList`, identity registry, and graph consolidation are
  preserved; graph removal of a consolidated RHS uses the same work-list API.
- Old iterator condition-variable waits and their application setup flag. The
  parser/graph publication mechanism already owns streaming waits, subscriptions
  and resumption. Loaded-list traversal helpers remain available.
- The printer's never-set old `wasProcessed` shortcut. A template is hidden only
  when its current graph representation proof permits it.

No helper implementation needed moving out of the deleted files: the shared
POV, infonView, identity-registry, vocabulary and parser primitives already live
outside them. Existing list/template, sparse occurrence, endpoint, registry,
parser and graph tests continue to exercise those primitives.

## Coverage accounting

All 1,091 graph registrations and all six language fixture files are unchanged.
67 registered tests that directly call the deleted engine are retired, along
with one unregistered old test declaration. Mixed test functions retain their
independent cases; for example, the end-relative extent checks and the
language-level typed-selector comparison remain. The 58 remaining helper tests
are retained even when their existing expectations fail.

`retired-aitem-tests.json` records the removed names and their previous results:
52 previously passed and 15 failed. Removing these internal assertions is not
counted as recovering language behavior. Their implementation remains available
in local Git history at `f92d04a`.

## Validation

- `codeDog Proteus.Lib.dog`: passed. The generated C++ contains no AItem,
  AgendaEngine, WorldManager, or InfonViewReasoner declarations.
- Compiled audit: **1,141 pass / 8 fail**, out of 1,149. All **1,091 graph
  tests pass**; retained helpers are **50 pass / 8 existing fail**. Every
  retained name has exactly its previous status.
- Six older language suites: **261 pass / 92 existing fail / 8 documented
  ambiguity skips**, out of 361. Every individual status is unchanged. Default
  tests remain **243 pass / 17 fail / 8 skips**. No fixture or expectation changed.
- `codeDog streamReplyTest.dog`: passed. Supported streaming selections
  `-f 1`, `2`, `4`, `5`, `6`, and `8`: **33/33 checks passed**, including
  replies before list closure and a split reply token. Cases 3 and 7 remain
  outside this migrated streaming coverage.
- No process errors, timeouts, or incomplete summaries in these audits.
  `git diff --check` passed.

The generic graph scheduler and semantic graph headers were unchanged; their
standalone/sanitizer suites were not rerun for this deletion. There was no GUI
integration pass.

Updated inventories are in `legacy-helper-status.json` and
`language-compatibility-status.json`. Raw logs are in
`/tmp/p4-aitem-compiled-final`, `/tmp/p4-aitem-language-final`, and
`/tmp/p4-aitem-stream-*.log`. The retirement ledger refers to the pre-cleanup
compiled audit at `/tmp/p4-null-compiled-final`.

The `.begin` typed-field naming collision remains a deferred design question.
No boundary naming or S-list/T-list semantic changes are part of this cleanup.
