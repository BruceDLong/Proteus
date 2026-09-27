# Numeric ranges, 2026-09-26

Numeric range evidence is shared by direct loose correspondence, strict interval
intersection, field predicates, and sequence predicates. The generic graph
scheduler is unchanged. The older `align2a` and `align2b` assertions now use the
approved shorter printing; their inputs are unchanged.

`NumericRange.h` implements exact half-open intervals and singleton values using
GMP rationals at the Proteus boundary. Unknown evidence, an unbounded domain, and
an empty intersection are distinct. `NumericRangeAccess.h` reads existing scalar
and additive-concat structure; it does not create a second semantic graph.
`NumericRelation.h` turns accepted evidence into priced Sets.
`ScalarPredicate.h` separates three-way predicate containment from relationship
matching. Relationships also check the endpoint mode before supplying a value.

Examples:

- `*4+_ ==3` accepts 3; `*4+_ ==4` rejects the endpoint.
- `(*3+_ +2)` denotes `[2,5)`. In `*10+(*3+_ +2)`, the outer size 10
  remains separate from the interval width 3.
- `*10+(*5+_ +2) = *10+(*4+_ +5)` becomes `*10+(*2+_ +5)`.
- Positive and negated field/sequence predicates use those same bounds.
  Partial overlap is unknown rather than proof of exclusion.

`=` and `==` require equal known endpoints at the current pair; `=:` and
`==:` permit unequal endpoints. `:` affects that pair, while `==` propagates
loose matching into members. Thus `*3+_ = *4+_` conflicts and `*3+_ == *4+_`
cannot match; either colon form permits the partial correspondence.

Accepted partial numeric matches preserve the target size. A bare unknown RHS
adds no value; an explicit additive range supplies its symbolic value bounds.
This explains the original `range/select3` output: the partial input `*3+_`
does not replace the template's size 4. Symbolic output uses the same requirement
planner as ordinary accepted sequence output. Partial correspondence does not
record a whole identity between differently sized infons.

Accepted point matches use the source's size-printing mode, as in Proteus3.
`*4+_ ==:*10+3` prints `*4+3`, while `_ ==*10+3` acquires size 10 and prints
`*10+3`. Without the colon, known unequal ends prevent that match. General
wildcards remain general, and compiled type meanings are unchanged.
Numeric range type-definition expansion is not included in this increment.

The original expression remains in `valueExpression`, and all RHS work remains
searchable. Representation proofs suppress redundant printing without deleting
constraints. A later intersection can narrow the result again. Shared patterns
and source expressions are read-only. Sized symbolic sequence inputs can publish
results; a wholly unknown input still waits. Later source values refine existing
output members without merging unequal sizes. Unknown operands and symbolic ranges keep
subscriptions; cancellation leaves their last published result in place.

## Source rules and number modes

The reference rules are `DirectConsolidationStrategy.evalForRange`,
`checkNumRange`, `mergeANDRanges`, and `ConcatReasoner.joinOnRight` in
`Reasoner.dog`. This migration uses their containment/addition/intersection
semantics, not their agenda control flow.

The current size modes are `fromCount`, `fromContext`, and `fromGiven`.
`fromContext` distinguishes an additive offset from radix concatenation.
`WorldManager.rangeRules` canonicalizes fractions and converts a known-size
inverted value to `size - value`. No modulo-to-zero mode was found. Both the
existing Proteus executable and Proteus4 leave `*4+4` as `*4+4`.

## Remaining boundaries

- `range/select1` ends with excluded values 6 and 7. The last attempted repeated
  intersection consumes that prefix but lacks its selected final member.
  Proteus3 retains earlier completed repetitions; current graph correspondence
  requires a complete route. General partial-parse publication needs to address
  this, rather than adding a special case to range matching. Offset filtering
  with a complete final repetition is tested and works.
- `merge/directRange/endReject` rejects correctly but keeps the RHS visible.
  The old expected text assumes rejected work is removed. The existing graph
  conflict/diagnostic policy is unchanged; see `numeric-literal-results.md`.
- Contextual/relational range operands, arbitrary radix range algebra, and
  symbolic union/difference output remain deferred. Missing evidence never
  counts as a proved mismatch.

## Verification

- Full CodeDog source regeneration and C++ build passed.
- Focused graph/parser checks: **973/973**, including the existing 934,
  20 numeric-range language cases, and 19 range lifecycle checks. The endpoint
  check exercises all four operators in both size directions, with known and
  unknown values (16 combinations).
- The preceding numeric-range checkpoint verified **1,225 interval pairs**, plus
  boundary/singleton checks; that unchanged arithmetic was not rerun here.
- Older defaults: **202/270**, up from 201; no previous pass lost, crashes,
  timeouts, or incomplete summaries. `range/select3` now passes with its original expectation. Older default
  inputs and expectations were unchanged in this follow-up.
- Other legacy suites, GUI integration, and sanitizers were not rerun.

Logs: `/tmp/p4-end-modes-verified-build.log`,
`/tmp/p4-end-modes-verified-focused.log`, and `/tmp/p4-end-modes-verified-audit/`. Run:

```sh
codeDog Proteus.Lib.dog
cd LocalBuild
./TestProteus -t unit/graphNumericRange/
./TestProteus -t ../graph/numeric-range-tests.tst:
```

The standalone arithmetic check compares 1,225 interval pairs against point
membership, and checks exact fractional/large-number boundaries:

```sh
g++ -std=c++17 -O2 graph/test_numeric_range.cpp -lgmpxx -lgmp -o /tmp/proteus4-range-test
/tmp/proteus4-range-test
```
