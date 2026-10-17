range/directUnknown, numeric range, _ ==7, 7, graph
range/directInside, numeric range, *4+_ ==3, 3, graph
range/directLower, numeric range, *4+_ ==0, 0, graph
range/offsetInside, numeric range, (*3+_ +2) ==4, 4, graph
range/intersection, numeric range, *10+(*5+_ +2) = *10+(*4+_ +5), *10+(*2+_ +5), graph
range/reversed, numeric range, *10+(*4+_ +5) = *10+(*5+_ +2), *10+(*2+_ +5), graph
range/field, numeric range, {1 0 6 7 8 4 5 6 7}.*10+(*3+_ +2), 4, graph
range/fieldLower, numeric range, {1 2 5}.*10+(*3+_ +2), 2, graph
range/fraction, numeric range, {1/3 1/2 3/4 1}.*2+(*1/2+_ +1/2), 1/2, graph
range/filter, numeric range, {[&{!*4+_|...} *4+_]|...}<~{1 2 0 3 444 555 666 777 2}, {1 2 0 3 2}, graph
range/offsetFilter, numeric range, {[&{!(*4+_ +2)| ...} (*4+_ +2)]|...}<~{1 2 0 3 4 5}, {2 3 4 5}, graph
range/noModulo, numeric range, *4+4, *4+4, graph
range/fieldUpper, upper endpoint excluded, {5 6 4}.*10+(*3+_ +2), 4, graph
range/negatedField, complement of an offset range, {2 4 5}.!(*3+_ +2), 5, graph
range/additiveOffsets, compose additive offsets, (*3+_ +1 +1) ==3, 3, graph
range/containedIntersection, narrower interval already supplied, *10+(*2+_ +5) = *10+(*5+_ +2), *10+(*2+_ +5), graph
range/explicitRHS, explicit partially matched value, *4+_ ==:*10+3, *4+3, graph
range/unknownSizeRHS, inferred explicit size, *_+_ ==*10+3, *10+3, graph
range/unknownOverlayRHS, acquire explicit number, _ ==*10+3, *10+3, graph
range/symbolicFilter, partial numeric members retain their endpoints, {[&{!*4+_|...} *4+_]|...}<~{1 0 *4+_ 444 555 666 777 *4+(*2+_ +1) *3+_}, {1 0 *4+_ *4+(*2+_ +1) *4+_}, graph
