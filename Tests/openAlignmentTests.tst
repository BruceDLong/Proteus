# D122: a checked bare open source has no positional constraints. These
# reductions stay partial and retain their nested requirements. Compiled
# openAlignment tests also check status, identity, cancellation and work.
alignment/header/nested, Preserve a pending nested requirement, {{_| _ ...}| {} ...}={...}, {{_| _ ... }| {} ... }, norm
alignment/header/prefix, Preserve known members beside pending requirements, {{_| _ ...}| {1} {} ...}={...}, {{_| _ ... }| {1} {} ... }, norm

# Heterogeneous members share an anonymous structural schema. Their individual
# nominal names and open data descriptions remain intact.
alignment/header/mixedNames, Apply shared fields without equating member types, {{properties:{title:$} data:{...}}|kanban:{properties:{title:'Board'} data:{1 ...}} terminal:{properties:{title:'Term'} data:{2 ...}} ...}, {{properties:{title:$} data:{ ... }}| kanban:{properties:{title:'Board'} data:{1 ... }} terminal:{properties:{title:'Term'} data:{2 ... }} ... }, norm
alignment/header/nominalGuard, Keep incompatible nominal requirements active, {terminal:{data:{...}}|kanban:{data:{1 ...}}}//:<REJECTED>, -, multi
