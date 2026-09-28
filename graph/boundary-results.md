# Nulls and identity-bearing boundaries

`NULL:` parses and prints a known zero-size, zero-value state whose value overlay
is `tUnknown`. `NULL:647` additionally names that occurrence. Bare `NULL` stays
a word. Parsing `?` explicitly assigns `tUnknown` while leaving size/value
unresolved. IDs are positive int64 values; zero, negatives and overflow are
rejected. Null state is recognized from the existing fields, without a new flag.

Each Agent registry owns an identity scope. Repeated IDs in that scope reference
the same occurrence. Standalone parsers own separate scopes; binding their
results into an Agent consistently remaps imported tokens. Core's
`parseAndExtractString` and `parseAndExtractStream` share the Agent scope by
default; their optional `independentIdentities=true` argument requests remapping
for an independent input. Reference/view copies retain tokens; fresh type copies
do not inherit an occurrence's name. Numeric IDs from separate scopes alone
never prove identity. Allocation does not wrap at the int64 limit.

## Graph contribution

`GraphNormalizer.findBoundary(source, end)` returns the ordinary cancellable
normalization handle. `.begin`/`.end` use the same boundary goal through path
selection. The registry stores the two boundary references for a source POV;
`POVDescription` exposes them without allocation. `BoundaryGraph.h` contributes
routes and requirements, and its goal publishes allocation/identity changes
through priced Sets. No boundary-specific work enters the generic scheduler.

Boundary nodes are ordinary null POVs, created only on demand. Explicit identities
between whole occurrences also relate their corresponding boundaries. Equal
values alone do not. Existing registry identity edges preserve aliases; two
positive IDs can therefore be proved to denote the same occurrence without
rewriting either identifier or deleting a POV. A printed ID by itself is not a
serialization of the registry's complete identity graph.

For a closed `L={T A B}`, the graph establishes:

```
L.begin = A.begin
A.end = B.begin
B.end = L.end
```

These relationships compose through nested T-lists and retain the source context
of a preceding path step such as `@[1].end`. Ordinary S-list adjacency adds none
of these identities. A null is its own beginning and end; old boundary references
are identified with that null if its previously unknown source later becomes null.
Adjacent null entries remain separate list positions and still count as members.

Boundary existence does not depend on coordinates, known size, a resolved type
definition, or complete input. An open T-list's end denotes its eventual end;
it does not become the received prefix's end until closure is known. Closed
empty T-lists identify beginning and end; an empty received prefix does not.
The composition goal stays subscribed while input/closure is pending. Parser
publication resumes it even when the boundary was requested before the parser
had identified the source as a T-list. Newly requested goals reconsider later
published source information through the same live graph.

The walk inspects stored members and shared list-boundary evidence. A sparse
billion-item list obtains boundary nodes without materializing its members.
This does not yet implement identities at arbitrary virtual interior positions.

## Compatibility changes

Four default temporal fixtures now follow the agreed contract: beginning, end,
named-source boundaries, and S-list boundary access. `.begin` is a boundary
query even if the S-list contains a member typed `begin`; the old fixture's
ordinary-field expectation is obsolete. Their inputs are unchanged. Assigned
numbers in these small output fixtures are deterministic allocation results;
structural tests assert identity relationships without depending on allocation order.

The existing open-tail overlay test now expects the explicit `tUnknown` produced
by `?`, rather than its former accidental `tUnset`. Its remaining assertions,
including later numeric acquisition, are unchanged.

## Validation

- Full CodeDog regeneration and C++ build passed.
- **26/26** new null/boundary regressions passed, covering identity scope/imports,
  round trips, invalid IDs/overflow, scalar/unknown/null boundaries, aliases,
  nested/empty/open/sparse T-lists, chained context, streaming input, later null
  refinement, null RHS acquisition, costs and cancellation.
- **1,091/1,091** registered graph tests passed, including those 26.
- All **125** legacy helper tests were rerun: **102 pass / 23 fail**, with exactly
  the same per-test statuses as the preceding baseline.
- Default language suite: **243 pass / 17 fail / 8 documented skips**, out of 268.
  Three boundary fixtures recover under the approved representation contract.
- All six old language suites: **261 pass / 92 fail / 8 skips**, out of 361.
  No previous passing status was lost. No crash, timeout or incomplete summary
  occurred in either audit.
- The independent scheduler/span/range executables and sanitizers were not rerun.

Exact refreshed inventories are in `language-compatibility-status.json` and
`legacy-helper-status.json`. Build and audit logs for this run are under
`/tmp/p4-null-final-build.log`, `/tmp/p4-null-compiled-final`, and
`/tmp/p4-null-language-final`.

## Deliberately later work

S-list/T-list conversions, temporal coordinates and units, `%tprev`/`%tnext`
across boundary identities, general singlet theory and the wider word/context
model remain separate design steps. The later
[AItem cleanup](aitem-cleanup-results.md) removed the legacy temporal helpers;
it did not migrate their unsupported behavior.
