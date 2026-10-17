# Consolidated RHS lifecycle

A closed strict equality can leave `wrkList` after its facts have been copied.
Its identity remains in the existing registry, so later information and
contradictions still reach the target.

Examples:

- `_ = *10+2` becomes `*10+2`, with an empty work list and a retained identity.
- `_ = 2` still prints `2`, but keeps its work: the source size is unknown.
- Open lists, member templates, typed/contextual obligations, and qualified
  correspondence (`==`, `=:`, `==:`) retain their work entries in this slice.
- A rejected assertion is not a consolidated equality. Rejection behavior is
  unchanged by this cleanup.

## One shared rule

`RHSRepresentation` now distinguishes represented current facts from complete
source facts. Consolidation requires matching known size/value/overlay for
scalars, or matching closed physical ordered lists with recursively complete
members. Type requirements and pending source work exclude consolidation.
The existing representation goal offers a priced `Consolidate RHS equality`
Set. No semantic change occurs during completion-metadata publication.

The Set records/reuses an `IdentityRelation`, retains the exact original
infonView owning the work, and removes the pending entry. `POVDescription`
exposes that relation with the original work context. Another POV sharing the
view therefore retains its constraints even after the initial handle is
released or the original POV changes views. Reusing that POV for new work
cannot overwrite the earlier view ownership; that new work stays pending.
Existing occurrence-specific
identities keep their own endpoint semantics. No parallel adjacency graph or
second list of retired RHS nodes is added.

`infonView.hasRHS()` checks both pending work and retained equalities. RHS
producers and path-write replay use it, preventing them from re-adding a
consolidated constraint. `workListContainsPOV()` and
`workListContainsInfonView()` retain their narrower storage meaning.

The relationship remains searchable; cleanup does not claim that future
changes can be ignored or that all equality traversal disappears. Completed
handles still require explicit re-normalization to reconsider new information.
An already-subscribed parent resumes automatically. Cancellation stops that
search while leaving its published facts and semantic relationships intact.

## Validation

- Full CodeDog source generation and C++ build passed.
- **1,002/1,002** focused graph/parser checks pass, including 18 new lifecycle
  cases. The generic graph scheduler and span algorithm are unchanged.
- All six older language suites retain exactly their previous statuses:
  **218/363** pass, including **203/270** defaults. No crashes, timeouts,
  incomplete summaries, or previously passing fixtures were lost.
- Registered legacy helper/parser-error checks remain **102/125**, unchanged.

The first regression run exposed repeated RHS insertion by path writes; the
shared `hasRHS()` check fixes that loop. It also exposed internal assertions
about retained work slots; those now check the semantic relation instead.

Logs: `/tmp/p4-rhs-verified-build.log`, `/tmp/p4-rhs-final-focused.log`, and
`/tmp/p4-rhs-audit-*`.

The lifecycle cases cover scalar/text/finite/nested-list cleanup, later
contradictions, a subscribed parent, cancellation, shared/replaced/reused views,
late size evidence, cost exclusion, partial/qualified/open relationships,
replay, duplicate insertion, and reuse of an existing identity in either
orientation. Existing path-write checks exercise replay through the same rule.
Internal assertions that previously required a resolved RHS to remain in
`wrkList` now verify the retained equality. Language fixtures are unchanged.

## Next compatibility issue

The old `AItemLifecycle.completeSourceWorkConstraint` removes terminal rejected
work, while current graph assertions report a conflict and preserve its
constraint. This explains some remaining direct-scalar output differences.
The later decision preserves rejected RHSs for other matches and reconsideration.
Goal-local rejection now distinguishes an exploratory contradiction from a
required failure; see [goal-rejection-results.md](goal-rejection-results.md).
Mismatched tentative grammar paths still stop propagation without consuming input.
