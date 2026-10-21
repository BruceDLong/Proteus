# Logical structural neighbours share the occurrence layout used by selection.
neighbours/entryPrevious, entryPrevious, {1 &{%prev 2} 3}@[2], 1, norm
neighbours/exitPrevious, exitPrevious, {1 &{2 3} %prev}@[4], 3, norm
neighbours/exitNext, exitNext, {1 &{2 %next} 4}@[3], 4, norm
neighbours/entryNext, entryNext, {%next &{2 3} 4}@[1], 2, norm
neighbours/twoSpans, twoSpans, {1 &{2 %next} &{4 5}}@[3], 4, norm
neighbours/deepPrevious, deepPrevious, {1 &{&{&{%prev}}}}@[2], 1, norm
neighbours/deepNext, deepNext, {&{&{&{%next}}} 7}@[1], 7, norm
neighbours/emptyPrevious, emptyPrevious, {1 &{} %prev}@[2], 1, norm
neighbours/emptyNext, emptyNext, {%next &{} 3}@[1], 3, norm
neighbours/deepEmptyPrevious, deepEmptyPrevious, {1 &{&{} &{&{}}} %prev}@[2], 1, norm
neighbours/deepEmptyNext, deepEmptyNext, {%next &{&{} &{&{}}} 7}@[1], 7, norm
neighbours/zeroOpenPrevious, zeroOpenPrevious, {1 &*0+{_|...} %prev}@[2], 1, norm
neighbours/zeroOpenNext, zeroOpenNext, {%next &*0+{_|...} 7}@[1], 7, norm
neighbours/absentPrevious, absentPrevious, {&{%prev 2} 3}@[1], UNDEFINED, norm
neighbours/absentNext, absentNext, {1 &{2 %next}}@[3], UNDEFINED, norm
neighbours/nestedBoundary, nestedBoundary, {1 {%prev 2} 3}, {1 {UNDEFINED 2} 3}, norm
neighbours/ordinaryAsPrevious, ordinaryAsPrevious, {1 {2 3} %prev}@[3], {2 3}, norm
neighbours/ordinaryAsNext, ordinaryAsNext, {%next {2 3} 4}@[1], {2 3}, norm
neighbours/ordinaryInsideEmbedded, ordinaryInsideEmbedded, {1 &{{2 3} %prev}}@[3], {2 3}, norm
neighbours/nestedEmbeddedBoundary, nestedEmbeddedBoundary, {1 {&{%prev 2}}}@[2]@[1], UNDEFINED, norm
neighbours/innerScope, innerScope, {1 {9 &{%prev}}}@[2]@[2], 9, norm
neighbours/unordered, unordered, {1\, %prev}, {1\, UNDEFINED}, norm
neighbours/parentTemplate, parentTemplate, {(%prev +1)|0 &{_ _} _}@[4], 3, norm
neighbours/embeddedTemplate, embeddedTemplate, {0 &{(%prev +1)|_ _} 9}@[3], 2, norm
neighbours/templateCoverage, templateCoverage, {0 &{(%prev +1)|_ _} 9}@[4], 9, norm
neighbours/temporalControl, temporalControl, {T 1 &{T %tprev 2} 3}@[2], 1, norm
neighbours/temporalStructural, temporalStructural, {T 1 &{%prev 2} 3}@[2], 1, norm
neighbours/conflict, conflict, {1 &{%prev=2}}@[2], <REJECTED>, norm
neighbours/knownHeadOpen, knownHeadOpen, {%next &{2 ...}}@[1], 2, norm
neighbours/lazySibling, lazySibling, {9^T &{1} %prev}@[3], 1, norm
neighbours/lazySuffix, lazySuffix, {%next &{7 9^T}}@[1], 7, norm
neighbours/groupedLayout, groupedLayout, {7 &({8}) %prev}@[3], 8, norm
neighbours/learnPrevious, learnPrevious, {_ &{%prev=7}}, {7 &{7}}, norm
neighbours/namedEntry, namedEntry, {#span={%prev 2}}\n{1 &span 3}@[2]//:1, -, multi
neighbours/namedExit, namedExit, {#span={2 3}}\n{1 &span %prev}@[4]//:3, -, multi
neighbours/namedNext, namedNext, {#span={2 %next}}\n{1 &span 4}@[3]//:4, -, multi
neighbours/namedEmpty, namedEmpty, {#empty={}}\n{1 &empty %prev}@[2]//:1, -, multi
neighbours/namedRepeated, namedRepeated, {#span={%prev}}\n{1 &span 3 &span}@[4]//:3, -, multi
neighbours/namedIsolation, namedIsolation, {#span={%prev}}\n{1 span 3}@[2]@[1]//:UNDEFINED, -, multi
neighbours/namedParameter, namedParameter, {#steps(amount)={(%prev +(%amount))|_ _}}\n{0 &steps(2) 9}@[3]//:4, -, multi
neighbours/knownDeclaredEndpoint, knownDeclaredEndpoint, {&*2+{2 3} %prev}, {&{2 3} 3}, norm
