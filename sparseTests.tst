# Sparse ordered traversal tests.
# Columns: name,display,input,expected,mode,worldFile

# The selector consumes the first 60 seconds as a compressed span, exposes the
# following single second for writing, then reads the same sparse location back.
sparse/split/writeInsideSparseSecondRun, Sparse split write inside symbolic seconds, [&*60+{second| ...} <_> &*3539+{second| ...}] <~ %W.secondRun = 80\n[&*60+{second| ...} <_> &*3539+{second| ...}] <~ %W.secondRun, second:80, world, TestFiles/sparseTestCases.pr

# Boundary writes should expose only the touched item and leave the remaining
# symbolic span compressed.
sparse/split/writeFirstSparseSecond, Sparse write first symbolic second, [<_> &*3599+{second| ...}] <~ %W.secondRun = 80\n%W.secondRun, secondRun:{second:80 &seconds:{second|  ... }}, world, TestFiles/sparseTestCases.pr
sparse/split/writeLastSparseSecond, Sparse write last symbolic second, [&*3599+{second| ...} <_>] <~ %W.secondRun = 80\n%W.secondRun, secondRun:{&seconds:{second|  ... } second:80}, world, TestFiles/sparseTestCases.pr

# Huge sparse counts should be skipped symbolically, not item-by-item.
sparse/scarcity/writeLastBillionSparseSeconds, Sparse write last in billion symbolic seconds, [&*999999999+{second| ...} <_>] <~ %W.billionSecondRun = 80\n%W.billionSecondRun, billionSecondRun:{&seconds:{second|  ... } second:80}, world, TestFiles/sparseTestCases.pr
sparse/scarcity/readAfterBillionSparsePrefix, Sparse read after billion symbolic seconds, [&*1000000000+{second| ...} <_>] <~ %W.hugeSparseWithTail, 808, world, TestFiles/sparseTestCases.pr

# Traversal should continue correctly after sparse spans are split or after
# adjacent sparse spans cover a single selector span.
sparse/traverse/readAfterAdjacentSparseSpans, Sparse read after adjacent symbolic spans, [&*120+{second| ...} <_>] <~ %W.twoSparseThenMarker, 123, world, TestFiles/sparseTestCases.pr
sparse/traverse/readFirstConcreteBetweenSparseSpans, Sparse read first concrete marker between symbolic spans, [&*120+{second| ...} <_> &*240+{second| ...} _ &*360+{second| ...}] <~ %W.sparseMarkers, 777, world, TestFiles/sparseTestCases.pr
sparse/traverse/readSecondConcreteBetweenSparseSpans, Sparse read second concrete marker between symbolic spans, [&*120+{second| ...} _ &*240+{second| ...} <_> &*360+{second| ...}] <~ %W.sparseMarkers, 888, world, TestFiles/sparseTestCases.pr

# Sparse-to-sparse alignment should handle exact-span matches, splitting the
# RHS sparse span, and splitting the LHS selector span as traversal advances.
sparse/split/writeAfterExactSparseSpan, Sparse write after exact symbolic span, [&*60+{second| ...} <_> &*59+{second| ...} _] <~ %W.twoSparseThenMarker = 80\n[&*60+{second| ...} <_> &*59+{second| ...} _] <~ %W.twoSparseThenMarker, second:80, world, TestFiles/sparseTestCases.pr
sparse/split/writeInsideFirstSparseRemainder, Sparse write after RHS symbolic span split, [&*59+{second| ...} <_> &*60+{second| ...} _] <~ %W.twoSparseThenMarker = 81\n[&*59+{second| ...} <_> &*60+{second| ...} _] <~ %W.twoSparseThenMarker, second:81, world, TestFiles/sparseTestCases.pr
sparse/split/writeAfterCrossingSparseBoundary, Sparse write after selector crosses symbolic boundary, [&*61+{second| ...} <_> &*58+{second| ...} _] <~ %W.twoSparseThenMarker = 82\n[&*61+{second| ...} <_> &*58+{second| ...} _] <~ %W.twoSparseThenMarker, second:82, world, TestFiles/sparseTestCases.pr
sparse/split/writeAfterSparseMinuteSkip, Sparse minute selector skips symbolic seconds, [&*1+{minute| ...} <_> &*3539+{second| ...}] <~ %W.secondRun = 83\n[&*60+{second| ...} <_> &*3539+{second| ...}] <~ %W.secondRun, second:83, world, TestFiles/sparseTestCases.pr

# Repeated edits in the same sparse parent should not make later positions or
# concrete tail markers unreachable.
sparse/traverse/writeBothSparseBoundaries, Sparse write first and last symbolic seconds, [<_> &*3599+{second| ...}] <~ %W.secondRun = 11\n[&*3599+{second| ...} <_>] <~ %W.secondRun = 22\n[<_> &*3599+{second| ...}] <~ %W.secondRun//:second:11\n[&*3599+{second| ...} <_>] <~ %W.secondRun//:second:22, -, world, TestFiles/sparseTestCases.pr
sparse/traverse/splitPreservesTailMarker, Sparse split preserves following concrete marker, [&*59+{second| ...} <_> &*60+{second| ...} _] <~ %W.twoSparseThenMarker = 81\n[&*120+{second| ...} <_>] <~ %W.twoSparseThenMarker//:123, -, world, TestFiles/sparseTestCases.pr

# Pending: a writable marked span should be able to cross sparse/concrete/sparse
# boundaries and merge the write back into the parent during re-normalization.
sparse/pending/writeSpanAcrossSparseConcreteSparse, Pending sparse write across sparse concrete sparse, [&*119+{second| ...} <&*3+{second| ...}> &*239+{second| ...} _ &*360+{second| ...}] <~ %W.sparseMarkers = {10 20 30}\n[&*119+{second| ...} <&*3+{second| ...}> &*239+{second| ...} _ &*360+{second| ...}] <~ %W.sparseMarkers, seconds:{10 20 30}, world, TestFiles/sparseTestCases.pr
