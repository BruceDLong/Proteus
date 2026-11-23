# Linear constraints over exact rational coordinates and discrete ticks.
linear/possible, possible, {#d(x\,y)=**((%x) -(%y))}\nd(..(10.25 10.35)\,..(10.25 10.35)) = 1/100//:<PARTIAL>, -, multi
linear/closedEnd, closedEnd, {#d(x\,y)=**((%x) -(%y))}\nd(..(10.25 10.35)\,..(10.25 10.35)) = 1/10//:<PARTIAL>, -, multi
linear/outside, outside, {#d(x\,y)=**((%x) -(%y))}\nd(..(10.25 10.35)\,..(10.25 10.35)) = 11/100//:<REJECTED>, -, multi
linear/openHigh, openHigh, {#d(x\,y)=**((%x) -(%y))}\nd(**(*1/10+_ +10.25)\,**(*1/10+_ +10.25)) = 1/10//:<REJECTED>, -, multi
linear/openLow, openLow, {#d(x\,y)=**((%x) -(%y))}\nd(**(*1/10+_ +10.25)\,**(*1/10+_ +10.25)) = -1/10//:<REJECTED>, -, multi
linear/openInterior, openInterior, {#d(x\,y)=**((%x) -(%y))}\nd(**(*1/10+_ +10.25)\,**(*1/10+_ +10.25)) = -1/100//:<PARTIAL>, -, multi
linear/same, same, {#d(x)=**((%x) -(%x))}\nd(..(10.25 10.35))//:d(..(10.25 10.35)):0, -, multi
linear/shift, shift, {#d(x)=**(%x +1 -(%x))}\nd(..(10.25 10.35))//:d(..(10.25 10.35)):1, -, multi
linear/radix, radix, {#d(x)=**(%x /2+0 -((.affine %x 1/2 0)))}\nd(..(10.25 10.35))//:d(..(10.25 10.35)):0, -, multi
linear/shiftedEquality, shiftedEquality, {#d(x)=(.affine %x 2 0) = (.affine %x 2 1)}\nd(_)//:<REJECTED>, -, multi
linear/fractionalLattice, fractionalLattice, {#d(x)=(.affine (++(%x *1+0)) 3/2 1/4)}\nd(_) = 5/4//:<REJECTED>, -, multi
linear/fractionalLatticeHit, fractionalLatticeHit, {#d(x)=(.affine (++(%x *1+0)) 3/2 1/4)}\nd(_) = 7/4//:<PARTIAL>, -, multi
linear/mixed, mixed, {#d(x\,y)=(.affine (++(%x *1+0)) 2 %y)}\nd(_\,..(0 1)) = 1/3//:<PARTIAL>, -, multi
linear/nonlinear, nonlinear, {#d(x\,y)=(.affine %x %y 0)}\nd(_\,_)//:<PARTIAL>, -, multi
linear/fractionalWhole, fractionalWhole, ++(..(1/4 3/4) *1+0), <REJECTED>, graph
linear/firstWhole, firstWhole, ++(..(1/4 5/4) *1+0) = 1, <PARTIAL>, graph
linear/emptyMultiple, emptyMultiple, (.positive-multiple ..(1 5) 12), <REJECTED>, graph
linear/rejectZero, rejectZero, (.positive-multiple _ 12) = 0, <REJECTED>, graph
linear/rejectFraction, rejectFraction, (.positive-multiple _ 12) = 1/2, <REJECTED>, graph
linear/refineMultiple, refineMultiple, (.positive-multiple (.positive-multiple _ 4) 6) = 18, <REJECTED>, graph
linear/refineHit, refineHit, (.positive-multiple (.positive-multiple _ 4) 6) = 24, <PARTIAL>, graph
linear/zeroGain, zeroGain, (.affine _ 0 42), 42, graph
linear/wholeOpenEnd, last even reading in repeated four-state input, {#d(x)=++(%x %x)}\nd(*4+_) = 6//:<PARTIAL>, -, multi
