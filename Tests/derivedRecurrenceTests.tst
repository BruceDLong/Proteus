# All projected windows use just the one-step listSpec in derivedRecurrence.pr.
derived/sharedSix, sharedSix, fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1]//:*3+1, -, world, TestFiles/derivedRecurrence.pr
derived/freshSix, freshSix, fresh(*3+1\,*3+_)@[7]@[1]//:*_+[*3+0 *3+1 *3+2], -, world, TestFiles/derivedRecurrence.pr
derived/sharedOne, sharedOne, fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[3]@[1]//:*_+[*3+0 *3+1 *3+2], -, world, TestFiles/derivedRecurrence.pr
derived/differentSeed, differentSeed, fixed(*3+_\,*3+2\,*_+[*3+0 *3+1 *3+2])@[7]@[2]//:*3+2, -, world, TestFiles/derivedRecurrence.pr
derived/concreteRight, concreteRight, fixed(*3+1\,*3+_\,*3+1)@[4]@[2]//:*3+0, -, world, TestFiles/derivedRecurrence.pr
derived/sixMillion, sixMillion, fixed(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[6000001]@[1]//:*3+1, -, world, TestFiles/derivedRecurrence.pr
derived/upperBoundary, upperBoundary, fixed(*3+_\,*3+2\,*3+1)@[6000001]@[2]//:*3+2, -, world, TestFiles/derivedRecurrence.pr
derived/farInterior, farInterior, fixed(*3+1\,*3+_\,*3+2)@[19683]@[1]//:*3+0, -, world, TestFiles/derivedRecurrence.pr
derived/noncommutingRows, value from stepping complete windows, {#update(left\,turn)={%left@[2] {{*3+0 *3+1 *3+2} {*3+1 *3+0 *3+2} {*3+0 *3+2 *3+1}}@[(%turn +1)]@[(%left@[1] +1)]} #bound(u\,v\,s)={T {%u %v} &*48+{T update(%tprev\,%s) | ...}} #unbound(u\,v)={T {%u %v} &*48+{T update(%tprev\,*_+[*3+0 *3+1 *3+2]) | ...}}}\nbound(*3+0\,*3+1\,*_+[*3+0 *3+1 *3+2])@[7]@[1]//:*_+[*3+0 *3+1], -, multi
derived/noninvertibleRows, value from stepping complete windows, {#update(left\,turn)={%left@[2] {{*3+1 *3+2 *3+2} {*3+2 *3+0 *3+0}}@[(%turn +1)]@[(%left@[1] +1)]} #bound(u\,v\,s)={T {%u %v} &*48+{T update(%tprev\,%s) | ...}} #unbound(u\,v)={T {%u %v} &*48+{T update(%tprev\,*_+[*2+0 *2+1]) | ...}}}\nbound(*3+0\,*3+1\,*_+[*2+0 *2+1])@[7]@[2]//:*_+[*3+0 *3+2], -, multi
derived/storedPairs, shared phase in six stored pair placeholders, {#advance(w\,p)={%w@[2] { {*3+0 *3+1 *3+2} {*3+1 *3+2 *3+0} {*3+2 *3+0 *3+1} }@[(%p +1)]@[(%w@[1] +1)]} #history(a\,b\,p)={T {%a %b} &{T advance(%tprev\,%p) | {_ _} {_ _} {_ _} {_ _} {_ _} {_ _} }} #fresh(a\,b)={T {%a %b} &{T advance(%tprev\,*3+_) | {_ _} {_ _} {_ _} {_ _} {_ _} {_ _} }}}\nhistory(*3+1\,*3+_\,*_+[*3+0 *3+1 *3+2])@[7]@[1]//:*3+1, -, multi
