# Synthetic world-loading regressions; no external World checkout is read.
# Columns: name,display,input,expected,mode,worldFile

world/load/kernel, Load synthetic kernel with symbolic size, _, _, world, TestFiles/world-load/symbolic/KernelKB.pr
world/load/publicKB, Load synthetic public model after symbolic kernel, _, _, world, TestFiles/world-load/symbolic/PublicKB.pr

# Positive control: verify the sibling kernel and imports independently of symbolic size.
world/load/finitePublic, Load finite kernel and origin-relative public imports, %W@[2]//:2\nfixture-public//:fixture-public:7, -, world, TestFiles/world-load/finite/PublicKB.pr
