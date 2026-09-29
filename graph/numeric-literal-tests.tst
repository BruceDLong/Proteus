# Numeric normalization through the public graph API. Source rule: Proteus3 WorldManager.rangeRules.
numeric/fraction, reduce a fraction, 123/246, 1/2, graph
numeric/sizedFraction, reduce with size, *10+8/2, *10+4, graph
numeric/sizedNegative, resolve sized negative, *10-1, *10+9, graph
numeric/unknownSizeNegative, retain unsized negative, -1, -1, graph
numeric/fractionConstraint, compare equivalent rationals, 6/12 = 1/2, 1/2, graph
numeric/negativeConstraint, normalize before equality, *10-1 = *10+9, *10+9, graph
numeric/rhs, normalize numeric RHS, _ = 2/4, 1/2, graph
numeric/nested, normalize list members, {6/12 *10-1}, {1/2 *10+9}, graph
numeric/definition, read only fraction definition, {#half=6/12}\nhalf//:half:1/2, -, multi
numeric/definitionStatus, definition requirement canonical equality, {#half=6/12 half:1/2}, {#half = 6/12 half:1/2}, graph
numeric/nestedNegative, normalize negative in unknown list, {*10-1 _ = 1/2}, {*10+9 1/2}, graph

# Signed payloads remain usable before their extent is known.
numeric/negativeDefinition, signed field definition, {#pose={xRot:0 xLoc:-1.1} pose.xLoc}, {#pose = {xRot:0 xLoc:-1.1} xLoc:-1.1}, graph
numeric/sizeFromDefinition, definition supplies size, {#digit=*10+_ digit:-1}, {#digit = *10+_ digit:*10+9}, graph
numeric/sizeAndValue, RHS supplies extent and equivalent value, -1 = *10+9, *10+9, graph
numeric/negativeSlice, last two members, {1 2 3 4}@[-2:*2+{...}], {3 4}, graph
numeric/negativeIndex, last member, {1 2 3}@[-1], 3, graph
