# Positional path selection

Read-only `@[n]` uses one-based positions; `@[-1]` selects the last logical
member. Only embedded lists contribute their members to the containing list's
positions. An ordinary nested list occupies one position and can be indexed by
the next path segment. Mixed paths such as `items@[2].wheel.position` compose
with the existing field-selection rules.

`PathSelection.h` replaces `FieldSelection.h`. `PathExpression` preserves the
parser's distinction between paths inside an index expression and continuation
paths after its closing bracket. Each segment demands its predecessor's result,
without requiring full normalization of every intermediate object.
`GraphPathSelection` retains the expression, source, selector, and result witness.
`ProteusPathGoal` publishes both field and positional results using the same Sets,
including oneOf alternatives, incremental aliases, and replay checks.

A positional segment translates its coordinate to the existing zero-based
`itemAt` demand. Its continuation wraps the navigation edges, preserving their
costs and lazy materialization events. The underlying list traversal and sparse
occurrence registry are unchanged.
Competing routes to one requested position supply one winner, while distinct
language alternatives retain their separate results.

The generic scheduler adds an optional `finishPass()` hook that returns a priced
batch of Sets with its finding's cost and trace. It runs after queued exploration,
before completion metadata. Positional selection uses it to publish its collected
result once; other goals retain immediate publication. This prevents a cheaper
partial result from repeatedly restarting the search and starving a slower
successful route. An excluded or unchanged batch is attempted only once per pass.
The scheduler still knows nothing about paths, indexes, or infons.

End-relative coordinates demand extent evidence; unknown embedded extents become
subgoals. Negative numeric reading copies the value before applying its sign,
avoiding the legacy FlexNum unary operator's mutation of its operand.

Known prefix positions can resolve in an explicitly open list. An end-relative
position waits until the end is known. A pending query resumes when its index,
list extent, or selected value becomes available, and cancellation stops that
resumption. A proved out-of-range position returns an empty oneOf. Zero,
non-integral values, and numbers outside the supported integer range remain
symbolic rather than being truncated or wrapped.

Choices use the existing path policy: visit each available alternative, retain
known results, and preserve unresolved branches in an open oneOf. Sparse access
creates only the demanded occurrence; skipping a billion-member span creates
none of its members. Repeating normalization preserves selected occurrences and
does not add redundant identities.

## Remaining boundaries

- Navigation still requires evidence that a typed list is ordered. An ordered
  definition can supply that evidence; an unknown ordering remains symbolic.
- Parser-owned unfinished lists remain under the existing navigation guard.
  This change supports explicitly open, parsed prefixes; it does not introduce
  speculative indexing while the parser can still reinterpret a list.
- Slices, writes, retained-parent semicolons, and constraints after a closing
  bracket remain deferred. Their syntax is preserved, not silently discarded.
- General signed expressions that require unresolved scalar-complement reasoning
  remain dependent on that reasoning. Literal end-relative indexes are supported.

## Validation

- Rebuilt from `Proteus.Lib.dog`; `Marker: Build Successful`.
- 760/760 focused graph, parser, and numeric checks, including 36 positional cases.
- 30/30 generic scheduler scenarios, including collected-publication replay and
  route-cost exclusion. The slow identity-route fixture previously timed out;
  it now completes, preserves the choice, and remains stable on normalization.
- Unchanged original default suite: 170/270 passing, up from 164/270. Six indexed
  read fixtures recovered; no previous pass lost, crashes, or timeouts.
- The other legacy suite counts retain their previously recorded checkpoints.

Logs: `/tmp/p4-position-accepted-build.log`,
`/tmp/p4-position-final-verified-summary.log`, and
`/tmp/p4-position-final-default/results.json`.
