# Structural relative positions

`%prev` and `%next` now contribute a position goal. The goal finds an occurrence,
then publishes an ordinary identity from the cursor expression to that occurrence.
Existing scalar Sets, list correspondence, expression evaluation and subscriptions
supply its information. There is no relative-query branch in the generic scheduler.

Examples:

```proteus
{*10+1 %prev}           // {*10+1 *10+1}
{%next %next *10+3}     // {*10+3 *10+3 *10+3}
{*10+1 _ = %prev}       // {*10+1 *10+1}
{3 (%prev+1)}           // {3 4}; numeric sizes remain unknown
```

## Relationships and context

- Loaded positions use the existing `prev`/`next` links exposed by
  `POVDescription`. Virtual occurrences use their existing source and index;
  the resulting indexed goal reuses the registry's canonical sparse edge.
  A billion-item list does not require a prefix walk or expansion.
- Syntactic RHS and arithmetic operand traversal carry the consuming occurrence
  in the query's existing domain-context field. Ordinary nested list members
  start their own context. Registry identities do not substitute a new context.
- A context-owned binding witness retains the selected occurrence and the
  consuming occurrence after the cursor's value is published. The original
  cursor mode remains available for revalidation. This is evidence about a
  reference, not another adjacency graph.
- Repeated normalization reuses the identity. A changed selected neighbor or
  one cursor expression reused in different occurrence contexts fails explicitly;
  it cannot silently overwrite a result already copied into the infonView.
- Cursor parsing supplies a placeholder zero size. Resolving the reference
  replaces that placeholder with unknown size before ordinary Sets acquire the
  referenced size; it does not assert that the referenced infon has size zero.

## Partial results and boundaries

Missing neighbors or missing information leave a symbolic subscribed result.
Publishing a neighbor, value or size through the context update boundary resumes
that search. Cancellation prevents its resumption. A closed-list boundary also
stays symbolic in this foundation; no new `UNDEFINED` policy is introduced.

Known values with unknown sizes remain partial. Conversely, a cursor whose
neighbor has a list value uses normal member correspondence and cannot acquire
`isNormed` until its required members are normalized.

This slice does not add temporal boundary semantics (`%tprev`/`%tnext`), part-path
selection, contextual definition/template instantiation, or a logical neighbor
walk across embedded spans. Those demands remain deferred. Sparse lookup uses
occurrences already owned by this graph context, so borrowed registry backlinks
are never promoted into invented owning pointers.

## Validation

- Full CodeDog regeneration and C++ test build pass.
- Relative-position checks: 33/33, including forward chains, RHS/operand context,
  nested/list results, independent member normalization, sparse creation and
  canonical reuse, cost exclusion, cancellation, late neighbor/value/size,
  boundary/cycle deferral, contradictory constraints and context revalidation.
- Existing focused graph/parser checks: 441/441; combined total 474/474.
- Original selected language fixtures: 38/38 plus the two grouping fixtures.
- Rebuilt standalone streaming harness: 33/33 supported checks (1, 2, 4, 5, 6, 8).
- The earlier `unsupportedTargetAtomic` fixture now uses `%tprev` to keep testing
  unsupported-target atomicity. Separate `%prev` tests verify that consistent
  named requirements resolve and contradictory ones fail, with shared definition
  bodies unchanged and no added type RHSs.
- The original `prev/plus1` recurrence fixture was also checked. It remains
  symbolic (`{T *256+_ = (%prev +1)| 3 ... }`); temporal lists and contextual
  repeated-template instantiation are still outside this slice.

Run the focused tests from `LocalBuild` with:

```sh
./TestProteus -t unit/graphRelative/
./TestProteus -t unit/graphListDefinitions/
```

No full legacy-suite or sanitizer pass is claimed for this checkpoint.
