# Proteus — A Practical Guide and Language Reference

## Contents

| Chapter | Section |
| --- | --- |
| [1](#chapter-1) | [What is Proteus?](#chapter-1) |
| [2](#chapter-2) | [Proteus files, CLIP, and the lessons](#chapter-2) |
| [3](#chapter-3) | [Basic syntax — how to read a Proteus file](#chapter-3) |
| [17](#chapter-17) | [Language reference](#chapter-17) |
| [Appendix B](#appendix-b) | [One-page syntax cheat sheet](#appendix-b) |

Chapters 4–16 remain in the [guide outline](Proteus-Guide-Outline.md).

<a id="chapter-1"></a>

## Chapter 1. What is Proteus?

You have a theater ticket. The seat number is legible: 12. The row is smudged. You can answer someone who asks “Which seat?” even though you can’t yet tell the usher where to take you. Later, you find the booking confirmation for that ticket. It gives the row as 3. Between them, the ticket and the confirmation tell you where to sit.

Proteus is a language for describing things and the relationships among them. The Proteus engine takes those descriptions and works out what follows from them. You can give it what you know now, ask a question, and get an answer that still contains unknowns.

### What goes into a model?

To find your seat, two numbers will do. A model of the same ticket for the box office might also include the performance, the price, and the person who booked it. In Proteus, you choose the distinctions that matter for the questions you want to ask.

Here is how to describe what we know about our ticket:

```text
ticket:{row:_, seat:12}
```

Read it aloud as “a ticket has a row and a seat. The row is an unknown number and the seat is 12.” The underscore lets us say something quite specific about the ticket even before we know both numbers.

Proteus calls a unit of information an *infon*. Here, the seat number is an infon. The ticket as a whole is also an infon, and its description tells us how to find the seat within it. That lets us ask about the whole ticket or about just one of its parts.

### What makes an inference possible?

Imagine finding a second booking confirmation that also says “seat 12.” Several rows have a seat 12, so that coincidence alone gives you no reason to take the row from that confirmation. What mattered in the original example was knowing that the confirmation described **the same ticket**. When two descriptions identify the same thing, the facts in one can answer questions left open by the other.

A relationship can also tell you something about *different* things. Suppose your tickets are in the same row, your companion sits immediately to your right, and seat numbers increase by one in that direction. Knowing your seat is 12 determines your companion’s seat as 13. Knowing only your companion’s seat is 13 would determine yours as 12. The relationship is the same whichever number you start with.

Later we’ll use Proteus to model a Rubik’s Cube, a puzzle made of pieces that move when you turn a face. Look at the front face and pick out the corner piece at its upper right. Give that face a clockwise quarter-turn. The same piece is now at the lower right. A model that describes how the turn moves each corner can tell us where our piece goes from its starting position.

First, let’s put the ticket description in a file and ask Proteus about it.

<a id="chapter-2"></a>

## Chapter 2. Proteus files, CLIP, and the lessons

A Proteus model can be saved in a plain text file with the extension `.pr`. You can read and edit it in a text editor. **CLIP** is the command-line interface to Proteus. It can load the world model, which supplies shared descriptions, or a file you choose for a particular session. The lessons assume you already have CLIP.

### Read a lesson file

Here is the complete file for this lesson, `ticket.pr`:

```text
{
    #row = _
    #seat = _
    #ticket = {row, seat}

    #my-ticket = ticket:{seat:12}
}
```

The outer braces group the declarations in the file. The first two declarations define reusable numeric templates, `row` and `seat`. The third defines a reusable `ticket` template with one of each. The last line applies `ticket`, supplies `seat:12`, and binds the resulting individual to the name `my-ticket`. Its row remains unknown.

The distinction matters when you ask a question: `ticket` applies the template, while `%my-ticket` refers to the individual already named in the file. To read the template itself, use `#ticket`. Thus `#` introduces a declaration in `#ticket = ...` and reads a template in `#ticket`; it is not limited to defining words.

### Open CLIP and ask a question

For these lessons, use **command mode**. From the directory where you normally run CLIP, you can open it with the standard world model using this shell command:

```sh
clip -m command
```

At the `clip>` prompt, type `quit` to return to your shell. To use the ticket model, put `ticket.pr` in the `assets` directory under the directory where you run CLIP, then start a session with this command:

```sh
clip -m command -w ticket.pr
```

If you normally invoke the executable as `./clip`, use that spelling in both commands. The `-w` option selects the lesson file as the world for this session. Once the startup messages finish, enter each query below at the prompt and press Enter.

| Query | Facts to look for |
| --- | --- |
| `ticket` | An application of the ticket template, with unknown row and seat. |
| `ticket:{seat:12}` | A new application with seat 12 and an unknown row. |
| `%my-ticket` | The individual named in the file, with seat 12 and an unknown row. |
| `%my-ticket.seat` | That individual's seat, with value 12. |

A dot selects a part of a description. The last two queries ask about the same named ticket. A separate application such as `ticket:{seat:12}` does not identify its result with `%my-ticket` merely because their seat values agree.

### Try a fact in a query, then save it in the model

Suppose the booking confirmation arrives. You can try a description containing its row:

| Query | Facts to look for |
| --- | --- |
| `ticket:{seat:12, row:3}` | A new ticket application with both facts supplied. |
| `ticket:{seat:12}` | Another application, whose row is still unknown. |
| `%my-ticket.row` | The named ticket's row is still unknown too. |

The first query describes a fresh application; it does not assert the row of `%my-ticket`. To keep the row in the lesson file, change its last declaration to:

```text
#my-ticket = ticket:{row:3, seat:12}
```

Save the file, type `quit`, then run `clip -m command -w ticket.pr` again. In the new session, `%my-ticket.row` refers to the row established as 3 by the edited file. Restarting loads a new model; it does not rebind an existing name inside the previous session.

### How to work through the lessons

Each lesson begins with a question and tells you which model to load. Before entering a query, predict as much of its answer as you can. Pay attention to which facts come from the file, which ones you supplied in the query, and which values remain open. The closing exercise changes one fact or relationship so you can see what depended on it.

**Try it.** Two people hold tickets for seat 12 at the same performance. In `ticket.pr`, add `#other-ticket = ticket:{row:8, seat:12}` just before the closing brace. Save the file and reopen the session. Predict the answers to `%my-ticket.seat` and `%other-ticket.seat`. Then choose a query that will tell the tickets apart.

*Example status: Chapters 2–3 are aligned with the language reference and the naming/join source fixtures at Proteus `2436d02f`. Tables state expected facts rather than captured CLIP output. Running the lesson listings and recording exact replies remains reference task A09.*

<a id="chapter-3"></a>

## Chapter 3. Basic syntax — how to read a Proteus file

How much does the line `#my-ticket = ticket:{seat:12}` tell you? You can see the seat number right there. To find out what else the line tells you, follow `ticket` back to its definition.

Start with the original `ticket.pr` from Chapter 2. If you did the exercise, keep your edited copy and use the listing below for this lesson.

```text
{
    #row = _       // A row number
    #seat = _
    #ticket = {row, seat}

    #my-ticket = ticket:{seat:12}
}
```

### Read a declaration from the outside in

In `#row = _`, `#` introduces the declaration of `row`. The expression to the right of `=` is its *body*. Here the literal body `_` defines a reusable template for an unknown number. The next line defines `seat` the same way. The text from `//` to the end of the line is a comment.

In `#ticket = {row, seat}`, the literal body is a list enclosed in braces. Its members apply the two descriptions we just defined. The comma makes this an unordered description: a row and a seat can be matched by what they are. That's why `ticket:{seat:12, row:3}` and `ticket:{row:3, seat:12}` describe the same seating information. That agreement alone does not identify two separate ticket applications.

Now read the last declaration aloud: **“Construct a ticket whose seat is 12, and name that individual my-ticket.”**

| Part | How to read it here |
| --- | --- |
| `#my-ticket =` | Bind the name `my-ticket` to the result on the right. The hyphen is part of the name. |
| `ticket` | Apply the reusable ticket template, supplying its row and seat fields. |
| `:{…}` | Qualify the ticket with the description inside these braces. |
| `seat:12` | Describe the seat field with the literal number `12`. |

The outer colon applies the whole braced description to the ticket. The inner colon applies `12` to its seat. You can now predict a detail that appears nowhere in the last line: `%my-ticket` has a row because the ticket template supplies one.

### Read a template, apply it, or refer to an individual

| Form | Meaning here |
| --- | --- |
| `#ticket` | Read the reusable template without applying it. |
| `ticket` | Apply the template. |
| `%my-ticket` | Refer to the individual bound by the declaration above. |
| `#other-template = #ticket` | Bind another name to the reusable template. |
| `#same-ticket = %my-ticket` | Bind another name to that same individual. |

The right-hand expression determines what a declaration binds: a literal body such as `{row, seat}` defines a template, while `ticket:{seat:12}` constructs an individual. A declaration can also name an individual selected from context, as in `#chosen-ticket = %ctx.ticket`. Once established, that binding keeps its selected occurrence even when later queries change the session context.

Individual aliases share facts because they refer to the same occurrence. Applying the template again produces a separate occurrence; equal seat numbers do not make the applications identical. Later references to our named ticket use `%my-ticket`, and `%my-ticket.seat` selects its seat.

### A name, a literal, and a number with a size

With `ticket.pr` loaded, compare `'ticket'` with `ticket`. The quotes enclose a *string literal*: the six letters you typed. The bare word applies the ticket template from this model. `#ticket` instead reads that template without applying it.

The number `12` is also a literal. Proteus lets you state a numeric size explicitly. In `*4+2`, the size is four and the selected value is two. Under a discrete four-state interpretation, read this as “value 2 among four possibilities.”

Imagine a theater with entrances numbered 0 through 3. If the model requires whole-number readings, `*4+_` leaves the entrance value open among those four states. The size alone does not impose that whole-number requirement: numeric infons can also have rational readings. To list exactly these four choices explicitly, write `*_+[0 1 2 3]`. Chapter 17, R02–R03 and R08, gives the fuller rules.

### Read the delimiters

Braces can nest. In `{{3 12} {8 12}}`, the outer list has two members. Each member is itself a list of two numbers. Spaces separate the numbers, and the inner braces tell you which numbers belong together. Line breaks and indentation can make the file easier to read; the braces still determine its structure. A comma carries the unordered meaning you saw in `{row, seat}`. Without commas, the type or context determines whether correspondence is ordered.

Ordinary parentheses group **one** expression. Joins use an explicit opener:

| Expression | Meaning / expected value |
| --- | --- |
| `{'Box ' 'office'}` | A list with two string members. |
| `('Box ')` | The single grouped string `'Box '`. |
| `**('Box ' 'office')` | Join the strings into `'Box office'`. |
| `++(*4+2 *3+1)` | A structural numeric sum, giving `*6+3`; see Chapter 17, R03–R04. |

The space *inside* `'Box '` belongs to the text and survives the join. The space *between* the quoted strings separates the operands. A join uses whitespace-separated operands; `(A B)` is not a join spelling. `**(...)` composes numeric, string or list operands according to their type, while multi-operand `++(...)` requires whole numeric operands.

### Try it — add a section to the ticket

Make a copy of the original lesson file called `ticket-section.pr`. Suppose the theater wants to print a section name such as “Balcony” on each ticket. Add `#section = $` above the ticket definition. The dollar sign stands for an unknown string. Then change the template declaration to `#ticket = {row, seat, section}`. Leave the declaration of `my-ticket` as written.

Load the copy with `clip -m command -w ticket-section.pr`. Query `%my-ticket`. Predict whether the new field will appear and what value it will have. Then compare:

| Query | Facts to look for |
| --- | --- |
| `%my-ticket.section` | The named ticket has a section field whose string value is unknown. |
| `ticket:{seat:12, section:'Balcony'}` | A fresh application with seat 12, section `'Balcony'`, and an unknown row. |
| `%my-ticket.section` | Still unknown: the separate application did not add a fact to this individual. |

Every application receives the section field because you put it in the ticket template. Supplying the field's value for one application does not supply it for another. To keep `'Balcony'` on `%my-ticket` in this lesson, add it to that individual's initializer in the file, then reload as in Chapter 2.

<a id="chapter-17"></a>

## Chapter 17. Language reference

This chapter derives its entries from the separately maintained [Proteus-Language-Reference.md](Proteus-Language-Reference.md). Update that source when a rule changes, then refresh this chapter and Appendix B. Chapters 4–16 remain outlined; their absence here does not change the reference's numbering.

**Coverage and status.** R01–R04 and R06–R10 are documented; R05's naming rules are present, while its detailed parameter and call reference remains task A04. Source and fixture expectations were inspected through Proteus `2436d02f`; this documentation pass did not rerun the engine. R10 explicitly distinguishes inspected bundled models from World kernel vocabulary pending source verification.

References below to **Appendix A** or task IDs such as **A09** mean the [remaining-work appendix in the separate language reference](Proteus-Language-Reference.md#appendix-a). The practical guide's planned Appendix A is its glossary. The entries retain their source evidence and implementation limits.

### Reference contents

| Entry | Subject | Coverage |
| --- | --- | --- |
| [R01](#r01) | [Lexical forms, comments, separators, grouping, precedence](#r01) | Documented below; checked through Proteus `2436d02f`; string escapes checked through `d450632` |
| [R02](#r02) | [Infons, literals, unknowns, size and value](#r02) | Documented below; checked through Proteus `2436d02f`; string escapes checked through `d450632` |
| [R03](#r03) | [Numeric operations, domains, ranges and resolution](#r03) | Documented below; checked through Proteus `2436d02f` |
| [R04](#r04) | [Ordered/unordered lists, joins, embedding and additive lists](#r04) | Documented below; checked through Proteus `2436d02f` |
| [R05](#r05) | [Definitions, templates, named individuals, parameters and scope](#r05) | Naming rules below; detailed call syntax to document |
| [R06](#r06) | [Selection, paths, indices and contextual references](#r06) | Documented below; checked through Proteus `2436d02f` |
| [R07](#r07) | [Identity, qualification, constraints and contradictions](#r07) | Documented below; checked through Proteus `2436d02f` |
| [R08](#r08) | [Alternatives, list specifications, repetition and folds](#r08) | Documented below; checked through Proteus `2436d02f` |
| [R09](#r09) | [Temporal lists, states, neighbor references, supports and verbs](#r09) | Documented below; checked through Proteus `2436d02f` |
| [R10](#r10) | [Units, scales and model-defined vocabulary](#r10) | Documented below; checked through Proteus `2436d02f`; World vocabulary source check remains A23 |

<!-- BEGIN DERIVED REFERENCE: R01-R10 -->
<a id="r01"></a>

### R01 — Lexical forms, comments, separators, grouping and precedence

#### Lexical forms

Examples are Proteus source, without the escaping used by the repository's `.tst` file format. `A`, `B` and `C` below stand for arbitrary infon descriptions.

| Syntax/example | Meaning / usage |
| --- | --- |
| `ticket`, `my-ticket`, `actor_1`, `person's` | Single word tokens. Start with a Unicode letter; continue with letters, decimal digits, combining marks, `_`, `-` or `'`. Spaces end the token. |
| `#ticket`, `ticket`, `%my-ticket` | Template access, application and individual reference are distinct forms; see R05. `#` also introduces declarations. |
| `#ident(item_1) = %item_1` | Parameter names and named-argument labels use the narrower letter-first, letters/digits/underscore form. Use ASCII spellings for these headers. |
| `42`, `0.25`, `3/4`, `0x2A`, `0b101010` | Decimal integer, decimal fraction, rational, hexadecimal and binary forms. A leading zero alone stays decimal: `010` means ten. |
| `-3`, `+3`, `*10+3`, `/10-3` | Signs and size/value markers belong to the infon description. Their numeric meanings are specified in R02–R03. |
| `'text'`, `"text"` | Single- and double-quoted string literals. The other quote character can appear directly inside. |
| `_`, `$`, `?` | Unknown number, unknown string and unknown type/value respectively. These are distinct tokens. |
| `%?` | One contiguous temporal-independence marker; `{% ?}` instead contains a self reference followed by an unknown. See R09. |
| `NULL:` | Null occurrence. An optional numeric identity label is contiguous: `NULL:42`; `{NULL: 42}` has two members. |
| `// note` | Comment through the end of the line. |
| `/* note */` | Block comment, ending at the first `*/`; blocks do not nest. |

**Spelling and case.** Ordinary words use Unicode normalization with case folding for lookup; their authored spelling is retained. Parameter names and argument labels are case-sensitive: `item` and `Item` differ. Reserved forms such as `%W`, `%U`, `%INSERT`, `%PROTEUS`, `^T` and `^S` require their stated case. A hyphen inside a word is part of that word: `item-1` is one token; use `**(item -1)` for a separate numeric displacement.

**Numbers.** Keep the digits of a decimal or rational together. Write `0.5` for a fractional literal. Scientific notation is outside the numeric token grammar.

#### Quoted string escapes

Proteus decodes the same escapes in single- and double-quoted literals before counting their bytes or publishing their values.

| Syntax | Decoded value |
| --- | --- |
| `\\`, `\'`, `\"`, `\?` | The named character. |
| `\a`, `\b`, `\f`, `\n`, `\r`, `\t`, `\v` | The corresponding C control byte. |
| `\0`, `\377`, `\xFF` | One byte. An octal escape consumes one to three digits. A `\x` escape consumes one or more hex digits up to the first non-hex character. The value must be at most 255. |
| `\u00E9`, `\U0001F642` | One Unicode scalar encoded as UTF-8. `\u` takes exactly four hex digits and `\U` exactly eight. |
| Backslash followed by a physical LF or CRLF | Continues the literal on the next line. The backslash and line break contribute no bytes. |

Unknown or incomplete escapes, byte overflow, Unicode surrogates, and values above U+10FFFF are syntax errors. `'\u00E9\U0001F642'` has the same six bytes as `'é🙂'`. Printing uses single quotes and preserves valid UTF-8. It escapes the delimiter, backslashes, and control bytes. Other bytes outside printable ASCII and valid UTF-8 sequences use fixed three-digit octal escapes.

**Implementation:** D135 in Proteus `d450632`. See the [reader and printer](https://github.com/BruceDLong/Proteus/blob/d450632365d51e6d528724e56c1a4f18ff10194d/ProteusStringLiteral.dog) and [string escape fixtures](https://github.com/BruceDLong/Proteus/blob/d450632365d51e6d528724e56c1a4f18ff10194d/Tests/stringEscapeTests.tst).

See the [accepted long-string design](Proteus-Language-Reference.md#long-string-literals-accepted-design) for A03.

#### Whitespace, separators and grouping

Spaces, tabs and line breaks separate tokens; indentation does not delimit a block. Comments act as separators where whitespace is accepted. Line breaks do not ordinarily terminate expressions; `%INSERT` is an exception because its resource specification extends to the end of the line. Keep multi-character tokens intact, including `**(`, `++(`, `..(`, `@[`, `%?`, `::`, `==`, `=:`, `==:`, `...`, `<<` and `>>`. Numeric operation prefixes such as `(.mod` are also contiguous; whitespace/comments may follow the operator name.

| Syntax/example | Meaning / usage |
| --- | --- |
| `{1 2 3}` | A literal list with space-separated members. Absence of commas leaves ordering to the type or context; it does not itself force ordered correspondence. |
| `{row:1, seat:12}` | Commas explicitly select unordered, descriptive member correspondence. They are semantic, not optional formatting. |
| `(A)` | Group exactly one infon, including its relationships and selectors. Grouping introduces no cursor shift; `()` and `(A B)` are invalid. |
| `**(A B)` | Multiplicative join/concatenation of an ordered operand list. Numeric, string and list operands have their respective composition rules. See R03–R04. |
| `++(*4+2 *3+1)` | Structural additive join, yielding `*6+3`. A multi-operand sum requires whole numeric operands. See R03–R04. |
| `++(&{*4+2 *3+1} *2+1)` | Both joins accept ordinary list specifications and `&` embeddings; embedded members become operands in place. This sum yields `*7+4`. |
| `*6+ ++(~2\| *4+2 *3+1)` | The inner `~2` constrains the operand count; the outer `*6` constrains the result extent. |
| `..(0 1)` | Closed rational interval with two endpoint expressions, including both endpoints. It imposes no infon size; distinct from the three-dot `...` span token. See R03. |
| `[A B]` | An intersection/selection body; not interchangeable with braces or parentheses. `*_+[1 2]` describes finite alternatives; see R08. |
| `{*256+_ \| 1 2}` | `\|` separates a list specification from its members. It is not a general member separator. |
| `{1 &{2 3} 4}` | `&` embeds the following list's members into the containing list. See R04 for correspondence and R05 for scope. |
| `{1 ...}` | `...` is an actual unknown/open span token. It is not merely prose shorthand. |
| `give(1, 2)` | An attached `(` starts a call. Commas separate arguments and do not make the call unordered. |
| `{give (1)}` | A space before `(` leaves a word application and a separate parenthesized expression. An intervening comment also breaks attachment. |
| `#give (actor, item) = {%actor %item}` | Declaration headers permit whitespace/comments before and inside the parameter list. Call attachment rules do not apply to this header. |
| `{_ 2}@[1]=1;` | `;` retains the selection's source after constraining the selected occurrence. It is a path operator, not a general statement terminator. See R06–R07. |

Join and interval operands are whitespace-separated, not comma-separated. Commas are also rejected in `[A B]` bodies. Call/header commas are a separate grammar; trailing commas there are rejected. A join is already one infon and can be passed directly: `give(**(1 +2))`.

A bare `**()` yields `?`; a known result kind can supply its empty identity. `++()` yields `*1+0`. An unembedded list remains one operand; `&%items` supplies its logical members. These joins require an ordered operand list, including their embedded sources.

#### Numeric operation forms

The following are fixed-arity numeric forms, introduced by a contiguous `(.name` prefix. Their operands are complete infon expressions separated by whitespace. They use no call commas or named-argument labels. Detailed domain and partial-result rules belong in R03.

| Syntax/example | Meaning / usage |
| --- | --- |
| `(.common-denominator {1/4 1/6})` | One operand: a finite list of rational observations. Returns the least common denominator of their reduced values; here `12`. |
| `(.mod -9/4 1)` | Two operands: coordinate and positive period. Returns the exact residue in `[0, period)`; here `3/4`. |
| `(.affine 3/4 -1/2 1/2)` | Three operands: coordinate, gain and offset. Computes gain × coordinate + offset; here `1/8`. |
| `(.positive-multiple 24 12)` | Two operands: count and factor. Checks positive integral operands and exact divisibility, returning the checked count; here `24`. |

#### Binding and precedence

Read Proteus by the following syntactic boundaries. There is no conventional infix multiplication-before-addition table for joins: each operand has its own size/value description, and a join combines its operands in sequence.

| Syntax/example | Binding / meaning |
| --- | --- |
| `*(**(2 +3))+1` | The grouped join supplies the size; the final `+1` supplies the value. |
| `*10+(**(2 +3))` | The grouped join supplies the value of a size-10 infon. The join can also be written directly in the slot: `*10+**(2 +3)`. |
| `**(*8+4 *4+3)` | Compose two complete numeric operands, yielding `*32+19`; the `+` signs belong to those operands. |
| `**(1 +2 *4+3)` | Combine operands from left to right: apply the `+2` displacement before composing the `*4+3` operand. Nest explicit joins to change composition boundaries. |
| `-(3)` | Negate the selected numeric value of the grouped operand; yields `-3`. |
| `-(*10+3)` | A displacement of `-3` retaining its ten-state source facts. It is not automatically reinterpreted as `*10+7`; `*10-(*10+3)` supplies that receiving extent. |
| `-({1 2}@[1])` | Select the number inside the group, then negate it; yields `-1`. `-({1 2})@[1]` instead attempts to negate a list and rejects. |
| `give(1).x` | The call is formed before selecting `x` from its result. |
| `cube^T@[1]^S.location` | Postfix views and bracketed selections continue on each preceding result, in written order. |
| `A = B = C` | Attach both identity constraints to `A`; this is not assignment associativity. |
| `A = B.x` | The right-hand description includes `B.x`. |
| `(A = B).x` | Select from the grouped, constrained description. |
| `A.x = B` | The constraint belongs to the selected `x`; adding `;` retains the source as above. |
| `A == (B = C)` | Preserve the qualified outer relationship and the inner constraint; flattening can change the meaning. See R07. |
| `[A] <~ B = C` | `= C` constrains the selection expression's result. To constrain its input, group that input: `[A] <~ (B = C)`. |

#### Sources and status

**Coverage:** R01 documented and refreshed after pulling Proteus `main` on 2026-11-24. This entry records syntax and binding; the referenced R02–R10 entries own the detailed operations and their runtime limits.

**Migration:** D129 replaces multi-operand `(A B)` with `**(A B)` and reserves `(A)` for grouping. For example, `(%item +1)` becomes `**(%item +1)`. Reciprocal syntax such as `/4+0` already existed; D131's exact rational evaluation belongs in R03.

**Implementation evidence:** Proteus `main` at [`2436d02f`](https://github.com/BruceDLong/Proteus/tree/2436d02f73d88db371efb364418aa65d86d26fef), particularly [the parser](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/infonIO.dog), [word normalization](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/ModelManager.dog), [default language fixtures](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/defaultTests.tst), [call fixtures](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/wordArgumentTests.tst), [path-constraint fixtures](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/path-write-language-tests.tst), [ordered joins and negation](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/orderedJoinTests.tst), [closed ranges](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/closedRangeTests.tst) and [numeric operation fixtures](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/rationalScaleTests.tst). Lexical helper details were checked in CodeDog at `f848e0c6`: [IOBuffer](https://github.com/BruceDLong/CodeDog/blob/f848e0c67df5f423975da583dec4db42448bd35c/LIBS/IOBuffer.Lib.dog), [Unicode normalization](https://github.com/BruceDLong/CodeDog/blob/f848e0c67df5f423975da583dec4db42448bd35c/LIBS/Unicode.CPP.Lib.dog) and [GNU word characters](https://github.com/BruceDLong/CodeDog/blob/f848e0c67df5f423975da583dec4db42448bd35c/LIBS/Unicode.CPP.GNU.Lib.dog).

**Source decisions:** D02D (grouped relationships), D02K–D02M (composition and size expressions), D101–D102 (headers, calls and postfix views), D114–D115 (template/individual forms), and D129–D134 (explicit joins, grouped negation, rational coordinates and domains), in the [decision log](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design-decisions.md). Sources and existing fixture expectations were inspected; no parser build or test rerun was performed for this documentation update. Parser acceptance alone does not establish runtime support.

<a id="r02"></a>

### R02 — Infons, literals, unknowns, size and value

An infon description supplies facts about a piece of information, including its size and value. The explicit form `*s+v` supplies size `s` and value `v`; either may be unknown or described by an expression. Numbers, strings and lists provide different value representations.

#### Literals and explicit sizes

| Syntax/example | Meaning / usage |
| --- | --- |
| `34` | Numeric value 34; the numeral alone does not establish its size. |
| `*64+34` | Numeric size 64 and value 34. In a discrete 64-state interpretation, select state 34, counting from zero. |
| `*_+34` | Numeric value 34 with explicitly unknown size. |
| `0.25`, `1/4` | Two spellings of the same exact rational value. Decimal notation does not imply floating-point approximation or measurement uncertainty. |
| `0xFF`, `0b11111111` | Numeric value 255 in hexadecimal or binary. Neither spelling supplies a byte width or a size of 256. |
| `*1+1/4` | A rational reading of one quarter with numeric extent 1. Numeric extent alone does not require a whole-number value. |
| `'Hello'`, `"Hello"` | The same five-byte text value, with size counted from the literal. |
| `*5+'Hello'` | The text with an explicitly supplied size of 5. |
| `{1 2}` | A closed list with two immediate members; its count-derived size is 2. List ordering is covered in R04. |
| `{{1 2} 3}` | Two immediate members: a nested list and the number 3. The nested list is one member at this level. |

`+34` is the contextual displacement form, distinct from bare `34` when composing numbers. A signed value can remain usable before its size is known: `-1` remains negative, while `*10-1` normalizes to `*10+9`. Signs, reciprocal sizes, composition and numeric domains are specified in R03.

#### Unknown and incomplete descriptions

| Syntax/example | Meaning / usage |
| --- | --- |
| `_` | Unknown numeric value; no size supplied. |
| `*_+_` | Numeric description with both size and value explicitly unknown. |
| `*64+_` | Known numeric size 64; value unknown. |
| `$` | Unknown string content and size. |
| `*4+$` | A four-byte string whose content is unknown. |
| `?` | Type and value unspecified; later facts may establish a number, string or list. |
| `{_ _}` | Two numeric members with unknown values. The two occurrences do not assert equal values or identity. |
| `{...}` | A list with an unspecified span of members and no established total count. |
| `*3+{...}` | A list with total extent 3 whose members have not been supplied. |

Unknowns record missing information; they are not default values or instructions to choose arbitrary values. Compatible identities can supply facts: `_ = 7` establishes 7, and `*16+_ = *16+9` establishes the sized value. Reusing a reference preserves the referenced occurrence and its constraints; writing another `_` does not create that connection. See R05–R07.

#### Size and completion rules

- **Representation matters.** Numeric size describes numeric extent; string size currently counts bytes, not Unicode code points or displayed characters; list size counts logical members. These are not interchangeable counts. Embedded `&{...}` contributes its members to the enclosing list's count. An unordered description may omit members, so its written member count does not establish its total size.
- **Given versus counted.** A closed string or count-derived list can supply its size by counting. An explicitly unknown size remains unknown merely from inspecting the payload: `*_+{1 2}` retains unknown size; `*_+'Hello'` has available text without an established string endpoint. Other constraints may supply the missing size.
- **Bounds versus size.** A numeric value or bounded coordinate interval does not determine a state count or subdivision density. For example, `..(0 1)` supplies inclusive coordinate bounds without supplying an infon size. Whole-state requirements and rational ranges are separate constraints; see R03.
- **Completion versus full knowledge.** Ordinary numeric normalization may finish with known value and unknown size once the supplied constraints have been checked. An operation that needs size must still establish it. An unknown-size string still needs endpoint evidence; unknown values and unfinished lists are not made complete by the numeric rule.
- **Unknown versus empty.** `0` is a known numeric zero, `''` an empty string, `{}` an empty list, and `NULL:` an explicit null occurrence. None is the unknown token `?`. `NULL:` has zero size and is distinct from a numeric zero whose size is unspecified.
- **Printed form.** Compact output may omit a counted or checked size. A shorter printed description does not mean the engine discarded the established size facts.

#### Sources and status

**Coverage:** R02 documented on 2026-11-24 against Proteus `2436d02f`, with quoted string escapes updated through `d450632`. This entry covers literal descriptions and partial knowledge. Operations, list correspondence, qualifications and identity rules continue in R03–R08.

**Implementation evidence:** [Parser](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/infonIO.dog), [size counting and null representation](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Proteus.Lib.dog), [literal and unknown fixtures](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/defaultTests.tst), [numeric literals](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/numeric-literal-tests.tst), [print modes](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/print-mode-tests.tst), [scaled rational readings](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/scaledPhaseTests.tst) and [closed ranges](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/closedRangeTests.tst).

**Source decisions:** D02F (numeric completion with unknown size), D02I–D02L (scalar facts, endpoints and representations), D03E (closed-list extent), and D131–D134 (rational readings, bounds and whole-state domains), in the [decision log](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design-decisions.md), with the [normalization contract](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design.md). Source and fixture expectations were inspected; examples were not rerun for this documentation edit.

<a id="r03"></a>

### R03 — Numeric operations, domains, ranges and resolution

Numeric readings use exact integers and rationals. Operations preserve applicable size, domain and identity constraints. A known reading does not imply a known size, subdivision count or measurement precision.

#### Composition, addition and signs

| Syntax/example | Result / meaning |
| --- | --- |
| `**(*4+2 *3+1)` | Multiplicative composition: `*12+7`. |
| `**(*4+2 +(*3+1))` | Apply the second operand's value as a displacement: `*4+3`. Its size does not multiply the receiving extent. |
| `++(*4+2 *3+1)` | Discrete structural addition: `*6+3`. Both operands contribute to the result's extent. |
| `**(1/2 +1/4)` | Add rational readings: `3/4`; size remains unspecified. |
| `**(*4+1 /4+0)` | Scale by the reciprocal radix: `*1+1/4`, possibly printed as `*1+0.25`. |
| `-(3)` | Negate the selected numeric value: `-3`. |
| `-(*10+3)` | A free displacement of −3 retaining its ten-state source facts. Double negation recovers `*10+3`. |
| `**(*12+8 -(*10+3))` | Apply the negative displacement in the join: `*12+5`. |
| `*10-(*10+3)` | Interpret −3 in the receiving extent 10: `*10+7`. |
| `*(**(2 +3))+1` | Use the expression's selected value as the size: `*5+1`. |
| `++()` | Empty structural sum: `*1+0`. |
| `**() = _` | Numeric empty-product identity: `*1+0`. Bare `**()` has no established result kind and yields `?`. |

For two ordinary numeric operands with sizes `sA`, `sB` and readings `vA`, `vB`, multiplicative composition gives size `sA × sB` and reading `vA × sB + vB`. A contextual `+(B)` or `-(B)` instead shifts the accumulated reading. Joins process operands in order; ordinary `(A)` groups one infon. Embedding and operand-list rules are in R04.

For discrete structural addition, the known-size result is `1 + Σ(si − 1)` and its reading is `Σvi`. Multi-operand sums require whole numeric readings and, when supplied, positive whole sizes. Missing sizes are not invented. A contextual sign does not turn `++(...)` into displacement-only addition; the opener selects the operation.

**Negative offsets are not general modulo.** The current receiving-extent rule adds the extent once to a negative reading: `*10-1` becomes `*10+9`, but `*10-12` becomes `*10-2`. Positive overflow is not automatically reduced: `*4+4` remains `*4+4`. Use a range constraint to check admissibility and `(.mod x p)` to request wrapping.

**Computed sizes.** A computed size after `*` currently needs a known nonnegative whole result. A direct rational radix such as `*1/3+0` is supported; `/3+0` supplies the same radix. A reciprocal divisor may be computed and must be positive; zero or negative rejects, and an unresolved divisor stays partial. Inverse solving from the owner's extent into an unresolved size expression is not established.

#### Numeric domains and ranges

The brackets below describe mathematical intervals, not Proteus bracket syntax. A plain bounded numeric domain admits rational readings; a whole-state or whole-tick requirement is additional information.

| Syntax/example | Domain / meaning |
| --- | --- |
| `_` | Unknown numeric reading without supplied bounds. |
| `*4+_` | Bounded numeric template with `0 ≤ v < 4`. A discrete interpretation additionally requires whole readings. |
| `*4+_ == 3` | Check the reading against the template; accepts and can print `3`. `*4+_ == 4` rejects at the excluded endpoint. |
| `**(*3+_ +2)` | Translate the domain to `[2,5)`. |
| `*10+**(*3+_ +2)` | The same value domain `[2,5)` with independent outer size 10. Domain width is not the outer size. |
| `..(0 1)` | Closed rational interval `[0,1]`; both endpoints included, size unspecified. |
| `..(-1/2 -1/2)` | A singleton domain, establishing the exact reading `-1/2`. |
| `..(0 2) = ..(1 3)` | Intersect the constraints, yielding `..(1 2)`. |
| `..(0 1) = ..(1 2)` | The shared included endpoint establishes `1`. |
| `**(..(0 1) +1/3)` | Translate both endpoints, preserving inclusion: `[1/3,4/3]`. |

Closed-range endpoints may be signed or fractional. Reversed endpoints or known nonnumeric endpoints reject; an unknown endpoint leaves the expression partial. Intersections retain an endpoint only if both inputs admit it. Scaling by a negative affine gain reverses endpoint order and carries the corresponding inclusion flags. A range with more than one possible reading does not by itself select a value.

#### Exact coordinate operations

These fixed-arity forms take whitespace-separated infon operands, including computed values and field selections. The `(.name` prefix is contiguous; ordinary call commas and named argument labels do not apply. Results do not acquire an inferred cardinality or automatically inherit an operand's nominal type or cyclic behavior.

| Syntax/example | Result / requirements |
| --- | --- |
| `(.affine 3/4 -1/2 1/2)` | Gain × reading + offset: `1/8`. Argument order is reading, gain, offset. |
| `(.mod -9/4 1)` | Exact Euclidean residue `3/4`, in `[0,1)`. Period must be positive; `(.mod 360 360)` is `0`. |
| `(.common-denominator {1/4 1/6})` | Least common multiple of reduced denominators: `12`. Requires a finite list of established rational readings; the empty list gives `1`. |
| `(.positive-multiple 24 12)` | Check positive whole operands and divisibility; return the checked count `24`. `18` fails the same factor-12 check. |

A missing observation or open observation list keeps the common-denominator operation partial. Unknown counts preserve their constraints: `(.positive-multiple _ 12)` does not choose 12. A separately known invalid factor rejects even while the count is unknown. Affine expressions can retain unresolved linear relationships; for example, `(.affine _ 0 42)` establishes `42`.

#### Resolution, shared unknowns and limits

- **Density remains a choice.** For a reading `a/b` in lowest terms, a subdivision count `N` gives a whole tick only when `b` divides `N`. Observations `1/4` and `1/6` therefore permit every positive multiple of 12. The least common denominator records a requirement; it does not select the minimum density or establish an infon size.
- **Closed versus cyclic coordinates.** A closed unit span with `N` equal subdivisions has positions `k/N` for whole `k` from 0 through `N`, including both endpoints. A cyclic unit interval uses 0 through `N−1` and wrapping identifies a whole turn with zero. The loaded model determines which convention applies; `..(0 1)` itself is closed.
- **Padding preserves the reading.** `++(*3+2 *254+0)` gives `*256+2`: a zero-valued structural part widens the extent. Logical state count, storage extent and coordinate resolution are separate facts; a decoder must still check the logical domain.
- **Precision is explicit.** A literal such as `0.25` is exact. A quantization cell or error interval must be represented separately. Half-open bins and closed error bounds retain their different endpoint rules; decimal display length does not supply either convention. Model vocabulary belongs in R10.
- **Shared references preserve dependence.** Repeated uses of one unknown are not independent choices. For `#twice(x)=++(%x %x)`, a four-state argument gives extent 7 and only readings 0, 2, 4 or 6. Three rejects; four can remain partial until the input is established. Subtracting the same rational reading from itself establishes zero without choosing its value or density.
- **Feasible is not yet proved.** Independent readings in `[10.25,10.35)` permit fractional differences strictly between −0.1 and 0.1. Both endpoints reject; a feasible interior assertion can still remain partial. General products of unresolved readings and unresolved integrality requirements may also stay partial. These rules do not promise a general inverse solver.

#### Sources and status

**Coverage:** R03 documented after confirming Proteus `main` was current at `2436d02f` on 2026-11-24. Numeric examples were checked against maintained source and fixture expectations, with arithmetic results checked separately. No engine build or test rerun was performed for this documentation edit. General repetition/folds and model-defined units continue in R08 and R10.

**Implementation evidence:** [Numeric evaluator](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/WorldManager.dog), [ordered joins and negation](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/orderedJoinTests.tst), [numeric ranges](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/numeric-range-tests.tst), [closed ranges](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/closedRangeTests.tst), [coordinate operations](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/rationalScaleTests.tst), [correlated rational readings](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/linearReadingTests.tst) and [scaled-phase fixtures](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/scaledPhaseTests.tst). The separately loaded [scaled-phase model](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Resources/scaledPhase.pr) illustrates explicit whole-tick requirements.

**Source decisions:** D02F, D02K and D02M (numeric completion, composition, ranges and computed sizes), and D129–D134 (joins, signs, reciprocal radices, exact coordinates and shared domains), in the [decision log](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design-decisions.md), with the [normalization contract](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design.md).

<a id="r04"></a>

### R04 — Ordered/unordered lists, joins, embedding and additive lists

A list describes member occurrences. Ordered correspondence uses logical position; unordered correspondence uses member descriptions and established matching facts. Nesting, embedding and joining have distinct effects on that structure.

#### List forms and headers

| Syntax/example | Meaning / usage |
| --- | --- |
| `{1 2 3}` | Three written members. With no comma, ordering remains inferable from type or context. |
| `{row:1, seat:12}` | Explicitly unordered description. Commas select descriptive correspondence, not positional pairing. |
| `{}` | Empty list. |
| `{_ _}` | Two numeric members with unknown values; their number is known. |
| `{...}` | List with an unspecified span of members; no total count established. |
| `*3+{...}` | Total extent 3 with unspecified members. The count alone does not request expansion. |
| `{~2\| 'A' 12}` | Inner size header: the same list extent as `*2+{'A' 12}`. |
| `{*256+_ \| 1 2}` | Apply the list specification to each logical member. `\|` ends the header; it does not separate successive members. |
| `{T 1 2}` | Timelike list. `T` and ordered/unordered correspondence are independent properties; temporal semantics are in R09. |
| `({1 2})` | Group one list-valued infon. This neither embeds its members nor joins them. |

**Ordering and extent.** Commas are semantic. An unordered description has an implicit remainder even without `...`; its written member count does not establish its total size. An explicit size can bound that total, and more known members than the stated total is a contradiction. Named record definitions can supply unordered layout and missing fields even when an instance initially supplies only one field. Written order is not a positional identity rule for such records.

**Headers.** On an ordinary list, inner `~size` and outer `*size+` supply the same size slot; specifying both is rejected. A header may combine `T`, `~size` and a list specification. An empty specification may be written with a leading `|`. Detailed specification, repetition and alternative rules belong in R08; `[A B]` is an intersection/selection body, not another spelling of `{A B}`.

#### Nesting and embedding

| Syntax/example | Logical structure / result |
| --- | --- |
| `{1 {2 3} 4}` | Three immediate members. The nested list is one member. |
| `{1 &{2 3} 4}` | Four logical members: 1, 2, 3, 4. The embedded wrapper may remain in the stored or printed description. |
| `{1 &{} 2}` | Two logical members. An empty embedded list contributes no positions. |
| `{1 &%items 4}` | Insert the logical members of the referenced list at that position. The reference must establish a suitable list layout. |
| `{1 &{2 3} %prev}@[4]` | The previous logical member is 3, so the selected result is `3`. |
| `{1 {2 3} %prev}@[3]` | The previous member is the ordinary nested list, so the result is `{2 3}`. |

Embedding works through nested embedded spans, grouped lists and named list applications. It provides logical membership without requiring the source to be physically flattened. Counted extents sum embedded lengths; ordinary nested lists each contribute one position. An unknown embedded endpoint does not become known merely because some members are visible.

`%prev` and `%next` cross embedded boundaries and skip empty spans. An ordinary nested list starts its own structural scope; its first member cannot use `%prev` to reach outside that list. An unordered list supplies no ordered neighbour. Unknown boundaries can leave a neighbour request partial. Selection and reference details continue in R06.

Ordinary nested lists also introduce lexical scopes; an embedded declaration uses the enclosing lexical scope. A list specification keeps its original coverage: embedding does not make its local requirement apply to unrelated outer siblings. Applicable enclosing specifications can constrain the embedded logical members. See R05 and R08.

#### Joins and additive lists

`**(...)` and `++(...)` evaluate an ordered operand list. Their operands are whitespace-separated; direct comma separators are invalid. A nested ordinary list contributes one operand, while `&` contributes its logical members as operands. Numeric formulas and domain requirements are in R03.

| Syntax/example | Result / meaning |
| --- | --- |
| `**(*4+2 *3+1)` | Numeric multiplicative composition: `*12+7`. |
| `**('a' 'bc')` | String concatenation: `'abc'`. |
| `**({1 2} {} {3 4})` | List concatenation: `{1 2 3 4}`. The empty list contributes no result members. |
| `**({{1 2}} {{3}})` | Preserve nesting inside each operand: `{{1 2} {3}}`. |
| `**(&{{1} {2}} {3})` | Embedding supplies two list-valued operands, followed by a third; result `{1 2 3}`. |
| `++(*4+2 *3+1)` | Discrete structural sum: `*6+3`. |
| `++(&{*4+2 *3+1} *2+1)` | Three numeric operands, yielding `*7+4`. With `**` and the same operands, the result is `*24+15`. |
| `++(&%items)` | Sum the logical members of a referenced ordered numeric list. |
| `++(*4+2\| _ _)` | The operand specification supplies two `*4+2` operands; result `*7+4`. |
| `*6+ ++(~2\| *4+2 *3+1)` | Two operands, result extent 6. A join's inner `~2` counts operands; outer `*6` constrains the result. |

**Result kind.** `**` uses the composition operation for numeric, text or list values; it does not implicitly convert a mixed list of values into text. Multi-operand `++` is discrete numeric addition, not text or list concatenation. Current singleton behavior preserves its sole member, including the fixture `++(({1 2}))`, which yields `{1 2}`; this does not establish a multi-operand list-sum operation.

**Empty operands.** `++()` gives `*1+0`. Bare `**()` gives `?`; an established result kind supplies the numeric, text or list identity `*1+0`, `''` or `{}`. An empty embedding contributes zero operands and traversal continues with the following sibling.

**Context.** Direct join operands inherit the join's evaluation position; they do not automatically become neighbouring enclosing-list items. Inside an explicit embedded list, contextual references use ordinary member positions: `++(&{*3+1 %prev})` gives `*5+2`. Grouping one operand does not shift the context.

#### Open structure and partial results

- **Spans are not single unknown members.** `{1 ... 4}` describes an unknown intervening span between known ends. `{1 _ 4}` instead supplies exactly one numeric member there. An explicit embedded open list such as `{1 &{2 ...} 4}` retains its prefix and requirements.
- **Count is not materialization.** A known extent may describe sparse capacity without storing every member. A suitable finite source or a targeted demand can establish members. Known widths allow supported sparse traversal to skip spans; they do not authorize arbitrary alignment or enumeration.
- **Shape and values are separate.** Members can acquire facts while the list remains partial. `*_+{1 2}` retains its explicitly unknown endpoint. Matching a bare `{...}` constraint supplies list-header information; it neither closes an open tail nor finishes pending members.
- **Joins need an operand endpoint and order.** An unresolved suffix, unknown embedded extent or unordered embedded source can keep a join partial. For example, `++(*3+2 ...)` retains its expression. Repeated references retain their correlations; the evaluator does not choose independent values for each use.
- **Correspondence needs evidence.** Closed positional correspondence, unique named-field matching and supported sparse cases do not imply arbitrary unordered permutation search or multiple-gap alignment. Unresolved placement can remain partial or unsupported; member-value contradictions at established corresponding positions reject.

#### Sources and status

**Coverage:** R04 documented after confirming Proteus `main` was current at `2436d02f` on 2026-11-24. Source and maintained fixture expectations were inspected; examples were not rerun for this documentation edit. The current singleton-sum exception and limits on open or unordered joins are recorded explicitly.

**Implementation evidence:** [List/header parser](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/infonIO.dog), [size counting](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Proteus.Lib.dog), [list and concatenation fixtures](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/defaultTests.tst), [ordered joins](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/orderedJoinTests.tst), [embedded neighbours](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/embeddedNeighbourTests.tst), [embedded lexical scope](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/unifiedNamingTests.tst), [open-span spelling](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/open-span-language-tests.tst) and [sparse traversal](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/sparseTests.tst).

**Source decisions:** D03E–D03K (list extent, composition, specifications and embedded spans), D03Z (unordered records), D103 (logical neighbours), D114 (lexical scope), D122 (bare open-list constraints), D125 (aligned sparse reads), and D129 (ordered joins), in the [decision log](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design-decisions.md), with the [normalization contract](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design.md). Later decisions govern where they extend the earlier implementation increments.

<a id="r05"></a>

### R05 — Definitions, templates and named individuals

#### Syntax and usage

Braced ellipses below abbreviate a body; they are not complete runnable models.

| Syntax/example | Meaning |
| --- | --- |
| `#ticket = {row, seat}` | Define a reusable template from a literal body. |
| `#ticket = #{row, seat}` | Explicitly construct and name a template value. |
| `#ticket` | Read the template without applying it. |
| `ticket` | Apply the template. |
| `#other = #ticket` | Bind another name to the template. |
| `#seat12 = #ticket:{seat:12}` | Define a refined template; leave the source template unchanged. |
| `#my-ticket = ticket:{seat:12}` | Construct one individual and bind its name. |
| `#my-ticket = %ctx.ticket` | Bind the name to an existing individual selected from context. |
| `#same-ticket = %my-ticket` | Give that same individual another name. |
| `%my-ticket` | Refer to the bound individual. |
| `%my-ticket.seat` | Select that individual's seat. |
| `#planet.listSpec` | Read the template's immediate list specification. |
| `#ident(item) = %item` | Define a parameterized template, even though its body is a cursor. |

#### Semantic rules

- **Declaration:** `#Name = ...` can bind either a template or an individual. The RHS and any parameter header determine which.
- **Template versus application:** `#word` reads a template; `word` applies it. A non-parameterized declaration with an ordinary typed initializer binds its one resulting individual. Literal bodies retain template meaning.
- **Identity:** An individual's aliases refer to the same occurrence. Later context changes do not redirect them. Equal values alone do not identify separate occurrences.
- **Unknowns:** An individual may contain unknown facts. Compatible constraints can establish them; known values and established name bindings cannot be replaced. Conflicting facts and rebinding reject.
- **Scope:** Names resolve in the nearest lexical scope, then globally. Ordinary nested lists introduce scopes; embedded `&{...}` shares the enclosing scope. Each reusable-template application has its own local bindings.
- **Named construction:** Individual constructors and aliases authored inside a concrete named initializer publish globally with its checked facts. Template applications do not export their local names by that rule.
- **History:** The bound individual or model may contain a T-list; the name binding itself has no temporal history.
- **Parameters and tags:** Parameterized headers remain templates. A template alias without a new signature inherits the source signature. Header usage/language decorations do not choose template versus individual.

#### Sources and status

**Settled decisions:** D114, “Unified template and individual bindings” (2026-10-24), and D115, “Concrete named construction and global participants” (2026-10-25), in the [repository decision log](https://github.com/BruceDLong/Proteus/blob/main/Docs/Design%20Docs/normalization-design-decisions.md). D114 supersedes D113; D115 adds ordinary typed construction.

**Implementation evidence:** [Normalization design — definitions and selection](https://github.com/BruceDLong/Proteus/blob/main/Docs/Design%20Docs/normalization-design.md#definitions-and-selection) and [unified naming tests](https://github.com/BruceDLong/Proteus/blob/main/Tests/unifiedNamingTests.tst). The decision log reports implementation and passing focused naming checks. Sources were inspected on 2026-11-24; examples were not executed again for this reference.

**Recorded limits:** Immediate template `.listSpec` reading is supported; general template-metadata editing, unresolved overload selection, temporary assumption names and names in demand-generated occurrences remain outside these decisions' implemented scope.

**Tutorial alignment:** Completed under A07 on 2026-11-24 in [Proteus-Guide.md](Proteus-Guide.md), Chapters 2–3. The examples distinguish template declarations, construction and selection of individuals, use `%my-ticket` for subsequent references, and explain `#ticket` as template access. Query tables describe expected facts; execution and captured CLIP output remain A09.

<a id="r06"></a>

### R06 — Selection, paths, indices and contextual references

Selection requests a member, span or associated value described by a source. A selected occurrence retains its source context and applicable constraints. Equal values at different positions do not make those occurrences identical. In schematic examples, `source` and `word` stand for an infon description and a requested descriptor.

#### Paths and descriptive selection

| Syntax/example | Meaning / usage |
| --- | --- |
| `source.word` | Select a member matching the descriptor. On an ordered source, search for the first proved match; this is not limited to a dictionary-key lookup. |
| `{2 3 'Cat' 'Hat'}.$` | Select the first string: `'Cat'`. Other supported scalar/type predicates use the same descriptive selection. |
| `{{a:1} {a:2}}@[2].a` | Select the second member, then its `a` part; result `a:2`. |
| `source.first`, `source.last` | First or last logical member in an ordered layout. `.last` needs an established endpoint. |
| `source^word` | Read an external property associated with this exact source occurrence. The property value is not identified with its bearer. |
| `source~word` | Resolve a descriptor through ordinary members first, then external properties only after a proved direct no-match. |
| `~word` | Shorthand for `%ctx~word`; the session context supplies the source. |
| `source.begin`, `source.end` | Identity-bearing boundary occurrences, printed as nulls. These are distinct from `.first` and `.last`; boundary identity alone does not supply a numeric endpoint. |
| `#planet.listSpec` | Read the template's immediate list specification. This is template access, not a search through an applied individual's members. |
| `cube^T@[1]^S.location` | Compose a temporal view, item selection, state view and member selection. `^T` and `^S` are reserved, case-sensitive view operations; see R09. |

Descriptors use model meaning and ordinary matching. An unknown earlier candidate can prevent a proved first match; ambiguity or a conflict is not permission to skip into external fallback. Unordered records use established descriptive correspondence, not arbitrary written position. A path can resolve a demanded member without completing unrelated members of the containing model.

Postfix operations continue on the preceding result. A call can supply the source, as in `give(1).x`. Ordinary `^word` is an external-property path, not a general parent-navigation operator; lowercase `^t` is not `^T`. Detailed identity and qualification rules are in R07.

#### Item indices, boundaries and slices

**Item indices are one-based; slice/search boundaries count preceding members from zero.** `@[index:spec]` supplies a selection specification after a boundary, not an inclusive end index. Coordinates may be expressions but must resolve to integers.

| Syntax/example | Result / meaning |
| --- | --- |
| `{'Cat' 'Hat' 'Bat'}@[1]` | First item: `'Cat'`. Zero is not an alternative first-item index. |
| `{'Cat' 'Hat' 'Bat'}@[-1]` | Last item: `'Bat'`; `@[-2]` selects `'Hat'`. |
| `{{1 2} {3 4}}@[2]@[1]` | Second outer member, first inner member: `3`. |
| `{1 &{2 3} 4}@[3]` | Third logical member: `3`. Embedded wrappers do not consume item positions. |
| `{10 20 30}@[**(1 +1)]` | Computed item index 2: `20`. |
| `{1 2 3 4}@[0:*2+{...}]` | Start before the first member and capture two members: `{1 2}`. |
| `{1 2 3 4 5 6 7 8 9 0}@[3:*4+{...}]` | Skip three members, then capture four: `{4 5 6 7}`. |
| `{1 2 3 4}@[-2:*2+{...}]` | Start two positions before the endpoint and capture two: `{3 4}`. |
| `{1 2 3 4 5 6 7}@[3:5]` | Search after boundary 3 for a member matching literal 5; result `5`. The `5` is a predicate, not an end index or span length. |

For an extent `N`, negative item index `−k` refers to item `N−k+1`; a negative slice boundary `−k` becomes boundary `N−k`. The endpoint must be proved for the selected source/view. An open tail does not make the last stored member the final member, and a derived view uses its own end rather than the ultimate source's end.

Indices address the established logical layout. Ordinary nesting contributes one member; embedding contributes its members. A named homogeneous model can establish unit grouping different from the stored representation; selecting a typed unit must prove that correspondence. Merely knowing a declared count does not justify an arbitrary regrouping. Units and supported sparse views continue in R10.

#### Explicit selection bodies

| Syntax/example | Result / meaning |
| --- | --- |
| `[2 3 4 5]` | Select the last logical member: `5`. Square brackets form a selection body, not an ordinary list literal. |
| `*3+['Cat' 'Hat' 'Bat' 'Dog']` | Select item 3 from the body: `'Bat'`. |
| `[1 2 3 <4> 5 6]` | `<...>` marks the requested occurrence; result `4`. |
| `[_ <_> _] <~ {1 2 3}` | Match the pattern against the supplied source and return the marked member: `2`. |
| `*(-2)+[<_> _] <~ {1 2 3 4}` | End-relative pattern placement; return the second-last member, `3`. |

`<~` supplies the source for correspondence; an embedded capture can request a span. Selection must establish the pattern's required conditions, including any suffix after a marked member. Multiple viable occurrences can keep a result partial. Repetition, filtering, alternative domains and more general intersection bodies are covered in R08.

#### Contextual references

| Syntax/example | Meaning / current status |
| --- | --- |
| `%`, `%self` | Current aliases for self. The inspected normalizer resolves the containing definition/application instance through the current occurrence context. A proposed distinction between these spellings is not implemented. |
| `%item` | Read the binding of parameter `item` in its application, when declared in that parameter scope. |
| `%my-ticket` | Refer to a named individual bound in the applicable lexical scope; aliases keep the same occurrence. See R05. |
| `%W` | The world occurrence supplied to the request. A missing world source leaves the request partial. |
| `%ctx.planet` | Select a matching recent model reference from the session interaction context. An established binding does not redirect when later context changes. |
| `%prev`, `%next` | Previous or next structural member in the current logical ordered scope. |
| `%tprev`, `%tnext` | Explicit temporal neighbours. They do not become structural-neighbour lookups in an ordinary S-list; see R09. |

Arguments resolve in the caller's scope; the body uses its own parameter bindings. Forward an outer parameter explicitly into a nested definition. Selecting an LHS does not rebind references inside an already supplied argument to the destination.

Structural neighbours cross embedded boundaries and skip empty spans. Ordinary nesting starts a separate scope; unordered membership supplies no ordered neighbour. A proved absent neighbour can yield `UNDEFINED`; an unknown endpoint or unsettled correspondence remains partial. Direct join operands use the join's evaluation position, as described in R04.

Session recency and modeled time are separate. Ordinary source-ordered assertions can establish context without advancing a T-list. Unordered evaluation order does not establish recency. The accepted temporal design interprets ordinary references at the common time-slice unless explicitly qualified; supported state/history views and their limits belong in R09. A stable reference is not automatically a frozen copy of its state.

#### Constraints through selections and implementation limits

- **Selected destination.** `{_ 2}@[1] = 1` constrains the first member. Ordinary `=` can establish unknown facts but rejects a conflicting established value. Explicit scalar replacement `<<=` is a separate operation governed by retained constraints; see R07.
- **Retaining the source.** `{_ 2}@[1] = 1;` returns the constrained source `{1 2}`. A semicolon retains that selection's source; it is not a general statement separator. Repeated semicolons in a chained path can retain successive source levels.
- **Source-relative context.** `{*10+1 _}@[2] = %prev;` gives `{*10+1 *10+1}`. The RHS neighbour is relative to the selected source occurrence, not a temporary result container.
- **Identity and writes.** Selected leaves and spans retain their source positions, qualifications and applicable list specifications. Regrouped results can have distinct containers while their leaves refer to original occurrences. A read does not grant additional write access; a failed checked update does not publish partial edits.
- **Unresolved work.** Unknown/nonintegral indices, unproved endpoints, ambiguous matches and unsupported layout conversions do not yield guessed selections. Exact endpoint knowledge and availability of the selected content are separate obligations.
- **Parsed but not provided.** `%U`, `%args` and `%vars` have parser spellings but no completed occurrence-provider branch in the inspected normalizer. Legacy `.asTList` and `.cut` also remain unsupported there. Immediate template `.listSpec` reading is supported; general template metadata selection or editing is not.

#### Sources and status

**Coverage:** R06 documented after confirming Proteus `main` was current at `2436d02f` on 2026-11-24. Source and maintained fixture expectations were inspected; examples were not rerun for this documentation edit. Current `%`/`%self` aliasing is distinguished from the proposed ergonomic split.

**Implementation evidence:** [Path/reference parser](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/infonIO.dog), [selection, context and association providers](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/WorldManager.dog), [paths, indices and selection fixtures](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/defaultTests.tst), [slice context](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/slice-language-tests.tst), [source-relative writes](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/path-write-language-tests.tst), [embedded neighbours](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/embeddedNeighbourTests.tst), [parameter bindings](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/wordArgumentTests.tst), [stable named references](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/unifiedNamingTests.tst), [selected replacement constraints](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/selectionReplacementTests.tst) and [retained source-position checks](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/ProteusTests.dog).

**Source decisions:** D03S and D03V (path lowering and endpoints), D046–D048 (selection context, replacement and external associations), D099 and D103 (scope and neighbours), D114 (named bindings), D118 and D123–D125 (selected occurrence preservation and sparse boundaries), in the [decision log](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design-decisions.md), with the [normalization contract](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design.md).

<a id="r07"></a>

### R07 — Identity, qualification, constraints and contradictions

An identity assertion relates descriptions of information. Normalization establishes compatible facts under that relationship's qualifications. Equal values alone do not identify occurrences: `{7 7}` contains two occurrences, while two aliases of one individual refer to the same occurrence. These relationships constrain descriptions; they are not Boolean comparison expressions returning true or false.

#### Relationship forms

`A` and `B` below stand for infon descriptions. For qualified matching, read the left side as the requested description and the right side as the supplied candidate/source; reversing the relationship can change the request.

| Syntax | Meaning / scope |
| --- | --- |
| `A = B` | Strict identity constraint. Combine compatible type, size, value and structural facts; known scalar sizes and values must agree. An unknown may acquire facts from the relationship. |
| `A == B` | Qualified whole-content matching. Supported correspondence can use different representations or decompositions; a structured mapping must reach the source endpoint. The qualification carries into member matching. |
| `A =: B` | Prefix qualification at the outer level. Compare from the beginning without equating outer endpoints; derived member relationships remain strict. |
| `A ==: B` | Qualified prefix matching. Combine qualified member correspondence with independent outer endpoints; a structured mapping may consume only a source prefix. |
| `word:description` | Supply a description for a named application, still subject to that word's meaning. This colon is part of the named description, distinct from a trailing colon on an identity operator. |
| `!A` | A negative matching constraint: exclude candidates proved to match `A`. This does not negate a numeric value. |
| `A <<= B` | Explicit replacement of a supported scalar destination with a checked new value. Retained meaning and containing constraints still apply. |

Whole-content correspondence does not require the same count in different representations: two string-valued result members can consume seven source bytes. Qualification is not unrestricted coercion; the engine must prove an admitted mapping and all required constraints. A successful qualified match does not by itself identify the template occurrence with the candidate occurrence.

#### Facts, whole matches and prefixes

| Syntax/example | Result / meaning |
| --- | --- |
| `_ = 7` | Establish numeric value 7 without inventing a size. |
| `*16+_ = *16+9` | Establish `*16+9`, retaining size 16. |
| `? = 'Hello'` | Establish a string representation and its value. `_ = 'Hello'` instead conflicts with the numeric requirement. |
| `*4+_ == 3` | Check a candidate against the numeric domain; accepts and can print `3`. `*4+_ == 4` rejects. |
| `$ == 'Hello'` | Whole string match, yielding `'Hello'` with its established endpoint. |
| `{*3+$ *4+$} == 'CatDogs'` | Match the complete text through two string parts: `{'Cat' 'Dogs'}`. |
| `*3+$ =: 'Hello'` | Acquire the three-byte prefix `'Hel'` while retaining the requested width. |
| `$ =: 'Hello'` | Acquire available text with unknown endpoint: `*_+'Hello'`. The RHS endpoint is not inherited. |
| `'Hello' =: 'Hel'` | The available common prefix agrees; retain `'Hello'`. Prefix compatibility does not truncate an already known literal. |
| `{{'A'\|...} 'R'} ==: 'AARON'` | Match a structured prefix, yielding `{{'A' 'A'} 'R'}`; the trailing `ON` is not consumed. |

The colon forms preserve an independently known target size and do not fill a missing size merely from the RHS endpoint. A known string width requires that many available bytes; insufficient text leaves the request partial. With unknown width, acquired text can remain an incomplete string. Prefix agreement does not prove that either description ends where the available prefix ends.

For numeric domains, strict identity intersects compatible constraints. A qualified match against an explicit range requires containment of the supplied domain; partial overlap alone can remain unresolved. Disjoint domains reject. Bounds, state count and density remain distinct, as specified in R03.

#### Relationship structure and named meanings

- **Several supplied identities.** `A = B = C` attaches both supplied relationships to `A`. It does not perform sequential assignments. Ordinary grouping usually preserves the result, but qualifiers and prerequisites must remain attached to the relationship they govern.
- **Qualified grouping.** In `A == (B = C)`, the inner identity belongs to the supplied description. Flattening it can incorrectly impose `C`'s representation directly on `A`. For example, `{*3+$ *4+$} == ($ = 'CatDogs')` matches through the checked string source.
- **Named constraints.** A word's definition, its given description, applicable list specifications and containing identities can all supply requirements. Omitting a field from an unordered description does not prove that field absent. Ancestor requirements apply through proved correspondence, not indiscriminately to every descendant.
- **Distinct names.** An explicit positive identity can combine differently named meanings after their definitions and contents prove compatible. A shared parent type or coincidentally equal payload does not establish that an unasserted nominal predicate matches another name. For the accepted `#front-color=color` and `#red=color:{hue:0}` definitions, `front-color = red` must check their meanings and propagate the hue constraint.
- **Aliases and instances.** `#same = %existing` binds another name to the existing occurrence. Applying a reusable definition does not identify each application with the declaration or with other applications. Facts learned through aliases remain facts about their shared referent; see R05.

#### Exclusions and missing proof

| Syntax/example | Meaning / outcome |
| --- | --- |
| `!_` | Exclude numeric candidates in supported predicate matching. `!$` analogously excludes strings. |
| `!*_+8 = *16+8` | The positive candidate matches the excluded value; reject. |
| `!*_+8 = *16+9` | Prove the candidate is not excluded; retain the negative description `!*_+8`. Do not turn it into positive value 9. |
| `!*_+5 = *_+_` | Missing value evidence prevents the exclusion test from finishing. |
| `!*_+5 = !*_+6` | Retain both non-equivalent exclusions. Neither supplies a positive value. |

Exclusion complements a proved comparison. Failure to prove a match is not proof of a nonmatch. A known conflicting type, size or value can establish nonmatching; missing information can leave it unresolved. A negative description may remain partial even after a particular candidate has been checked, because no positive value was established. Arithmetic sign reversal uses `-(A)` instead; see R03.

Current support does not establish a general complement solver for arbitrary negative lists, unknown-type templates, qualified negative relationships or compound expressions. A proved predicate nonmatch can be skipped during selection; it does not make the source contradictory.

#### Replacement and contradictions

Ordinary identities reveal or restrict information. An encountered conflict on a required relationship rejects that request: `7 = 8` cannot replace 7 with 8. Missing information is different: `_ = _` remains unknown, and `-1 = 9` can remain partial until an extent is available; `-1 = *10+9` establishes the compatible size-10 reading.

`123 <<= 456` explicitly replaces the scalar value with 456. The supported operation accepts an available numeric or string RHS, discards the old local value/extent evidence and checks the new value against retained requirements. An anonymous RHS retains the destination's declared name; a named RHS supplies its own primary name. This does not rebind a named individual.

A local string literal's old length may change under replacement, while a width required by a retained listSpec still applies. Replacing `'Tiffany'` with `'KTiffany'` is compatible with a `$` member requirement but conflicts with a retained `*7+$` requirement. Declared types and literal-value requirements likewise continue to constrain the replacement. List replacements and qualified or negative replacement relationships are not generally supported.

Selections retain their actual destination occurrences. Required checks precede publication; a conflicting update does not publish some members and leave others unchanged. The `;` source-retention syntax and source-relative RHS context are covered in R06.

| Outcome | Interpretation |
| --- | --- |
| Complete | The requested compatible facts are established. Some unrequested facts, such as numeric size, may remain unknown. |
| Partial | Available evidence does not establish the requested result or a contradiction. Constraints and possible alternatives may remain. |
| Waiting | Required input or a requested prerequisite is still pending. |
| Rejected | A required relationship has a proved incompatibility or excludes its candidate. |
| Unsupported | The current engine lacks the required semantic route; this is not a proof of contradiction. |

Default normalization checks the requirements of the routes it uses; it does not promise an exhaustive coherence search over every unused relationship. A printed residual expression alone may not distinguish a partial request from a rejected one; inspect the reported outcome. Unknown `?`, explicit `NULL:` and unavailable-selection `UNDEFINED` have their own meanings and are not substitutes for a rejection status.

#### Sources and status

**Coverage:** R07 documented after confirming Proteus `main` was current at `2436d02f` on 2026-11-24. Source and maintained fixture expectations were inspected; examples were not rerun for this documentation edit. Structured matching, exclusions and replacement are described only within their stated supported scope.

**Implementation evidence:** [Relationship parser](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/infonIO.dog), [qualification flags](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/infonList.dog), [matching and publication](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/WorldManager.dog), [identity, parsing and exclusion fixtures](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/defaultTests.tst), [prefix text](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/qualified-text-tests.tst), [numeric domains](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/numeric-range-tests.tst), [named aliases](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/unifiedNamingTests.tst) and [replacement guards](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/selectionReplacementTests.tst).

**Source decisions:** D02A–D02D and D02F (goals, identity structure and completion), D02I–D02K (scalar constraints, prefixes, exclusions and ranges), D03S–D03T (prefix and structured correspondence), D047 (explicit scalar replacement), D059–D061 (ancestor requirements and compatible named identities), D114 (bindings), and D123 (retained replacement constraints), in the [decision log](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design-decisions.md), with the [normalization contract](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design.md).

<a id="r08"></a>

### R08 — Alternatives, list specifications, repetition and folds

A list specification supplies requirements for its members. Repetition applies those requirements at distinct occurrences. An alternative domain retains possible scalar answers; a fold combines an ordered list of operands. These constructs compose through ordinary constraints and correspondence, within the supported cases below.

#### Finite alternatives

| Syntax/example | Meaning / result |
| --- | --- |
| `*_+[1 2 3]` | A scalar may be any of the three candidates. No candidate is chosen merely because it appears first or last. |
| `2 = *_+[1 2 3]` | The constraint leaves the answer `2`. |
| `'Beta' = *_+['Alpha' 'Beta']` | The constraint leaves `'Beta'`. |
| `_ = *_+[5 6 7]` | Retain the partial domain `*_+[5 6 7]`; the numeric requirement does not determine one value. |
| `*_+[...] <~ {1 2 3}` | Obtain alternatives from the finite supplied input. The otherwise unconstrained open body is supplied by that input. |
| `[1 2 3]` | Ordinary last-member selection, yielding `3`; omitting `*_+` changes the request. See R06. |

Each candidate includes its own constraints. A branch is removed only when those requirements prove it incompatible. Unknown values, missing evidence, waits and unsupported work are not false branches. If a finite, exhausted domain has no viable candidate, the request rejects. One survivor supplies its facts; several provably equal complete scalar answers may also collapse. Otherwise the alternatives remain partial.

Constraints between two domains retain viable combinations; facts from different branches cannot be combined into an invented answer. An open remainder is not an exhausted domain, and equal scalar answers do not identify distinct source occurrences. The finite scalar route does not support arbitrary list-valued alternatives or candidates whose unresolved work cannot be retained. General conditional branching inside expressions and models needs its own supported correspondence; this syntax alone does not establish it.

#### List specifications

`S` below stands for a member specification; `A` and `B` stand for member descriptions. The `|` separates the specification from the represented members; it does not separate alternatives.

| Syntax/example | Meaning / usage |
| --- | --- |
| `{S \| A B}` | Apply `S` to both member occurrences, in addition to each member's own facts. |
| `{*256+_ \| 1 2}` | Two numeric members, each constrained to size 256. Their values are 1 and 2. |
| `{7 \| _ _}` | Both represented members must have value 7. The specification is a requirement, not an overridable default. |
| `{_ \| ...}` | An open numeric-member list; neither its count nor its member values are established. |
| `*4+{*256+_ \| ...}` | Four member positions, each with the byte-sized numeric requirement. The outer 4 counts members, not their numeric states. |
| `{~4 *256+_ \| ...}` | Inner-header spelling of that same count and specification. On an ordinary list, do not supply both inner and outer size. |
| `{\| 1 2}` | Explicitly empty specification; the two members supply their own descriptions. |
| `++(*4+2 \| _ _)` | A specification on a join's operand list supplies two `*4+2` operands; their sum is `*7+4`. |

Every applicable requirement remains in force. A local specification on an embedded list constrains that span's logical members; an enclosing specification can constrain them too. An ordinary nested list is one member, so its parent's requirement applies to the nested list as a whole. Contextual references are interpreted at the corresponding occurrence while retaining their definition/argument scope; shared templates are not mutated into the first member's result.

A specification describes separate occurrences, not one occurrence aliased into every position. Conversely, explicit references to the same individual retain their shared constraints. A closed empty list has no members requiring template evaluation. A successfully applied specification remains stored even when compact printing hides it; later reads and replacements still enforce it.

#### Repetition and structured correspondence

`...` denotes an open span. With a list specification, further members must satisfy that specification; a count or proved correspondence may establish how many there are. An open span need not contain at least one member.

| Syntax/example | Meaning / result |
| --- | --- |
| `{'A' \| ...}` | An unspecified number of string-valued members, each equal to `'A'`; this is a list, not a string literal. |
| `{*3+$ \| ...} == 'CatHatBatDog'` | Match the whole string in three-byte pieces: `{'Cat' 'Hat' 'Bat' 'Dog'}`. |
| `*4+{*3+$ \| ...} ==: 'catHatDogPig'` | Require four output members, each consuming three bytes from the source prefix. |
| `{{_ _} \| ...} == {1 2 3 4 5 6 7 8}` | Group the complete source into pairs: `{{1 2} {3 4} {5 6} {7 8}}`. |
| `{{'A' \| ...} 'AARON'} == 'AAAARON'` | The following literal constrains the repetition's endpoint: `{{'A' 'A'} 'AARON'}`. |

Whole matching with `==` must reach the source endpoint; `==:` permits a prefix. The output count can differ from the number of source bytes or members consumed. Repetition is not specified as a greedy or shortest-match choice: required suffixes and other constraints must hold. Several viable assignments can leave the result partial; an unbounded repeated pattern that consumes no source positions cannot justify arbitrary output.

A known count is not an instruction to materialize every position. Supported sparse reads can obtain a requested occurrence while retaining the remaining repetition symbolically. A plain open repetition without adequate evidence stays open. Structural neighbours and temporal production have their separate scope and readiness rules in R06 and R09.

#### Repeated selection and filtering

A selection body used as a list specification can consume a source span and return one selected member or capture for each output. The consumed span and returned value are distinct. `<~` supplies the source.

| Syntax/example | Meaning / result |
| --- | --- |
| `{[_ _] \| ...} <~ {1 2 3 4 5 6 7 8}` | Consume pairs and return each pair's last member: `{2 4 6 8}`. |
| `{*2+[...] \| ...} <~ {1 2 3 4 5 6 7 8}` | Repeated second-item selection; this fixture also yields `{2 4 6 8}`. |
| `{[&{!*4+_ \| ...} *4+_] \| ...} <~ {1 2 0 3 444 555 666 777 2}` | Skip proved nonmatches, then return the next matching numeric member; yields `{1 2 0 3 2}`. |

Existing members written after a projected specification constrain its output members; they do not consume raw input positions themselves. Required pattern suffixes must still match even when the returned member occurs earlier in the pattern.

A range candidate wholly inside the predicate domain can match; a disjoint candidate can be skipped. Partial overlap remains unresolved. When the next repetition cannot finish, an unbounded projection may retain a proved output prefix together with its open remainder, specification and source relationship. Reaching the end of currently available input does not always produce a closed filtered list. Ambiguity or unknown predicate results do not authorize speculative output. See R03 for numeric domains and R07 for proof of nonmatching.

#### Folds over ordered operands

The existing joins provide the supported folds. Use `&` to supply a list's logical members as operands; without embedding, the list remains one operand.

| Syntax/example | Meaning / result |
| --- | --- |
| `++(&{*4+2 *3+1} *2+1)` | Structural sum of three operands: `*7+4`. |
| `**(&{*4+2 *3+1} *2+1)` | Ordered multiplicative composition of those operands: `*24+15`. |
| `**(&{'a' 'bc'})` | Concatenate text operands: `'abc'`. |
| `**(&{{1 2} {3 4}})` | Concatenate list operands: `{1 2 3 4}`. |
| `++(&%items)` | Sum the logical numeric members of the referenced ordered list. |
| `*6+ ++(~2 \| *4+2 *3+1)` | Inner count 2 constrains the operands; outer extent 6 constrains the result. |

For positive finite whole-number operand sizes, `++` has size `1 + sum(si − 1)` and value `sum(vi)`. Its identity is `*1+0`. `**` uses the numeric, string or list composition rules and identities in R03–R04. Repeating a known `*4+2` operand twice under `++` gives `*7+4`; this is structural addition, not multiplication of its sizes.

Repeated references preserve correlation. If both operands refer to the same unknown four-state value, their sum can only be 0, 2, 4 or 6; the evaluator must not fill the two references independently. Open or unordered operand layouts do not supply a completed ordered fold. No arbitrary callable fold operator is established by these two forms.

**Repetition shorthand status.** The inspected parser does not implement the discussed `^^N` or `^L` notation as repetition/exponentiation operators. Use list specifications and explicit joins for the supported forms above. Existing `^word` paths and the reserved `^T`/`^S` views have the meanings in R06 and R09.

#### Sources and status

**Coverage:** R08 documented against Proteus `main` at `2436d02f`, confirmed current for this edit. Source and maintained fixture expectations were inspected; examples were not rerun. Finite scalar alternatives, supported structured/projected repetitions and ordered joins do not establish general recursive grammars, arbitrary structured alternatives or unrestricted symbolic correspondence.

**Implementation evidence:** [List and selection parser](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/infonIO.dog), [alternative collection and pattern matching](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/WorldManager.dog), [alternative, repetition and projection fixtures](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/defaultTests.tst), [numeric filtering](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/numeric-range-tests.tst), [join specifications and correlations](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/orderedJoinTests.tst) and [sparse member demands](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/sparseTests.tst).

**Source decisions:** D03G–D03K (member requirements and open spans), D03R–D03T (repeated and structured correspondence), D03W–D03X (range predicates and projected repetition), D04A (finite scalar alternatives), D054 (retained specifications), D125 (sparse occurrence reads) and D129 (ordered joins), in the [decision log](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design-decisions.md), with the [normalization contract](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design.md).

<a id="r09"></a>

### R09 — Temporal lists, states, neighbour references, supports and verbs

A temporal list describes successive states at a chosen level. An S-state describes the parts present together at that level; a part can have its own lower-level history. State count, temporal support and elapsed duration are distinct. Neither a list index nor the `T` marker supplies a clock, unit of time or alignment with another history.

#### Temporal lists and neighbours

`T` is a list-header marker, not an ordinary member. Temporal interpretation and member correspondence remain separate: the ordinary ordering, embedding and extent rules still apply. In the examples below, the temporal layout establishes the succession.

| Syntax/example | Meaning / result |
| --- | --- |
| `{T 4 8}` | A represented two-state history with readings 4 and 8. No duration is asserted. |
| `{T {4 5} {8 9}}` | Two states, each with two simultaneous parts. The inner lists do not themselves introduce temporal succession. |
| `{T 4 &{8 12} 16}` | Four logical temporal members; embedding does not introduce another temporal level. |
| `{T 4 {T 8 12} 16}` | The ordinary nested history retains its own temporal level. |
| `{T 1 %tprev 3}` | The second member reads its temporal predecessor; result `{T 1 1 3}`. |
| `{T 1 %tnext 3}` | The second member reads its temporal successor; result `{T 1 3 3}`. |
| `{1 %tprev 3}` | No temporal context is established; the temporal reference yields `UNDEFINED` here. It does not fall back to `%prev`. |
| `{T {T 1 2} {T %tprev 4}}` | At the nested span boundary, the first member of the second span reads the preceding span's final member: 2. |

`%prev` and `%next` address structural neighbours in the current logical ordered scope. `%tprev` and `%tnext` use the enclosing temporal level, including supported entry/exit navigation between adjacent temporal spans. Embeddings are transparent to logical positions; ordinary S-nesting retains its structural scope. Independent histories in separate S-members do not acquire a common predecessor merely by being stored beside each other.

A proved absent neighbour differs from an unproved endpoint or unavailable successor. Open structure or pending evidence can keep navigation partial or waiting. Referencing a later state does not by itself prove that state exists. Item and span indices follow R06's one-based item and zero-based boundary conventions.

#### Current state retention and independence

The current engine acquires a state's supplied constraints, definitions and applicable parent requirements before retaining facts from its checked predecessor. Ordinary unknown or omitted parts can retain previous facts. An explicitly established new scalar value supplies that later state's value; unchanged record parts can carry forward. Retention preserves the later occurrence's identity and does not modify the earlier state.

| Syntax/example | Current behavior |
| --- | --- |
| `{T 7 _ %? _ 9 _}` | The second state acquires 7; the third is independently unknown, so the fourth also remains unknown. The sixth acquires 9. The history remains partial. |
| `{T {x:1 y:2} {x:_ y:%?} {x:4}}` | State 2 retains x:1 but leaves y unknown. State 3 supplies x:4 and retains the unknown y. The history remains partial. |
| `%?` | Withhold implicit predecessor retention at this scope. Supply no value and assert no inequality. This is one contiguous token. |
| `%? = 7` | Explicit evidence can still establish an independent value. Independence does not disable ordinary constraints. |
| `{T *10+0 *10+1 = %tprev}` | Reject: the explicit identity requires the later value 1 to agree with predecessor value 0. |
| `{T **(%tprev +1) \| 0 _ _}` | The contextual rule supplies successive values; result `{T 0 1 2}`. |

Independence on a field leaves siblings' retention available. Independence at a compound scope withholds predecessor retention for that value; it is not a reset to zero or an instruction to choose different facts. An ordinary `_` does not request independence. Once a previous value has become unknown, later retention cannot invent the value that preceded that unknown.

**Design discrepancy to retain.** D062 proposed treating preservation as a required identity even when a later description supplies a conflicting known value. D063 recorded that proposal's reference-status issue. The later implemented retention path and D105 tests permit supplied changes while retaining unspecified parts, as above. The earlier strict-preservation proposal is not the current engine rule; reconciling the intended contract and historical wording is Appendix A22. Ordinary explicit identities and retained list specifications still reject incompatible facts under R07.

#### State and history views

The case-sensitive postfix forms `^S` and `^T` operate on established occurrences. A selected state's T-view is currently a one-member window over that same state, not a generated lower-level evolution.

| Syntax/example | Meaning / result |
| --- | --- |
| `{T 4 8}^T@[2]^S` | Retain the explicit history, select its second member and read the state: `8`. |
| `{T 4 8}@[2]^S^T` | View the selected state's support as a one-member history: `{T 8}`. |
| `{T 4 8}@[2]^S^T@[1]^S` | Return to the same selected occurrence, with reading 8. |
| `{T 4 8}@[2]^T^T@[1]^S` | Repeated T conversion adds no extra temporal level; result `8`. Repeated S conversion likewise retains the state. |
| `{T {4 5} {8 9}}@[2]@[1]^T@[1]^S` | The part with value 8 shares the represented second state's support. |
| `{T 4 {T 8 12} 16}@[2]^T@[2]^S` | Select within the independent nested history; result `12`. |
| `{T 4 8 ...}@[2]^S^T@[1]^S` | A represented state can supply its view without enumerating the open future; result `8`. |
| `{T 4 8}^S` | Unsupported: select a temporal member before requesting its S-view. A one-member stored prefix does not remove this requirement. |

Views preserve original occurrence context, applicable list specifications, overlays and access qualifications. They can be passed as arguments or bound to individual names. A checked assertion through a view addresses the source occurrence; conflicts do not partly update its predecessor or siblings. Viewing a state does not grant permission to overwrite known facts.

#### Temporal supports and parts

A **support** identifies the represented temporal extent over which a state or part is being viewed. It is distinct from both the viewed occurrence and that subject's entire history. In the implemented discrete views:

- A temporal member establishes its own support within its history.
- An ordinary S-part borrows its containing represented state's support. Nested S-containment can share that time while retaining separate structural and lexical scopes.
- A part's shared support does not establish its lifetime. Collecting same-named fields across a whole's states would require an additional persistence and correspondence proof.
- A nested T-list retains its own temporal level. The engine does not align independent histories by equal values, matching names or equal ordinal indices.

For a supported state or part, `.begin` and `.end` expose the support's identity-bearing boundaries. The state's S- and T-views agree on those boundaries; parts can share the whole state's boundaries. Printed `NULL:` labels identify boundary occurrences, not timestamps or measured durations. Shared boundaries do not identify different parts with each other.

General rooting, retiming and whole/part alignment across separate histories remain outside this increment. In particular, several Earth states within one Sol state require more than matching their list indices. A foreign lifetime used inside another subject's state may report unsupported pending a support-correspondence proof. See Appendix A21.

#### Verbs, production and completion

A verb-like definition contributes a later state or constraints through ordinary template application. There is no separate imperative verb evaluator. Models may read `%tprev`, supply changed parts and retain unaffected parts through the state rules above. Calls retain R05's parameter rules; they do not authorize external physical effects.

| Syntax/example | Meaning / result |
| --- | --- |
| `#inc(item)=**(%item +1)` | Define a parameterized value transformation usable in a temporal state. |
| `{T 0 inc(item::%tprev)}` | With that definition, the later application has value 1. The predecessor remains 0. |
| `#count(step)={T **(%tprev +(%step)) \| 0 _ _}` | Define a three-state history; `count(step::2)` supplies readings 0, 2 and 4. |
| `{T **(%tprev +1) \| 0 ...} = {T _ _ _}` | The finite source establishes three target positions; acquire `{T 0 1 2}`. |
| `*1000000+{T **(%tprev +1) \| 0 ...}` | A counted history whose requested states can be produced without materializing the whole suffix. |
| `{T **(%tprev +1) \| 0 &{T !3 \| ...} 3 &{T !6 \| ...} 6 &{T !9 \| ...} 9}` | Required marker states and repeated-segment predicates establish the successive values 0 through 9 and the endpoint. |

Production needs evidence of membership as well as a rule for a value. A declared extent, finite source or required later segment can provide that evidence. Bare `{T **(%tprev +1) | 0 ...}` does not establish another occurrence merely because a reader asks for it. At a segment boundary, an unresolved candidate or more than one viable continuation leaves the request partial. Failure of a required transition is a contradiction, not an invented endpoint.

Completion belongs to the model. A destination condition can constrain an activity's final state; it is not an implicit stop condition for every verb. An unknown result is not proof that an activity ended. Supported generated histories can use explicit parameters, including computed scalar arguments; parameter-dependent extents and general object arguments remain separate boundaries.

Requested occurrences retain identity across value eviction and replay; durable checked assertions remain constraints. Reacquiring an earlier state is not a general inverse-transition solver. Some finite table recurrences can provide later scalar values directly under D128, but a value projection does not establish every intervening occurrence or temporal support. General ambiguous production, external effects and cross-history alignment are not implied by these cases.

#### Sources and status

**Coverage:** R09 documented against Proteus `main` at `2436d02f`, confirmed current for this edit. Source, maintained language fixtures and ordinary CodeDog test expectations were inspected; examples were not rerun. Earlier temporal working notes contain proposals and obsolete spellings; the implementation boundaries above follow the later decisions and current tests. Appendix A05 is complete as a documentation task; A21–A22 retain the newly identified follow-ups.

**Implementation evidence:** [Temporal acquisition](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/WorldTemporalAcquisition.dog), [state/support views](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/WorldTemporalViews.dog), [temporal production](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/WorldTemporalProduction.dog), [retention and neighbour matching](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/WorldManager.dog), [ordinary CodeDog retention and production checks](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/ProteusTests.dog), [temporal-view fixtures](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/temporalViewTests.tst), [temporal neighbour fixtures](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/defaultTests.tst), [embedded context](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/embeddedNeighbourTests.tst), [parameterized temporal calls](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/wordArgumentTests.tst) and [finite recurrence projection](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/WorldRecurrenceProjection.dog).

**Source decisions:** D044 and D069 (temporal navigation and ordinary state-change definitions), D062–D063 (preservation proposal and reference caveat), D100–D102 (template application, completion and calls), D104–D111 (ordinary acquisition and demand production), D116–D117 (represented state and part supports), and D128 (finite recurrence projections), in the [decision log](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design-decisions.md), with the [normalization contract](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design.md).

<a id="r10"></a>

### R10 — Units, scales and model-defined vocabulary

Proteus supplies language forms for definitions, constraints, numeric operations and list correspondence. Models supply the meanings of words such as `second`, `minute`, `turns` and `degrees`. A useful distinction is between **grouping units**, which describe a span of smaller members, and **numeric scales**, which relate coordinate readings. A name alone supplies neither a conversion factor nor a measurement convention.

#### Language forms and supplied meanings

| Form/example | Source of meaning |
| --- | --- |
| `#minute = *60+{second \| ...}` | Ordinary definition syntax. The model chooses the name, member type and count; `minute` is not a built-in time unit. |
| `source.minute` | Built-in descriptive selection applied to a model-defined descriptor. A suitable homogeneous layout can establish a grouped unit view. |
| `phase-reading(1/2)` | An ordinary call to a supplied definition, not a reserved numeric operation. |
| `(.affine 3/4 -1/2 1/2)` | A built-in numeric form, giving `1/8`. The model chooses what the reading, gain and offset represent. |
| `degrees:90` | A named numeric description. Merely naming it `degrees` does not cause unrelated arithmetic to convert units or wrap values. |

Use R05's template and individual distinctions when defining vocabulary. Shared public application models belong to the separate World repository; Proteus also carries small, self-contained test models. Their definitions need not use the same sizes or conventions. A missing definition does not gain a meaning from an English word, and a test fixture is not evidence that its vocabulary is loaded in every application.

#### Homogeneous units and typed selection

The supported unit-conversion route follows finite, positive integral counts of one repeated named member type, ending at an unconstrained numeric base definition. The definitions establish a common base unit and exact widths. Matching widths alone do not make unrelated base types compatible. Cyclic definitions, heterogeneous layouts and extra value restrictions do not establish this simple conversion proof.

For the following table, use the deliberately small fixture definitions `#second=_`, `#minute=*3+{second|...}`, `#hour=*2+{minute|...}` and `#day=*2+{hour|...}`. Thus a fixture minute contains three seconds, an hour six seconds and a day twelve. These are test sizes, not civil-time definitions.

| Syntax/example | Result / meaning |
| --- | --- |
| `hour:{second \| 1 2 3 4 5 6}.minute` | First minute-sized group: `minute:{1 2 3}`. |
| `hour:{second \| 1 2 3 4 5 6}@[2]` | Second member in the hour's minute grouping: `minute:{4 5 6}`. |
| `hour:{second \| 1 2 3 4 5 6}@[-1]` | Last minute, also `minute:{4 5 6}`; requires a proved source endpoint. |
| `hour:{second \| 1 2 3 4 5 6}@[0:*2+{minute \| ...}]` | Two-minute span: `{minute:{1 2 3} minute:{4 5 6}}`. |
| `day:{second \| 1 2 3 4 5 6 7 8 9 10 11 12}@[2]@[1]` | First minute of the second hour: `minute:{7 8 9}`. |
| `hour:{minute \| minute:{second \| 1 2 3} minute:{second \| 4 5 6}}@[2]@[1]` | Select the second minute, then its first second: `4`. |

The source's represented extent and proved layout determine the available coordinates. A declaration's unit shape does not close an independently open source or justify an arbitrary regrouping. Item and span boundaries retain R06's indexing rules. This operation groups source occurrences; it does not average their numeric readings or merely divide a displayed number.

**Identity and constraints.** A regrouped container is distinct from the supplying list. Its represented leaves retain their original source occurrences, inherited unit qualifications and access constraints. Later selections and permitted assertions follow those sources. Equal payloads at different positions remain distinct, and a failed multi-member assertion publishes no partial edits. Compact printing may hide a member's inherited unit name or an applied list specification without removing that meaning.

#### Sparse units and adjacent windows

The sparse fixture uses a different definition: `#second=_` and `#minute=*60+{second|...}`. With these definitions, `*4+{minute|...}@[3]` can return the third native minute as `minute:*60+{second|...}`, retaining its known count and unknown contents. Repeated reads retain that occurrence's identity without expanding the skipped minutes.

A typed `<minute>` capture requests that composite unit; a numeric `<_>` capture still requests a base-unit member. The unit, alignment and source extent must justify the coordinate. A sixty-second prefix cannot be treated as one minute if its surrounding correspondence does not establish the required alignment.

An established composite unit window can supply adjacent windows of the same width through ordinary neighbour navigation. Its leaves keep their own structural neighbours. Unknown endpoints remain partial; changing the supplying layout requires fresh coordinate evidence. This does not establish temporal alignment between independent histories; see R09.

Current limits include native sparse composite writes, arbitrary regrouping inside composite gaps, unproved ancestor predicates and general heterogeneous conversion. An available compressed recipe does not prove that those operations are valid. These unit-correspondence extensions are tracked in Appendix A24.

#### Numeric scales and grid conventions

`Resources/scaledPhase.pr` supplies `phase-reading(turn)`, `scaled-phase(n,k)`, `phase-tick(n,turn)` and `tick-width(n)`. The examples below require that model. Its phase domain includes zero and excludes one full turn; `n` is a positive whole subdivision count and `k` a whole, zero-based tick with `0 ≤ k < n`.

| Syntax/example | Result / meaning |
| --- | --- |
| `phase-reading(1/2)` | A half-turn reading and 180 degrees without selecting a subdivision count. |
| `scaled-phase(4,1).degrees` | First tick after zero on a four-division grid: `degrees:*360+90`. |
| `scaled-phase(3,1).turns` | Exact `turns:*1+1/3`; a rounded decimal is not the same rational value. |
| `phase-tick(4,1/2)` | The half-turn is `*4+2` on this grid. |
| `phase-tick(6,1/2)` | The same coordinate is `*6+3` on a different grid. |
| `phase-tick(4,1/8)` | Reject: the reading lies between whole ticks. `phase-tick(8,1/8)` gives `*8+1`. |
| `scaled-phase(4,4)` | Reject at the excluded endpoint; the model does not automatically wrap the tick to zero. |
| `tick-width(3)` | Exact width `1/3` of a turn. |

A tick coordinate is distinct from a list-item index: tick zero can be represented by the first item, addressed with `@[1]`. The subdivision count 4 in the quarter-turn example is also distinct from the degree representation's extent 360. Neither number measures temporal duration.

Wrapping is an explicit operation or model convention. For example, `(.mod 360 360)` gives 0; a plain `*360+_` supplies a numeric domain rather than an automatic wrap rule. `(.affine x a b)` computes `a*x+b` in the chosen coordinates, and its scalar result does not automatically inherit the input's nominal unit, state extent or cyclic convention. Detailed operation rules are in R03.

#### Recorded World kernel vocabulary

The following names and signatures are recorded in the Proteus design contract under D132–D134. They are ordinary definitions supplied by the World kernel. That separate repository could not be retrieved for this edit, so their current source and loading path remain a verification task in Appendix A23; the bundled phase model above was inspected directly.

| Vocabulary/example | Recorded meaning |
| --- | --- |
| `rat-closed(lo,hi)` | Model wrapper for a closed rational interval, using the built-in `..(lo hi)` relationship. |
| `shared-scale({1/4 1/6}).multiple-of` | Record the required divisor of a compatible subdivision count; here 12. It does not choose the count 12. |
| `refine-scale(scale,reading)` | Return a new scale incorporating another reading's denominator requirement. |
| `scale-density(scale,n)` | Check a proposed positive whole count against the scale's required divisor. |
| `scale-width(scale,n)` | Obtain the exact reciprocal width for a compatible count. |
| `tick-at(scale,n,turn)` | Convert a coordinate in the closed unit interval into a whole tick on the chosen scale. |
| `turn-at-tick(scale,n,k)` | Convert a whole tick back to its coordinate; the closed convention admits the final tick `k=n`. |
| `sample-turn(segment,progress)` | Apply a represented segment's signed span and origin to progress in `[0,1]`, without choosing or allocating a tick grid. |

For readings `1/4` and `1/6`, compatible subdivision counts are all positive multiples of 12. Both 12 and 24 satisfy the factor requirement; 18 does not. A missing count stays unresolved. The primitive `(.common-denominator {1/4 1/6})` computes the factor 12, while `(.positive-multiple 24 12)` checks a particular count. The model supplies the record and its interpretation.

The closed unit interval admits both endpoints: `n` equal subdivisions give ticks 0 through `n`. The bundled cyclic phase grid instead uses ticks 0 through `n−1`. A caller must use the selected model's endpoint convention; sharing the word “turn” does not make these domains interchangeable.

#### Resolution, storage and measurement

- **Density is not chosen by a reading.** An exact rational coordinate can be known before its grid. Multiple compatible densities remain possible, and several readings can constrain one shared count without choosing it.
- **Storage and logical extent are separate.** `++(*3+2 *254+0)` widens the structural extent to `*256+2` while retaining reading 2. A model decoding a three-state value from that wider representation must still enforce the logical domain and whole-state requirement.
- **Reported precision needs a model.** `0.25` is an exact rational literal. It does not automatically mean a measurement rounded to two decimal places. A measurement model must supply a quantization cell, tolerance or error bound.
- **Endpoint policy matters.** D134 records lower-inclusive, upper-exclusive floor bins, translated bins for nearest-quantum measurements, and closed intervals for explicit symmetric error bounds. Tie rules and error conventions belong to those models rather than decimal formatting.
- **Shared facts remain shared.** Two uses of one reading or subdivision count retain their correlation. Distinct observations do not become identical because their units or printed values match. General inverse solving and products of unresolved readings retain R03's limits.

Model documentation should identify the defining file/version, required imports, base units, origin, conversion rule, endpoints, wrapping policy and any density or error assumptions. This is especially useful when similarly named application models use different conventions.

#### Sources and status

**Coverage:** R10 documented against Proteus `main` at `2436d02f`, confirmed current for this edit. Bundled model definitions, current implementation and maintained fixture expectations were inspected; examples were not rerun. World kernel names are explicitly identified as design-contract documentation pending direct source verification. Appendix A06 is complete as a documentation task; A23–A24 retain the source and implementation follow-ups.

**Implementation and example evidence:** [Model ownership and fixture loading](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Resources/README.md), [small unit definitions](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Resources/TestFiles/timeTestCases.pr), [typed unit selections](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/timeTests.tst), [sparse unit definitions](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Resources/TestFiles/sparseTestCases.pr), [sparse captures](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/sparseTests.tst), [unit correspondence implementation](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/WorldManager.dog), [bundled phase model](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Resources/scaledPhase.pr), [phase examples](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/scaledPhaseTests.tst), [exact coordinate operations](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/rationalScaleTests.tst) and [linear-domain checks](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Tests/linearReadingTests.tst).

**Source decisions:** D040–D042 (homogeneous unit coordinates, inherited types and sparse windows), D124–D125 (source occurrence identity and native sparse units), D127 (adjacent unit windows), and D131–D134 (scaled rational readings, closed coordinates, shared scales and measurement constraints), in the [decision log](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design-decisions.md), with the [normalization contract](https://github.com/BruceDLong/Proteus/blob/2436d02f73d88db371efb364418aa65d86d26fef/Docs/Design%20Docs/normalization-design.md).

<!-- END DERIVED REFERENCE: R01-R10 -->

<a id="appendix-b"></a>

## Appendix B. One-page syntax cheat sheet

The compact forms below are derived from Chapter 17's source reference. The matching [Proteus-Cheat-Sheet.pdf](Proteus-Cheat-Sheet.pdf) is laid out as one printable page; [Proteus-Cheat-Sheet.md](Proteus-Cheat-Sheet.md) is its editable text. Chapters 2–3 give the introductory walkthrough.

<!-- BEGIN DERIVED CHEAT SHEET -->
### Values and unknowns — R01–R03

| Purpose | Spelling / tiny example | Note |
| --- | --- | --- |
| Known / unknown | `12   1/4   _   $   ?` | Number, exact rational; unknown number, string, or type/value. |
| Size and value | `*4+2` | Size 4, value 2. Size alone does not require whole readings. |
| Text / comments | `'text'   // note   /* note */` | Strings are byte-sized. Backslash escapes are not decoded. |

### Templates and individuals — R05

| Purpose | Spelling / tiny example | Note |
| --- | --- | --- |
| Define a template | `#ticket = {row, seat}` | Assume #row = _ and #seat = _. |
| Read / apply | `#ticket` / `ticket` | Read the template; apply it. |
| Construct and name | `#my-ticket = ticket:{seat:12}` | Bind one individual; its row remains unknown. |
| Refer / select a part | `%my-ticket.seat` | Read the bound individual's seat. |
| Alias a template | `#other = #ticket` | Another name for the template. |
| Alias an individual | `#same = %my-ticket` | Another name for the same occurrence. |

### Lists and grouping — R01, R04

| Purpose | Spelling / tiny example | Note |
| --- | --- | --- |
| Group one expression | `({1 2})` | One grouped list. () and (A B) are invalid. |
| Describe members | `{row:3, seat:12}` | Commas mean unordered; without commas, context decides order. |
| Embed members | `{1 &{2 3} 4}` | Four logical members; a nested {2 3} alone counts as one. |
| Open span / known count | `*3+{...}` | Three member positions; their contents are unspecified. |

### Joins and folds — R03–R04, R08

| Purpose | Spelling / tiny example | Note |
| --- | --- | --- |
| Numeric composition | `**(*4+2 *3+1)` | Result *12+7. Whitespace separates ordered operands. |
| Join strings | `**('Box ' 'office')` | Result 'Box office'. Also supports list concatenation. |
| Structural sum | `++(*4+2 *3+1)` | Result *6+3. Multi-operand sums require whole numbers. |
| Fold list members | `++(&%items)` | Supply an ordered numeric list. Empty ++() gives *1+0. |

### Selection and paths — R06

| Purpose | Spelling / tiny example | Note |
| --- | --- | --- |
| Item index | `{10 20 30}@[1]` | First item: 10; @[-1] needs a known endpoint. |
| Slice after a boundary | `{1 2 3}@[0:*2+{...}]` | Two members from boundary 0; item indices start at 1. |
| Marked pattern | `[_ <_> _] <~ {1 2 3}` | Return 2; <~ supplies the source for matching. |
| Structural neighbours | `%prev   %next` | Structural neighbours in a logical ordered scope. |

### Identity and qualification — R07

| Purpose | Spelling / tiny example | Note |
| --- | --- | --- |
| Strict identity | `_ = 7` | Value 7; incompatible required known facts reject. |
| Whole content | `$ == 'Hello'` | Whole matching must reach the source endpoint. |
| Prefix | `*3+$ =: 'Hello'` | Result 'Hel'; ==: also qualifies members. |
| Exclude a match | `!_` | Exclude proved numeric matches; missing proof stays open. |

### Alternatives and repetition — R08

| Purpose | Spelling / tiny example | Note |
| --- | --- | --- |
| Finite alternatives | `*_+[1 2 3]` | Retain viable candidates; plain [1 2 3] selects 3. |
| Member specification | `{*256+_ \| 1 2}` | Each member meets the specification before \|. |
| Repeated requirement | `*4+{*256+_ \| ...}` | Four members under a specification; ^^N and ^L are absent. |

### Time and state — R09

| Purpose | Spelling / tiny example | Note |
| --- | --- | --- |
| Temporal neighbours | `{T 1 %tprev 3}` | The second state reads 1; %tnext reads the successor. |
| Independence | `%?` | Withhold implicit retention; assert no inequality. |
| History / state views | `{T 4 8}^T@[2]^S` | Read state 2 as 8, retaining its occurrence. |

### Coordinates and models — R03, R10

| Purpose | Spelling / tiny example | Note |
| --- | --- | --- |
| Closed interval | `..(0 1)` | Closed bounds, independent of size or density. |
| Explicit wrap | `(.mod -9/4 1)` | Residue 3/4; size alone does not request wrapping. |
| Model-defined units | `#minute = *60+{second \| ...}` | Requires second; the model supplies units and conversions. |

Names require the indicated model definitions. These are separate forms, not one combined program. Unknown, partial, rejected and unsupported are distinct outcomes. Detailed call coverage remains A04; runtime verification remains A09. Quoted string escapes are implemented under D135. Long strings remain A03.
<!-- END DERIVED CHEAT SHEET -->
