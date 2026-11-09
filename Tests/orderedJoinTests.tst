# The operand list is traversed in place, including siblings after an embedding.
join/inlineSum, embedded addends then sibling, ++( &{*4+2 *3+1} *2+1 ), *7+4, graph
join/inlineProduct, embedded radix digits then sibling, **( &{*4+2 *3+1} *2+1 ), *24+15, graph
join/direct, direct operands, ++(*4+2 *3+1), *6+3, graph
join/displacement, contextual displacement, **(*4+2 +(*3+1)), *4+3, graph
join/empty, empty multiplicative join, **(), ?, graph
join/zero, zero-copy sum, ++(), *1+0, graph
join/emptyEmbedding, skip empty span, ++(*4+2 &{} *3+1), *6+3, graph
join/nested, nested and empty spans, ++( &{&{} *4+2 &{*3+1}} *2+1 ), *7+4, graph
join/mixed, sum inside radix composition, **(++(*4+2 *3+1) *2+1), *12+7, graph
join/big, exact beyond machine word, ++(*18446744073709551615+18446744073709551614 *3+2), *18446744073709551617+18446744073709551616, graph
join/computedSize, additive join as size, *++(*4+2 *3+1)+1, *3+1, graph
join/listOperand, ordinary list operand stays whole, **({{1 2}} {{3}}), {{1 2} {3}}, graph
join/singleton, grouped list stays one operand, ++(({1 2})), {1 2}, graph
join/embeddedText, text across a nested span, **('a' &{'b' &{'c'}} 'd'), 'abcd', graph
join/parameter, reference between siblings, {#total(xs)=++(*2+1 &%xs *2+1)}\ntotal({*4+2 *3+1})//:total({*4+2 *3+1}):*8+5, -, multi
join/sameValue, repeated known parameter, {#twice(x)=++(%x %x)}\ntwice(*4+2)//:twice(*4+2):*7+4, -, multi
join/parse, contiguous syntax round trip, ++( &{*4+2 *3+1} *2+1 ), ++(&{*4+2 *3+1} *2+1), parse
join/correlatedOdd, repeated four-state input excludes odd totals, {#twice(x)=++(%x %x)}\ntwice(*4+_) = 3//:<REJECTED>, -, multi
join/offsetBounds, known offset excludes a smaller sum, ++(*4+2 *3+_) = 1, <REJECTED>, graph
join/listSpec, per-member requirement supplies numeric operands, ++(*4+2| _ _), *7+4, graph
join/emptySpanOnly, empty embedding supplies no multiplicative operands, **(&{}), ?, graph
join/stringOrder, an ordinary nested list is one list operand, **(&{{1} {2}} {3}), {1 2 3}, graph
join/named, named occurrence contributes its list members, {#items={_|...}}\nitems:{*4+2 *3+1}\n#xs=%ctx.items\n++(&%xs *2+1)//:*7+4, -, multi
join/operandExtent, operand count differs from result extent, ++(~2| *4+2 *3+1), *6+3, graph
join/explicitExtent, both operand and result extents, *6+ ++(~2| *4+2 *3+1), *6+3, graph
join/emptyText, empty product with text kind, **() = $, '', graph
join/emptyNumeric, empty product with numeric kind, **() = _, *1+0, graph
join/emptyInText, nested empty product takes text identity, **('a' **() 'b'), 'ab', graph
join/listItem, select first concatenated element, **(&{{1} {2}} {3})@[1], 1, graph
join/listItem2, select second concatenated element, **(&{{1} {2}} {3})@[2], 2, graph
join/listItem3, select third concatenated element, **(&{{1} {2}} {3})@[3], 3, graph
join/listGroup, select group, ({1 2})@[1], 1, graph
join/listTpl, select templated result, **({1} &{{2} {3}})@[3], 3, graph
join/sibling, previous embedded member, ++(&{*3+1 %prev}), *5+2, graph
join/outer, outer previous, {*3+1 ++(%prev *2+1)}, {*3+1 *4+2}, graph
join/previousGrouped, previous embedded member viewed in group, ++(&{*3+1 (%prev)}), *5+2, graph
join/offsetEvenOdd, shifted repeated input excludes even values, {#twice(x)=**(++(%x %x) +1)}\ntwice(*4+_) = 4//:<REJECTED>, -, multi
join/source, join used as embedded source layout, {0 &**(&{{1} {2}} {3}) 4}@[2], 1, graph
join/source2, second element in first operand, {0 &**(&{{1 2} {3 4}} {5}) 6}@[3], 2, graph
join/pattern, joined list supplies pattern, [_ <_> _] <~ **(&{{1} {2}} {3}), 2, graph
join/parameterSource, select from join over embedded parameter, {#pick(xs)=**(&%xs {4})@[2]}\npick({{1} {2}})//:pick({{1} {2}}):2, -, multi
join/next, next embedded member, ++(&{%next *3+1}), *5+2, graph
join/emptyListKind, empty product with list kind, **() = {}, {}, graph
join/openTail, unresolved suffix keeps the join expression, ++(*3+2 ...), ++(*3+2 ... ), graph
join/unorderedEmbedding, unordered members supply no operand order, ++(&{*4+2\, *3+1}), ++(&{*4+2\, *3+1}), graph
join/computedKernel, group a computed size on an open list, * **(20000 +_) +{T epoch| ...}, ***(20000 +_)+{T epoch|  ... }, parse
