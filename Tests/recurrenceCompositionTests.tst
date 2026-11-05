# Recurrence value proofs compose through ordinary normalization.
recurrence/composition/direct, direct, fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1]//:*3+1, -, world, TestFiles/derivedRecurrence.pr
recurrence/composition/parenthesized, parenthesized, (fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1])//:*3+1, -, world, TestFiles/derivedRecurrence.pr
recurrence/composition/inList, inList, {fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1]}//:{*3+1}, -, world, TestFiles/derivedRecurrence.pr
recurrence/composition/addition, addition, (fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1] +1)//:*3+2, -, world, TestFiles/derivedRecurrence.pr
recurrence/composition/parameter, parameter, {#id(x)=%x}\nid(fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1])//:id(fixed(*3+1\, *3+_\, *_+[*3+0 *3+1 *3+2])@[7]@[1]):*3+1, -, world, TestFiles/derivedRecurrence.pr
recurrence/composition/checked, checked, (fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1])= *3+1//:*3+1, -, world, TestFiles/derivedRecurrence.pr
recurrence/composition/contradiction, contradiction, (fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1])= *3+2//:<REJECTED>, -, world, TestFiles/derivedRecurrence.pr
recurrence/composition/namedCall, namedCall, fixed(a::*3+1\,b::*3+_\,p::*_+[*3+0 *3+1 *3+2])@[7]@[1]//:*3+1, -, world, TestFiles/derivedRecurrence.pr
recurrence/composition/constantIndex, constantIndex, fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[(6 +1)]@[1]//:*3+1, -, world, TestFiles/derivedRecurrence.pr
recurrence/forwarded, forwarded, {#forward(a\,p)=fixed(%a\,*3+_\,%p)@[7]@[1]}\nforward(*3+1\,*_+[*3+0 *3+1 *3+2])//:forward(*3+1\, *_+[*3+0 *3+1 *3+2]):*3+1, -, world, TestFiles/derivedRecurrence.pr
recurrence/filter, filter, fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[3]@[1]=*3+2//:*3+2, -, world, TestFiles/derivedRecurrence.pr
recurrence/record, record, {left:(fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1])\, right:(fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1])}//:{left:*3+1\, right:*3+1}, -, world, TestFiles/derivedRecurrence.pr
recurrence/templateAlias, templateAlias, {#aliased=#fixed}\naliased(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1]//:*3+1, -, world, TestFiles/derivedRecurrence.pr
recurrence/namedStep, namedStep, {#named(a\,b\,p)={T {%a %b} &*6+{T advance(p::%p\,w::%tprev) | ...}}}\nnamed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1]//:*3+1, -, world, TestFiles/derivedRecurrence.pr
recurrence/lexicalShadow, lexicalShadow, {#block()={#advance(w\,p)={%w@[2] {{*3+0 *3+0 *3+0} {*3+0 *3+0 *3+0} {*3+0 *3+0 *3+0}}@[(%p +1)]@[(%w@[1] +1)]} #local(a\,b\,p)={T {%a %b} &*6+{T advance(%tprev\,%p) | ...}} local(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1]}}\nblock()@[3]//:*3+0, -, world, TestFiles/derivedRecurrence.pr
recurrence/exactOrdinal, exactOrdinal, {#huge(a\,b\,p)={T {%a %b} &*9223372036854775806+{T advance(%tprev\,%p) | ...}}}\nhuge(*3+1\,*3+_\,*3+1)@[9007199254740993]@[1]//:*3+2, -, world, TestFiles/derivedRecurrence.pr
recurrence/signedMaximum, signedMaximum, {#huge(a\,b\,p)={T {%a %b} &*9223372036854775806+{T advance(%tprev\,%p) | ...}}}\nhuge(*3+1\,*3+_\,*3+1)@[9223372036854775807]@[1]//:*3+1, -, world, TestFiles/derivedRecurrence.pr
