# Read-only positional slices

`items@[start:spec]` selects at a boundary and checks the selected information
against `spec`. Positive `start` counts skipped logical items; unlike `@[n]`,
it is zero-based. For example, `{1 2 3 4}@[1:*2+{...}]` selects `{2 3}`.
A negative start is relative to the known end: `@[-2:*2+{...}]` selects the
last two items. An unknown end remains subscribed.

The expression after `:` is a Proteus specification, not an end index.
A scalar specification selects and checks one member; a fixed-extent list
specification selects that many logical members. Empty windows at valid
boundaries yield `{}`. A proved impossible extent or incompatible specification
yields an empty oneOf; an unavailable position or undecided predicate waits.
Windows are not silently clipped. Only embedded lists flatten into logical
positions; an ordinary nested list occupies one item.

## Implementation

The parser is unchanged. `PathExpression` retains the slice specification in
`GraphPathSelection`. Positional traversal carries the owning query, so both its
coordinate and its specification remain available through identities and oneOf
branches. The existing path goal publishes the result and preserves unresolved
alternatives. A proved-empty branch no longer creates a spurious open tail.

`selectSpanWindow` in `SpanAlignment.h` selects from the shared `ListStructure.h`
layout and its extent equations. It skips known gaps arithmetically and returns
source pieces with offsets and counts. It never creates implicit occurrences.
`SliceSelection.h` prices a lazy edge to manifest those pieces only when the
route is taken. The generic graph scheduler is unchanged.

Concrete pieces use the existing `appendTailMember` identity-backed views.
Their expressions are evaluated at their original source occurrence. Copying an
unresolved `%prev` or `%next` expression into the slice would incorrectly give it
a second context. Ordinary Sets acquire size, value and overlay from the source;
nominal type labels and reusable definition evidence are retained.

Sparse pieces share the source repetition family with an adjusted start ordinal.
Selecting a 900-million-item window from a billion-item repetition creates one
compressed piece and no occurrences. Subsequent indexing reaches the same
canonical occurrence from the slice or its source. Extent-only unknown spans
also remain compressed and symbolic.

A known window shape does not hide source refinements. Selected source pieces
advertise optional normalization routes with their original owner stacks. This
lets an RHS fill an embedded gap, or a relative cursor cross an embedded boundary,
without requiring unrelated parts to normalize before a path continues.

The saved window proof is checked against the live layout and source revisions.
Unchanged pieces retain their POVs and identities when a missing embedded span
becomes concrete. Repeating normalization reuses the result rather than adding
new identities or RHSs.

Predicate matching and RHS representation share `listHeaderOnly`: an empty open
ordered body constrains interpretation and extent without demanding particular
member values. A later RHS can therefore supply the slice width without needing
a slice-specific merge rule. Typed candidates request refinement only when their
facts cannot already be read. Normalizing a path choice also normalizes its
available candidates, without asserting that each alternative equals the choice
root. Path continuation itself still asks only for the predecessor's selection.

## Remaining boundaries

- This increment slices logical lists; substring selection needs text-position
  edges.
- Variable-width/repeating slice specifications whose extent is unknown remain
  symbolic. General grammar-based span capture is separate work.
- A constrained repetition specification still needs a compressed predicate
  proof; it is not silently accepted as a wildcard.
- A window entering the virtual tail of a repetition that already has a physical
  prefix awaits a tail occurrence mapping. Loaded prefix-only windows can use
  the ordinary stored layout.
- Writes, retained-parent semicolons, and constraints after the closing bracket
  remain deferred. Typed ordering and parser-owned unfinished lists retain the
  existing positional-navigation rules.
- The legacy `slice/simpleSugar` fixture expects `@[3:5]` over `{1 2 3 4 ...}`
  to return `5`. The source implementation treats `3` as a skipped-item count
  and `5` as the specification, which contradicts the fourth item (`4`). The
  new path checks that predicate and returns an empty choice. The original
  fixture has not been changed.

## Validation

- Regenerated from `Proteus.Lib.dog`; `Marker: Build Successful`.
- 805/805 focused graph, parser, and numeric checks, including 43 slice API
  cases and two embedded source-context language cases.
- 30/30 generic search scenarios, 220/220 span cases, and 18/18 list-structure
  cases. The span cases include empty boundaries, unknown gaps, overflow-safe
  rejection, and billion-item compressed windows.
- Unchanged original defaults: 172/270 passing, up from 170/270. Recovered
  `slice/select` and `slice/sugar`; no previous pass lost, crashes, or timeouts.
  The remaining 98 fixtures still fail and have not been rewritten.
- Replay checks preserve result/member identities; late width, offset, value,
  extent, and embedded-layout publication resume through the same handle.
  Cancellation prevents resumption. Concrete results set `isNormed` only after
  their members resolve; unresolved sparse results remain symbolic.

Logs: `/tmp/p4-slice-final-build.log`,
`/tmp/p4-slice-final-verified-summary.log`,
`/tmp/p4-slice-verified-focused.json`, and
`/tmp/p4-slice-final-default/results.json`.
