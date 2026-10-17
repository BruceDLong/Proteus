# Unordered definitions and supplied record correspondence

Later clarification: explicit normalization fills the requested item's immediate
parts; incidental traversal stays lazy. See
[normalization-parts-results.md](normalization-parts-results.md). The historical
partial-normalization observations below predate that distinction.

Unordered definition bodies now participate in the existing read-only type
compiler. A bare reference can create independent member POVs from its declared
parts. A supplied partial description, including one used inside a definition,
keeps its stated parts without importing every part of its parent type.

```proteus
#record={X:*10+1, Y:*10+2}
record                            // record:{X:*10+1, Y:*10+2}
#alias=record:{X:*10+1}
alias                             // alias:{X:*10+1}
```

The unbounded record still has unknown total size. Explicit size is a bound on
all parts; it is never inferred from the number of described unordered fields.
Full normalization still requires known extent and normalized members.

Unordered conjunctions and independently supplied record RHSs share
`MemberCorrespondence`: existing identities take precedence; otherwise matching
uses a unique nominal type correspondence. This does not make type words unique
field names. Competing matches, unnamed descriptions, and insufficient evidence
stay pending. Aliases follow the existing primary/added-type rule; unrelated
same-shaped fields and sibling aliases remain distinct.

```proteus
{X:*10+_, Y:*10+2}={Y:*10+2, X:*10+1}
// {X:*10+1, Y:*10+2}, with corresponding member identities
```

Matched fields are checked by `ProteusDefinitionPlan` before the Set executes.
A contradiction in any matched field or in the total extent rejects the batch
before it publishes values, new members, or identities. Ordinary graph traversal
then follows the published identities, including later information. Missing
parts explicitly supplied by an RHS may be added; omitted definition parts are
not inserted into an already supplied partial description.

`RecordStructure` uses the same identity matching primitive to prove that a
partial RHS is represented, allowing unrelated target parts to remain. Replay
reuses member POVs and identities. Definitions stay read-only, and type expansion
adds no definition-application RHSs. Matching/composition work contributes to the
existing Set cost. There is no new graph node type or scheduler algorithm.

A three-state order requirement distinguishes ordered, unordered, and
unspecified order. This preserves the existing behavior of local member
templates, which need not change their containing list's order.

## Subsequent descriptive-selection work (2026-10-03)

Finite repeated type words in a newly instantiated record now create separate
parts. In an existing record, distinguishing supplied properties can prove a
unique correspondence through `MemberCorrespondence`; unknown or competing
matches still defer. No permutation search was added. An unordered record's
implied omitted parts no longer make `typedDescription` treat it as an explicitly
unfinished positional stream. Incidental nested descriptions remain lazy.

The historical boundaries and counts below describe the earlier increment.

## Boundaries

This increment does not supply general unordered structural predicates, resolve
duplicate-type ambiguity using distinguishing properties, or implement prefixed
virtual definition spans. Mixed ordered/unordered definition conjunctions stay
pending. A bare unordered definition containing ambiguous duplicate type
requirements also stays pending until its member correspondence can be retained
and rechecked without guessing.

The older color fixture still needs composition between an unordered parent
meaning and an ordered partial typed description, together with correct handling
of the parent's still-useful definition after RHS adoption. Rubix additionally
needs prefixed/embedded definition spans. These fixtures remain failures; no
old language expectation has been changed. The graph's optional-definition rule
must not be weakened merely to fill every omitted part.

## Validation

- Full CodeDog source regeneration/build passed.
- All **20 new checks** pass: bare/bounded/nested bodies, partial aliases,
  permuted conjunctions and fields, additional parts, type aliases, explicit
  extent and value conflicts, ambiguity, cost filtering, late meaning,
  cancellation, source immutability, and replay.
- The existing independent-fields test now verifies identities to the correctly
  permuted members; its former pending-only assertion was obsolete. No old
  language fixture, expectation, or skip was changed.
- Compiled audit: **1,226 pass / 8 existing fail**, including **1,176/1,176**
  graph tests. Every other pre-existing compiled status is unchanged.
- Six older language suites: **265 pass / 88 fail / 8 documented skips**.
  Defaults: **247 pass / 13 fail / 8 skips**. Every fixture status is unchanged.
- Standalone streaming build passed; supported selections 1, 2, 4, 5, 6,
  and 8 pass **33/33 checks**. Selections 3 and 7 remain outside migrated coverage.
- `git diff --check` passed. No new sanitizer or GUI integration pass is claimed.

Per-fixture logs are in `/tmp/p4-record-final`; source build logs are
`/tmp/p4-record-finalbuild.log` and `/tmp/p4-record-streambuild.log`.

