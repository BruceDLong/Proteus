# Explicit word calls and application-local parameter bindings.
parser/calls/named, named, give(actor::Bruce\, item::'box'\, object::Arthur), give(actor::Bruce\, item::'box'\, object::Arthur), parse
parser/calls/positional, positional, give( Bruce \, 'box' \, Arthur ), give(Bruce\, 'box'\, Arthur), parse
parser/calls/space, space, {give (3)}, {give (3)}, parse
parser/calls/empty, empty, give(), give(), parse
parser/calls/comment, comment, give(item:: /* item */ 3), give(item::3), parse
parser/calls/nested, nested, outer(inner(value::3)), outer(inner(value::3)), parse
parser/calls/field, field, turn(item::%prev).x, turn(item::%prev).x, parse
parser/calls/view, view, turn(item::%prev)^T@[1]^S, turn(item::%prev)^T@[1]^S, parse
parser/calls/forward, forward, {#forward(item)=turn(item::%item)}, {#forward(item) = turn(item::%item)}, parse
parser/calls/embedded, embedded, {&pair(first::1\,second::2)}, *_+{&pair(first::1\, second::2)}, parse
parameters/identity, identity, {#ident(item)=%item}\nident(item::7)//:ident(item::7):7, -, multi
parameters/positional, positional, {#ident(item)=%item}\nident(7)//:ident(7):7, -, multi
parameters/order, order, {#pair(first\, second)={%first %second}}\npair(second::2\, first::1)//:pair(second::2\, first::1):{1 2}, -, multi
parameters/arithmetic, arithmetic, {#inc(item)=(%item +1)}\ninc(item::4)//:inc(item::4):5, -, multi
parameters/instances, instances, {#ident(item)=%item}\n{ident(3) ident(8)}//:{ident(3):3 ident(8):8}, -, multi
parameters/nested, nested, {#ident(item)=%item #outer(item)=ident(item::%item)}\nouter(item::9)//:outer(item::9):9, -, multi
parameters/prev, prev, {#ident(item)=%item}\n{7 ident(item::%prev)}//:{7 ident(item::%prev):7}, -, multi
parameters/embedded, embedded, {#pair(first\, second)={%first %second}}\n{0 &pair(1\, 2) 3}//:{0 &pair(1\, 2):{1 2} 3}, -, multi
parameters/field, field, {#pick(item)=%item.x}\npick(item::{x:7})//:pick(item::{x:7}):7, -, multi
parameters/selected, selected, {#pair(first\, second)={%first %second}}\npair(1\, 2)@[2]//:2, -, multi
parameters/missing, missing, {#pair(first\, second)={%first %second}}\npair(first::1)//:pair(first::1):{1 ?}, -, multi
parameters/independent, independent, {#constant(item)=5}\nconstant()//:constant():5, -, multi
parameters/constrained, constrained, {#fixed(item)=%item=7}\nfixed()//:fixed():7, -, multi
parameters/repeatedInner, repeatedInner, {#ident(item)=%item #duo(item)={ident(2) ident(3)}}\nduo()//:duo():{ident:2 ident:3}, -, multi
parameters/localShared, localShared, {#fixed(item)={%item=7 %item}}\nfixed()//:fixed():{7 7}, -, multi
parameters/localReverse, localReverse, {#fixed(item)={%item %item=7}}\nfixed()//:fixed():{7 7}, -, multi
parameters/lazyField, lazyField, {#pick(item)=%item.x}\npick(item::{x:7 unavailable^T})//:pick(item::{x:7 unavailable^T}):7, -, multi
parameters/tprev, tprev, {#inc(item)=(%item +1)}\n{T 0 inc(item::%tprev)}//:{T 0 inc(item::%tprev):1}, -, multi
parameters/recordBinding, recordBinding, {#pick(item)=%item.x}\n{ {x:7} pick(item::%prev)}//:{{x:7} pick(item::%prev):7}, -, multi
parameters/sameRequest, sameRequest, {#ident(item)=%item}\n{ident(2) ident(3) ident(4)}//:{ident(2):2 ident(3):3 ident(4):4}, -, multi
parameters/listSpec, listSpec, {#count(step)={T (%tprev +(%step)) | 0 _ _}}\ncount(step::2)//:count(step::2):{T 0 2 4}, -, multi
parameters/listSpecSupplied, listSpecSupplied, {#filled(item)={(%item) | ...}}\nfilled(7):{_ _}//:filled(7):{7 7}, -, multi
parameters/learnCaller, learnCaller, {#fixed(item)=%item=7}\n{_ fixed(item::%prev)}//:{7 fixed(item::%prev):7}, -, multi
parameters/nestedArgumentScope, nestedArgumentScope, {#pick(item)=%item@[1] #outer(value)=pick(item::{%value 2})}\nouter(value::3)//:outer(value::3):3, -, multi
parameters/nestedSameWord, nestedSameWord, {#ident(item)=%item}\nident(ident(3))//:ident(ident(3)):3, -, multi
parameters/unusedArgument, unusedArgument, {#constant(item)=5}\nconstant(item::7^T)//:constant(item::7^T):5, -, multi
parameters/recordResult, recordResult, {#point(x\, y)={x:(%x)\, y:(%y)}}\npoint(3\,4)//:point(3\, 4):{x:3\, y:4}, -, multi
parameters/recordField, recordField, {#point(x\, y)={x:(%x)\, y:(%y)}}\npoint(3\,4).x//:x:3, -, multi
parameters/stringArgument, stringArgument, {#ident(item)=%item}\nident('box')//:ident('box'):'box', -, multi
parameters/bare, bare, {#fixed(item)=%item=7}\nfixed//:fixed:7, -, multi
