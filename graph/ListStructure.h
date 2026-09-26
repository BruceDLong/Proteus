#pragma once
#include "SpanAlignment.h"
#include <limits>
#include <set>

namespace proteus4 {
// Structural events over existing nodes. Ordinary nested lists occupy one
// position; only embedded lists are entered. Owners preserve each boundary.
enum class ListPhase { enter, member, leave, done, cycle };
template<class Node> struct ListPosition {
    Node node;
    ListPhase phase = ListPhase::enter;
    std::vector<Node> owners{};
};

// An occurrence can have usable siblings even when its containing POV has
// not been supplied. Container walks decide separately where to stop.
template<class Node, class Access>
ListPosition<Node> adjacentListPosition(Access& access, ListPosition<Node> at,
                                        bool publishedOnly = false, bool reverse = false) {
    auto neighbor = access.next(at.node, reverse);
    if (neighbor && (!publishedOnly || !access.inputPending(neighbor))) {
        at.node = neighbor; at.phase = ListPhase::member;
    } else if (!at.owners.empty()) {
        at.node = at.owners.back(); at.owners.pop_back(); at.phase = ListPhase::leave;
    } else at.phase = ListPhase::done;
    return at;
}

// One step, usable either as a graph edge or by a finite read-only planner.
// Graph walks can request only published neighbours. Finite inspection uses
// stored neighbours so unfinished data remains an obligation for its consumer.
// No implicit occurrence is materialized and no semantic rule runs.
template<class Node, class Access>
ListPosition<Node> nextListPosition(Access& access, ListPosition<Node> at, bool publishedOnly = false, bool reverse = false) {
    auto visible = [&](Node node) { return node && publishedOnly && access.inputPending(node) ? Node{} : node; };
    if (at.phase == ListPhase::enter) {
        for (const auto& owner : at.owners) if (access.sameInfon(owner, at.node)) {
            at.phase = ListPhase::cycle; return at;
        }
        if (auto first = visible(access.first(at.node, reverse))) {
            at.owners.push_back(at.node); at.node = first; at.phase = ListPhase::member;
        } else at.phase = ListPhase::leave;
    } else if (at.phase == ListPhase::member && access.subList(at.node)) {
        at.phase = ListPhase::enter;
    } else if (at.phase == ListPhase::member || at.phase == ListPhase::leave) {
        if (at.owners.empty()) at.phase = ListPhase::done;
        else return adjacentListPosition(access, std::move(at), publishedOnly, reverse);
    }
    return at;
}

// Boundary evidence counts only stored siblings. Embedded sizes count their
// logical positions; an open tail remains a gap. No virtual member is created.
// Navigation charges these finite inspection visits to the resulting route.
template<class Node> struct ListBoundary {
    std::int64_t extent = -1, prefix = 0, tail = -1;
    std::uint64_t visits = 1;
    bool valid = true;
    std::vector<Node> missingExtents;
};
template<class Node, class Access>
ListBoundary<Node> listBoundary(Access& access, const Node& owner) {
    ListBoundary<Node> result;
    result.extent = access.listExtent(owner);
    if (result.extent == -2) { result.valid = false; return result; }
    std::set<Node> seen;
    for (auto member = access.first(owner, false); member; member = access.next(member, false)) {
        ++result.visits;
        if (!seen.insert(member).second || access.inputPending(member)) {
            result.valid = false; return result;
        }
        auto width = access.subList(member) ? access.listExtent(member) : 1;
        if (width == -2) { result.valid = false; return result; }
        if (width < 0) result.missingExtents.push_back(member);
        else if (width > std::numeric_limits<std::int64_t>::max() - result.prefix) {
            result.valid = false; return result;
        } else result.prefix += width;
    }
    if (!result.missingExtents.empty()) { result.prefix = -1; return result; }
    if ((result.extent) < 0 && !access.openTail(owner)) result.extent = result.prefix;
    if (result.extent >= 0) {
        result.tail = result.extent - result.prefix;
        if (result.tail < 0 || (!access.openTail(owner) && result.tail != 0)) result.valid = false;
    }
    return result;
}

template<class Node, class Access>
bool listPositionSupported(Access& access, const ListPosition<Node>& at) {
    return at.phase != ListPhase::cycle &&
        (at.phase != ListPhase::member || access.subList(at.node) ||
         access.correspondingMembers(at.node, at.node));
}

// The common interpretation of member positions, open spans, and size facts.
// A projection lets definition plans retain instance slots rather than POVs.
// Equations retain owner identity even when embedded positions are flattened.
template<class Position> struct ListLayoutBuilder {
    SpanLayout<Position> layout;
    std::vector<std::size_t> starts;
    template<class Node, class Access, class Project>
    void record(const ListPosition<Node>& at, Access& access, Project project) {
        if (at.phase == ListPhase::enter) starts.push_back(layout.segments.size());
        if (at.phase == ListPhase::member && !access.subList(at.node))
            layout.segments.push_back({project(at.node), false, 1});
        if (at.phase == ListPhase::leave) {
            if (access.openTail(at.node)) layout.segments.push_back({project(at.node), true, -1});
            auto extent = access.listExtent(at.node);
            if (extent == -2) layout.valid = false;
            layout.equations.push_back({starts.back(), layout.segments.size(), extent, project(at.node)});
            starts.pop_back();
        }
    }
};

// Finite consumers reuse exactly the incremental walk. Eligibility remains a
// caller obligation: recognizing structure cannot prove a type or relationship.
template<class Node, class Position, class Access, class Accept, class Project>
bool readListLayout(Access& access, const Node& root, SpanLayout<Position>& result,
                    std::uint64_t& visits, Accept accept, Project project) {
    ListLayoutBuilder<Position> builder;
    for (ListPosition<Node> at{root}; at.phase != ListPhase::done; at = nextListPosition(access, std::move(at))) {
        if (at.phase == ListPhase::enter || at.phase == ListPhase::member) ++visits;
        if (!listPositionSupported(access, at) || !accept(at)) return false;
        builder.record(at, access, project);
        if (!builder.layout.valid) return false;
    }
    result = std::move(builder.layout);
    return true;
}
} // namespace proteus4
