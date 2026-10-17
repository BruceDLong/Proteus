# Design documents

Start with [the normalization design](normalization-design.md) and
[the decision log](normalization-design-decisions.md). Later decisions supersede
earlier checkpoint descriptions. Some introductory status summaries still need
reconciliation with those later decisions.

## Semantic references and working notes

- [Field selection](field-selection-contract.md): nominal type identity and
  ordered versus unordered selection.
- [Assignment and replacement](force-id-contract.md): the distinction between
  ordinary equality and `forceID`. Its implementation-status section predates
  the current scalar replacement support.
- [Open questions](open-questions.md) and
  [time and words](time-word-design-plan.md): unresolved topics and earlier
  commitments to reconcile with the latest decision log.
- [Temporal semantics](Proteus_Temporal_Semantics.md) and
  [temporal rooting terminology](Proteus_Temporal_Rooting_Terminology.md): working
  notes on S/T contexts, boundaries, identity, presence and rooting. Older syntax
  and proposed behavior are not guarantees about the current implementation.
- [Copy taxonomy](infon-copy-taxonomy.md) and
  [identity and storage design](infon-identity-storage-design.md): distinctions
  between semantic identity, references, copying and physical storage sharing.
  Their source inventories describe the previous engine.

## Historical evidence

[Historical](Historical/README.md) holds dated implementation reports, experiments,
performance measurements and superseded plans. This includes milestone reports
for the current engine when later changes have superseded their status or storage
details. Keep their examples and evidence distinct from current implementation
instructions.

Build, test and integration instructions are in [Docs/README.md](../README.md).
