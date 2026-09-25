# Lazy member templates

A declaration such as `#many=*1000000000+{*10+7| ...}` now supplies a list
extent and one shared member template. Normalizing `many` installs that
compressed representation without creating a billion member nodes.
`findItem(many, 999999999)` obtains the registry's canonical occurrence POV;
normalizing that POV supplies its size, overlay and value. Other occurrences
remain virtual. The positional goal itself does not demand the item's value.

The existing definition walk now exposes the `listSpec` relationship and, for
an implicit occurrence, its source-list relationship. Stable read-only POV
wrappers expose infons that had no POV of their own. Compiled requirements can
contain either finite positions or one repeated member requirement. Compatible
repeated constraints combine once; combining a repeated requirement with a
finite definition applies it to those finite positions.

Application still prepares and prices a Set batch before changing any instance
fields. Existing closed physical lists receive the requirement at every member.
Empty open instances receive a read-only `listSpec` reference, not repeated RHS
copies or identities back to definition bodies. Each demanded occurrence gets
independent writable storage through the existing repetition registry. Attaching
a template preserves the finite correspondence route for a compressed span
without existing registry occurrences. A complete finite RHS may fill and close
that span, after which its physical members can reuse checked evidence. A member
may itself be a finite list. Already-demanded occurrences also participate in a
parent normalization; no loop visits the unknown positions.

Shared templates stay read-only. Their facts, named definitions, RHSs and nested
members participate in validation evidence. Template replacement invalidates
that evidence. The occurrence's source is retained for the semantic context's
lifetime because the registry's source backlink is non-owning. Partial values
can subscribe and resolve later without specializing their siblings or template.
Repeated normalization can reuse checked data without another application Set.

A nonempty compressed span remains symbolic even when its size and member
constraint are known. Finding one occurrence does not establish that every
member is present and normalized. A zero-size span can close and normalize.

This slice admits context-free homogeneous open templates, finite closed
instances and closed literal lists carrying templates. Open spans with concrete
prefixes, embedded repeated spans, context-dependent member templates, unordered
records, and general sparse correspondence remain deferred. Parser streaming
and positional language selectors retain their existing separate boundaries;
these examples use the graph API, not a newly migrated selector syntax path.

Validation (2026-09-25):

- `codeDog Proteus.Lib.dog` and `codeDog streamReplyTest.dog`: complete source
  generation and C++ builds passed.
- `unit/graphTemplates/`: **36/36**. Covers billion-index lazy access, direct
  templates, finite/nested members, aliases, combined requirements, independent
  values, stable identities, occurrence lifetime, late information, cancellation,
  cost exclusion, changed/replaced templates, evidence reuse, finite copying,
  zero extent, atomic conflicts, and explicit unsupported-shape boundaries.
  A second POV sharing list storage cannot bypass the guard on existing sparse
  occurrence identities.
- Finite definitions **41/41**; scalar definitions **36/36**; graph foundation
  **155/155**; entry/lifecycle **16/16**; growing RHS streams **19/19**; parser
  **53/53**; original `word/deref` fixture **1/1**. Total: **357/357**.
- Standalone streaming cases `1, 2, 4, 5, 6, 8`: **33/33 checks passed**.
- `git diff --check` passed.
- The former `listTemplateDeferred`, `templateDeferred`, and
  `listRHS/unsupportedSourceWork` fixtures now check their newly supported
  behavior. Unsupported contextual/prefix/embedded repetition remains explicitly
  tested as symbolic, including repetition revealed through an embedded alias.

Reproduce the template checks from `LocalBuild`:

```sh
./TestProteus -t unit/graphTemplates/
./TestProteus -t unit/graphListDefinitions/
./TestProteus -t unit/graphFoundation/
```

The full legacy suite and sanitizers were not rerun. The generic graph runtime
and span-alignment algorithm are unchanged. This is a private local Proteus4
change; the original Proteus checkout was not modified.
