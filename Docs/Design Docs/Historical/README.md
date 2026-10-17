# Historical design and implementation records

These documents record the engine and models at their stated checkpoint. Many
describe the abandoned native graph engine or the earlier AItem normalizer.
Others document milestones of the current CodeDog WorldManager. Their APIs,
test counts, performance measurements and remaining-work lists are historical.

Use [the current design](../normalization-design.md) and
[later decisions](../normalization-design-decisions.md) for the maintained
contract. Original commands, commit references and temporary evidence paths are
retained for historical interpretation; retired source files are available in
Git history, not necessarily in the current checkout. Moving a report here does
not invalidate its semantic examples or require restoring its old implementation.

Useful recent records include:

- [Ancestor member requirements](member-requirements-2026-10-13.md).
- [TCursor supplied work](tcursor-work-2026-10-14.md); D071 later changes cache
  ownership and backward movement.
- [Viewer materialization](tcursor-viewer-2026-10-14.md).
- [Rubix verbs and viewer integration](rubix-verbs-2026-10-14.md).

Machine-readable inventories, language fixtures, audit scripts and captured
diagnostic assets remain in [graph/](../../../graph/). Their locations have not
changed.
