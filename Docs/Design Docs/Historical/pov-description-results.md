# Shared POV description

The adapter now has one goal-independent description of the existing storage
in `POVDescription.h`. It exposes physical member boundaries, adjacent POVs,
parent/view links, size and value slots, member specifications, work entries,
registry identities, and existing sparse occurrences. It creates no graph nodes
or registry records. Work descriptions retain their RHS POV, including its
relationship flags. Identity descriptions retain the original evidence record.
The same registry relation is available in either direction.

The size/value dispatcher uses one relationship loop. Definition evidence,
RHS representation checks, grouping checks, and the shared list walker use the
same access paths. Separate work-source and identity-source copying helpers and
specialized intersection/concatenation member accessors were removed. A single
member-or-boundary edge helper replaces the repeated start/advance logic for
ordinary normalization, concatenation, and intersection extent inspection.

"Once" means one definition of how to read each relationship, with the existing
POV/infonView as its storage. It does not mean describing a POV only once in its
lifetime. The view reads current storage, so later publications and replacement
infon storage remain visible. Relationship enumeration creates temporary
descriptors, not cached adjacency. It preserves work-before-identity order and
distinguishes work from identity evidence even when endpoints match. Registry
lookup remains a linear scan; this change introduces no index or cache.

Sparse availability and read-only occurrence lookup delegate to the existing
POV semantics. Inspection never expands a virtual region. A size publication
can make the sparse indexed route available; only following the priced edge
materializes its requested occurrence. Unsupported or unfinished work remains
in the description, with eligibility decided by its consumer.

## Validation

- Full CodeDog generation and C++ build pass.
- New adapter integration tests: 10/10.
- Existing focused graph/parser checks: 401/401.
- Selected original language fixtures: 38/38, plus the original grouping fixture 1/1.
- Rebuilt standalone streaming harness: 33/33 checks across supported cases 1, 2, 4, 5, 6, and 8.
- `git diff --check` passes.
- No existing test inputs or expected results changed.

The adapter cases cover physical links and slots; unresolved loose work;
inverse registry access and retained provenance; distinct work and registry
relations with matching endpoints; new work and replacement storage; separate
POV contexts over shared infon data; a subscribed normalization waking for a
new identity; separate size/value goals sharing one identity; one canonical
occurrence in a billion-item virtual list; and a new size exposing sparse access.

Run the new cases from `LocalBuild` with:

```sh
./TestProteus -t unit/graphAdapter/
```

## Boundaries

The generic search engine, costs, Set validation, subscription protocol, and
semantic eligibility rules are unchanged. Goal-specific interpretation remains
in the adapter: describing a link does not establish its meaning or prove a
constraint. Sequence steps now use the common trace labels `list position` and
`list boundary`; goal kinds still distinguish their purpose.

This consolidates repeated structural interpretations; it is not a wholesale
replacement of the semantic dispatcher or a reduction of every algorithm to
simple adjacency. Existing finite planning callbacks remain synchronous.
Contextual paths, additional sparse forms, and unsupported definition/grouping
forms remain later work. No full legacy-suite pass is claimed.
