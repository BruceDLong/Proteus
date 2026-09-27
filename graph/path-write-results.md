# Path constraints and retained parents

Dot, item, and fixed-extent slice paths now apply their following RHSs to a
uniquely selected source occurrence. They use the existing constraint/Set
machinery: a contradictory known value fails instead of being overwritten.
An ambiguous or unresolved selection cannot write to an arbitrary candidate.
Repeated normalization reuses the target and does not append the same RHS again.

The parser already distinguishes a returning `partRef` from retained `parts`.
`PathExpression` now lowers that tree into the same selection queries used by
reads. RHSs following a segment belong to that segment's selected occurrence;
for `@[index]`, paths and constraints inside the brackets still belong to the
index expression. Required retained paths and preceding writes are ordinary
subgoals. The generic search manager is unchanged.

For example, after `.wheel.spoke.length=*10+7`, no semicolon returns `length`,
`;` returns `spoke`, and `;;` returns `wheel`. A returned parent's child paths
must complete before its normalization can settle. Its outer view owns those
obligations separately from the parent's selection query, avoiding an ownership
cycle between the parent and its retained children.

Published queries preserve the selected occurrence's original owner stack.
Relative RHSs such as `%prev` and `%next` are evaluated there, including across
embedded boundaries. Copied headers do not cause those source-owned RHSs or
list members to be normalized again under the query's location. Their source
link remains available to graph traversal and supplies subsequent refinements.

Bare `.first` and `.last` on ordered sources use the shared positional route.
On unordered sources those words retain nominal field-selection semantics.
Sparse writes use the registry's canonical occurrence; targeting item 100 in a
billion-item repetition does not expand its physical list or affect item 101.

## Boundaries

- Ambiguous writes, absent targets, and targets behind unresolved earlier
  candidates remain symbolic. No missing member is constructed by path writing.
- Unknown end-relative positions wait; a later end publication resumes the same
  handle. Cancellation prevents subsequent application.
- Ordered list correspondence still requires ordering evidence. For example,
  `{part:{_ _}}.part={*10+7 *10+8};` remains symbolic when `part` has no declaration
  proving order. Selecting and updating a scalar inside such an object works.
- Variable-width slice capture, text slicing, and general unordered structural
  correspondence retain their existing boundaries.

## Validation

- Regenerated from `Proteus.Lib.dog`; `Marker: Build Successful`.
- 850/850 focused graph, parser, and numeric checks, including 29 path-write
  cases and four relative-write language compositions.
- Coverage includes conflicting known values, leaf/parent/root returns,
  sibling writes, embedded and sparse sources, fixed slices, unresolved earlier
  candidates, late end publication, cancellation, duplicate-RHS replay, and
  release of retained queries after shutdown. The lifetime regression was
  reproduced before separating the returned view from its selection query.
- Unchanged original defaults: 186/270 passing, up from 177 at the end-origin
  checkpoint and 172 before these three increments. All seven `write/` fixtures
  and `read/first`/`read/last` recovered. No previous pass was lost; there were
  no crashes or timeouts. The remaining 84 fixtures still fail.
- The generic search, span, and list-structure code is unchanged from the
  end-origin checkpoint, where its 30/220/18 cases passed.

Logs: `/tmp/p4-write-final-build.log`, `/tmp/p4-write-verified-summary.log`,
`/tmp/p4-write-verified-focused.json`, and
`/tmp/p4-write-final-default/results.json`.
