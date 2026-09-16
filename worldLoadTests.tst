# World model load smoke tests.
# Columns: name,display,input,expected,mode,worldFile

world/load/kernel, Load protected kernel knowledge base, _, _, world, public/KernelKB.pr
world/load/publicKB, Load public knowledge base model, _, _, world, public/PublicKB.pr
world/name/milkyWay, Resolve the Milky Way occurrence and navigate back to its name, Milky-Way^name, name:{spelling:'Milky Way'\, pronunciation:$}, world, public/PublicKB.pr
world/name/andromeda, Resolve the Andromeda occurrence and navigate back to its name, Andromeda^name, name:{spelling:'Andromeda'\, pronunciation:$}, world, public/PublicKB.pr
world/name/andromedaIdentity, Use a proper name as the RHS of a name identity, name=Andromeda, name:{spelling:'Andromeda'\, pronunciation:$}, world, public/PublicKB.pr
# The `_` constrains the selector; it is not part of the selected abstract
# source value and must not leak into the returned infon.
world/sparse/readSecondAfterMillionYearOffset, Read second inside sparse world time, [&*15432+{million-year| ...} <second:_>] <~ %W, second, world, public/PublicKB.pr
world/sparse/writeSecondAfterMillionYearOffset, Write second inside sparse world time, [&*15432+{million-year| ...} <second:_>] <~ %W = 80\n[&*15432+{million-year| ...} <second:_>] <~ %W, second:80, world, public/PublicKB.pr
