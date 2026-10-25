# Proteus library class reference

Source snapshot: `main` at `e2ee4c982204`, reviewed 2026-10-25.

This inventory covers **187 distinct names** declared with `model` or `struct`
in `Proteus.Lib.dog` and its recursive production `#include` tree: **22 source
files, 275 declaration blocks**. It excludes tests (including the `TestSpec`
entry point), theory, viewer/CLI programs, generated files and types supplied by
CodeDog libraries such as `FlexNum`, `Threads` and `strBuf`.

A `model` and its `struct` implementation/extension are one class here. Repeated
`struct` blocks also extend that same class. Enum wrappers, empty identity tokens,
the native ownership/lock bindings and CodeDog's `GLOBAL` pseudo-struct are included
because they are explicit declarations in this source tree. Inline `mode[...]`
fields are fields, not additional classes.

Entries are grouped by their main declaration file and alphabetized within each
group. The declaration column links every production declaration, including
extensions in other files. A bare line link belongs to that section's file.
Comments describe the records and methods currently present; declarations alone
do not establish complete support for the associated language feature.

The central call path is `ProteusCore` → `Agent` → `WorldManager` →
`WorldRequest`. The underlying model is `infonView`/`infonState`, with contextual
occurrences represented by `POV` and lists stored in `PovList`. `WorldPOVGraph`
provides the reasoning system's virtual access to that storage. `WorldDemand`
owns occurrence acquisition; `TCursor` is a consumer facade over it.

## Core model, identities and host interface

Main declarations: [Proteus.Lib.dog](../Proteus.Lib.dog) — 34 names.

| Class | Comment | Declarations |
|---|---|---|
| `Agent` | Owns the world, identity registry, interaction context and reasoning sessions; delegates normalization to WorldManager and closes its sessions at shutdown. | struct [2399](../Proteus.Lib.dog#L2399) |
| `BoundaryReference` | Keeps a source occurrence together with its canonical begin and end boundary POVs. | struct [329](../Proteus.Lib.dog#L329) |
| `EndRelativeExtentResult` | Returns whether an end-relative extent is exact, unknown/unbounded or incompatible, with its exact count when known. | struct [1312](../Proteus.Lib.dog#L1312) |
| `EndRelativeExtentStatus` | Enum distinguishing an exact extent from an unknown/unbounded or incompatible extent. | struct [42](../Proteus.Lib.dog#L42) |
| `EndRelativeReference` | Retains a symbolic negative-offset selection, its source and its eventual resolved occurrence and identity evidence. | model [86](../Proteus.Lib.dog#L86), struct [96](../Proteus.Lib.dog#L96) |
| `EndRelativeWindowAvailability` | Describes the availability, extent and source endpoints of a requested window measured from a list's end. | struct [1317](../Proteus.Lib.dog#L1317) |
| `EndRelativeWindowContentStatus` | Enum distinguishing available, pending, out-of-range and incompatible end-relative window contents. | struct [48](../Proteus.Lib.dog#L48) |
| `ExternalAssociationReference` | Records a reversible external relationship asserted with `^`, retaining distinct anchor, predicate, value and optional witness occurrences. | model [247](../Proteus.Lib.dog#L247), struct [256](../Proteus.Lib.dog#L256) |
| `GLOBAL` | CodeDog's shared global declarations, rather than a domain object: supplies helpers, constants, the model manager, inflection service, printer and built-in function names. | struct [2293](../Proteus.Lib.dog#L2293), struct [2392](../Proteus.Lib.dog#L2392); Functions.dog struct [2](../Functions.dog#L2); ModelManager.dog struct [3](../ModelManager.dog#L3), struct [144](../ModelManager.dog#L144); infonIO.dog struct [1731](../infonIO.dog#L1731); infonList.dog struct [3](../infonList.dog#L3) |
| `IdentityEvidenceKind` | Enum identifying why an identity relationship was established, such as an assertion, temporal boundary, reference or list correspondence. | struct [58](../Proteus.Lib.dog#L58) |
| `IdentityRegistry` | Owns an Agent's identity evidence and reversible selection, temporal, repetition and external-association records. | model [334](../Proteus.Lib.dog#L334), struct [352](../Proteus.Lib.dog#L352) |
| `IdentityRelation` | One explicit identity fact between two contextual POVs, retaining the evidence kind and normalization that established it. | model [73](../Proteus.Lib.dog#L73), struct [80](../Proteus.Lib.dog#L80) |
| `infItr` | Small retained iterator/context record holding a parent iterator, top POV and alternative identifier; it is not the normalizer's traversal cursor. | model [2285](../Proteus.Lib.dog#L2285) |
| `InfonIdentity` | Opaque occurrence-identity token; its printed integer is interpreted within an identity scope. | struct [287](../Proteus.Lib.dog#L287) |
| `InfonIdentityScope` | Allocates and claims identity tokens and consistently remaps identities imported from independent scopes, under a mutex. | struct [288](../Proteus.Lib.dog#L288) |
| `infonState` | Stores one value or size overlay: numeric/string/list payload, evaluation mode, listSpec, temporal/open-tail flags and representation metadata. | model [1327](../Proteus.Lib.dog#L1327), struct [2385](../Proteus.Lib.dog#L2385) |
| `infonView` | Main infon representation: combines value and size overlays with type qualifications, cursor/path syntax, lexical bindings, pending relationships and parser/runtime metadata. | model [1423](../Proteus.Lib.dog#L1423), struct [1544](../Proteus.Lib.dog#L1544); WorldCheckedViews.dog struct [4](../WorldCheckedViews.dog#L4); WorldDemandCache.dog struct [8](../WorldDemandCache.dog#L8); WorldManager.dog struct [1017](../WorldManager.dog#L1017); WorldPrivateMembers.dog struct [16](../WorldPrivateMembers.dog#L16); infonIO.dog struct [5](../infonIO.dog#L5) |
| `OrderMode` | Enum recording whether a view's ordering is unknown, ordered or unordered. | struct [36](../Proteus.Lib.dog#L36) |
| `PartPath` | One navigation/selection step, retaining its operand, optional specification and continuation after bracketed selection. | model [2367](../Proteus.Lib.dog#L2367), struct [2382](../Proteus.Lib.dog#L2382) |
| `ProteusCore` | Host-facing engine facade for parsing, normalization, world loading, stream management, results, errors and shutdown; exposes overridable integration hooks. | struct [2544](../Proteus.Lib.dog#L2544) |
| `RepetitionFamily` | Gives a repeated sparse span a stable identity space and indexes only the logical occurrences actually demanded. | model [272](../Proteus.Lib.dog#L272), struct [281](../Proteus.Lib.dog#L281) |
| `RepetitionOccurrence` | Names one logical member of a repetition, with a stable token, optional ordinal and symbolic/materialized POVs. | model [260](../Proteus.Lib.dog#L260), struct [268](../Proteus.Lib.dog#L268) |
| `SelectionReference` | Records one accepted source occurrence for a selection, preserving its distinct occurrence token and source/result provenance. | model [142](../Proteus.Lib.dog#L142), struct [157](../Proteus.Lib.dog#L157) |
| `SelectionReferenceKind` | Enum recording the kind of selection that produced a reference, including positional, part-path, external and word resolution. | struct [102](../Proteus.Lib.dog#L102) |
| `SelectionRequest` | Retains an unresolved or resolved selection operation separately from its accepted occurrences, including query, consumer and optional range specification. | model [123](../Proteus.Lib.dog#L123), struct [140](../Proteus.Lib.dog#L140) |
| `SelectionResolution` | Collects the requests and accepted references for a selection together with its resolution state. | model [162](../Proteus.Lib.dog#L162), struct [167](../Proteus.Lib.dog#L167) |
| `SelectionResolutionState` | Enum distinguishing no resolution, pending resolution, one accepted occurrence and multiple accepted occurrences. | struct [116](../Proteus.Lib.dog#L116) |
| `SizeMode` | Enum recording whether size comes from counting, context or an explicit given constraint. | struct [35](../Proteus.Lib.dog#L35) |
| `TemporalBoundaryIdentity` | Retains adjacent temporal spans and the identity equating the earlier span's end with the later span's begin. | model [1280](../Proteus.Lib.dog#L1280), struct [1289](../Proteus.Lib.dog#L1289) |
| `TemporalContainmentIdentity` | Retains a host span, a contained span and the identities aligning their begin and end boundaries. | model [1299](../Proteus.Lib.dog#L1299), struct [1309](../Proteus.Lib.dog#L1309) |
| `TemporalProjectionReference` | Records the reversible relationship between an item and its temporal-list view, including canonical boundary views. | model [173](../Proteus.Lib.dog#L173), struct [182](../Proteus.Lib.dog#L182) |
| `TemporalRootingReference` | Retains the evidence aligning a child's temporal boundary with a split in a containing temporal span. | model [229](../Proteus.Lib.dog#L229), struct [240](../Proteus.Lib.dog#L240) |
| `TemporalSplitReference` | Represents one unresolved temporal cut, its prefix/suffix witnesses and boundary identities, without enumerating candidate states. | model [196](../Proteus.Lib.dog#L196), struct [222](../Proteus.Lib.dog#L222) |
| `ThreadedNormalizer` | Worker that owns a stream's normalization handle and session, coordinates parser completion and reports reasoning errors through the host. | struct [2504](../Proteus.Lib.dog#L2504) |

## POVs, lists and view mappings

Main declarations: [infonList.dog](../infonList.dog) — 10 names.

| Class | Comment | Declarations |
|---|---|---|
| `ImplicitItemMap` | Maps a virtual member POV to its source sparse span and logical zero-based source index. | struct [30](../infonList.dog#L30) |
| `POV` | A contextual occurrence or relationship around an infonView; carries list links, parent context, relationship qualifications, identity/provenance and engine occurrence metadata. | model [71](../infonList.dog#L71), struct [1403](../infonList.dog#L1403); WorldGeneratedOccurrences.dog struct [53](../WorldGeneratedOccurrences.dog#L53); WorldManager.dog struct [1002](../WorldManager.dog#L1002); WorldPrivateMembers.dog struct [9](../WorldPrivateMembers.dog#L9); WorldTemporalProduction.dog struct [9](../WorldTemporalProduction.dog#L9) |
| `PovList` | Linked container of POV occurrences, providing list edits and traversal with ordinary or private weak backward links. | model [1208](../infonList.dog#L1208), struct [1404](../infonList.dog#L1404) |
| `POVPedigree` | Stores the numeric source marker used to identify where a POV was created. | struct [20](../infonList.dog#L20) |
| `ReferenceAliasViewMap` | Retains the exact source occurrence denoted by a local alias, separately from the registry's semantic identity evidence. | struct [65](../infonList.dog#L65) |
| `RepetitionSpanMap` | Maps a sparse span into a shared RepetitionFamily using the span's starting ordinal. | struct [37](../infonList.dog#L37) |
| `TemporalBoundaryViewMap` | Identifies a cut in a source temporal list, including symbolic end tracking and an optional known cut index. | struct [43](../infonList.dog#L43) |
| `TemporalProjectionViewMap` | Identifies which endpoint of a registry-owned item/temporal-view projection a POV represents. | struct [53](../infonList.dog#L53) |
| `TemporalSplitViewMap` | Links a symbolic cut POV back to the registry-owned temporal split that defines it. | struct [59](../infonList.dog#L59) |
| `ViewMap` | Maps a derived view/span to its source POV and the source occurrence corresponding to its local first item. | struct [23](../infonList.dog#L23) |

## Parsing and formatting

Main declarations: [infonIO.dog](../infonIO.dog) — 5 names.

| Class | Comment | Declarations |
|---|---|---|
| `InfonPrintMode` | Enum selecting compressed, expanded or debug rendering; context-free formatting uses compressed output for expanded requests. | struct [1728](../infonIO.dog#L1728) |
| `InfonViewPrinter` | Formats infonViews, including types, expressions, lists, sizes and debug information, while serializing printer access with a mutex. | struct [1789](../infonIO.dog#L1789) |
| `parseContext` | Enum identifying the syntactic context being parsed, such as the main infon, RHS, listSpec or definition. | struct [3](../infonIO.dog#L3) |
| `ppStatus` | Parser-progress enum distinguishing buffer-end/incomplete states, completed input, syntax errors and other errors. | struct [45](../infonIO.dog#L45) |
| `ProteusParser` | Parses source buffers into infonViews and POV relationships; tracks lexical/parameter scopes, identity imports, errors and parser-completion publication. | struct [46](../infonIO.dog#L46) |

## Vocabulary, lexical scope and interaction context

Main declarations: [ModelManager.dog](../ModelManager.dog) — 13 names.

| Class | Comment | Declarations |
|---|---|---|
| `AgentContext` | Maintains the Agent's bounded, source-ordered topic history for context references while preserving occurrence aliases and invalidating changed membership evidence. | struct [159](../ModelManager.dog#L159) |
| `ModelManager` | Stores and looks up vocabulary definitions, supports word forms and inferred list types, and retains finite ordered-unit measurement/conversion helpers. | struct [299](../ModelManager.dog#L299) |
| `OrderedSpanConverter` | Records compatible from/to unit types, their common base type and each unit's base count; used by ModelManager's conversion helper/cache. | model [281](../ModelManager.dog#L281), struct [290](../ModelManager.dog#L290) |
| `OrderedSpanMeasure` | Result of measuring a finite homogeneous unit in a named base unit, including validity and base count. | model [273](../ModelManager.dog#L273), struct [279](../ModelManager.dog#L279) |
| `OrderedSpanTraversalWindow` | Retained data-only record for a valid left/right POV traversal window; no current production consumer was found. | model [292](../ModelManager.dog#L292), struct [297](../ModelManager.dog#L297) |
| `vocabularySpec` | Normalizes a language specification and extracts the base language used for vocabulary lookup. | struct [129](../ModelManager.dog#L129) |
| `WordArgument` | One attached word-call argument: optional binding name plus its infon expression. | model [61](../ModelManager.dog#L61), struct [65](../ModelManager.dog#L65) |
| `WordArguments` | Container preserving the arguments supplied to an attached word call. | model [66](../ModelManager.dog#L66), struct [69](../ModelManager.dog#L69) |
| `WordDefn` | A named declaration with meaning, model ID, parameter signature, usage/language tags, lexical scope and stable generated-declaration identity. | model [27](../ModelManager.dog#L27), struct [58](../ModelManager.dog#L58); WorldGeneratedOccurrences.dog struct [6](../WorldGeneratedOccurrences.dog#L6) |
| `WordScope` | Lexical scope with a parent, global-scope marker and declarations indexed by name. | struct [21](../ModelManager.dog#L21) |
| `WordScopeDeclarations` | Collects declaration paths for one spelling within a lexical scope. | struct [18](../ModelManager.dog#L18) |
| `WordScopePath` | Locates declaration syntax through weak member references and parent paths, including embedded syntax that shares a scope. | struct [14](../ModelManager.dog#L14) |
| `WordUse` | One use of a word/type: preserves its original and normalized spelling, selected definition/model, inflection and grammatical metadata. | model [71](../ModelManager.dog#L71), struct [127](../ModelManager.dog#L127) |

## Word-resolution planning representations

Main declarations: [WordRules.dog](../WordRules.dog) — 9 names.

This file supplies representations, estimation and definition enumeration. Its broader resolution-tier orchestration remains a policy skeleton; its older introductory resolver references are not evidence of an active complete resolver.

| Class | Comment | Declarations |
|---|---|---|
| `WordResolutionAxis` | One source or descriptor operand in a factored resolution plan, retaining lazily collected definitions and an optional nested plan. | model [162](../WordRules.dog#L162), struct [169](../WordRules.dog#L169) |
| `WordResolutionBranch` | Bookkeeping for one definition pair at one resolution tier, with estimated cost and attempted state. | model [176](../WordRules.dog#L176), struct [183](../WordRules.dog#L183) |
| `WordResolutionCandidate` | Potential selected occurrence plus its property witness, definition provenance, resolution tier, calculation cost and semantic rank. | model [185](../WordRules.dog#L185), struct [199](../WordRules.dog#L199) |
| `WordResolutionCostEstimate` | Computes saturated lower/upper work estimates before definition-pair materialization; records when a complete upper bound is unavailable. | model [74](../WordRules.dog#L74), struct [85](../WordRules.dog#L85) |
| `WordResolutionCostModel` | Policy weights for estimating definition expansion and each word-resolution tier; these weights do not determine semantic precedence. | model [63](../WordRules.dog#L63), struct [70](../WordRules.dog#L70) |
| `WordResolutionPlan` | Retains source/descriptor axes, current tier, cost estimates, opened branches and candidates without eagerly expanding their Cartesian product. | model [210](../WordRules.dog#L210), struct [225](../WordRules.dog#L225) |
| `WordResolutionState` | Enum describing a resolution plan's progress or outcome, including unique/multiple, missing, cost-deferred and cyclic states. | struct [49](../WordRules.dog#L49) |
| `WordResolutionTier` | Enum ordering direct members, exact source external relationships, typed properties and arbitrary immediate properties. | struct [42](../WordRules.dog#L42) |
| `WordRulePlanner` | Provides a definition-enumeration helper within a policy skeleton; broader tier orchestration remains unimplemented. | struct [243](../WordRules.dog#L243) |

## Stream input and worker lifetimes

Main declarations: [InfonStreamLoader.dog](../InfonStreamLoader.dog) — 8 names.

| Class | Comment | Declarations |
|---|---|---|
| `AssetFileConnector` | Default asset-file connector; reads input lines into the shared stream path and prints results/errors through the connector protocol. | struct [140](../InfonStreamLoader.dog#L140) |
| `InfonConnector` | Extensible protocol boundary exposing input/output buffers, availability, input closure and result/error callbacks to an InfonStream. | model [80](../InfonStreamLoader.dog#L80), struct [137](../InfonStreamLoader.dog#L137) |
| `InfonStream` | Owns one connector-to-parser-to-normalizer pipeline, including origin, target infon, worker threads and completion/shutdown coordination. | struct [258](../InfonStreamLoader.dog#L258) |
| `InfonStreamLoader` | Manages active and opening streams, origin records, relative stream resolution, asynchronous insertion and the shutdown barrier. | struct [373](../InfonStreamLoader.dog#L373) |
| `InfonStreamSpec` | Parses and reconstructs a stream locator's protocol, connection/content arguments and optional host/port components. | struct [8](../InfonStreamLoader.dog#L8) |
| `ThreadedInfonParser` | Parser worker retaining its parser, result and connector, with synchronized completion signaling and input/error cleanup. | struct [187](../InfonStreamLoader.dog#L187) |
| `ThreadedStreamInput` | Worker that pumps connector input and turns transport exceptions into failed, closed input. | struct [246](../InfonStreamLoader.dog#L246) |
| `ThreadedStreamOpen` | Worker opening an inserted stream asynchronously while keeping its operation inside the loader's shutdown barrier. | struct [350](../InfonStreamLoader.dog#L350) |

## English inflection

Main declarations: [xlators/xlator_en.dog](../xlators/xlator_en.dog) — 1 name.

| Class | Comment | Declarations |
|---|---|---|
| `Inflect` | English word-form service backed by inflection tables and rules, including singular/plural recognition and conversion. | struct [6](../xlators/xlator_en.dog#L6) |

## Reasoning interface and shared engine records

Main declarations: [WorldManager.dog](../WorldManager.dog) — 72 names.

| Class | Comment | Declarations |
|---|---|---|
| `TCursor` | Consumer-facing temporal cursor facade; delegates acquisition, selection, movement, required-field preparation and explicit write-back to WorldDemand. | struct [8402](../WorldManager.dog#L8402) |
| `WorldAlternatives` | Collection of checked branch frames representing alternatives without merging their occurrences or dropping their conjunctions. | struct [281](../WorldManager.dog#L281) |
| `WorldArgumentBinding` | One application-local parameter binding, retaining the supplied expression, caller context, omitted-argument obligations and production snapshot information. | struct [173](../WorldManager.dog#L173); WorldGeneratedOccurrences.dog struct [70](../WorldGeneratedOccurrences.dog#L70) |
| `WorldBindings` | A definition application's parameter environment, mapping parameter names to bindings and retaining lexical/production scope information. | struct [182](../WorldManager.dog#L182); WorldGeneratedOccurrences.dog struct [64](../WorldGeneratedOccurrences.dog#L64) |
| `WorldBounds` | Represents numeric value sets as an unbounded set, a point or a half-open interval; implements containment and intersection. | struct [27](../WorldManager.dog#L27) |
| `WorldCapture` | Retains a selected source position together with the checked facts that justified it. | struct [296](../WorldManager.dog#L296) |
| `WorldChoiceMerge` | Tracks pairwise relationship solving between left/right alternative sets and collects the resulting checked alternatives. | struct [284](../WorldManager.dog#L284) |
| `WorldControl` | Shared model lock, root, identity registry and input-wait registry; remains alive when callers retain request handles beyond the manager. | struct [820](../WorldManager.dog#L820) |
| `WorldCorrespondence` | Resumable working state for matching a requested member/span/list against a source, including walks, predicates, captures, projections and alternatives. | struct [405](../WorldManager.dog#L405); WorldRecordProjection.dog struct [10](../WorldRecordProjection.dog#L10) |
| `WorldCorrespondenceQuery` | Describes requested semantic coverage, target and qualified relationship, distinguishing predicate tests from asserted-member acquisition. | struct [324](../WorldManager.dog#L324); WorldRecordProjection.dog struct [4](../WorldRecordProjection.dog#L4) |
| `WorldCorrespondenceResult` | Returns accepted source positions, checked facts and mapping evidence, including positional/span and record-projection results. | struct [330](../WorldManager.dog#L330); WorldRecordProjection.dog struct [7](../WorldRecordProjection.dog#L7) |
| `WorldCursorUse` | Empty ownership token whose shared use protects an acquired value from eviction while a consumer or checkpoint still uses it. | struct [988](../WorldManager.dog#L988) |
| `WorldCursorValue` | Metadata for an acquired occurrence value: readiness, revision, original representation, use lease, permanence, cache proof and deferred eviction. | struct [989](../WorldManager.dog#L989); WorldDemandCache.dog struct [81](../WorldDemandCache.dog#L81); WorldDemandRetention.dog struct [3](../WorldDemandRetention.dog#L3) |
| `WorldCursorWrite` | Stages a detached target branch and member mapping so write-back attaches it only after all affected writes pass checking. | struct [846](../WorldManager.dog#L846) |
| `WorldDefinitions` | Collection of declarations sharing one name, preserving alternatives rather than silently choosing one. | struct [517](../WorldManager.dog#L517) |
| `WorldDefinitionTable` | Name-indexed declaration table for one lexical scope at one owning occurrence. | struct [509](../WorldManager.dog#L509) |
| `WorldDemand` | Normalizer-owned occurrence demand: performs navigation/acquisition, required-field preparation, waiting, replay, caching, retention and checked write-back. | struct [7940](../WorldManager.dog#L7940); WorldDemandCache.dog struct [126](../WorldDemandCache.dog#L126); WorldDemandLayout.dog struct [3](../WorldDemandLayout.dog#L3); WorldDemandRetention.dog struct [32](../WorldDemandRetention.dog#L32) |
| `WorldFieldDemand` | Work item connecting a required field with its source, acquired value, destination and contextual position. | struct [7930](../WorldManager.dog#L7930) |
| `WorldFieldMerge` | Tracks member frames and the sequence being assembled while compatible record fields are merged. | struct [520](../WorldManager.dog#L520) |
| `WorldFrame` | One resumable reasoning/acquisition operation, retaining checked facts, pending relationships, parameters, traversal state and continuation dependencies. | struct [719](../WorldManager.dog#L719); WorldListLayout.dog struct [40](../WorldListLayout.dog#L40); WorldProductionFrontier.dog struct [120](../WorldProductionFrontier.dog#L120); WorldTemporalAcquisition.dog struct [8](../WorldTemporalAcquisition.dog#L8); WorldTemporalProduction.dog struct [14](../WorldTemporalProduction.dog#L14) |
| `WorldInputWaiters` | Requests registered to continue when one particular infon's parser publishes readiness. | struct [826](../WorldManager.dog#L826) |
| `WorldList` | Shared tree of checked member sequences with represented length, sparse-span extent information and pending-requirement summaries. | struct [104](../WorldManager.dog#L104) |
| `WorldListContinuation` | Reusable structural continuation from a completed positional correspondence; validates scope/source revisions without retaining member facts or expression frames. | struct [461](../WorldManager.dog#L461) |
| `WorldListCursor` | Traverses a WorldList tree and optionally flattens embedded members while retaining its traversal stack. | struct [119](../WorldManager.dog#L119) |
| `WorldListPosition` | One pending sequence/index pair in a WorldListCursor traversal. | struct [115](../WorldManager.dog#L115) |
| `WorldListProgress` | Holds reusable list continuations and their generation/hit/store/miss counters for incremental demands. | struct [486](../WorldManager.dog#L486) |
| `WorldManager` | Replaceable public reasoning interface: normalizes requests, manages their execution and publication, resumes parser waits and supplies demands/cursors through WorldPOVGraph. | struct [3885](../WorldManager.dog#L3885); WorldDemandCache.dog struct [57](../WorldDemandCache.dog#L57); WorldDemandRetention.dog struct [12](../WorldDemandRetention.dog#L12); WorldDemandSelection.dog struct [4](../WorldDemandSelection.dog#L4); WorldExpandedPrint.dog struct [124](../WorldExpandedPrint.dog#L124); WorldGeneratedOccurrences.dog struct [432](../WorldGeneratedOccurrences.dog#L432); WorldProductionFrontier.dog struct [137](../WorldProductionFrontier.dog#L137); WorldTemporalAcquisition.dog struct [22](../WorldTemporalAcquisition.dog#L22); WorldTemporalProduction.dog struct [66](../WorldTemporalProduction.dog#L66) |
| `WorldMapping` | Persistent capture mapping that shares earlier assignments and can retain an alternative mapping without enumerating complete paths. | struct [302](../WorldManager.dog#L302) |
| `WorldMatch` | One active pattern slot with its capture, persistent mapping and ambiguity marker. | struct [309](../WorldManager.dog#L309) |
| `WorldMember` | One checked member frame plus whether that occurrence is embedded in its containing sequence. | struct [100](../WorldManager.dog#L100) |
| `WorldModelLock` | Native scoped-lock binding for the recursive model mutex; provides synchronization rather than reasoning behavior. | struct [3](../WorldManager.dog#L3) |
| `WorldModelMutex` | Native recursive-mutex binding used to protect model operations and session coordination. | struct [2](../WorldManager.dog#L2) |
| `WorldNameBinding` | Resolved or pending name binding: retains declaration/origin and either an immutable template or an exact contextual occurrence, plus construction/export state. | struct [497](../WorldManager.dog#L497) |
| `WorldOpenAlignment` | Resumable alignment state for prefix/suffix constraints around a symbolic open gap, tracking matched and newly contributed members. | struct [159](../WorldManager.dog#L159) |
| `WorldOpenLayout` | Checked prefix and suffix sequences surrounding one symbolic gap, with their anchored member counts. | struct [148](../WorldManager.dog#L148) |
| `WorldOpenScan` | Stack entry for examining a frame's member sequence while discovering or consuming an open layout. | struct [155](../WorldManager.dog#L155) |
| `WorldOrderedLayoutProof` | Cached evidence that a particular header revision/value has a stable ordered extent. | struct [205](../WorldManager.dog#L205) |
| `WorldOutcome` | Enum of request/operation outcomes: ready, complete, partial, waiting, rejected, unsupported, cancelled and error. | struct [21](../WorldManager.dog#L21) |
| `WorldParseResult` | Result of the manager's closed-input parsing path, carrying either a target POV or an error string. | struct [981](../WorldManager.dog#L981) |
| `WorldPath` | Resumable evaluation of a PartPath, including context/neighbor navigation, selection operations, external relationships and nested constraints. | struct [698](../WorldManager.dog#L698); WorldProductionFrontier.dog struct [132](../WorldProductionFrontier.dog#L132) |
| `WorldPattern` | One correspondence predicate with its source position, owning frame and inherited requirements. | struct [315](../WorldManager.dog#L315) |
| `WorldPatternChart` | Shared, resumable matching chart for structured or text patterns; memoizes subproblems and retains packed source captures/results. | struct [381](../WorldManager.dog#L381) |
| `WorldPatternMatchProblem` | One memoized template-at-source-boundary subproblem, including pending states, child matching and results indexed by accepted end position. | struct [363](../WorldManager.dog#L363) |
| `WorldPatternMatchState` | One partial structured-pattern match, retaining selected frame, current slot/end, output sequence and proof. | struct [353](../WorldManager.dog#L353) |
| `WorldPatternProof` | Persistent proof node sharing prior/child matches and recording ambiguity or introduced temporal states. | struct [341](../WorldManager.dog#L341) |
| `WorldPatternResult` | An accepted pattern match with its selected frame, source end position, checked frame and proof. | struct [347](../WorldManager.dog#L347) |
| `WorldPatternStarts` | Indexes matching subproblems by starting source position for one pattern frame. | struct [380](../WorldManager.dog#L380) |
| `WorldPosition` | Contextual graph address retaining node/owner, parent, lexical parameters, input context, occurrence, write authority and optional generated address. | struct [187](../WorldManager.dog#L187); WorldGeneratedOccurrences.dog struct [78](../WorldGeneratedOccurrences.dog#L78); WorldProductionFrontier.dog struct [133](../WorldProductionFrontier.dog#L133) |
| `WorldPOVGraph` | Virtual read/write interface over existing infon/POV storage; supplies relationship traversal, contextual positions, checked views and semantic publication without copying an adjacency graph. | struct [1029](../WorldManager.dog#L1029); WorldCheckedViews.dog struct [31](../WorldCheckedViews.dog#L31); WorldDemandCache.dog struct [85](../WorldDemandCache.dog#L85); WorldGeneratedOccurrences.dog struct [99](../WorldGeneratedOccurrences.dog#L99); WorldListLayout.dog struct [45](../WorldListLayout.dog#L45); WorldRecordProjection.dog struct [18](../WorldRecordProjection.dog#L18); WorldTemporalProduction.dog struct [40](../WorldTemporalProduction.dog#L40) |
| `WorldProjectionRoute` | Retains member positions along a projection route so nested correspondence preserves occurrence context. | struct [404](../WorldManager.dog#L404) |
| `WorldPublication` | Prepared model update containing target/frame, replacement or appended list data, alternatives and any field reordering. | struct [833](../WorldManager.dog#L833) |
| `WorldRequest` | Caller-visible normalization handle and request execution state; exposes status/result/cancellation and retains resumable frames, staged changes and work counters. | struct [851](../WorldManager.dog#L851); WorldCheckedViews.dog struct [10](../WorldCheckedViews.dog#L10); WorldListLayout.dog struct [35](../WorldListLayout.dog#L35); WorldProductionFrontier.dog struct [93](../WorldProductionFrontier.dog#L93); WorldRecordProjection.dog struct [13](../WorldRecordProjection.dog#L13); WorldTemporalProduction.dog struct [18](../WorldTemporalProduction.dog#L18) |
| `WorldRequiredField` | Tree of named field requirements, including whether a field value itself is required and which descendants must be prepared. | struct [7923](../WorldManager.dog#L7923) |
| `WorldRequirement` | Persistent tree of qualified frame/relationship requirements, allowing scopes to compose without copying existing requirement chains. | struct [596](../WorldManager.dog#L596) |
| `WorldRequirementCursor` | Stack-based iterator over a persistent WorldRequirement tree, with visit accounting. | struct [602](../WorldManager.dog#L602) |
| `WorldScalar` | Checked fact summary for a frame: value/range/size, type qualifications, identity, ordering, temporal/list structure, alternatives and unresolved evidence. | struct [526](../WorldManager.dog#L526) |
| `WorldScopedDefinitions` | Indexes occurrence-specific declaration tables within a lexical scope. | struct [514](../WorldManager.dog#L514) |
| `WorldSelection` | Tracks scalar-coordinate or member selection through a logical walk, including the selected source and candidate/domain checks. | struct [264](../WorldManager.dog#L264) |
| `WorldSession` | Agent-owned request lifetime: tracks request registration/release, closed state, a cache identity token and generated-series metadata. | struct [972](../WorldManager.dog#L972); WorldDemandCache.dog struct [5](../WorldDemandCache.dog#L5); WorldGeneratedOccurrences.dog struct [94](../WorldGeneratedOccurrences.dog#L94) |
| `WorldSparseBatch` | Collects checked edits to one sparse source span and orders them by descending index for coordinated publication. | struct [653](../WorldManager.dog#L653) |
| `WorldSparseEdit` | One checked frame to publish at a particular logical index in a sparse span. | struct [649](../WorldManager.dog#L649) |
| `WorldStatus` | Snapshot of an operation's outcome and explanatory detail string. | struct [829](../WorldManager.dog#L829) |
| `WorldText` | Shared text-expression node borrowing an immutable source string/window or joining two text branches without eagerly copying bytes. | struct [58](../WorldManager.dog#L58) |
| `WorldTextCursor` | Stack-based reader over WorldText leaves, tracking the current source window and traversal visits. | struct [70](../WorldManager.dog#L70) |
| `WorldTextSpan` | A length-bounded region of a WorldText expression with a retained start offset. | struct [65](../WorldManager.dog#L65) |
| `WorldUnit` | Definition-derived homogeneous unit description: named unit, base/member unit, count and coordinate width. | struct [618](../WorldManager.dog#L618) |
| `WorldUnitLevel` | One traversal level when converting a source layout into unit-coordinate segments. | struct [632](../WorldManager.dog#L632) |
| `WorldUnitOrigin` | Retains the source and coordinate mapping needed to relate unit-based sparse selections to their original members. | struct [640](../WorldManager.dog#L640) |
| `WorldUnitSegment` | One represented or gap segment in unit coordinates, retaining its source occurrence, start and width. | struct [625](../WorldManager.dog#L625) |
| `WorldUnitSelection` | Resumable unit-coordinate selection, retaining requested span, traversed levels/segments and checked result for possible write-through. | struct [678](../WorldManager.dog#L678) |
| `WorldWalk` | Resumable logical-layout traversal across embedded and named structure, preserving scope, direction, limits, absence evidence and wait state. | struct [228](../WorldManager.dog#L228) |
| `WorldWalkLevel` | One level in a WorldWalk stack, retaining the next/adjacent occurrence and local coordinate/extent evidence. | struct [211](../WorldManager.dog#L211) |

## Demand-cache evidence

Main declarations: [WorldDemandCache.dog](../WorldDemandCache.dog) — 5 names.

| Class | Comment | Declarations |
|---|---|---|
| `WorldDemandHeaderIdentity` | Empty identity token for a particular description header used by demand-cache proofs; does not own the model. | struct [4](../WorldDemandCache.dog#L4) |
| `WorldDemandScopeProof` | Shared proof of one enclosing scope's description/occurrence identities and revisions, linked to its parent's proof. | struct [25](../WorldDemandCache.dog#L25) |
| `WorldDemandSessionIdentity` | Empty token distinguishing session lifetimes in cache and request-registration evidence without owning the session. | struct [3](../WorldDemandCache.dog#L3) |
| `WorldDemandSite` | Non-owning snapshot of a qualified demand position, including occurrence, parameters, input and write/embedding flags. | struct [12](../WorldDemandCache.dog#L12) |
| `WorldDemandValueProof` | Cache-validity evidence tying an acquired value to its site, scope, session, world, language, registry and production/write revisions. | struct [37](../WorldDemandCache.dog#L37) |

## Acquired-value retention

Main declarations: [WorldDemandRetention.dog](../WorldDemandRetention.dog) — 1 name.

| Class | Comment | Declarations |
|---|---|---|
| `WorldRetentionEntry` | One retained occurrence and its use token, stored in a demand's recent-value or checkpoint retention slots. | struct [7](../WorldDemandRetention.dog#L7) |

## Expanded output

Main declarations: [WorldExpandedPrint.dog](../WorldExpandedPrint.dog) — 1 name.

| Class | Comment | Declarations |
|---|---|---|
| `WorldExpandedPrint` | Resumable, bounded expanded-print operation using ordinary occurrence demands; retains emitted text, waiting state, truncation and completion status. | struct [3](../WorldExpandedPrint.dog#L3) |

## Generated occurrence identity, storage and writes

Main declarations: [WorldGeneratedOccurrences.dog](../WorldGeneratedOccurrences.dog) — 11 names.

| Class | Comment | Declarations |
|---|---|---|
| `WorldGeneratedAddress` | Identifies a generated ordinal within an owning scope, optionally retaining the represented source member's structural key. | struct [30](../WorldGeneratedOccurrences.dog#L30) |
| `WorldGeneratedAnchorIdentity` | Empty model-site identity token used to key generated occurrences without retaining the model through the key. | struct [4](../WorldGeneratedOccurrences.dog#L4) |
| `WorldGeneratedAssertions` | Container of durable generated-occurrence write scopes, separate from disposable generated-value caches. | struct [49](../WorldGeneratedOccurrences.dog#L49) |
| `WorldGeneratedBindingIdentity` | Stable identity for a declaration/application parameter environment, including whether the binding has been canonicalized. | struct [11](../WorldGeneratedOccurrences.dog#L11) |
| `WorldGeneratedDeclarationIdentity` | Empty declaration identity token preserved by declaration copies so generated scope keys need not own the declaration/model. | struct [5](../WorldGeneratedOccurrences.dog#L5) |
| `WorldGeneratedScopeKey` | Structural key for a represented or generated occurrence, combining anchor, parameter binding, ordinal and parent scope. | struct [16](../WorldGeneratedOccurrences.dog#L16) |
| `WorldGeneratedScopeMatches` | Memoized set of right-hand scope keys already compared with one left-hand key during scope comparison. | struct [27](../WorldGeneratedOccurrences.dog#L27) |
| `WorldGeneratedScopePair` | Pair of scope keys queued for structural comparison. | struct [23](../WorldGeneratedOccurrences.dog#L23) |
| `WorldGeneratedSeries` | Session-local generated history metadata: owning position, recent/leased occurrence caches, retention limits, allocation counters and production frontier. | struct [81](../WorldGeneratedOccurrences.dog#L81); WorldProductionFrontier.dog struct [72](../WorldProductionFrontier.dog#L72) |
| `WorldGeneratedWriteProof` | Validity/readiness evidence for a durable generated write's source description and header revision. | struct [37](../WorldGeneratedOccurrences.dog#L37) |
| `WorldGeneratedWriteScope` | Durable generated values and their proofs indexed by ordinal within one qualified owner scope, with a write revision. | struct [43](../WorldGeneratedOccurrences.dog#L43) |

## Deterministic temporal production

Main declarations: [WorldProductionFrontier.dog](../WorldProductionFrontier.dog) — 11 names.

| Class | Comment | Declarations |
|---|---|---|
| `WorldProductionBindingCopy` | Associates a live parameter environment with its detached snapshot and the production frontier that uses it. | struct [73](../WorldProductionFrontier.dog#L73) |
| `WorldProductionCheckpoint` | Production resume/replay point retaining the generated ordinal, segment position and checked occurrence value. | struct [16](../WorldProductionFrontier.dog#L16) |
| `WorldProductionDependency` | Snapshot of a production input's header revision, overlays, listSpec and actual arguments, used to detect stale production evidence. | struct [30](../WorldProductionFrontier.dog#L30) |
| `WorldProductionEvidenceIdentity` | Empty token identifying a particular valid production proof without retaining its frontier or model graph. | struct [39](../WorldProductionFrontier.dog#L39) |
| `WorldProductionFrontier` | Retains a deterministic temporal production program, dependencies, parameter scopes, checkpoints, wait/outcome state and work counters. | struct [40](../WorldProductionFrontier.dog#L40) |
| `WorldProductionHeader` | Retains one source POV and its description while the producer analyzes represented headers and their dependencies. | struct [26](../WorldProductionFrontier.dog#L26) |
| `WorldProductionPrefixLevel` | Stack entry for walking a represented prefix, retaining the current body and next member. | struct [22](../WorldProductionFrontier.dog#L22) |
| `WorldProductionRule` | One production requirement pairing its description with the parameter environment in which it must be interpreted. | struct [3](../WorldProductionFrontier.dog#L3) |
| `WorldProductionRun` | One resumable generated-occurrence demand, tracking target ordinal, segment, candidates, accepted value and production scratch state. | struct [100](../WorldProductionFrontier.dog#L100) |
| `WorldProductionScratch` | Temporarily saves outer request caches and parameter/context state while an isolated production step uses ordinary acquisition frames. | struct [78](../WorldProductionFrontier.dog#L78) |
| `WorldProductionSegment` | One structural production segment: recipe, parameter scope, requirements, optional represented source and finite or unbounded repetition count. | struct [7](../WorldProductionFrontier.dog#L7) |

## Detached checked views

Main declarations: [WorldCheckedViews.dog](../WorldCheckedViews.dog) — 1 name.

| Class | Comment | Declarations |
|---|---|---|
| `WorldCheckedViewWork` | Stack entry for building a detached representation of checked facts, retaining publication and member/alternative iteration state. | struct [22](../WorldCheckedViews.dog#L22) |

## Logical-layout proofs

Main declarations: [WorldListLayout.dog](../WorldListLayout.dog) — 2 names.

| Class | Comment | Declarations |
|---|---|---|
| `WorldListLayout` | Resumable proof of a list's logical structure and template obligations, including named layout ownership, outcome and parser wait. | struct [12](../WorldListLayout.dog#L12) |
| `WorldTemplateLayout` | Tracks the proof that a listSpec describes one occurrence, retaining source/scope, visited meanings and progress/outcome. | struct [4](../WorldListLayout.dog#L4) |

## Temporal acquisition context

Main declarations: [WorldTemporalAcquisition.dog](../WorldTemporalAcquisition.dog) — 1 name.

| Class | Comment | Declarations |
|---|---|---|
| `WorldTemporalFrontier` | Shared previous-state and list-continuation context for one logical temporal sequence, including embedded storage spans. | struct [3](../WorldTemporalAcquisition.dog#L3) |

## Finite temporal production

Main declarations: [WorldTemporalProduction.dog](../WorldTemporalProduction.dog) — 1 name.

| Class | Comment | Declarations |
|---|---|---|
| `WorldProducedList` | Private finite-production layout reserving introduced target occurrences and mapping represented members before their recipes are evaluated and published. | struct [3](../WorldTemporalProduction.dog#L3) |

## Weak ownership binding

Main declarations: [WorldPrivateMembers.dog](../WorldPrivateMembers.dog) — 1 name.

| Class | Comment | Declarations |
|---|---|---|
| `WorldWeakReference` | Generic native weak-pointer binding supplying set/clear/lock/expired operations; supports non-owning lifetime links, not solver behavior. | struct [3](../WorldPrivateMembers.dog#L3) |

## Files containing only extensions of the classes above

| File | Classes extended |
|---|---|
| [Functions.dog](../Functions.dog) | `GLOBAL` |
| [WorldDemandLayout.dog](../WorldDemandLayout.dog) | `WorldDemand` |
| [WorldDemandSelection.dog](../WorldDemandSelection.dog) | `WorldManager` |
| [WorldRecordProjection.dog](../WorldRecordProjection.dog) | `WorldCorrespondence`, `WorldCorrespondenceQuery`, `WorldCorrespondenceResult`, `WorldPOVGraph`, `WorldRequest` |

The semantic contract and current boundaries are recorded in the
[normalization design](<Design Docs/normalization-design.md>) and
[decision log](<Design Docs/normalization-design-decisions.md>).
