# Optional seventh column: compressed (default), expanded, or debug.
# columns: name,display,input,expected,mode(parse|norm|multi|world|graph),world-file,print-mode
# Expanded uses bounded occurrence demands with an Agent context.
printMode/default, Unmarked fixtures keep compressed output, *2+{1 *1+{2}}, {1 {2}}, parse
printMode/compressed, Explicit compressed matches the default, *2+{1 *1+{2}}, {1 {2}}, parse,,compressed
printMode/debugNested, Debug mode reaches nested values, *2+{1 *1+{2}}, *2+{1 *1+{2}}, parse,,debug
printMode/afterDebug, Debug mode does not leak into later calls, *2+{1 *1+{2}}, {1 {2}}, parse
printMode/blank, An empty marker uses compressed output, *2+{1 2}, {1 2}, parse,,
printMode/expandedReserved, Expanded preserves nested compressed formatting, *2+{1 *1+{2}}, {1 {2}}, parse,,expanded
printMode/debugNorm, Debug after normalization includes counted extent, {1 2}, *2+{1 2}, norm,,debug
printMode/debugMulti, Multi-line result checks use the selected mode, {1 2}//:*2+{1 2}\n{3}//:*1+{3}, -, multi,,debug
printMode/debugWorld, World result checks use the selected mode, {1 2}, *2+{1 2}, world,,debug
printMode/debugGraph, Graph result checks use the selected mode, {1 2}, *2+{1 2}, graph,,debug
printMode/expandedParentSpec, Expanded applies parent listSpec, {T (%tprev +1)| 0 _ _}, {T 0 1 2}, parse,,expanded
printMode/expandedOverlay, Expanded applies parent overlay, {T _ _ _} == {T 1 2 3}, {T 1 2 3}, parse,,expanded
printMode/expandedEmbedded, Expanded traverses logical embedded members, {1 &{2 3} 4}, {1 2 3 4}, parse,,expanded
printMode/afterExpanded, Expanded mode does not leak into later calls, {1 &{2 3} 4}, {1 &{2 3} 4}, parse
# The same temporal and embedded inputs retain their recipes in the other modes.
printMode/parentSpecCompressed, Compressed retains the temporal recipe, {T (%tprev +1)| 0 _ _}, {T (%tprev +1)| 0 _ _}, parse,,compressed
printMode/parentSpecDebug, Debug retains the temporal recipe and extent, {T (%tprev +1)| 0 _ _}, *3+{T (%tprev +1)| 0 _ _}, parse,,debug
printMode/overlayCompressed, Compressed retains the overlay, {T _ _ _} == {T 1 2 3}, {T _ _ _} == {T 1 2 3}, parse,,compressed
printMode/overlayDebug, Debug retains the overlay and extents, {T _ _ _} == {T 1 2 3}, *3+{T _ _ _} == *3+{T 1 2 3}, parse,,debug
printMode/embeddedCompressed, Compressed retains embedded members, {1 &{2 3} 4}, {1 &{2 3} 4}, parse,,compressed
printMode/embeddedDebug, Debug retains embedded members and extents, {1 &{2 3} 4}, *4+{1 &*2+{2 3} 4}, parse,,debug
# Result formatting can also be selected after whole-history normalization.
printMode/temporalNormCompressed, Compressed normalized result, {T 1 _ 2}, {T 1 1 2}, norm,,compressed
printMode/temporalNormExpanded, Expanded normalized result, {T 1 _ 2}, {T 1 1 2}, norm,,expanded
printMode/temporalNormDebug, Debug normalized result, {T 1 _ 2}, *3+{T 1 1 2}, norm,,debug
