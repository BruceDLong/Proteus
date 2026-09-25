#pragma once

// A read-only, finite proof that an RHS's current facts are already expressed
// by the target. This does not prove either endpoint fully normalized and never
// removes a relationship. Unknown source fields impose no new requirements.
// Layout planning uses the same compressed-span equations as correspondence;
// this first proof supports only closed physical layouts, without filling gaps.
class RHSRepresentation {
    std::shared_ptr<ProteusGraphAccess> access_;
    bool layout(const POVNode& node, SpanLayout<POVNode>& result, bool source,
                std::set<std::shared_ptr<infonView>>& ancestors) {
        ++work.visits;
        if (!access_->closedPhysicalList(node) || !access_->orderedList(node) ||
            access_->memberTemplate(node) || !access_->inputError(node).empty() ||
            (source && (access_->hasTypeRequirements(node) || !access_->workSources(node).empty())) ||
            !ancestors.insert(node->pItem).second) return false;
        const auto begin = result.segments.size();
        for (auto member = access_->first(node); member; member = access_->next(member)) {
            ++work.visits;
            if (access_->subList(member)) {
                if (!layout(member, result, source, ancestors)) return false;
            } else if (access_->correspondingMembers(member, member)) {
                result.segments.push_back({member, false, 1});
            } else return false;
        }
        const auto extent = access_->listExtent(node);
        if (extent == -2) return false;
        result.equations.push_back({begin, result.segments.size(), extent, node});
        ancestors.erase(node->pItem);
        return true;
    }
public:
    Work work{0, 0};
    bool represented = false;
    RHSRepresentation(std::shared_ptr<ProteusGraphAccess> access, POVNode target, POVNode source)
        : access_(std::move(access)) {
        std::vector<std::pair<POVNode, POVNode>> pending{{std::move(target), std::move(source)}};
        std::set<std::pair<POVNode, POVNode>> visited;
        while (!pending.empty()) {
            auto pair = pending.back(); pending.pop_back();
            auto left = pair.first, right = pair.second;
            ++work.visits;
            // A cyclic proof cannot establish itself. Decline conservatively;
            // actual graph traversal still follows every retained constraint.
            if (!visited.insert(pair).second || !access_->inputError(left).empty() ||
                !access_->inputError(right).empty() || access_->hasTypeRequirements(right) ||
                access_->occurrenceOwner(right) || access_->memberTemplate(right) ||
                (!access_->scalarCandidate(right) && !access_->literalList(right))) return;
            auto actual = access_->typeFacts(left), required = access_->typeFacts(right);
            if (!required.valid || !required.accepts(actual)) return;
            if (access_->literalList(right)) {
                SpanLayout<POVNode> a, b;
                std::set<std::shared_ptr<infonView>> ancestors;
                if (!layout(left, a, false, ancestors) || !layout(right, b, true, ancestors)) return;
                auto plan = alignSpans(a, b);
                if (!plan.valid || !plan.complete || !plan.fills.empty()) return;
                pending.insert(pending.end(), plan.identities.begin(), plan.identities.end());
            }
            for (auto rhs : access_->workSources(right)) pending.emplace_back(left, rhs);
        }
        represented = true;
    }
};
