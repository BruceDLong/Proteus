# Qualified scalar string relationships

`=:` and `==:` now supply scalar string prefix evidence through the graph's
existing scalar-relation goal. `NumericRelation` became `ScalarRelation`; it
keeps numeric range matching and adds the string endpoint rule. Sequence/list
matching still uses its existing successful-continuation walk.

| Expression | Result | Completion |
|---|---|---|
| `$ =: 'Hello'` | `*_+'Hello'` | Size remains unknown and subscribed |
| `? ==: 'Hello'` | `*_+'Hello'` | Overlay and value are known; size remains unknown |
| `*3+$ =: 'Hello'` | `'Hel'` | Known width selects the prefix |
| `'Hello' =: 'Hel'` | `'Hello'` | Existing compatible literal is preserved |
| `$ == 'Hello'` | `'Hello'` | Whole matching also supplies size |
| `*4+$ == 'Hello'` | Unchanged | Ends do not match; no successful continuation |

The missing behavior was reproduced against Proteus3's `mergeLooseStrings`
and `shouldCopySize` rules in `Reasoner.dog`. All seven examples in
[qualified-text-tests.tst](qualified-text-tests.tst) passed on the reference
executable and failed before this change in Proteus4. They now pass in Proteus4.

## Implementation

`ScalarPredicate.h::matchText` makes a read-only, finite comparison. Qualified
endpoints are separate from payload facts: the facts reader can explicitly
inspect a relationship payload without treating the relationship as equality.
The ordinary predicate and definition readers keep their previous behavior.

An accepted relation supplies one priced Set through `ScalarRelation.h`. It
uses the existing field-copy primitive for overlay, value, and any proven size.
Copied text uses counted size only when the width is known; otherwise its
explicit unknown size remains visible. Whole matching checks the available
text before copying, so it cannot publish a prefix that its own next pass
would reject as incomplete. No recursive normalization or shadow
graph is introduced. The graph's existing revision/subscription mechanism
reconsiders unresolved matching and late size evidence.

Qualified work remains searchable. The successful comparison does not record
whole identity, remove the relationship, overwrite a literal target, or mutate
the source. A mismatch stops that match and leaves the RHS available elsewhere.
Replay reuses the existing state rather than adding redundant work or identities.

## Validation

- Full CodeDog generation and C++ build passed.
- **1,025/1,025** focused graph/parser checks passed, including nine new
  lifecycle checks and seven reference language cases. The endpoint check
  covers 48 combinations of relationship mode, target shape, and known/unknown
  source size. Whole correspondence never publishes a truncated value while
  its source endpoint remains unresolved.
- Lifecycle checks cover unknown size, late text, late size, cancellation,
  cost exclusion, rejected-source reuse, short input, nonintegral width,
  source immutability, completion flags, and replay.
- All six older language suites were audited: **220/363** pass, including
  **205/270** defaults. Exactly `merge/str/looseSize` and
  `merge/unknown/looseSize` changed from fail to pass. No prior passes were lost,
  and no crashes, timeouts, or incomplete summaries occurred.
- Registered legacy helpers remain **102/125**. Existing test expectations
  were not changed. The graph scheduler was unchanged; its earlier sanitizer
  results are not a full-engine sanitizer claim for this increment.

Logs: `/tmp/p4-qualified-final-build.log`, `/tmp/p4-qualified-focused.log`, and
`/tmp/p4-qualified-audit-*`. Exact older fixture statuses remain in
[language-compatibility-status.json](language-compatibility-status.json).

## Boundaries

This scalar plan accepts plain, context-free payloads. It does not silently skip
source work or type obligations; those retain the existing expression/sequence
routes and their limitations. In particular, qualified scalar prefixes whose
RHS still requires ordinary work or contextual/type resolution can remain
pending. Unknown text, insufficient bytes for a requested width, and unsupported
widths also remain symbolic. Parser-owned open text and general streaming cursor
publication are separate work. Cost continues to choose admissible routes;
this change adds no new cost model.
