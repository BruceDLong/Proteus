# Completed repetition prefixes

A repeated `<~` selection publishes its completed occurrences even when the
last attempted occurrence lacks information:

```proteus
{[_ _]|...} <~ {1 2 3}
// {[_ _]| 2 ... } == {1 2 3}
```

The selected `2` is available, the output size remains unknown, and `isNormed`
stays false. The template and source relationship remain visible and the handle
stays subscribed. Adding `4` to the source completes the correspondence as
`{2 4}`, reusing the first result POV. Cancellation preserves the published
prefix and stops subsequent extension.

A known source end does not invent the missing selected member or close this
unfinished result. This also applies to the old `range/select1` example: it
publishes `2 3 4 5`, but the trailing excluded `6 7` leaves an unresolved
repetition. Its expected text now includes that tail, pattern, and source
relationship, following the user's explicit decision. The input is unchanged.
Adding a later in-range value completes the pending repetition.

## Shared implementation

`SequenceCorrespondence` already exposes the boundary after each accepted
repetition. The goal collects those findings and uses the existing `finishPass`
hook after complete routes have had their opportunity. Among reached prefix
paths, it publishes only a completed boundary shared by the competing
continued derivations. Different complete parses retain the existing first
successful parse policy. This is not a general ambiguity chart or a guarantee
that arbitrary future constraints cannot invalidate a result.

The existing result compiler builds an open root requirement for that boundary.
`ProteusDefinitionPlan` and `SpanAlignment` install its proved members without
inferring the final root size. The finding retains its route cost and the Set
retains its planning/allocation cost. The generic scheduler is unchanged.

The result prefix remains visible to shared span access even though its
`listSpec` still describes the next input match. Consequently, other RHS list
constraints can align with published results, including detecting conflicting
member values. An explicit result size remains a bound; reaching it cannot
silently discard unmatched source input. Publication does not mark the feed or
template represented until the whole correspondence is proved.

The source and template are read-only during normalization. Replay reuses
result members, source identities, and the same template. The implementation
replays the correspondence on a source revision; it does not yet checkpoint
an incremental parsing chart.

## Boundaries

- This increment admits loaded finite list/string sources with an unfinished
  final repetition and later source revisions. Parser-owned/open-tail input
  remains deferred: its pending cursor must be modeled before it can be
  admitted without falsely declaring the source finished.
- Ordinary whole-list `==` matching remains atomic. It does not acquire this
  repeated-selection prefix policy.
- A conflicting assertion may be discovered after prefix publication; the
  handle then reports failure and retains its constraints. A partial result is
  not proof that every enclosing obligation has been satisfied.
- RHS removal, general parse retraction, recursive grammars, and arbitrary
  provisional output to external stream consumers remain separate work.

## Validation

- Full CodeDog source build and final native header build passed.
- **984/984** focused graph/parser checks pass, including 11 new prefix lifecycle
  checks and strengthened existing incomplete/rejected-repeat assertions.
- All six older language suites were rerun: **218/363** pass. Defaults are
  **203/270**, including the revised `range/select1`; no previous pass was lost.
  The other five suite counts are unchanged. No crashes, timeouts, or incomplete
  test summaries occurred.
- Registered legacy helper/parser-error checks remain **102/125**, unchanged.
  Those tests include direct assertions about legacy internals.

The new cases cover later list/text input, a later unknown value, continuation
of range filtering, cancellation, replay, template/source preservation, cost
limits, open-input deferral, competing partial routes, their common prefix,
and compatible/conflicting enclosing RHSs. Existing finite-output-count and
ordinary whole-text atomicity tests also pass.

Logs: `/tmp/p4-prefix-final-build.log`, `/tmp/p4-prefix-native-build.log`,
`/tmp/p4-prefix-verified-focused.log`, and `/tmp/p4-prefix-audit-*`.

```sh
codeDog Proteus.Lib.dog
cd LocalBuild
./TestProteus -t unit/graphRepeatPrefix/
./TestProteus -t unit/graphInner/
./TestProteus -t unit/graphNumericRange/
./TestProteus -t ../defaultTests.tst:range/select1
```
