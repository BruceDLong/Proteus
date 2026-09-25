#pragma once

// Whole loose correspondence between a repeated finite group and a loaded
// scalar sequence. The relation maps member spans, never the two list roots:
// their extents count different items. All instance writes wait for the Set.
class GroupedListPlan {
    struct Group {
        POVNode node;
        bool append = false;
        std::shared_ptr<ProteusDefinitionPlan> plan;
    };
    std::shared_ptr<ProteusGraphAccess> access_;
    POVNode target_, source_;
    DefinitionSpec member_;
    std::vector<POVNode> sources_;
    std::vector<Group> groups_;
    bool satisfied_ = false;
    bool sourceLayout(const POVNode& node, bool root, std::set<std::shared_ptr<infonView>>& ancestors) {
        ++work.visits;
        if ((root ? !access_->groupingSource(node) :
             (!access_->closedPhysicalList(node) || !access_->orderedList(node) ||
              access_->memberTemplate(node) || access_->hasTypeRequirements(node) ||
              !access_->workSources(node).empty())) || !access_->inputError(node).empty() ||
            !ancestors.insert(node->pItem).second) return false;
        auto start = sources_.size();
        for (auto item = access_->first(node); item; item = access_->next(item)) {
            ++work.visits;
            if (access_->subList(item)) {
                if (!sourceLayout(item, false, ancestors)) return false;
            } else {
                if (!access_->correspondingMembers(item, item) || !access_->scalarCandidate(item) ||
                    access_->hasTypeRequirements(item) || !access_->typeFacts(item).valid ||
                    !access_->inputError(item).empty()) return false;
                sources_.push_back(item);
            }
        }
        auto extent = access_->listExtent(node);
        if (extent == -2) return false;
        if (extent >= 0 && static_cast<std::uint64_t>(extent) != sources_.size() - start)
            throw std::runtime_error("source extent disagrees with grouped correspondence");
        ancestors.erase(node->pItem);
        return true;
    }
    void members(const POVNode& group, std::vector<POVNode>& result) const {
        for (auto item = access_->first(group); item; item = access_->next(item)) {
            if (access_->subList(item)) members(item, result);
            else result.push_back(item);
        }
    }
public:
    Work work{0, 0};
    bool supported = false, represented = false;
    GroupedListPlan(std::shared_ptr<ProteusGraphAccess> access, POVNode target, POVNode source,
                    const DefinitionSpec& requirement)
        : access_(std::move(access)), target_(std::move(target)), source_(source) {
        if (!requirement->repeated || !access_->literalList(target_) || !access_->orderedList(target_) ||
            (!access_->templatePrefixList(target_) && !access_->closedPhysicalList(target_))) return;
        member_ = requirement->repeated;
        // Fixed arity is established by the definition walk. Variable-width,
        // nested grouping and contextual grammar need different mapping goals.
        if (!member_->list || member_->repeated || member_->members.empty()) return;
        for (const auto& child : member_->members) if (child->list) return;
        std::set<std::shared_ptr<infonView>> ancestors;
        if (!sourceLayout(source, true, ancestors)) return;
        const auto width = member_->members.size();
        if (sources_.size() % width) throw std::runtime_error("incomplete final group in whole loose correspondence");
        const auto count = sources_.size() / width;
        if (count > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) return;
        const auto extent = access_->listExtent(target_);
        if (extent == -2) return;
        if (extent >= 0 && static_cast<std::uint64_t>(extent) != count)
            throw std::runtime_error("group count disagrees with target extent");
        std::vector<POVNode> existing;
        for (auto item = access_->first(target_); item; item = access_->next(item)) {
            ++work.visits;
            if (!access_->correspondingMembers(item, item)) return;
            existing.push_back(item);
        }
        if (existing.size() > count || (!access_->openTail(target_) && existing.size() != count))
            throw std::runtime_error("group count disagrees with target members");
        const auto prefix = existing.size();
        existing.resize(count);
        // Reuse each already-demanded canonical POV. Distinct reference views
        // of the same index need a joint alias plan; defer rather than lose one.
        for (auto occurrence : access_->demandedOccurrences(target_)) {
            auto index = occurrence->implicitItem.sourceIndex;
            if (index < 0 || static_cast<std::uint64_t>(index) >= count)
                throw std::runtime_error("demanded occurrence exceeds grouped extent");
            if (existing[index] && existing[index] != occurrence) return;
            existing[index] = occurrence;
        }
        bool ready = !access_->openTail(target_) && extent >= 0;
        std::set<std::shared_ptr<infonView>> writable;
        for (std::size_t i = 0; i < count; ++i) {
            auto spec = std::make_shared<DefinitionRequirement>(*member_);
            for (std::size_t j = 0; j < width; ++j) {
                auto facts = access_->typeFacts(sources_[i * width + j]);
                auto child = std::make_shared<DefinitionRequirement>(*member_->members[j]);
                if (!child->facts.combine(facts)) throw std::runtime_error("source conflicts with group member requirement");
                spec->members[j] = child;
            }
            auto plan = std::make_shared<ProteusDefinitionPlan>(access_, existing[i], spec);
            if (!plan->supported) return;
            for (const auto& binding : plan->bindings) if (binding.first->node &&
                !writable.insert(binding.first->node->pItem).second) return;
            work.visits = addCost(work.visits, plan->work.visits);
            work.creations = addCost(work.creations, plan->work.creations);
            if (i >= prefix && !existing[i]) work.creations = addCost(work.creations, 2);
            ready = ready && plan->satisfied() && i < prefix;
            groups_.push_back({existing[i], i >= prefix, plan});
        }
        represented = ready;
        for (std::size_t i = 0; i < groups_.size(); ++i) {
            std::vector<POVNode> destinations;
            if (groups_[i].node && groups_[i].plan->satisfied()) members(groups_[i].node, destinations);
            for (std::size_t j = 0; j < width; ++j) {
                auto sourceItem = sources_[i * width + j];
                auto dest = j < destinations.size() ? destinations[j] : POVNode{};
                if (!dest || !access_->hasDirectIdentity(dest, sourceItem)) {
                    ++work.creations; ready = false;
                }
                if (represented) {
                    RHSRepresentation proof(access_, dest, sourceItem);
                    work.visits = addCost(work.visits, proof.work.visits);
                    represented = proof.represented;
                }
            }
        }
        satisfied_ = ready; supported = true;
    }
    bool satisfied() const { return supported && satisfied_; }
    bool apply() {
        if (!supported) return false;
        // Check context ownership for every existing endpoint before the first
        // write, just as strict correspondence does at its mutation boundary.
        access_->bind(target_); access_->bind(source_);
        for (const auto& source : sources_) access_->bind(source);
        for (const auto& group : groups_)
            for (const auto& binding : group.plan->bindings)
                if (binding.first->node) access_->bind(binding.first->node);
        bool changed = false;
        for (std::size_t i = 0; i < groups_.size(); ++i) {
            auto& group = groups_[i];
            if (group.append) {
                if (group.node) access_->appendOccurrence(target_, group.node);
                else {
                    group.node = access_->appendDefinitionMember(target_, member_->label);
                    group.plan->bindTarget(group.node);
                }
                changed = true;
            }
            changed |= group.plan->apply();
            std::vector<POVNode> destinations; members(group.node, destinations);
            for (std::size_t j = 0; j < destinations.size(); ++j) {
                auto source = sources_[i * member_->members.size() + j];
                auto dest = destinations[j];
                access_->bind(dest); access_->bind(source);
                if (!access_->hasDirectIdentity(dest, source)) {
                    access_->registry->recordIdentity(dest, source, iekListCorrespondence, 0); changed = true;
                }
            }
        }
        return access_->finishTail(target_, true, static_cast<std::int64_t>(groups_.size())) || changed;
    }
};
