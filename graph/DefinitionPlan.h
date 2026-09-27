#pragma once

// A proved data contradiction differs from an implementation/ownership error.
// Assertion goals report it; speculative correspondence can reject that route.
class DefinitionConflict : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Compiled requirements are certificate data, not graph nodes. Their source
// graph is walked through existing POVs before any instance Set is offered.
struct DefinitionRequirement {
    GraphTypeFacts facts;
    NumericBounds numeric; // optional symbolic value constraint, not infSize
    // An open requirement may carry a proved prefix with an unresolved tail.
    // Compiling prefixed/embedded virtual definitions remains a separate
    // obligation from publishing a sequence proof's explicit members.
    bool list = false, open = false;
    std::vector<std::shared_ptr<DefinitionRequirement>> members;
    // A homogeneous constraint on each occurrence, not N copied positions.
    std::shared_ptr<DefinitionRequirement> repeated;
    POVNode templateSource;
    std::vector<std::string> types;
    std::string label;
    // Explicit positions and an unmaterialized tail share the runtime's span
    // equations. Cardinality does not allocate one requirement per position.
    SpanLayout<std::shared_ptr<DefinitionRequirement>> memberLayout() const {
        SpanLayout<std::shared_ptr<DefinitionRequirement>> result;
        for (const auto& member : members) result.segments.push_back({member, false, 1});
        if (open) result.segments.push_back({{}, true, -1});
        std::int64_t extent = -1;
        if (facts.hasSize) {
            auto size = facts.size;
            if (size < 0 || size.getDenominator() != 1 ||
                size > std::numeric_limits<std::int64_t>::max()) { result.valid = false; return result; }
            extent = size.getNumerator().get_si();
        }
        result.equations.push_back({0, result.segments.size(), extent, {}});
        result.solve();
        return result;
    }
};
using DefinitionSpec = std::shared_ptr<DefinitionRequirement>;

inline DefinitionSpec combineDefinitions(const DefinitionSpec& left, const DefinitionSpec& right) {
    auto result = std::make_shared<DefinitionRequirement>(*left);
    if (!result->facts.combine(right->facts))
        throw DefinitionConflict("incompatible definition requirements");
    if(right->numeric.valid) {
        result->numeric=result->numeric.valid ? result->numeric.intersect(right->numeric) : right->numeric;
        if(result->numeric.empty) throw DefinitionConflict("incompatible numeric value requirements");
    }
    for (const auto& name : right->types)
        if (std::find(result->types.begin(), result->types.end(), name) == result->types.end()) result->types.push_back(name);
    // Equivalent conjunctions share evidence regardless of application order.
    std::sort(result->types.begin(), result->types.end());
    if (result->label.empty()) result->label = right->label;
    if (result->list && right->list) {
        if (result->open && right->open) {
            if (result->repeated && right->repeated)
                result->repeated = combineDefinitions(result->repeated, right->repeated);
            else if (right->repeated) {
                result->repeated = right->repeated; result->templateSource = right->templateSource;
            }
        } else if (result->open || right->open) {
            auto repeated = result->open ? result->repeated : right->repeated;
            auto finite = result->open ? right : result;
            result->members = finite->members;
            if (repeated) for (auto& member : result->members) member = combineDefinitions(member, repeated);
            result->open = false; result->repeated.reset(); result->templateSource.reset();
        } else {
            if (result->members.size() != right->members.size())
                throw DefinitionConflict("incompatible finite definition lengths");
            for (std::size_t i = 0; i < result->members.size(); ++i)
                result->members[i] = combineDefinitions(result->members[i], right->members[i]);
        }
    } else if (right->list) {
        result->list = true; result->open = right->open; result->members = right->members;
        result->repeated = right->repeated; result->templateSource = right->templateSource;
    }
    if (result->list && !result->memberLayout().valid)
        throw DefinitionConflict("incompatible definition extent");
    return result;
}

inline bool sameDefinition(const DefinitionSpec& a, const DefinitionSpec& b) {
    if (a == b) return true;
    if (!a || !b || a->list != b->list || a->open != b->open || !a->facts.same(b->facts) || !a->numeric.same(b->numeric) ||
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
    // Accepted correspondence may prove embedded extents without publishing
    // them first. Keep that evidence in the atomic plan, not in live nodes.
    std::map<POVNode, std::int64_t> extents_;
    std::map<POVNode, Place> places_;
    std::map<std::shared_ptr<infonView>, GraphTypeFacts> proposed_;
    std::map<std::shared_ptr<infonView>,NumericBounds> proposedNumbers_;
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
                    ((access_->physicalList(at.node) ||
                      (access_->fedTemplate(at.node) && access_->templatePrefixList(at.node)) ||
                      (extents_.count(at.node) && access_->unreferencedTemplateSpan(at.node))) &&
                     access_->definitionOrderedCandidate(at.node));
            }, [&](const POVNode& member) { return Position{place(member), {}}; });
        for (auto& equation : result.equations) {
            auto found = extents_.find(equation.owner.place->node);
            if (found == extents_.end()) continue;
            if (equation.expected >= 0 && equation.expected != found->second)
                throw std::logic_error("accepted extent conflicts with list size");
            equation.expected = found->second;
        }
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
            for (auto item = access->first(target->node, false); item; item = access->next(item, false)) {
                if (!access->correspondingMembers(item, item)) { supported = false; return; }
                members.push_back(item);
            }
            auto extent = actual; extent.combine(requirement->facts);
            if (open && extent.hasSize && extent.size < static_cast<std::int64_t>(members.size()))
                throw DefinitionConflict("repeated definition prefix exceeds extent");
            if (!open) {
                GraphTypeFacts count; count.valid = true; count.hasSize = true;
                count.size = static_cast<std::int64_t>(members.size());
                if (!count.combine(actual) || !count.combine(requirement->facts))
                    throw DefinitionConflict("incompatible repeated definition extent");
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
        NumericView number;
        if (target->node) {
            actual = access_->typeFacts(target->node);
            if(requirement->numeric.valid) {
                NumericRangeAccess reader(*access_);number=reader.read(target->node);
                work.visits=addCost(work.visits,reader.work.visits);
                if(number.bounds.valid) actual=number.facts;
                auto previous=proposedNumbers_.find(target->node->pItem);
                if(previous!=proposedNumbers_.end()) number.bounds=previous->second;
            }
            if (!actual.valid) { supported = false; return; }
            auto found = proposed_.find(target->node->pItem);
            if (found != proposed_.end()) actual = found->second;
        }
        auto combined = actual;
        if (!combined.combine(requirement->facts))
            throw DefinitionConflict("incompatible definition requirements");
        if (target->node) proposed_[target->node->pItem] = combined;
        bindings.push_back({target, requirement});
        if (!requirement->list) {
            if(requirement->numeric.valid) {
                NumericView result;result.facts=combined;result.expression=true;
                result.bounds=number.bounds.valid ? number.bounds.intersect(requirement->numeric) : requirement->numeric;
                if(result.bounds.empty) throw DefinitionConflict("incompatible numeric value requirements");
                if(target->node) proposedNumbers_[target->node->pItem]=result.bounds;
                if(!number.bounds.same(result.bounds) || !requirement->facts.accepts(actual)) {
                    work.creations=addCost(work.creations,result.bounds.point ? 1 : 5);
                    writes.push_back([access,target,result] {return NumericRangeAccess::write(*access,target->node,result);});
                }
                return;
            }
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
        auto shape = requirement->memberLayout();
        if (!shape.valid) throw DefinitionConflict("incompatible definition extent");
        for (const auto& segment : shape.segments)
            right.segments.push_back({{{}, segment.node}, segment.gap, segment.width});
        right.equations.push_back({0, right.segments.size(), shape.extent(), {{}, requirement}});
        auto plan = alignSpans(left, right);
        if (!plan.valid) throw DefinitionConflict("incompatible finite definition extent");
        if (!plan.splices.empty() || !plan.crossings.empty()) { supported = false; return; }
        // An unconstrained virtual tail need not be filled to prove the type.
        // Every explicit required position must still be mapped unambiguously.
        std::size_t mapped = plan.identities.size();
        for (const auto& fill : plan.fills) if (fill.owner.place) mapped += fill.sources.size();
        if ((!requirement->open && !plan.complete) || mapped != requirement->members.size()) {
            supported = false; return;
        }
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
            if (!owner) continue; // Actual members already satisfy an unconstrained source gap.
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
    ProteusDefinitionPlan(std::shared_ptr<ProteusGraphAccess> access, const POVNode& target, const DefinitionSpec& requirement,
                          std::map<POVNode, std::int64_t> extents = {})
        : access_(std::move(access)), extents_(std::move(extents)) { prepare(place(target), requirement); }
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
