#pragma once

// Compiled requirements are certificate data, not graph nodes. Their source
// graph is walked through existing POVs before any instance Set is offered.
struct DefinitionRequirement {
    GraphTypeFacts facts;
    bool list = false;
    std::vector<std::shared_ptr<DefinitionRequirement>> members;
    // A homogeneous constraint on each occurrence, not N copied positions.
    std::shared_ptr<DefinitionRequirement> repeated;
    POVNode templateSource;
    std::vector<std::string> types;
    std::string label;
};
using DefinitionSpec = std::shared_ptr<DefinitionRequirement>;

inline DefinitionSpec combineDefinitions(const DefinitionSpec& left, const DefinitionSpec& right) {
    auto result = std::make_shared<DefinitionRequirement>(*left);
    if (!result->facts.combine(right->facts))
        throw std::runtime_error("incompatible definition requirements");
    for (const auto& name : right->types)
        if (std::find(result->types.begin(), result->types.end(), name) == result->types.end()) result->types.push_back(name);
    // Equivalent conjunctions share evidence regardless of application order.
    std::sort(result->types.begin(), result->types.end());
    if (result->label.empty()) result->label = right->label;
    if (result->list && right->list) {
        if (result->repeated && right->repeated) {
            result->repeated = combineDefinitions(result->repeated, right->repeated);
        } else if (result->repeated || right->repeated) {
            auto repeated = result->repeated ? result->repeated : right->repeated;
            auto finite = result->repeated ? right : result;
            result->members = finite->members;
            for (auto& member : result->members) member = combineDefinitions(member, repeated);
            result->repeated.reset(); result->templateSource.reset();
        } else {
            if (result->members.size() != right->members.size())
                throw std::runtime_error("incompatible finite definition lengths");
            for (std::size_t i = 0; i < result->members.size(); ++i)
                result->members[i] = combineDefinitions(result->members[i], right->members[i]);
        }
    } else if (right->list) {
        result->list = true; result->members = right->members;
        result->repeated = right->repeated; result->templateSource = right->templateSource;
    }
    return result;
}

inline bool sameDefinition(const DefinitionSpec& a, const DefinitionSpec& b) {
    if (a == b) return true;
    if (!a || !b || a->list != b->list || !a->facts.same(b->facts) ||
        a->types != b->types || a->members.size() != b->members.size() ||
        !sameDefinition(a->repeated, b->repeated)) return false;
    for (std::size_t i = 0; i < a->members.size(); ++i)
        if (!sameDefinition(a->members[i], b->members[i])) return false;
    return true;
}

// A finite, side-effect-free plan. It uses the same span planner as ordinary
// correspondence, but its destinations are instance slots and its sources are
// requirements. Allocation, writes and certificates wait for the priced Set.
class ProteusDefinitionPlan {
public:
    struct Slot { POVNode node; };
    using Place = std::shared_ptr<Slot>;
    struct Position {
        Place place;
        DefinitionSpec requirement;
        bool operator==(const Position& other) const { return place == other.place && requirement == other.requirement; }
    };
    std::vector<std::pair<Place, DefinitionSpec>> bindings;
    std::vector<std::function<bool()>> writes;
    Work work{0, 0};
    bool supported = true;
    std::size_t orderWrites = 0;
private:
    std::shared_ptr<ProteusGraphAccess> access_;
    std::map<POVNode, Place> places_;
    std::map<std::shared_ptr<infonView>, GraphTypeFacts> proposed_;
    std::vector<std::shared_ptr<infonView>> plannedLists_;
    Place place(const POVNode& node) {
        auto& result = places_[node];
        if (!result) result = std::make_shared<Slot>(Slot{node});
        return result;
    }
    void layout(const POVNode& node, SpanLayout<Position>& result) {
        supported = readListLayout(*access_, node, result, work.visits,
            [&](const auto& at) {
                return at.phase != ListPhase::enter ||
                    (access_->physicalList(at.node) && access_->definitionOrderedCandidate(at.node));
            }, [&](const POVNode& member) { return Position{place(member), {}}; });
    }
    void prepareRepeated(const Place& target, const DefinitionSpec& requirement, GraphTypeFacts actual) {
        auto access = access_;
        bool adopt = !target->node || access->adoptableList(target->node);
        bool open = adopt || access->openTail(target->node);
        std::vector<POVNode> members;
        if (!adopt) {
            if (!access->definitionOrderedCandidate(target->node) ||
                (!access->physicalList(target->node) && !access->templatePrefixList(target->node))) {
                supported = false; return;
            }
            for (auto item = access->first(target->node); item; item = access->next(item)) {
                if (!access->correspondingMembers(item, item)) { supported = false; return; }
                members.push_back(item);
            }
            auto extent = actual; extent.combine(requirement->facts);
            if (open && extent.hasSize && extent.size < static_cast<std::int64_t>(members.size()))
                throw std::runtime_error("repeated definition prefix exceeds extent");
            if (!open) {
                GraphTypeFacts count; count.valid = true; count.hasSize = true;
                count.size = static_cast<std::int64_t>(members.size());
                if (!count.combine(actual) || !count.combine(requirement->facts))
                    throw std::runtime_error("incompatible repeated definition extent");
            }
            auto demanded = access->demandedOccurrences(target->node);
            members.insert(members.end(), demanded.begin(), demanded.end());
        }
        if (adopt) writes.push_back([access, target] { return access->adoptDefinitionList(target->node); });
        if (!requirement->facts.accepts(actual)) writes.push_back([access, target, requirement] {
            return access->applyTypeFacts(target->node, requirement->facts);
        });
        if (!target->node || !access->definitionOrderKnown(target->node)) {
            ++orderWrites;
            writes.push_back([access, target] { return access->setDefinitionOrder(target->node); });
        }
        if (open && (!target->node || !access->memberTemplate(target->node))) {
            ++work.creations;
            writes.push_back([access, target, requirement] {
                return access->installMemberTemplate(target->node, requirement->templateSource);
            });
        }
        auto finalFacts = actual;
        finalFacts.combine(requirement->facts);
        if (open && finalFacts.hasSize && finalFacts.size == 0) writes.push_back([access, target] {
            return access->finishTail(target->node, true, 0);
        });
        for (const auto& member : members) prepare(place(member), requirement->repeated);
    }
    void prepare(const Place& target, const DefinitionSpec& requirement) {
        if (!supported) return;
        ++work.visits;
        auto access = access_;
        GraphTypeFacts actual;
        actual.valid = true;
        if (target->node) {
            actual = access_->typeFacts(target->node);
            if (!actual.valid) { supported = false; return; }
            auto found = proposed_.find(target->node->pItem);
            if (found != proposed_.end()) actual = found->second;
        }
        auto combined = actual;
        if (!combined.combine(requirement->facts))
            throw std::runtime_error("incompatible definition requirements");
        if (target->node) proposed_[target->node->pItem] = combined;
        bindings.push_back({target, requirement});
        if (!requirement->list) {
            if (!requirement->facts.accepts(actual)) writes.push_back([access, target, requirement] {
                return access->applyTypeFacts(target->node, requirement->facts);
            });
            return;
        }
        // Shared/cyclic writable list storage needs an explicit joint plan.
        // Do not independently fill the same underlying list twice in a batch.
        if (target->node) {
            auto infon = target->node->pItem;
            if (std::find(plannedLists_.begin(), plannedLists_.end(), infon) != plannedLists_.end()) {
                supported = false; return;
            }
            plannedLists_.push_back(infon);
        }
        if (requirement->repeated) { prepareRepeated(target, requirement, actual); return; }
        SpanLayout<Position> left, right;
        if (!target->node || access_->adoptableList(target->node)) {
            writes.push_back([access, target] { return access->adoptDefinitionList(target->node); });
            left.segments.push_back({{target, {}}, true, -1});
            left.equations.push_back({0, 1, -1, {target, {}}});
        } else layout(target->node, left);
        if (!supported) return;
        for (const auto& member : requirement->members) right.segments.push_back({{{}, member}, false, 1});
        right.equations.push_back({0, right.segments.size(), static_cast<std::int64_t>(right.segments.size()), {{}, requirement}});
        auto plan = alignSpans(left, right);
        if (!plan.valid) throw std::runtime_error("incompatible finite definition extent");
        if (!plan.complete) { supported = false; return; }
        if (!requirement->facts.accepts(actual)) writes.push_back([access, target, requirement] {
            return access->applyTypeFacts(target->node, requirement->facts);
        });
        if (!target->node || !access_->definitionOrderKnown(target->node)) {
            ++orderWrites;
            writes.push_back([access, target] { return access->setDefinitionOrder(target->node); });
        }
        for (const auto& pair : plan.identities) prepare(pair.first.place, pair.second.requirement);
        for (const auto& fill : plan.fills) {
            auto owner = fill.owner.place;
            for (const auto& source : fill.sources) {
                auto member = std::make_shared<Slot>();
                auto spec = source.requirement;
                work.creations = addCost(work.creations, 2);
                writes.push_back([access, owner, member, spec] {
                    member->node = access->appendDefinitionMember(owner->node, spec->label); return true;
                });
                prepare(member, spec);
            }
            // Closing a zero-width gap is a real write; an already closed
            // physical list does not produce a fill in the alignment plan.
            writes.push_back([access, owner, fill] {
                return access->finishTail(owner->node, fill.close, fill.extent);
            });
        }
    }
public:
    ProteusDefinitionPlan(std::shared_ptr<ProteusGraphAccess> access, const POVNode& target, const DefinitionSpec& requirement)
        : access_(std::move(access)) { prepare(place(target), requirement); }
    // Deferred allocation clients can prepare a member before the priced Set
    // creates its POV. All other slots and writes remain instance-local.
    void bindTarget(const POVNode& node) { place({})->node = node; }
    bool satisfied() const { return supported && writes.empty(); }
    bool dataSatisfied() const { return supported && writes.size() == orderWrites; }
    bool apply() {
        if (!supported) return false;
        bool changed = false;
        for (const auto& write : writes) changed |= write();
        return changed;
    }
};
