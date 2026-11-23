# Exact arithmetic through normal parse, normalization and equality.
lcd/shared, combine readings, (.common-denominator {1/4 1/6}), 12, graph
lcd/reduced, repeated equivalent fractions, (.common-denominator {2/8 3/12 1/4}), 4, graph
lcd/empty, no imposed denominator, (.common-denominator {}), 1, graph
lcd/whole, whole signed turns, (.common-denominator {0 -2 4}), 1, graph
lcd/signed, order and signs, (.common-denominator {-1/6 0 1/4}), 12, graph
lcd/computed, runtime values, (.common-denominator {**(1/2 /2+0) 1/6}), 12, graph
lcd/unknown, missing observation, (.common-denominator {1/4 _}), <PARTIAL>, graph
lcd/open, more observations may arrive, (.common-denominator {1/4 ...}), <PARTIAL>, graph
lcd/scalar, needs observations as list, (.common-denominator 3), <REJECTED>, graph
lcd/text, nonnumeric member, (.common-denominator {1/4 'one'}), <REJECTED>, graph
wrap/positive, multi-period, 1/4 = (.mod 13/4 1), 1/4, graph
wrap/negative, negative input, 3/4 = (.mod -9/4 1), 3/4, graph
wrap/end, period maps to origin, (.mod 360 360), 0, graph
wrap/signedExact, negative fractional period count, 1/12 = (.mod -1/6 1/4), 1/12, graph
wrap/fractional, exact rational period, 5/12 = (.mod 7/4 2/3), 5/12, graph
wrap/zeroPeriod, reject empty period, (.mod 12 0), <REJECTED>, graph
wrap/negativePeriod, period must be positive, (.mod 3 -1), <REJECTED>, graph
wrap/nonnumeric, numeric operands required, (.mod 'abc' 1), <REJECTED>, graph
wrap/different, different rational residue, (.mod 7/4 2/3) = 1/3, <REJECTED>, graph
lcd/large, arbitrary precision product, (.common-denominator {1/10000000019 1/10000000033}), 100000000520000000627, graph
lcd/largeReduced, reduction before lcm, (.common-denominator {10000000019/100000000520000000627 1/10000000033}), 10000000033, graph
lcd/groupedRange, grouped closed range keeps its syntax, (..(0 1)) == 1, 1, graph
affine/gain, signed fractional gain and offset, 1/8 = (.affine 3/4 -1/2 1/2), 1/8, graph
affine/constant, zero span, (.affine 7 0 3), 3, graph
affine/name, use a named numeric reading, (.affine degrees:90 2 1), 181, graph
affine/negative, negative origin, -1/2 = (.affine 1/2 1 -1), -1/2, graph
affine/text, numeric arguments required, (.affine 0 'gain' 0), <REJECTED>, graph
forward/field, project an argument through a second call, {#f(x)=(%x.q) #g(y)=(f(%y))}\ng({q:10})//:g({q:10}):10, -, multi
multiple/whole, select a compatible count, (.positive-multiple 24 12), 24, graph
multiple/unknown, compatible counts remain unknown, (.positive-multiple _ 12), <PARTIAL>, graph
multiple/badDivisor, invalid divisor even without a count, (.positive-multiple _ 0), <REJECTED>, graph
multiple/badCount, fractional count, (.positive-multiple 3/2 1), <REJECTED>, graph
multiple/bothUnknown, no count or divisor, (.positive-multiple _ _), <PARTIAL>, graph
multiple/notDivisible, nonmultiple, (.positive-multiple 18 12), <REJECTED>, graph
