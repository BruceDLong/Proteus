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

template<class Node> struct SpanPlan {
    struct Fill { Node owner; std::vector<Node> sources; bool close; std::int64_t extent; };
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
    return plan;
}
} // namespace proteus4
