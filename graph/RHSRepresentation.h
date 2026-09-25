#pragma once

// A read-only, finite proof that an RHS's current facts are already expressed
// by the target. This does not prove either endpoint fully normalized and never
// removes a relationship. Unknown source fields impose no new requirements.
// Layout planning uses the same compressed-span equations as correspondence;
// this first proof supports only closed physical layouts, without filling gaps.
class RHSRepresentation {
    std::shared_ptr<ProteusGraphAccess> access_;
    bool layout(const POVNode& node, SpanLayout<POVNode>& result, bool source) {
        return readListLayout(*access_, node, result, work.visits,
            [&](const auto& at) {
                return at.phase != ListPhase::enter ||
                    (access_->closedPhysicalList(at.node) && access_->orderedList(at.node) &&
                     !access_->memberTemplate(at.node) && access_->inputError(at.node).empty() &&
                     (!source || (!access_->hasTypeRequirements(at.node) && access_->workSources(at.node).empty())));
            }, [](const POVNode& member) { return member; });
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
                if (!layout(left, a, false) || !layout(right, b, true)) return;
                auto plan = alignSpans(a, b);
                if (!plan.valid || !plan.complete || !plan.fills.empty()) return;
                pending.insert(pending.end(), plan.identities.begin(), plan.identities.end());
            }
            for (auto rhs : access_->workSources(right)) pending.emplace_back(left, rhs);
        }
        represented = true;
    }
};
