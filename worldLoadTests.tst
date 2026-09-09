# World model load smoke tests.
# Columns: name,display,input,expected,mode,worldFile

world/load/foundation, Load protected bootstrap foundation model, _, _, world, public/foundation.pr
world/load/publicKB, Load public knowledge base model, _, _, world, public/PublicKB.pr
world/sparse/readSecondBeforeSolarSystemFormation, Read second inside sparse world time before our solar system, [&*15431+{million-year| ...} <second:_>] <~ %W, second:_, world, public/PublicKB.pr
world/sparse/writeSecondBeforeSolarSystemFormation, Write second inside sparse world time before our solar system, [&*15431+{million-year| ...} <second:_>] <~ %W = 80\n[&*15431+{million-year| ...} <second:_>] <~ %W, second:80, world, public/PublicKB.pr
world/astronomy/solarSystemAtFormation, Read our galaxy at the solar-system formation epoch, [&*15432+{million-year| ...} <galaxy>] <~ %W, galaxy:{T solar-system:{T star:'Sun'\, planets:{{T countries\, ... }|  ... }\, ... }}, world, public/PublicKB.pr
