# One-based numeric projections near the end of the finite six-window layout.
periodic/e0r0c1x1, scalar projection, history(*3+1\,*3+_\,*3+0)@[393211]@[1]//:*3+1, -, world, TestFiles/periodicHistory.pr
periodic/e1r1c2x2, scalar projection, history(*3+2\,*3+_\,*3+1)@[393212]@[2]//:*3+0, -, world, TestFiles/periodicHistory.pr
periodic/e2r2c1x1, scalar projection, history(*3+1\,*3+_\,*3+2)@[393213]@[1]//:*3+0, -, world, TestFiles/periodicHistory.pr
periodic/e2r3c2x1, scalar projection, history(*3+1\,*3+_\,*3+2)@[393214]@[2]//:*3+2, -, world, TestFiles/periodicHistory.pr
periodic/e1r4c2x1, scalar projection, history(*3+_\,*3+1\,*3+1)@[393215]@[2]//:*3+0, -, world, TestFiles/periodicHistory.pr
periodic/e0r5c1x2, scalar projection, history(*3+_\,*3+2\,*3+0)@[393216]@[1]//:*3+2, -, world, TestFiles/periodicHistory.pr
periodic/e2r5c2x0, scalar projection, history(*3+0\,*3+_\,*3+2)@[393216]@[2]//:*3+0, -, world, TestFiles/periodicHistory.pr
periodic/overlapRight, one component, history(*3+1\,*3+_\,*3+1)@[19683]@[1]//:*3+2, -, world, TestFiles/periodicHistory.pr
periodic/overlapLeft, one component, history(*3+1\,*3+_\,*3+1)@[19682]@[2]//:*3+2, -, world, TestFiles/periodicHistory.pr
periodic/absentRaw, one component, history(*3+1\,*3+_\,*3+2)@[393211]@[2]//:*3+_, -, world, TestFiles/periodicHistory.pr
periodic/thirdTemporalUpdate, overlapping temporal recurrence, {#advance(w\,p)={%w@[2] {{0 1 2} {1 2 0} {2 0 1}}@[(%p +1)]@[(%w@[1] +1)]} #history(a\,b\,phase)={T {%a %b} &{T advance(%tprev\,%phase) | {_ _} {_ _} {_ _}}}}\nhistory(1\,_\,1)@[4]@[2]//:0, -, multi
periodic/recoverPhase, recover phase, {#difference={{*3+0 *3+1 *3+2} {*3+2 *3+0 *3+1} {*3+1 *3+2 *3+0}} #phase(w)={difference@[(%w@[1] +1)]@[(%w@[3] +1)]}}\nphase({*3+1 *3+_ *3+0})@[1]//:*3+2, -, multi
