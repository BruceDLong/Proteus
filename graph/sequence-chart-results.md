# Shared whole-sequence correspondence

This step addresses matching one list template against one supplied sequence.
Parallel constraint intersections are intentionally a later step. Input may be a
loaded ordered list or text; both use the existing semantic transition rules.

## Change

Previously the visited-state key included every enclosing grammar frame. A
pattern at one input position was recognized again through each enclosing
alternative. Deduplicating those full-stack queries did not share the work.

`SequenceChart.h` separates three things:

- A call identifies a pattern POV, source and input cursor. Root and member
  matching remain distinct; member calls omit their callers' grammar stacks.
- A local state retains that call's current grammar position, list count and
  input cursor. A waiter records where a completed child should return. New
  waiters consume already known results, and new results wake existing waiters.
- Packed proof links retain alternative predecessors and child results. Shared
  recognition does not equate differently shaped output derivations. Only when
  publishing a result do we reconstruct a complete proof for the existing
  `ProteusDefinitionPlan` compiler.

The chart invokes `SequenceCorrespondence` for semantic transitions and return
rules. It does not implement another scalar comparator, span walker, merger, or
copy of the infon graph. Each chart transition still goes through the graph
scheduler's cost queue. Missing information, subscriptions, cancellation and
priced Sets retain the existing lifecycle. Mismatch contributes no continuation.
A first complete, publishable proof can produce its Set immediately; if its
plan is incompatible, another retained proof can be considered. `finishPass`
also checks proofs added to an already visited result by later converging routes.

`GoalSpec::expand` permits this per-goal traversal state without putting Proteus
semantics in `GraphSearch`. Ordinary goals still use the graph adapter. Chart
goals do not copy the full route into every frontier entry. They retain their
compact proof links instead. Proof reconstruction allows the same completed
subtree to be used several times, including finite zero-width repetition.

## Measurements

Counts below include normalization/subgoal work, not just recognition, with
normal notifications enabled. These are operation counts, not timing claims.
The before build uses the parent implementation and the same generated test
program. No fixture-specific matching rules or changed expectations are used.

Shared nested alternatives:

```
#p0='B'
#p1=*_+[{p0} p0]
#p2=*_+[{p1} p1]
...
{pN 'X'} == {'B' 'Y'}
```

Each level adds two grammar references and doubles possible derivations. The
last member mismatches, so finding an early successful parse cannot mask work.

| Depth | Old steps | Chart steps | Old scalar matches | Chart scalar matches |
| ---: | ---: | ---: | ---: | ---: |
| 2 | 236 | 266 | 7 | 5 |
| 4 | 677 | 600 | 31 | 9 |
| 8 | 6,897 | 1,674 | 511 | 17 |

A separate case uses no word definitions:

```
{{{{'A'|...}|...}|...} 'X'} == {'A' ... 'A' 'Y'}
```

| Number of A items | Old steps | Chart steps |
| ---: | ---: | ---: |
| 2 | 200 | 181 |
| 4 | 623 | 355 |
| 8 | 5,133 | 1,139 |
| 16 | 87,161 | 5,571 |

A successful list case is
`{&{*_+['A' {'A'}]|...} 'X'} == {'A' 'A' 'X'}`. It produces
`{&{'A' 'A'} 'X'}`. The chart retains all four derivations of its two ambiguous
occurrences before one is published. Source/template contents remain unchanged.

## Validation

- Regenerated with `codeDog Proteus.Lib.dog`.
- Seven `unit/graphSequenceChart/` regressions passed: nested repetition/choice work
  growth, successful list publication and re-normalization, late input,
  cancellation, cost exclusion, and all four packed derivations.
- All 1,298 graph checks passed (1,291 existing plus seven new); focused sequence coverage includes text,
  grammar choices, inner selection and repeated-prefix publication.
- Older `defaultTests.tst:parse/` language tests: 14/14 passed, including
  `parse/lang` and list grouping.
- Generic graph runtime: 59/59 scenarios passed.
- The complete legacy/world suite was not rerun for this checkpoint.

## Boundaries

- Repeated `<~` prefix publication keeps its existing common-route algorithm.
  Its ambiguity/streaming behavior has not been migrated to packed proofs.
- A semantic publication still invalidates/rebuilds the affected chart pass.
  This change shares work within a pass; persistent incremental charts remain
  future work.
- Packed proofs are internal recognition evidence, not yet an intermediate
  semantic `oneOf` that other connected intersections can constrain.
- Sharing recognition does not make arbitrary semantic constraints polynomial.
  Trying incompatible output proofs can still be expensive. Definition/type
  compilation has its own costs: deep shared-definition probes still spend
  substantial time outside the reduced chart transition count.
- A pre-existing typed-list completion gap remains. For example, after declaring
  `#p0='B'`, `{p0 'X'} == {'B' 'X'}` fills the result but retains its RHS and
  subscription. The same behavior was reproduced before this change; this
  checkpoint does not claim to fix it. The successful untyped list regression
  checks full completion, not just populated fields.
