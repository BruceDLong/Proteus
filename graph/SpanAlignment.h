#pragma once

// Finite ordered-layout planning. Nodes are opaque handles supplied by the
// domain walk; a gap represents a region, never one node per unknown member.
#include <algorithm>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace proteus4 {
template<class Node> struct SpanLayout {
    struct Segment { Node node; bool gap = false; std::int64_t width = 1; };
    struct Equation { std::size_t begin, end; std::int64_t expected; Node owner; };
    std::vector<Segment> segments;
    std::vector<Equation> equations;
    bool valid = true;

    std::int64_t total(std::size_t begin, std::size_t end) const {
        std::int64_t sum = 0;
        for (auto i = begin; i < end; ++i) {
            auto width = segments[i].width;
            if (width < 0 || width > std::numeric_limits<std::int64_t>::max() - sum) return -1;
            sum += width;
        }
        return sum;
    }
    bool solve() {
        bool changed = true;
        while (changed && valid) {
            changed = false;
            for (const auto& equation : equations) {
                if (equation.expected < 0) continue;
                std::int64_t known = 0;
                std::vector<std::size_t> unknown;
                for (auto i = equation.begin; i < equation.end; ++i) {
                    auto width = segments[i].width;
                    if (width < 0) unknown.push_back(i);
                    else if (width > equation.expected - known) { valid = false; break; }
                    else known += width;
                }
                if (!valid) break;
                auto remainder = equation.expected - known;
                if (unknown.empty()) { if (remainder) valid = false; }
                else if (unknown.size() == 1 || remainder == 0) {
                    for (auto i : unknown) segments[i].width = remainder;
                    changed = true;
                }
            }
        }
        return valid;
    }
    bool concrete() const {
        for (const auto& segment : segments) if (segment.gap && segment.width != 0) return false;
        return true;
    }
    std::int64_t ownerExtent(const Node& owner) const {
        for (const auto& equation : equations)
            if (equation.owner == owner) return total(equation.begin, equation.end);
        return -1;
    }
    std::int64_t extent() const {
        auto result = total(0, segments.size());
        if (result >= 0) return result;
        for (const auto& equation : equations)
            if (equation.begin == 0 && equation.end == segments.size() && equation.expected >= 0)
                return equation.expected;
        return -1;
    }
};

// Select a fixed window from a finite structural description. Unknown gaps
// remain obligations; arithmetic skips even very large known runs.
template<class Node> struct SpanWindow {
    struct Piece { Node node; bool gap; std::int64_t offset, count; };
    std::vector<Piece> pieces;
    bool complete = false, outside = false;
};
template<class Node>
SpanWindow<Node> selectSpanWindow(SpanLayout<Node> layout, std::int64_t start, std::int64_t count) {
    SpanWindow<Node> result;
    if (start < 0 || count < 0 || !layout.solve()) return result;
    auto extent = layout.extent();
    if (extent >= 0 && (start > extent || count > extent - start)) {
        result.outside = true; return result;
    }
    for (const auto& segment : layout.segments) {
        if (start == 0 && count == 0) break;
        if (segment.width < 0) return result;
        if (start >= segment.width) { start -= segment.width; continue; }
        auto take = std::min(count, segment.width - start);
        if (take) result.pieces.push_back({segment.node, segment.gap, start, take});
        count -= take; start = 0;
    }
    result.complete = start == 0 && count == 0;
    return result;
}

template<class Node> struct SpanPlan {
    struct Fill { Node owner; std::vector<Node> sources; bool close; std::int64_t extent; };
    struct Splice { Node owner; std::vector<typename SpanLayout<Node>::Segment> sources; };
    // P + leftGap = rightGap + S. A common middle is valid only when
    // shorter overlaps of the written P/S regions have been excluded.
    struct Crossing { Node left, right; std::vector<Node> prefix, suffix; std::int64_t middle = -1; };
    std::vector<Crossing> crossings;
    std::vector<Splice> splices;
    std::vector<std::pair<Node, Node>> identities;
    std::vector<Fill> fills;
    bool valid = true, complete = false;

    void fill(const SpanLayout<Node>& layout, const Node& owner,
              std::vector<Node> sources, bool close) {
        if (sources.empty() && !close) return;
        for (const auto& previous : fills) if (previous.owner == owner) {
            if (previous.sources != sources || previous.close != close) valid = false;
            return;
        }
        fills.push_back({owner, std::move(sources), close, layout.ownerExtent(owner)});
    }
};

template<class Node> SpanPlan<Node> alignSpans(SpanLayout<Node> left, SpanLayout<Node> right) {
    SpanPlan<Node> plan;
    if (!left.solve() || !right.solve()) { plan.valid = false; return plan; }
    auto leftSize = left.extent(), rightSize = right.extent();
    if (leftSize >= 0 && rightSize >= 0 && leftSize != rightSize) { plan.valid = false; return plan; }
    if (leftSize >= 0) right.equations.push_back({0, right.segments.size(), leftSize, {}});
    if (rightSize >= 0) left.equations.push_back({0, left.segments.size(), rightSize, {}});
    if (!left.solve() || !right.solve()) { plan.valid = false; return plan; }

    auto closeZeros = [&](const SpanLayout<Node>& layout) {
        for (const auto& segment : layout.segments)
            if (segment.gap && segment.width == 0) plan.fill(layout, segment.node, {}, true);
    };
    closeZeros(left); closeZeros(right);
    if (left.concrete() || right.concrete()) {
        const bool reverse = !right.concrete();
        const auto& described = reverse ? right : left;
        const auto& concrete = reverse ? left : right;
        std::vector<Node> members;
        for (const auto& segment : concrete.segments) if (!segment.gap) members.push_back(segment.node);
        // The concrete side determines a total even if its explicit size slot
        // is unknown. Infer only unique gap widths, never enumerate partitions.
        auto resolved = described;
        resolved.equations.push_back({0, resolved.segments.size(), static_cast<std::int64_t>(members.size()), {}});
        if (!resolved.solve()) { plan.valid = false; return plan; }
        auto apply = [&](std::size_t i, std::size_t position) {
            const auto& segment = resolved.segments[i];
            if (segment.gap) {
                auto length = static_cast<std::size_t>(segment.width);
                plan.fill(resolved, segment.node,
                          std::vector<Node>(members.begin() + position, members.begin() + position + length), true);
            } else {
                plan.identities.push_back(reverse ? std::make_pair(members[position], segment.node)
                                                  : std::make_pair(segment.node, members[position]));
            }
        };
        std::size_t i = 0, position = 0;
        for (; i < resolved.segments.size() && resolved.segments[i].width >= 0; ++i) {
            auto width = static_cast<std::uint64_t>(resolved.segments[i].width);
            if (width > members.size() - position) { plan.valid = false; return plan; }
            apply(i, position); position += width;
        }
        if (i == resolved.segments.size()) {
            plan.complete = position == members.size();
            if (!plan.complete) plan.valid = false;
        } else {
            auto end = members.size();
            for (auto j = resolved.segments.size(); j > i && resolved.segments[j - 1].width >= 0; --j) {
                auto width = static_cast<std::uint64_t>(resolved.segments[j - 1].width);
                if (width > end - position) { plan.valid = false; return plan; }
                end -= width; apply(j - 1, end);
            }
        }
        return plan;
    }

    // Both descriptions retain gaps. Establish only anchored regions. A final
    // tail may acquire a known finite prefix without claiming its end is known.
    auto significant = [](const SpanLayout<Node>& layout) {
        std::vector<std::size_t> result;
        for (std::size_t i = 0; i < layout.segments.size(); ++i)
            if (!layout.segments[i].gap || layout.segments[i].width != 0) result.push_back(i);
        return result;
    };
    auto l = significant(left), r = significant(right);
    std::size_t li = 0, ri = 0;
    while (li < l.size() && ri < r.size()) {
        const auto& a = left.segments[l[li]];
        const auto& b = right.segments[r[ri]];
        if (a.gap || b.gap) break;
        plan.identities.emplace_back(a.node, b.node); ++li; ++ri;
    }
    auto extend = [&](const SpanLayout<Node>& target, const std::vector<std::size_t>& targetSteps,
                      std::size_t at, const SpanLayout<Node>& source,
                      const std::vector<std::size_t>& sourceSteps, std::size_t from) {
        if (at >= targetSteps.size() || at + 1 != targetSteps.size()) return;
        const auto& gap = target.segments[targetSteps[at]];
        if (!gap.gap) return;
        std::vector<Node> sources;
        while (from < sourceSteps.size()) {
            const auto& segment = source.segments[sourceSteps[from++]];
            if (segment.gap || (gap.width >= 0 && sources.size() >= static_cast<std::uint64_t>(gap.width))) break;
            sources.push_back(segment.node);
        }
        bool close = gap.width >= 0 && sources.size() == static_cast<std::uint64_t>(gap.width);
        plan.fill(target, gap.node, std::move(sources), close);
    };
    extend(left, l, li, right, r, ri);
    extend(right, r, ri, left, l, li);
    auto le = l.size(), re = r.size();
    while (le > li && re > ri) {
        const auto& a = left.segments[l[le - 1]];
        const auto& b = right.segments[r[re - 1]];
        if (a.gap || b.gap) break;
        plan.identities.emplace_back(a.node, b.node); --le; --re;
    }
    // A single remaining hole can contain the other description's entire
    // middle, including holes of its own. Each referenced hole must denote
    // its complete owner; a tail within a loaded owner needs an offset view.
    auto splice = [&](const SpanLayout<Node>& target,const std::vector<std::size_t>& ts,
                      std::size_t begin,std::size_t end,const SpanLayout<Node>& source,
                      const std::vector<std::size_t>& ss,std::size_t from,std::size_t to) {
        if(end!=begin+1 || to<=from+1) return;
        const auto& gap=target.segments[ts[begin]];
        if(!gap.gap) return;
        typename SpanPlan<Node>::Splice part{gap.node,{}};
        for(auto i=from;i<to;++i) {
            auto index=ss[i];const auto& segment=source.segments[index];
            if(segment.node==gap.node) return;
            if(segment.gap && std::none_of(source.equations.begin(),source.equations.end(),[&](const auto& equation) {
                return equation.owner==segment.node && equation.begin==index && equation.end==index+1;
            })) return;
            part.sources.push_back(segment);
        }
        // A splice subsumes any prefix extension of this same hole.
        plan.fills.erase(std::remove_if(plan.fills.begin(),plan.fills.end(),[&](const auto& fill) {return fill.owner==gap.node;}),plan.fills.end());
        plan.splices.push_back(std::move(part));
    };
    splice(left,l,li,le,right,r,ri,re);
    splice(right,r,ri,re,left,l,li,le);
    auto crossing=[&](const SpanLayout<Node>& a,const std::vector<std::size_t>& ai,std::size_t ab,std::size_t ae,
                      const SpanLayout<Node>& b,const std::vector<std::size_t>& bi,std::size_t bb,std::size_t be) {
        if(ae<=ab+1 || be<=bb+1 || !plan.splices.empty()) return;
        const auto& leftGap=a.segments[ai[ae-1]];const auto& rightGap=b.segments[bi[bb]];
        if(!leftGap.gap || !rightGap.gap || leftGap.node==rightGap.node) return;
        typename SpanPlan<Node>::Crossing part{leftGap.node,rightGap.node,{},{},-1};
        for(auto i=ab;i<ae-1;++i) {const auto& at=a.segments[ai[i]];if(at.gap) return;part.prefix.push_back(at.node);}
        for(auto i=bb+1;i<be;++i) {const auto& at=b.segments[bi[i]];if(at.gap) return;part.suffix.push_back(at.node);}
        if(leftGap.width>=0) {
            if(static_cast<std::uint64_t>(leftGap.width)<part.suffix.size()) return;
            part.middle=leftGap.width-static_cast<std::int64_t>(part.suffix.size());
        }
        if(rightGap.width>=0) {
            if(static_cast<std::uint64_t>(rightGap.width)<part.prefix.size()) return;
            auto width=rightGap.width-static_cast<std::int64_t>(part.prefix.size());
            if(part.middle>=0 && part.middle!=width) return;
            part.middle=width;
        }
        plan.crossings.push_back(std::move(part));
    };
    crossing(left,l,li,le,right,r,ri,re);
    crossing(right,r,ri,re,left,l,li,le);
    return plan;
}
// There are only finitely many shorter overlaps of the written ends. A
// definite mismatch excludes one; unknown compatibility cannot exclude it.
// A known nonnegative middle width already fixes their relative positions.
template<class Crossing,class Incompatible>
bool crossingHasMiddle(const Crossing& part,Incompatible incompatible) {
    if(part.middle>=0) return true;
    for(std::size_t width=1;width<=std::min(part.prefix.size(),part.suffix.size());++width) {
        bool excluded=false;
        for(std::size_t i=0;i<width;++i)
            if(incompatible(part.prefix[part.prefix.size()-width+i],part.suffix[i])) {excluded=true;break;}
        if(!excluded) return false;
    }
    return true;
}
// Shape coverage is weaker than a concrete correspondence. Unconstrained
// source gaps add no facts, but every written source position and every bounded
// subspan must already be represented. No fill into the actual layout is proof.
template<class Node>
bool representsSpanShape(const SpanLayout<Node>& actual,const SpanLayout<Node>& required,const SpanPlan<Node>& plan) {
    if(!plan.valid || !plan.crossings.empty()) return false;
    auto actualGap=[&](const Node& owner) {
        return std::any_of(actual.segments.begin(),actual.segments.end(),[&](const auto& segment) {return segment.gap && segment.node==owner;});
    };
    for(const auto& fill:plan.fills) if(actualGap(fill.owner)) return false;
    for(const auto& splice:plan.splices) if(actualGap(splice.owner)) return false;
    for(const auto& segment:required.segments) if(!segment.gap &&
        std::none_of(plan.identities.begin(),plan.identities.end(),[&](const auto& pair) {return pair.second==segment.node;})) return false;
    for(const auto& equation:required.equations) if(equation.expected>=0) {
        if(equation.begin==0 && equation.end==required.segments.size() && actual.extent()==equation.expected) continue;
        for(auto i=equation.begin;i<equation.end;++i)
            if(required.segments[i].gap && required.segments[i].width!=0) return false;
    }
    return true;
}
} // namespace proteus4
