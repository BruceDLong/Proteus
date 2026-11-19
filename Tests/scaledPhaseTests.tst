# A checked forward and inverse reading of the model in Resources/scaledPhase.pr.
phase/quarter, first tick on a four-division grid, scaled-phase(4\,1)//:scaled-phase(4\, 1):{turns:*1+0.25\, degrees:*360+90}, -, world, scaledPhase.pr
phase/third, equality with an exact fraction, *1+1/3 = scaled-phase(3\,1).turns//:turns:*1+1/3, -, world, scaledPhase.pr
phase/fine, prime subdivision count, *1+1/97 = scaled-phase(97\,1).turns//:turns:*1+1/97, -, world, scaledPhase.pr
phase/nearThird, rounded decimal differs, scaled-phase(3\,1).turns = 0.333333333333//:<REJECTED>, -, world, scaledPhase.pr
phase/thirdDegrees, exact third in degrees, scaled-phase(3\,1).degrees//:degrees:*360+120, -, world, scaledPhase.pr
phase/zero, first grid point, scaled-phase(4\,0).degrees//:degrees:*360+0, -, world, scaledPhase.pr
phase/last, last grid point, scaled-phase(4\,3).degrees//:degrees:*360+270, -, world, scaledPhase.pr
phase/end, upper endpoint, scaled-phase(4\,4)//:<REJECTED>, -, world, scaledPhase.pr
phase/below, negative tick, scaled-phase(4\,-1)//:<REJECTED>, -, world, scaledPhase.pr
phase/fractionalTick, whole ticks only, scaled-phase(4\,1/2)//:<REJECTED>, -, world, scaledPhase.pr
phase/zeroCount, zero subdivisions, scaled-phase(0\,0)//:<REJECTED>, -, world, scaledPhase.pr
phase/negativeCount, negative subdivisions, scaled-phase(-4\,1)//:<REJECTED>, -, world, scaledPhase.pr
phase/fractionalCount, fractional subdivisions, scaled-phase(3/2\,1)//:<REJECTED>, -, world, scaledPhase.pr
phase/knownTurn, known phase with no chosen grid, phase-reading(1/2)//:phase-reading(1/2):{turns:1/2\, degrees:*360+180}, -, world, scaledPhase.pr
phase/unknownCount, keep the unresolved divisor, phase-tick(_\,1/2)//:phase-tick(_\, 1/2), -, world, scaledPhase.pr
phase/unknownTick, preserve an unspecified tick, scaled-phase(4\,_)//:scaled-phase(4\, _), -, world, scaledPhase.pr
phase/bothUnknown, preserve both unknowns, scaled-phase(_\,_)//:scaled-phase(_\, _), -, world, scaledPhase.pr
phase/fourths, halfway on four divisions, phase-tick(4\,1/2)//:phase-tick(4\, 1/2):*4+2, -, world, scaledPhase.pr
phase/sixths, same reading on six divisions, phase-tick(6\,1/2)//:phase-tick(6\, 1/2):*6+3, -, world, scaledPhase.pr
phase/offGrid, four divisions exclude an eighth, phase-tick(4\,1/8)//:<REJECTED>, -, world, scaledPhase.pr
phase/onGrid, eight divisions admit an eighth, phase-tick(8\,1/8)//:phase-tick(8\, 1/8):*8+1, -, world, scaledPhase.pr
phase/inverseZeroCount, zero subdivisions with a supplied reading, phase-tick(0\,0)//:<REJECTED>, -, world, scaledPhase.pr
phase/inverseNegativeCount, negative subdivisions with a supplied reading, phase-tick(-4\,0)//:<REJECTED>, -, world, scaledPhase.pr
phase/inverseFractionalCount, fractional subdivisions with a supplied reading, phase-tick(3/2\,0)//:<REJECTED>, -, world, scaledPhase.pr
phase/width, exact width on a three-part grid, 1/3 = tick-width(3)//:tick-width:1/3, -, world, scaledPhase.pr
phase/upperReading, one turn reaches the excluded endpoint, phase-tick(4\,1)//:<REJECTED>, -, world, scaledPhase.pr
phase/negativeReading, reading below zero, phase-tick(4\,-1/4)//:<REJECTED>, -, world, scaledPhase.pr
phase/sharedCount, one count supplied to both uses, {#two-ticks(n)={phase-tick(%n\,1/4) phase-tick(%n\,1/2)}}\ntwo-ticks(4)//:two-ticks(4):{phase-tick:*4+1 phase-tick:*4+2}, -, world, scaledPhase.pr
phase/independent, two instances retain different subdivisions, {phase-tick(4\,1/2) phase-tick(6\,1/2)}//:{phase-tick(4\, 1/2):*4+2 phase-tick(6\, 1/2):*6+3}, -, world, scaledPhase.pr

# Reciprocal scalar sizes through the normal parser and normalization path.
radix/direct, known denominator, /4+0, *0.25+0, graph
radix/computed, computed denominator, **(*4+1 /(++(2 2))+0), *1+0.25, graph
radix/unknown, preserve unresolved reciprocal, /_+0, /_+0, graph
radix/zero, reject division by zero, /0+0, <REJECTED>, graph
radix/third, compare the exact rational radix, *1/3+0 = /3+0, *1/3+0, graph
radix/exact, thirds remain exact in arithmetic, **(*3+1 /3+0 *360+0) = *360+120, *360+120, graph
radix/rationalDivisor, reciprocal of a fractional scale factor, *2/3+0 = /3/2+0, *2/3+0, graph
radix/grouped, grouped numeric offset, *1/4+1 = /4+(1), *1/4+1, graph
