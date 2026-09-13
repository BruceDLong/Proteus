# World model load smoke tests.
# Columns: name,display,input,expected,mode,worldFile

world/load/kernel, Load protected kernel knowledge base, _, _, world, public/KernelKB.pr
world/load/publicKB, Load public knowledge base model, _, _, world, public/PublicKB.pr
world/sparse/readSecondAfterMillionYearOffset, Read second inside sparse world time, [&*15432+{million-year| ...} <second:_>] <~ %W, second:_, world, public/PublicKB.pr
world/sparse/writeSecondAfterMillionYearOffset, Write second inside sparse world time, [&*15432+{million-year| ...} <second:_>] <~ %W = 80\n[&*15432+{million-year| ...} <second:_>] <~ %W, second:80, world, public/PublicKB.pr
