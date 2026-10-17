# Definition prefixes and collection meaning

Ordered definitions now contribute their written prefix while leaving unresolved
positions virtual. For example:

```proteus
#digit = *10+_
#series = *1000000000+{digit | 1 2 ...}
series
```

Normalization publishes two members, with their digit size and value facts.
It retains the large extent and shared template. An indexed goal can reach the
last virtual occurrence without allocating intervening items. Unknown total
extents and open prefixes without a template also remain symbolic.

The existing definition compiler reads the stored prefix. Ordered conjunction
now combines common positions, retains the longer allowed prefix, checks closed
ends, and applies member-template requirements through the same combination
code. Conflicting facts or bounds reject before the plan writes instance data.
The repeated-member plan allocates only written positions. When a requested
occurrence later becomes a concrete prefix member, it reuses that POV, including
when the request came through another view of the same infon. Distinct competing
occurrence views at one position still require a joint identity plan and defer.

Indexed graph access uses ListStructure boundary evidence to distinguish the
known prefix from the virtual tail. The existing occurrence registry retains
its identities. Neither graph inspection nor definition compilation materializes
virtual members. No scheduler branch, domain-specific node, or application RHS
was introduced.

Definition compilation also reuses WordForms collection inference: a collection
with a singular typed template can supply its plural nominal label to ordinary
field correspondence. This reads the shared source without normalizing it or
recursively expanding a declaration for the inferred spelling. Explicit labels
retain precedence. Supplied typed descriptions keep the existing lazy policy.

## Remaining boundary

Embedded open definition spans still need composition through the shared span
planner. Competing aliases, contextual requirements, and recursive definitions
retain their existing deferred behavior. This increment does not change the
policy for enforcing a named definition against already supplied member values.
The user retired the synthetic Rubix materialization fixture rather than using
its conflicting placeholder values to decide that policy. The real model and
future verb-application direction are recorded in language-compatibility-results.md.

## Validation

- Full source regeneration and both test/streaming builds passed.
- All 18 new generic tests pass. Coverage includes known and unknown extents,
  untemplated prefixes, conjunctions, atomic conflicts, sparse lookup, occurrence
  preservation after a late update, replay, cost exclusion, read-only compilation,
  nested collections, partial descriptions, and explicit collection labels.
- Two existing internal tests now require the published prefix and unresolved
  tail: graphTemplates/bodyPrefix and graphListDefinitions/openBodyPrefix.
  They previously asserted that prefix definition application was unsupported.
- All 1,224 registered graph checks pass; compiled total: 1,274 pass / 8 existing
  failures. Every retained pre-existing compiled status is unchanged.
- The remaining defaults report 248 pass / 11 fail / 8 documented skips. Across
  six language suites: 266 pass / 86 fail / 8 skips. The one fewer failure comes
  from the user-requested fixture removal, not a semantic recovery. All surviving
  language expectations and statuses are unchanged.
- Supported streaming selections 1, 2, 4, 5, 6, and 8 pass 33/33 checks. No GUI
  integration, sanitizer pass, or successful verb application is claimed.

Final compiled/default/stream logs: /tmp/p4-prefix-verified. Other language-suite
logs: /tmp/p4-prefix-final. Final source build: /tmp/p4-prefix-final-test-build.log;
streaming build: /tmp/p4-prefix-stream-build.log.
