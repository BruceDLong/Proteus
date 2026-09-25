#pragma once
#include "SpanAlignment.h"

namespace proteus4 {
// Structural events over existing nodes. Ordinary nested lists occupy one
// position; only embedded lists are entered. Owners preserve each boundary.
enum class ListPhase { enter, member, leave, done, cycle };
template<class Node> struct ListPosition {
    Node node;
    ListPhase phase = ListPhase::enter;
    std::vector<Node> owners{};
};

// One step, usable either as a graph edge or by a finite read-only planner.
// Graph walks can request only published neighbours. Finite inspection uses
// stored neighbours so unfinished data remains an obligation for its consumer.
// No implicit occurrence is materialized and no semantic rule runs.
template<class Node, class Access>
ListPosition<Node> nextListPosition(Access& access, ListPosition<Node> at, bool publishedOnly = false) {
    auto visible = [&](Node node) { return node && publishedOnly && access.inputPending(node) ? Node{} : node; };
    if (at.phase == ListPhase::enter) {
        for (const auto& owner : at.owners) if (access.sameInfon(owner, at.node)) {
            at.phase = ListPhase::cycle; return at;
        }
        if (auto first = visible(access.first(at.node))) {
            at.owners.push_back(at.node); at.node = first; at.phase = ListPhase::member;
        } else at.phase = ListPhase::leave;
    } else if (at.phase == ListPhase::member && access.subList(at.node)) {
        at.phase = ListPhase::enter;
    } else if (at.phase == ListPhase::member || at.phase == ListPhase::leave) {
        if (at.owners.empty()) at.phase = ListPhase::done;
        else if (auto next = visible(access.next(at.node))) {
            at.node = next; at.phase = ListPhase::member;
        } else {
            at.node = at.owners.back(); at.owners.pop_back(); at.phase = ListPhase::leave;
        }
    }
    return at;
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
