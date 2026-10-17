# Optional seventh column: compressed (default), expanded, or debug.
# columns: name,display,input,expected,mode(parse|norm|multi|world|graph),world-file,print-mode
# Expanded currently preserves compressed output until cursor printing exists.
printMode/default, Unmarked fixtures keep compressed output, *2+{1 *1+{2}}, {1 {2}}, parse
printMode/compressed, Explicit compressed matches the default, *2+{1 *1+{2}}, {1 {2}}, parse,,compressed
printMode/debugNested, Debug mode reaches nested values, *2+{1 *1+{2}}, *2+{1 *1+{2}}, parse,,debug
printMode/afterDebug, Debug mode does not leak into later calls, *2+{1 *1+{2}}, {1 {2}}, parse
printMode/blank, An empty marker uses compressed output, *2+{1 2}, {1 2}, parse,,
printMode/expandedReserved, Expanded is reserved without changing storage, *2+{1 *1+{2}}, {1 {2}}, parse,,expanded
printMode/debugNorm, Debug after normalization includes counted extent, {1 2}, *2+{1 2}, norm,,debug
printMode/debugMulti, Multi-line result checks use the selected mode, {1 2}//:*2+{1 2}\n{3}//:*1+{3}, -, multi,,debug
printMode/debugWorld, World result checks use the selected mode, {1 2}, *2+{1 2}, world,,debug
printMode/debugGraph, Graph result checks use the selected mode, {1 2}, *2+{1 2}, graph,,debug
