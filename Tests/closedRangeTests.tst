closed/parse, closed interval syntax, ..(-1/3 2/3), ..(-1/3 2/3), parse
closed/unknown, interval remains ordered with unknown size, ..(0 1), ..(0 1), graph
closed/touch, two closed intervals share a point, ..(0 1) = ..(1 2), 1, graph
closed/overlap, intersection retains the upper endpoint, ..(0 2) = ..(1 3), ..(1 2), graph
closed/halfOpen, excluded endpoint in half-open intersection, ..(1 2) = **(*1+_ +0), <REJECTED>, graph
closed/shift, translate both endpoints equally, **(..(0 1) +1/3) == 4/3, 4/3, graph
closed/belowAfterShift, shift past zero, ..(0 1) == **(0 -1/4), <REJECTED>, graph
closed/backFromHalf, apply a displacement inside the range, ..(0 1) == **(1/2 -1/4), 1/4, graph
closed/reversed, reversed bounds, ..(1 0), <REJECTED>, graph
closed/nonnumeric, reject text endpoint, ..(0 'x'), <REJECTED>, graph
closed/unknownBound, retain the unknown threshold, ..(0 _), ..(0 _), graph
closed/mixedUpper, intersect closed with half-open upper, ..(0 2) = **(*2+_ +0), **(*2+_ +0), graph
closed/lastStep, fourth tick and endpoint on one scale, ..(0 1) == **(3/4 +1/4), 1, graph
closed/lastStepPrime, same step at a finer density, ..(0 1) == **(96/97 +1/97), 1, graph
closed/firstStepPrime, same grid first step, 1/97 = (..(0 1) == **(0 +1/97)), 1/97, graph
closed/onePoint, equal endpoints deduce the sole coordinate, ..(-1/2 -1/2), -1/2, graph
closed/onePointOutside, a positive singleton excludes negatives, ..(2 2) = -1, <REJECTED>, graph
closed/fromTurns, degrees preserve both endpoints, **(..(0 1) *360+0) = 360, 360, graph
closed/toTurns, reciprocal preserves the upper endpoint, **(..(0 360) /360+0) == 1, 1, graph
