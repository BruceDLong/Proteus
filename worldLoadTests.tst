# World model load smoke tests.
# Columns: name,display,input,expected,mode,worldFile

world/load/foundation, Load protected bootstrap foundation model, _, _, world, public/foundation.pr
world/load/publicKB, Load public knowledge base model, _, _, world, public/PublicKB.pr
world/sparse/readSecondAfterMillionYearOffset, Read second inside sparse world time, [&*15432+{million-year| ...} <second:_>] <~ %W, second:_, world, public/PublicKB.pr
world/sparse/writeSecondAfterMillionYearOffset, Write second inside sparse world time, [&*15432+{million-year| ...} <second:_>] <~ %W = 80\n[&*15432+{million-year| ...} <second:_>] <~ %W, second:80, world, public/PublicKB.pr
