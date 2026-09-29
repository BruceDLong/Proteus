# Ordinary assignment and forceID

On 2026-09-29 the user clarified: assigning a conflicting known value is a
contradiction unless `forceID` is used. The parser spells that flag `<<=`.
This applies through all selectors and unit representations, independent of
whether the list is temporal. Ordinary `=` can supply an unknown value; it
must not replace a known value merely because the target was reached through
a path or intersection.

## What is implemented

The parser stores `forceID` on the RHS POV, and the printer preserves `<<=`.
Proteus3 consumes the flag in its force-identity merge handler. The graph engine
has no consumer for it yet: `merge/ident/force`, whose input is `123<<=456` and
expected result is `456`, remains a failing implementation test. This change
does not implement replacement or weaken ordinary equality checks.

The old language-test helper checked printed text but did not inspect the
normalization handle's failure status. It now recognizes `<REJECTED>` in the
expected-output field or a `//:<REJECTED>` line assertion. That assertion waits
for normalization and requires `handle.failed()`. A following line can inspect
the same world to check that values were preserved. Compatible and unresolved
inputs do not satisfy the rejection assertion.

## Corrected fixtures

Nine existing sparse/unit fixtures used ordinary `=` but expected replacement
of known members. Their write expressions are unchanged. They now require a
reported rejection followed by a read asserting the original source values.
The sparse crossing case checks the entire original compressed parent, including
both known markers and the untouched adjacent gaps. The existing test names are
retained so the checkpoint history can still be compared.

Five already report rejection and pass the corrected assertions:

- `unit/nestedDay/writeMinutesAcrossHourNegativeSpan`
- `unit/world/dayFromSeconds/writeNestedSecond`
- `unit/world/dayFromSeconds/writeNestedLastSecondNegative`
- `unit/world/dayFromSeconds/writeLastMinuteNegativeSpan`
- `unit/world/dayFromSeconds/writeTwoMinutesNegativeSpan`

Four retain the source values but fail to report the contradiction:

- `unit/nestedDay/writeMinutesAcrossHourNegativeIntersection`
- `unit/world/dayFromSeconds/writeLastMinuteNegativeIntersection`
- `unit/world/dayFromSeconds/writeTwoMinutesNegativeIntersection`
- `sparse/pending/writeSpanAcrossSparseConcreteSparse`

These are implementation failures, not unresolved language policy. They remain
failing; unchanged printing alone does not qualify them as passing.

## Small reproducer and next implementation boundary

`[<_>] <~ {8} = 80` currently prints `80` with `failed=false` and
`symbolicValues=true`. The source correspondence remains unresolved after the
result acquires the conflicting RHS. The same behavior occurs for an untyped
fixed span, `[<&*2+{...}>] <~ {8 9} = {80 90}`, so it does not depend on unit
names or temporal structure.

The next engine change must preserve the dependency between a selected result
and its input correspondence while checking the result's ordinary constraints.
A mismatch on an exploratory grammar path must still abandon that path; this
must not become a blanket rejection of every unsuccessful parse route. Explicit
`forceID` should then be implemented as its own semantic operation with deliberate
source-update and identity behavior, rather than bypassing ordinary comparisons.

## Validation

The regenerated test target built successfully. The nine exact write-expression
probes reported five rejections and four missing rejections. Harness controls
confirmed that `8=80` satisfies `<REJECTED>`, while `8=8` and `_` correctly fail
that assertion. The corrected sparse audit is 20 passes / 3 failures. The time
suite is 28 passes / 7 failures / 1 five-second timeout; the timeout is the same
nested read documented at the preceding checkpoint.

The ordinary full command now reports **40/1638 failures/tests**, down from
45/1638 through five corrected expectations, with no engine behavior changes
and no newly failing fixture names. Focused path-write, rejection, and unit
regressions pass **67/67** (29 + 10 + 28). The `parser/ident/force` check passes;
`merge/ident/force` still fails with `123 <<= 456` instead of `456`.

Commands:

```sh
codeDog Proteus.Lib.dog
# From LocalBuild:
./TestProteus
./TestProteus -t unit/graphPathWrite/
./TestProteus -t unit/graphRejection/
./TestProteus -t unit/graphOrderedUnits/
./TestProteus -t ../defaultTests.tst:parser/ident/force
./TestProteus -t ../defaultTests.tst:merge/ident/force
```

The current bounded language audit is in
[language-compatibility-status.json](language-compatibility-status.json).
[sparse-unit-status.json](sparse-unit-status.json) retains the earlier `646672a`
snapshot, before these expectation corrections.
