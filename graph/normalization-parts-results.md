# Explicit normalization and lazy part access

The user clarified the demand boundary: normalizing an item should fill it in
at least one level; otherwise its parts should be filled lazily.

The public normalization entry point now requests immediate definition parts.
The Proteus goal carries that request through graph revisions and into a
selected result, so adopting a cheaper partial RHS cannot silently weaken it.
Related RHSs, lookup sources, and already supplied nested descriptions remain
lazy. The generic graph scheduler is unchanged. Reusable meaning evidence records
which declared types were actually traversed. A later explicit request for a
previously lazy child therefore cannot mistake supplied-part evidence for its
whole definition; stronger demands compile any missing meaning.

An ordered partial typed description matches its existing fields nominally to
the definition and inserts missing fields around the original POVs. An unordered
description uses the same correspondence without positional constraints. All
member checks are planned before any field is added. Duplicate/uncertain matches
stay pending; contradictory facts or an insufficient explicit extent reject the
plan. A counted size is revisable, while an explicit size is not.

Unspecified list order is independently inferred from declared type headers,
including aliases. This is a priced Set over existing declaration POVs, with no
shared definition mutation. A comma is explicit unordered syntax; spaces leave
order inferable. Conflicting explicit orders are not silently converted. Meaning
certificates include the inferred order, so later header changes invalidate
stale compiled requirements.

The implementation retains existing compiler boundaries: ambiguous meanings,
context-dependent definition bodies that cannot yet be compiled, and unresolved
virtual structures can still leave the requested expansion symbolic. No arbitrary
member is invented to resolve an ambiguous match. Unordered expansion does not
assert that a type definition enumerates all possible parts.

## Validation

- Full CodeDog regeneration/build passed; final header changes also compiled.
- All **30 new tests** pass: inferred order, aliases, cache invalidation,
  explicit/lazy demand, nested descriptions, later child normalization, member
  identity, insertion order, explicit bounds, rejection, replay, and late meaning.
- All **1,206 graph tests** pass. Full compiled audit: **1,256 pass / 8 existing
  failures**; no pre-existing compiled status changed.
- Six language suites: **266 pass / 87 fail / 8 documented skips**. Defaults:
  **248 pass / 12 fail / 8 skips**. No new crashes, timeouts, or incomplete runs.
- Standalone streaming source build passed; supported selections 1, 2, 4, 5,
  6, and 8 pass **33/33 checks**. No sanitizer or GUI integration pass is claimed.
- Two legacy expectations changed explicitly: `query/chain3` now contains the
  omitted color of the returned spoke; `word/setColor` prints all three fields
  with the unknown numeric sizes supplied by their declarations. No other old
  fixture expectation or ambiguity skip changed.
- The partial-description and supplied-record unit assertions now distinguish
  direct normalization from incidental traversal. The rejected-projection test
  still tests an exploratory path through a lazily accessed container.

Audit logs: `/tmp/p4-parts-final`. Source build: `/tmp/p4-parts-finalbuild.log`;
final header build: `/tmp/p4-parts-coverage-build.log`. Streaming build:
`/tmp/p4-parts-streambuild.log`.

