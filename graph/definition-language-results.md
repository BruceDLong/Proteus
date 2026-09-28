# Language-tagged definitions

The graph catalogue now registers declarations with usage tags. Selection uses
the existing ModelManager language rule: an untagged declaration applies in the
current context, and a tagged declaration applies when its first tag matches
the context's base language or full locale. A French declaration does not make
an otherwise unambiguous English declaration ambiguous. Two applicable meanings
remain unresolved. Additional qualifiers such as `<en name>` are retained as
unresolved meaning-selection obligations, not discarded.

A GraphNormalizer captures its vocabulary at initialization. `init()` and
`initWithRegistry()` use the system locale; `initForLanguage()` permits an explicit
language for an isolated context. Agent initializes its locale before world
loading. Changing language or rebinding existing applications inside a running
graph context remains part of the deferred context-binding design. No shared
WordDefn, WordUse, or definition body is rewritten during catalogue selection.

This is a prerequisite for the tagged cube/color definitions; it does not yet
implement their unordered or embedded definition composition.

Validation: full CodeDog regeneration/build passed; 15/15 new language tests
passed (base/region/foreign languages, ambiguity, qualifiers, replay, late
publication, cancellation, cost, isolated contexts, and the public Agent path).
All 1,142 graph tests pass. Compiled audit: 1,192 pass / 8 existing fail. Default
audit: 247 pass / 13 existing fail / 8 documented skips. No pre-existing status,
fixture, expectation, or ambiguity skip changed. Logs are in
`/tmp/p4-language-tag-compiled`, `/tmp/p4-language-tag-default`, and
`/tmp/p4-definition-language-build2.log`.

The historical implementation was checked in ModelManager.wordValidForLang and
indexDefinition. The available Proteus3 executable predates the current `#`
definition syntax, so it did not provide a valid runtime comparison for these
probes. No historical runtime success is claimed.
