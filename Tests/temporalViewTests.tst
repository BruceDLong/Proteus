# Views of explicitly represented discrete histories use ordinary normalization.
view/history/read, T view keeps an explicit history, {T 4 8}^T@[2]^S, 8, norm
view/state/window, A selected state supplies its own temporal window, {T 4 8}@[2]^S^T, {T 8}, norm
view/state/roundTrip, State and temporal views retain the selected occurrence, {T 4 8}@[2]^S^T@[1]^S, 8, norm
view/state/directT, T can select the support of a represented state directly, {T 4 8}@[2]^T@[1]^S, 8, norm
view/state/repeatT, Repeating T does not add another temporal level, {T 4 8}@[2]^T^T@[1]^S, 8, norm
view/state/repeatS, Repeating S preserves the selected state, {T 4 8}@[2]^S^S, 8, norm
view/parent/listSpec, A view acquires the original parent listSpec, {T (%tprev +1)| 0 _ _}@[3]^S^T@[1]^S, 2, norm
view/parent/overlay, A view acquires the original parent overlay, ({T _ _} == {T 3 7})@[2]^S, 7, norm
view/embedded/state, Embedded storage keeps logical temporal context, {T 4 &{8 12} 16}@[3]^S^T@[1]^S, 12, norm
view/nested/history, An ordinary nested history keeps its own temporal level, {T 4 {T 8 12} 16}@[2]^T@[2]^S, 12, norm
view/field/path, Paths after S address the selected record, {#x=_}\n{T {x:1} {x:2}}@[2]^S.x, x:2, multi
view/name/history, Names retain an individual's explicit history, {#x=_ #body=#{T {x\, ...}|...}}\n#Earth=body:{T {x:0} {x:1}}\n%Earth^T@[2]^S.x, x:1, multi
view/name/state, A named selected state retains its support, {#x=_ #body=#{T {x\, ...}|...}}\n#Earth=body:{T {x:0} {x:1}}\n#Now=%Earth@[2]^S\n%Now^T@[1]^S.x, x:1, multi
view/write/state, A checked field assertion survives another view, {#x=_ #body=#{T {x\, ...}|...}}\n#Earth=body:{T {x:0} {x:%?}}\n%Earth^T@[2]^S.x=7//:x:%? = 7\n%Earth@[2].x//:x:%? = 7\n%Earth@[1].x//:x:0, -, multi
view/write/window, A whole window assertion constrains its source state, {#body=#{T _|...}}\n#Earth=body:{T 0 %?}\n%Earth@[2]^T={T 7}//:{T %? = 7}\n%Earth@[2]^S//:%? = 7\n%Earth@[1]^S//:0, -, multi
view/support/boundaries, State and span views share the same support boundaries, {#body=#{T _|...}}\n#Earth=body:{T 4 8}\n%Earth@[2]^S.begin//:NULL:1\n%Earth@[2]^T.begin//:NULL:1\n%Earth@[2]^S.end//:NULL:2\n%Earth@[2]^T.end//:NULL:2, -, multi
view/prefix/represented, A view of a supplied prefix does not enumerate its open tail, {T 4 8 ...}@[2]^S^T@[1]^S, 8, norm
view/parameter/history, A definition can select a supplied history through its T view, {#sample(item)=%item^T@[2]^S}\nsample(item::{T 4 8}), sample(item::{T 4 8}):8, multi
view/parameter/state, Forwarding a selected state retains its original history support, {#sample(item)=%item^T@[1]^S}\nsample(item::{T 4 8}@[2]^S), sample(item::{T 4 8}@[2]^S):8, multi
view/support/explicitTime, An explicit selected time remains explicit inside another history, {#body=#{T _|...}}\n#Earth=body:{T 4 8}\n#Now=%Earth@[2]^S\n{T %Now^T^T@[1]^S}, {T 8}, multi
view/definition/context, An explicit represented layout retains T context from its definition, {#timeline=#{T _|...}}\ntimeline:{4 8}^T@[2]^S, 8, multi
view/part/scalar, A scalar part borrows its containing state's support, {T {4 5} {8 9}}@[2]@[1]^T@[1]^S, 8, norm
view/part/nested, Nested S parts share the represented whole state, {T {{4 5}} {{8 9}}}@[2]@[1]@[2]^S^T@[1]^S, 9, norm
view/part/field, A named part view reads the original field, {#x=_}\n{T {x:4} {x:8}}@[2]^S.x^T@[1]^S, x:8, multi
view/part/embedded, Embedded storage preserves the whole state support, {T &{{4 5} {8 9}}}@[2]@[1]^T@[1]^S, 8, norm
view/part/prefix, A part view does not enumerate an open history, {T {4 5} {8 9} ...}@[2]@[1]^T@[1]^S, 8, norm
view/part/overlay, A part view acquires enclosing overlay constraints, ({T {_ _} {_ _}} == {T {4 5} {8 9}})@[2]@[1]^T@[1]^S, 8, norm
view/part/listSpec, A part view acquires the whole state's listSpec, {T {4 5}| {_ _} {_ _}}@[2]@[2]^T@[1]^S, 5, norm
view/part/innerHistory, An inner T list keeps its independent temporal level, {T {{T 4 8}} {{T 3 7}}}@[2]@[1]^T@[2]^S, 7, norm
view/part/parameter, A forwarded part retains its support, {#sample(item)=%item^T@[1]^S}\nsample(item::{T {4 5} {8 9}}@[2]@[1]^S), sample(item::{T {4 5} {8 9}}@[2]@[1]^S):8, multi
view/part/name, A named part and published window retain the original occurrence, {#x=_ #body=#{T {x\, ...}|...}}\n#System=body:{T {x:4} {x:8}}\n#Part=%System@[2]^S.x^S\n#Window=%Part^T\n%Window@[1]^S, x:8, multi
view/part/write, A checked part window write is visible through the whole, {#x=_ #body=#{T {x\, ...}|...}}\n#System=body:{T {x:4} {x:%?}}\n%System@[2].x^T={T 8}//:{T x:%? = 8}\n%System@[2]^S.x//:x:%? = 8\n%System@[1]^S.x//:x:4, -, multi
view/part/definitionContext, Part views use the T context established by an ordinary definition, {#x=_ #body=#{T {x\, ...}|...}}\nbody:{{x:4} {x:8}}@[2].x^T@[1]^S, x:8, multi
view/part/boundaries, Whole and part views retain shared boundary identity across requests, {#x=_ #body=#{T {x\, ...}|...}}\n#System=body:{T {x:4} {x:8}}\n#Part=%System@[2].x^S\n%System@[2]^S.begin//:NULL:1\n%Part^S.begin//:NULL:1\n%Part^T.begin//:NULL:1\n%System@[2]^S.end//:NULL:2\n%Part^T.end//:NULL:2, -, multi
