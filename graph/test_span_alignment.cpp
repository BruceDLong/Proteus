#include "SpanAlignment.h"
#include <cassert>
#include <iostream>
using namespace proteus4;
using Layout = SpanLayout<int>;
static Layout concrete(int n, int base = 100) {
    Layout layout;
    for (int i = 0; i < n; ++i) layout.segments.push_back({base + i, false, 1});
    layout.equations.push_back({0, layout.segments.size(), n, 900});
    return layout;
}
int main() {
    int cases = 0;
    for (int prefix = 0; prefix < 4; ++prefix) for (int suffix = 0; suffix < 4; ++suffix)
        for (int gap = 0; gap < 6; ++gap) {
            Layout left;
            for (int i = 0; i < prefix; ++i) left.segments.push_back({i + 1, false, 1});
            left.segments.push_back({500, true, -1});
            for (int i = 0; i < suffix; ++i) left.segments.push_back({i + 10, false, 1});
            left.equations.push_back({static_cast<std::size_t>(prefix), static_cast<std::size_t>(prefix + 1), -1, 500});
            auto right = concrete(prefix + gap + suffix);
            for (bool reverse : {false, true}) {
                auto plan = reverse ? alignSpans(right, left) : alignSpans(left, right);
                assert(plan.valid && plan.complete && plan.fills.size() == 1);
                const auto& fill = plan.fills[0];
                assert(fill.owner == 500 && fill.close && fill.extent == gap);
                assert(fill.sources.size() == static_cast<std::size_t>(gap));
                for (int i = 0; i < gap; ++i) assert(fill.sources[i] == 100 + prefix + i);
                assert(plan.identities.size() == static_cast<std::size_t>(prefix + suffix));
                ++cases;
            }
        }
    {
        Layout left; left.segments = {{1,false,1},{500,true,-1},{501,true,-1},{2,false,1}};
        auto plan = alignSpans(left, concrete(5));
        assert(plan.valid && !plan.complete && plan.fills.empty());
        assert((plan.identities == std::vector<std::pair<int,int>>{{1,100},{2,104}})); ++cases;
        plan = alignSpans(left, concrete(2));
        assert(plan.valid && plan.complete && plan.fills.size() == 2);
        assert(plan.fills[0].sources.empty() && plan.fills[1].sources.empty()); ++cases;
    }
    {
        Layout left; left.segments = {{500,true,-1}};
        left.equations.push_back({0,1,1000000000,500});
        Layout right; right.segments = {{100,false,1},{101,false,1},{600,true,-1}};
        auto plan = alignSpans(left, right);
        assert(plan.valid && !plan.complete && plan.fills.size() == 1);
        assert((plan.fills[0].sources == std::vector<int>{100,101}));
        assert(!plan.fills[0].close && plan.fills[0].extent == 1000000000); ++cases;
        right = left; right.segments[0].node = 600;
        plan = alignSpans(left, right);
        assert(plan.valid && !plan.complete && plan.fills.empty()); ++cases;
        plan = alignSpans(left, concrete(2));
        assert(!plan.valid); ++cases;
    }
    {
        Layout left; left.segments = {{1,false,1},{500,true,-1},{2,false,1}};
        Layout right; right.segments = {{101,false,1},{600,true,-1},{102,false,1}};
        auto plan = alignSpans(left, right);
        assert(plan.valid && !plan.complete && plan.fills.empty() && plan.identities.size() == 2); ++cases;
    }
    {
        Layout left; left.segments = {{1,false,1},{500,true,-1},{2,false,1}};
        left.equations.push_back({0,3,1,900});
        auto plan = alignSpans(left, concrete(3));
        assert(!plan.valid && plan.identities.empty() && plan.fills.empty()); ++cases;
    }
    {
        Layout left; left.segments = {{500,true,-1}}; left.equations.push_back({0,1,2,500});
        Layout right; right.segments = {{100,false,1},{101,false,1},{600,true,-1}};
        auto plan = alignSpans(left, right);
        assert(plan.valid && plan.complete && plan.fills.size() == 2); ++cases;
    }
    for(int prefix=0;prefix<4;++prefix) for(int suffix=0;suffix<4;++suffix) {
        if(!prefix && !suffix) continue;
        Layout a;a.segments={{500,true,-1}};a.equations={{0,1,-1,500}};
        Layout b;
        for(int i=0;i<prefix;++i) b.segments.push_back({100+i,false,1});
        b.segments.push_back({600,true,-1});b.equations.push_back({static_cast<std::size_t>(prefix),static_cast<std::size_t>(prefix+1),-1,600});
        for(int i=0;i<suffix;++i) b.segments.push_back({200+i,false,1});
        for(bool reverse:{false,true}) {
            auto plan=reverse ? alignSpans(b,a) : alignSpans(a,b);
            assert(plan.valid && !plan.complete && plan.splices.size()==1 && plan.fills.empty());
            assert(plan.splices.front().owner==500 && plan.splices.front().sources.size()==b.segments.size());++cases;
        }
        b.equations.clear();auto plan=alignSpans(a,b);
        assert(plan.splices.empty());++cases;
    }
    {
        Layout a;a.segments={{1,false,1},{500,true,-1},{2,false,1}};a.equations={{0,3,-1,900},{1,2,-1,500}};
        Layout b;b.segments={{101,false,1},{600,true,-1},{102,false,1}};b.equations={{0,3,-1,901},{1,2,-1,600}};
        assert(representsSpanShape(a,b,alignSpans(a,b)));++cases;
        b.segments.insert(b.segments.begin()+1,{103,false,1});b.equations={{0,4,-1,901},{2,3,-1,600}};
        assert(!representsSpanShape(a,b,alignSpans(a,b)));++cases;
        b.segments={{101,false,1},{600,true,-1},{102,false,1}};b.equations={{0,3,-1,901},{1,2,2,600}};
        assert(!representsSpanShape(a,b,alignSpans(a,b)));++cases;
        a=concrete(3);b.segments={{101,false,1},{600,true,-1}};b.equations={{0,2,-1,901}};
        assert(representsSpanShape(a,b,alignSpans(a,b)));++cases;
        assert(!representsSpanShape(b,a,alignSpans(b,a)));++cases;
    }
    {
        Layout a;a.segments={{1,false,1},{500,true,-1},{2,false,1}};a.equations={{0,3,-1,900},{1,2,-1,500}};
        Layout b;b.segments={{101,false,1},{102,false,1},{600,true,1000000000},{103,false,1},{104,false,1}};
        b.equations={{0,5,1000000004,901},{2,3,1000000000,600}};
        auto plan=alignSpans(a,b);
        assert(plan.valid && plan.splices.size()==1 && plan.splices.front().sources.size()==3);++cases;
        assert(plan.splices.front().sources[1].width==1000000000);++cases;
    }
    // Crossing ends share a middle only when finite overlaps are excluded.
    for(int prefix=1;prefix<=3;++prefix) for(int suffix=1;suffix<=3;++suffix) {
        Layout a,b;
        for(int i=0;i<prefix;++i) a.segments.push_back({10+i,false,1});
        a.segments.push_back({500,true,-1});b.segments.push_back({600,true,-1});
        for(int i=0;i<suffix;++i) b.segments.push_back({20+i,false,1});
        for(bool reverse:{false,true}) {
            auto plan=reverse ? alignSpans(b,a) : alignSpans(a,b);
            assert(plan.valid && !plan.complete && plan.crossings.size()==1);
            const auto& part=plan.crossings.front();
            assert(part.left==500 && part.right==600 && part.prefix.size()==prefix && part.suffix.size()==suffix);
            assert(crossingHasMiddle(part,[](int a,int b){return a!=b;}));++cases;
            assert(!crossingHasMiddle(part,[](int,int){return false;}));++cases;
            auto ambiguous=part;ambiguous.prefix.back()=ambiguous.suffix.front();
            assert(!crossingHasMiddle(ambiguous,[](int a,int b){return a!=b;}));++cases;
            assert(!representsSpanShape(a,b,plan));++cases;
        }
        for(int width:{0,7,1000000000}) {
            a.segments.back().width=width+suffix;
            auto plan=alignSpans(a,b);
            assert(plan.valid && plan.crossings.size()==1 && plan.crossings.front().middle==width);
            assert(crossingHasMiddle(plan.crossings.front(),[](int,int){return false;}));++cases;
        }
    }
    {
        Layout a,b;a.segments={{1,false,1},{500,true,1}};b.segments={{600,true,1},{2,false,1},{3,false,1}};
        assert(!alignSpans(a,b).valid);++cases;
        a.segments.back().width=-1;b.segments.front().node=500;
        assert(alignSpans(a,b).crossings.empty());++cases;
        b.segments.front().node=600;a.segments.insert(a.segments.begin()+1,{501,true,-1});
        assert(alignSpans(a,b).crossings.empty());++cases;
    }
    // Window boundaries are independent of the compressed representation.
    for (int width : {0,1,7,1000000000}) {
        Layout layout;layout.segments={{1,false,1},{2,true,width},{3,false,1}};
        layout.equations.push_back({0,3,width+2,900});
        auto run=selectSpanWindow(layout,1,width);
        assert(run.complete && !run.outside && run.pieces.size()==(width ? 1u : 0u));++cases;
        if(width) assert(run.pieces[0].node==2 && run.pieces[0].offset==0 && run.pieces[0].count==width);
        auto boundary=selectSpanWindow(layout,width,2);
        assert(boundary.complete && boundary.pieces.size()==2 && boundary.pieces.back().node==3);++cases;
        assert(selectSpanWindow(layout,width+2,0).complete);++cases;
        assert(selectSpanWindow(layout,width+1,2).outside);++cases;
    }
    {
        Layout layout;layout.segments={{1,false,1},{2,true,-1},{3,false,1}};
        assert(selectSpanWindow(layout,0,1).complete);++cases;
        auto unknown=selectSpanWindow(layout,1,1);
        assert(!unknown.complete && !unknown.outside);++cases;
        layout.equations.push_back({0,3,5,900});
        auto solved=selectSpanWindow(layout,2,3);
        assert(solved.complete && solved.pieces.size()==2 && solved.pieces[0].offset==1);++cases;
        auto huge=selectSpanWindow(layout,INT64_MAX,INT64_MAX);
        assert(huge.outside);++cases;
    }
    std::cout << "Span alignment: " << cases << "/" << cases << " cases passed\n";
}
