# Proteus — Syntax Cheat Sheet

Derived from [Proteus-Language-Reference.md](Proteus-Language-Reference.md), R01–R10, for Proteus `2436d02f`. This is the editable source of the [one-page printable sheet](Proteus-Cheat-Sheet.pdf). For the walkthrough, see [Chapter 2](Proteus-Guide.md#chapter-2) and [Chapter 3](Proteus-Guide.md#chapter-3); [Chapter 17](Proteus-Guide.md#chapter-17) contains the fuller rules.

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

Names require the indicated model definitions. These are separate forms, not one combined program. Unknown, partial, rejected and unsupported are distinct outcomes. Detailed call coverage remains A04; runtime verification remains A09. String escapes and long strings remain A01–A03.
