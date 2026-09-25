#include "ListStructure.h"
#include <cassert>
#include <iostream>
#include <map>
using namespace proteus4;

// A tiny storage adapter: list semantics are checked independently of Proteus
// mutation and graph scheduling. Node IDs stand for POVs; storage IDs for infons.
struct Access {
    struct Node { int first = 0, next = 0, storage = 0; bool embedded = false, open = false, pending = false;
                  std::int64_t extent = -1; bool ordinary = true; };
    std::map<int, Node> nodes;
    int first(int n) { return nodes.at(n).first; }
    int next(int n) { return nodes.at(n).next; }
    bool subList(int n) { return nodes.at(n).embedded; }
    bool openTail(int n) { return nodes.at(n).open || inputPending(n); }
    bool inputPending(int n) { return nodes.at(n).pending; }
    std::int64_t listExtent(int n) { return inputPending(n) ? -1 : nodes.at(n).extent; }
    bool correspondingMembers(int n, int) { return nodes.at(n).ordinary; }
    bool sameInfon(int a, int b) { return a == b || (nodes.at(a).storage && nodes.at(a).storage == nodes.at(b).storage); }
};
using Layout = SpanLayout<int>;
bool read(Access& a, int root, Layout& layout, std::uint64_t& visits) {
    return readListLayout(a, root, layout, visits, [](const auto&) { return true; }, [](int n) { return n; });
}
bool same(const Layout& a, const Layout& b) {
    if (a.valid != b.valid || a.segments.size() != b.segments.size() || a.equations.size() != b.equations.size()) return false;
    for (std::size_t i=0; i<a.segments.size(); ++i) {
        const auto& x=a.segments[i]; const auto& y=b.segments[i];
        if (x.node!=y.node || x.gap!=y.gap || x.width!=y.width) return false;
    }
    for (std::size_t i=0; i<a.equations.size(); ++i) {
        const auto& x=a.equations[i]; const auto& y=b.equations[i];
        if (x.begin!=y.begin || x.end!=y.end || x.expected!=y.expected || x.owner!=y.owner) return false;
    }
    return true;
}
int main() {
    int cases=0;
    {
        // Merely changing embedding changes how many parent positions exist.
        Access a; a.nodes={{1,{2,0,0,false,false,false,2}}, {2,{3,5}}, {3,{0,4}}, {4,{}}, {5,{}}};
        Layout nested; std::uint64_t visits=0; assert(read(a,1,nested,visits));
        assert(nested.solve() && nested.segments.size()==2 && nested.segments[0].node==2); ++cases;
        a.nodes[2].embedded=true; a.nodes[2].extent=2; a.nodes[1].extent=3;
        Layout embedded; visits=0; assert(read(a,1,embedded,visits));
        assert(embedded.solve() && embedded.segments.size()==3 && embedded.segments[0].node==3);
        assert(embedded.ownerExtent(2)==2 && embedded.ownerExtent(1)==3); ++cases;
        // The graph can stop between every event; its accumulated evidence is
        // identical to a finite read, including nested owner equations.
        ListLayoutBuilder<int> builder;
        for(ListPosition<int> at{1}; at.phase!=ListPhase::done; at=nextListPosition(a,std::move(at)))
            builder.record(at,a,[](int n){return n;});
        assert(same(builder.layout,embedded)); ++cases;
        Layout slots; visits=0;
        assert(readListLayout(a,1,slots,visits,[](const auto&){return true;},[](int n){return n+100;}));
        assert(slots.segments[0].node==103 && slots.ownerExtent(102)==2); ++cases;
    }
    {
        // Unknown spans remain one segment, even with a billion-item extent.
        Access a; a.nodes={{1,{2,0,0,false,false,false,1000000002}}, {2,{0,3}},
                          {3,{0,4,0,true,true,false,1000000000}}, {4,{}}};
        Layout layout; std::uint64_t visits=0; assert(read(a,1,layout,visits));
        assert(layout.solve() && layout.segments.size()==3 && layout.segments[1].gap);
        assert(layout.segments[1].width==1000000000 && visits<10 && a.nodes.size()==4); ++cases;
        a.nodes[3].extent=-1; a.nodes[1].extent=-1; visits=0;
        assert(read(a,1,layout,visits) && layout.solve() && layout.segments[1].width==-1); ++cases;
    }
    {
        Access a; a.nodes={{1,{2,0,0,false,false,false,0}}, {2,{0,0,0,true,false,false,0}}};
        Layout layout; std::uint64_t visits=0;
        assert(read(a,1,layout,visits) && layout.solve() && layout.segments.empty() && layout.equations.size()==2); ++cases;
        a.nodes[2].extent=1; visits=0;
        assert(read(a,1,layout,visits) && !layout.solve()); ++cases;
        a.nodes[2].extent=-2; visits=0; assert(!read(a,1,layout,visits)); ++cases;
    }
    {
        // Cycles compare underlying infons, while sibling reuse is legal.
        Access a; a.nodes={{1,{2,0,7}}, {2,{0,0,7,true}}};
        Layout layout; std::uint64_t visits=0; assert(!read(a,1,layout,visits) && visits<10); ++cases;
        a.nodes={{1,{2,0,1}}, {2,{4,3,7,true}}, {3,{4,0,7,true}}, {4,{}}}; visits=0;
        assert(read(a,1,layout,visits) && layout.solve() && layout.segments.size()==2);
        assert(layout.equations.size()==3 && layout.segments[0].node==layout.segments[1].node); ++cases;
    }
    {
        // Only graph streaming walks hide unfinished input. A finite consumer
        // sees the stored member and can retain its unresolved obligation.
        Access a; a.nodes={{1,{2,0,0,false,false,true,9}}, {2,{0,3}}, {3,{0,0,0,false,false,true}}};
        ListLayoutBuilder<int> builder;
        for(ListPosition<int> at{1}; at.phase!=ListPhase::done; at=nextListPosition(a,std::move(at),true))
            builder.record(at,a,[](int n){return n;});
        assert(builder.layout.segments.size()==2 && builder.layout.segments[1].gap && builder.layout.extent()==-1); ++cases;
        Layout stored; std::uint64_t visits=0; assert(read(a,1,stored,visits));
        assert(stored.segments.size()==3 && stored.segments[1].node==3); ++cases;
        visits=0; assert(!readListLayout(a,1,stored,visits,[&](const auto& at){return !a.inputPending(at.node);},[](int n){return n;})); ++cases;
    }
    std::cout << "List structure: " << cases << "/" << cases << " cases passed\n";
}
