# Proteus — A Practical Guide and Language Reference

*Proposed outline for the human-readable manual*

The opening chapters teach readers to write a description and ask what follows from it. The later chapters follow three models far enough to see how the choice of identities, states, and relationships affects the questions Proteus can answer.

## Part I — Reading and writing Proteus

1. **[What is Proteus?](Proteus-Guide.md#chapter-1)** Introduce a modeling language and the engine that reasons with its descriptions. Use a ticket with a known seat and an unknown row to distinguish what a model says, what follows, and what remains open. Explain how identities and relationships let descriptions contribute information to one another.
2. **[Proteus files, CLIP, and the lessons](Proteus-Guide.md#chapter-2)** Read `ticket.pr`, open CLIP with the world model or a lesson file, recognize prompts and expected facts, and distinguish a fresh template application from a query about `%my-ticket`. Practice editing the file and loading it again. Introduce the pattern used in later lessons.
3. **[Basic syntax — how to read a Proteus file](Proteus-Guide.md#chapter-3)** Walk through the quick-start listing line by line. Point out a declaration, its body, a field, a literal, and an unknown. Distinguish `#ticket` template access, `ticket` application and `%my-ticket` individual reference. Read `#my-ticket = ticket:{seat:12}` as construction and naming of one individual, explaining the roles of `#`, `=`, braces, and the two colons in context. Compare a model name with a quoted text literal. Cover comments, whitespace and separators, grouping and nesting, and the basic shapes of numeric and list expressions. End with a small edit the reader can make and run.
4. **Infons, identity, and partial knowledge.** Introduce possible states, selected values, and unknowns. Compare two tickets bearing the same seat number with two descriptions of the same ticket. Ask when learning the row in one place determines the row in the other.
5. **Numbers and their domains.** Read `*size+value`, beginning with a face that can occupy four orientations. Cover composition, decomposition, signs, ranges, and numerical resolution. Distinguish the number of possible states from the value currently selected.
6. **Lists and composition.** Build an ordered list, nest it, and embed its members with `&{...}`. Use small examples to distinguish ordinary lists, multiplicative/concatenating `**(...)` joins, and structural additive `++(...)` joins. Show which items become neighbors in each case.
7. **Descriptions, definitions, and references.** Describe a part by its properties, give the description a name, and refer to the part from a larger object. Introduce scope, paths, parameters, and argument binding as the examples need them.
8. **Constraints, alternatives, and repetition.** Use a list specification to constrain every member, an alternative to narrow the possibilities, and a neighbor reference to relate adjacent members. Show how a rule shared by a sequence can leave individual values open.
9. **Ask the engine what follows.** Run one relationship with different facts supplied. Compare a determined answer, several compatible possibilities, and incompatible descriptions. Show how to inspect the relevant part of the result or diagnostic.

10. **Worked model — what does “red” mean?** Resume this chapter when the revised kernel color model is ready. Define a color in terms of hue, saturation, and brightness (HSB), then give a name to a description that constrains some of those properties. Compare storing the text `'red'` on a face with applying the color description to that face. Ask for the face’s hue and brightness in each case. Use the color definitions from `kernel.pr` to inspect the range of hues around zero that qualify as red, and any constraints the model places on saturation and brightness. A particular sample adds measurements. Choose a hue within the range, full saturation, and full brightness, then express that specific red in RGB. Work backward from a nonblack gray and explain why its hue remains undetermined. Reuse these definitions when building the cube.

## Part II — Modeling discrete systems over time

11. **State, change, and verbs.** Start with one spindle in one of four phases. Describe its phase before a quarter-turn and after it, keeping the spindle's identity across both occurrences. Use a short T-list to introduce temporal order, reference to a previous state, and which properties persist. Give the turn a reusable verb definition with explicit participants. Compose two turns, then ask which single turn is compatible with a given pair of phases. Distinguish the order of the turns from their duration.

12. **Worked model — the tLang parser.** Choose one short input and a matching rule from the actual tLang grammar. Show the input and the structure it describes side by side. Follow one successful parse across successive states, recording the input position, the rule being matched, and the structure recognized so far. Pick the step where the structure first becomes identifiable and explain which constraint makes that possible. Change one input element and trace the two parses to their first difference. Use the parser's own rule names and operations in the published walkthrough.

13. **Worked model — the Rubix cube.** Begin with the six-spindle core in `rubixCube.pr`, its caps, and the four phases of each spindle. Give each cap a color using the descriptions from Chapter 10. Select the red-centered face, then ask for its hue through the cap’s color. Build up to a face turn by introducing the identities, positions, and orientations of the movable pieces. Follow one marked edge through a front turn and a top turn. Reverse the order and locate the same edge again. Use a turn followed by its inverse to check the model. End with an inference question: given two cube states and the constraint that exactly one face turned, which turns fit the observations?

## Part III — Modeling analog systems over time

14. **Quantities, scales, and trajectories.** Return to the turning face. Knowing the start and end phases leaves open when the turn occurred and how far it had progressed halfway through. Introduce an angle, a time interval, and an explicit motion assumption. Compare what the endpoints determine with what a constant angular speed adds. Use this example to explain units, bounded and cyclic ranges, sampling, and the resolution of a numeric infon.

15. **Color changing over time.** Return to the color model from Chapter 10. Take hue from 350° to 10° while holding saturation and brightness fixed at their maxima. Several paths share those endpoints. A 20° increase at constant speed puts the midpoint at 0°. Use the existing HSB–RGB relationship to read the midpoint in RGB. Then round the observations to byte values and ask which hues remain compatible with them.

## Part IV — Using Proteus as a reference

16. **Running and inspecting models.** File loading, interactive use, selecting results, reading diagnostics, and reproducing the worked examples. Organize entries by what the reader is trying to do.
17. **[Language reference](Proteus-Guide.md#chapter-17)** R01–R10 are now included as a derived chapter; detailed R05 call coverage remains task A04. Maintain the chapter's source separately in [Proteus-Language-Reference.md](Proteus-Language-Reference.md), so decisions can be recorded before the corresponding lessons are written. Group forms by purpose in terse tables: syntax/example, meaning, and usage where needed. Use one expression or declaration per example, with short notes for scope, identity, exceptions, and open questions. Include grouping and precedence, list forms, definitions and calls, references, temporal constructs, and numeric domains. Link each settled entry to its source decisions and record implementation status; leave rationale and history in the repository decision log. No walkthroughs or exercises.

## Appendices

- **A. Glossary.** Define terms such as *infon*, *identity*, *size*, *value*, *list specification*, *phase*, and *T-list*. Point each entry to its first worked example.
- **[B. One-page syntax cheat sheet](Proteus-Guide.md#appendix-b).** Added as Appendix B and as [Proteus-Cheat-Sheet.md](Proteus-Cheat-Sheet.md), with a one-page [Proteus-Cheat-Sheet.pdf](Proteus-Cheat-Sheet.pdf) for printing. Derive this from the separate language reference, arranging common forms by what the reader wants to do, with columns for purpose, spelling, and a tiny example. Link back to Chapters 2 and 3 for the walkthrough and Chapter 17 for full rules; do not maintain an independent set of semantic rules.
- **C. Example index and version notes.** Map modeling questions to runnable listings and record the engine revision used. Reserve word-rules for a later optional section on shorthand.

## Drafting the worked chapters

Give each case study a complete model and one small trace a reader can reproduce. Open with a question about the system, introduce each definition when the question needs it, and close by varying one known fact. When drafting the manual, run the listings against the documented engine revision and record its actual answers, including any information that remains unknown.

## Reference capture and tutorial alignment — 2026-11-24

The separate reference now contains R01–R10. R05 documents definitions, templates and individuals under D114–D115; its detailed parameter/call coverage remains task A04. The coverage map and source notes distinguish inspected implementation from open decisions and pending verification.

**A07 complete:** Chapters 2–3 of [Proteus-Guide.md](Proteus-Guide.md) distinguish reusable templates, constructed individuals and names bound to selected occurrences. `#word` reads a template, bare `word` applies it, and `%Name` refers to a bound individual. The corrected ticket queries use `%my-ticket`; the obsolete bare-name workaround is removed. Carry the same distinctions into Chapter 7 when it is written.

**A08 complete:** Chapter 17 is derived from the current R01–R10 entries, including their source notes and limits. Appendix B and the Markdown/PDF cheat sheet share a compact set of forms for grouping, explicit joins, bracketed selection, naming, constraints, repetition, temporal views and coordinates. Chapters 4–16 and the other planned appendices are still unwritten.

**Execution remains A09:** The revised introductory tables state expected facts, not newly captured CLIP replies. Run the lesson listings and reference examples against the documented engine revision before publication. The World kernel check remains A23.
