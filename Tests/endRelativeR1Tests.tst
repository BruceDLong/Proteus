# R1 contract tests for the end-relative intersection redesign.
#
# This file is intentionally NOT included by defaultTests.tst. It records the
# approved behavior before the redesign moves code. Some cases are expected to
# fail against the current vertical prototype; their failures are summarized
# in theory/end-relative-implementation-review.md.
#
# Run all cases from LocalBuild with:
#   ./TestProteus -T 3 -t ../endRelativeR1Tests.tst:

# Exact extent and forward selection.
r1/extent/closedLast, Closed concrete list selects its last item, *(-1)+[<_>] <~ {1 2 3 4}, 4, norm
r1/extent/exactSparseUnknownLast, Exact sparse extent locates the final position while its value is unknown, *(-1)+[<second>] <~ *4+{second| ...}, second:_, world, TestFiles/timeTestCases.pr
r1/extent/outOfRangeRejects, End-relative pattern larger than source rejects, *(-5)+[<_> _ _ _ _] <~ {1 2 3 4}, UNDEFINED, norm

# Marked correspondence. The second test has two compatible minute windows;
# the content constraint proves that the actual end-relative match was used.
r1/correspondence/markedAfterFirst, Marked term after the first pattern item selects the final item, *(-2)+[_ <_>] <~ {1 2 3 4}, 4, norm
r1/correspondence/contentConstrainedSuffix, Content constraint maps the actual final typed span, *(-1)+[<minute:{4 5 6}>] <~ hour:{second| 1 2 3 4 5 6}, minute:{4 5 6}, world, TestFiles/timeTestCases.pr

# Invalid unit requests must reject instead of manufacturing a typed result.
r1/unit/incompatibleRejects, Incompatible requested unit rejects, *(-1)+[<minute>] <~ {'a' 'b' 'c'}, UNDEFINED, world, TestFiles/timeTestCases.pr
r1/unit/nonintegralRejects, Nonintegral source-unit conversion rejects, {#second=_ #pair=*2+{second| ...} #triple=*3+{second| ...}}\n*(-1)+[<triple>] <~ *2+{pair| pair:{second| 1 2} pair:{second| 3 4}}//:UNDEFINED, -, multi

# Composite writes validate the entire operation before changing any source
# leaf. The source's inherited unfinished-tail representation is part of the
# pre-write value and is intentionally preserved on rejection.
r1/write/shortRhsIsAtomic, Short RHS leaves every mapped source value unchanged, *(-1)+[<minute>] <~ %W.day = minute:{100 110}\n%W.day, day:{second| 1 2 3 4 5 6 7 8 9 10 11 12 ... }, world, TestFiles/timeTestCases.pr
r1/write/shapeMismatchIsAtomic, Structurally incompatible RHS leaves every mapped source value unchanged, *(-1)+[<minute>] <~ %W.day = minute:{100 {110 120}}\n%W.day, day:{second| 1 2 3 4 5 6 7 8 9 10 11 12 ... }, world, TestFiles/timeTestCases.pr

# Sparse traversal must be proportional to the selected window, not the
# million-item prefix. A successful result remains a typed sparse minute.
r1/sparse/largeSuffix, Large sparse source selects its final minute without prefix expansion, *(-1)+[<minute>] <~ *1000000+{second| ...}, minute:{second| ... }, world, TestFiles/sparseTestCases.pr

# These R1 requirements need compiled/lifecycle assertions rather than a final
# normalized string. Add them to a dedicated test helper during checkpoint R2:
#
# - an open source with no exact extent remains pending and never selects its
#   current first or last item;
# - an exact extent with unavailable selected contents retains its resolved
#   coordinate while matching waits;
# - an end-relative selection on a derived view stops at that view's end, not
#   the ultimate source end;
# - a selected view remains usable after its intersection wrapper is released;
# - conflicting duplicate mapped targets reject before the first write; and
# - extending an ultimate source does not move an already resolved stable view.
