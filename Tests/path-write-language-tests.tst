write/prev, previous from selected source, {*10+1 _}@[2]=%prev;, {*10+1 *10+1}, norm
write/next, next from selected source, {_ *10+2}@[1]=%next;, {*10+2 *10+2}, norm
write/embeddedPrev, previous across embedded span, {*10+1 &{_}}@[2]=%prev;, {*10+1 &{*10+1}}, norm
write/embeddedNext, next across embedded span, {&{_} *10+2}@[1]=%next;, {&{*10+2} *10+2}, norm
