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
    std::cout << "Span alignment: " << cases << "/" << cases << " cases passed\n";
}
