#pragma once

// A proved data contradiction differs from an implementation/ownership error.
// Assertion goals report it; speculative correspondence can reject that route.
class DefinitionConflict : public GoalRejected {
public:
    using GoalRejected::GoalRejected;
};

// An unsupported or ambiguous conjunction is not a contradiction.
class DefinitionPending : public std::exception {};

// Compiled requirements are certificate data, not graph nodes. Their source
// graph is walked through existing POVs before any instance Set is offered.
struct DefinitionRequirement {
    GraphTypeFacts facts;
    NumericBounds numeric; // optional symbolic value constraint, not infSize
    // An open requirement may carry a proved prefix with an unresolved tail.
    // Compiling prefixed/embedded virtual definitions remains a separate
    // obligation from publishing a sequence proof's explicit members.
    bool list = false, open = false;
    int order = uOrdered; // a local member template can leave container order unspecified
    std::vector<std::shared_ptr<DefinitionRequirement>> members;
    // A homogeneous constraint on each occurrence, not N copied positions.
    std::shared_ptr<DefinitionRequirement> repeated;
    POVNode templateSource;
    struct Choice { POVNode source; std::vector<std::shared_ptr<DefinitionRequirement>> alternatives; };
    std::vector<Choice> choices;
    std::vector<std::string> types;
    // Names whose declarations were actually traversed, distinct from names
    // merely present on a supplied partial description.
    std::vector<std::string> expandedTypes;
    std::string label; // normalized nominal identity, used by comparisons
    std::shared_ptr<WordUse> writtenType;
    void captureLabel(const POVNode& node) {
        writtenType=node->pItem->type;
        if(!writtenType && !node->pItem->addedTypes.empty()) writtenType=node->pItem->addedTypes.front();
        if(writtenType) label=writtenType->typeID();
    }
    POVNode appendMember(ProteusGraphAccess& access,const POVNode& owner,const POVNode& before={}) const {
        auto member=access.insertDefinitionMember(owner,writtenType ? "" : label,before);
        // Keep spelling/model metadata separate from the normalized comparison
        // key, and never share mutable WordUse state with the declaration.
        if(writtenType) member->pItem->type=std::make_shared<WordUse>(*writtenType);
        return member;
    }
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

inline DefinitionSpec combineDefinitions(const DefinitionSpec& left, const DefinitionSpec& right, Work& work) {
    auto result = std::make_shared<DefinitionRequirement>(*left);
    if (!result->facts.combine(right->facts))
        throw DefinitionConflict("incompatible definition requirements");
    if(right->numeric.valid) {
        result->numeric=result->numeric.valid ? result->numeric.intersect(right->numeric) : right->numeric;
        if(result->numeric.empty) throw DefinitionConflict("incompatible numeric value requirements");
    }
    for(const auto& choice:right->choices)
        if(std::none_of(result->choices.begin(),result->choices.end(),[&](const auto& old){return old.source==choice.source;}))
            result->choices.push_back(choice);
    for (const auto& name : right->types)
        if (std::find(result->types.begin(), result->types.end(), name) == result->types.end()) result->types.push_back(name);
    for(const auto& name:right->expandedTypes)
        if(std::find(result->expandedTypes.begin(),result->expandedTypes.end(),name)==result->expandedTypes.end())
            result->expandedTypes.push_back(name);
    // Equivalent conjunctions share evidence regardless of application order.
    std::sort(result->types.begin(), result->types.end());
    std::sort(result->expandedTypes.begin(), result->expandedTypes.end());
    if (result->label.empty()) {result->label = right->label;result->writtenType=right->writtenType;}
    if (result->list && right->list) {
        if(result->order==uUnknown) result->order=right->order;
        else if(right->order!=uUnknown && result->order!=right->order) throw DefinitionPending{};
        if(result->order==uUnordered) {
            if(result->repeated || right->repeated) throw DefinitionPending{};
            MemberCorrespondence mapping(result->members.size(),right->members.size(),
                [](std::size_t,std::size_t){return false;},[&](std::size_t a,std::size_t b) {
                    const auto& x=result->members[a];const auto& y=right->members[b];
                    if(x->label.empty() || y->label.empty()) return ScalarMatch::unknown;
                    return compareNominal(y->label,y->types,x->label,x->types);
                },work);
            if(!mapping.unambiguous) throw DefinitionPending{};
            for(const auto& pair:mapping.pairs) result->members[pair.first]=combineDefinitions(result->members[pair.first],right->members[pair.second],work);
            for(auto member:mapping.missing) result->members.push_back(right->members[member]);
        } else if (result->open && right->open) {
            if (result->repeated && right->repeated)
                result->repeated = combineDefinitions(result->repeated, right->repeated,work);
            else if (right->repeated) {
                result->repeated = right->repeated; result->templateSource = right->templateSource;
            }
        } else if (result->open || right->open) {
            auto repeated = result->open ? result->repeated : right->repeated;
            auto finite = result->open ? right : result;
            result->members = finite->members;
            if (repeated) for (auto& member : result->members) member = combineDefinitions(member, repeated,work);
            result->open = false; result->repeated.reset(); result->templateSource.reset();
        } else {
            if (result->members.size() != right->members.size())
                throw DefinitionConflict("incompatible finite definition lengths");
            for (std::size_t i = 0; i < result->members.size(); ++i)
                result->members[i] = combineDefinitions(result->members[i], right->members[i],work);
        }
    } else if (right->list) {
        result->list = true; result->order=right->order; result->open = right->open; result->members = right->members;
        result->repeated = right->repeated; result->templateSource = right->templateSource;
    }
    if (result->list && !result->memberLayout().valid)
        throw DefinitionConflict("incompatible definition extent");
    return result;
}

inline bool sameDefinition(const DefinitionSpec& a, const DefinitionSpec& b) {
    if (a == b) return true;
    if (!a || !b || a->list != b->list || a->open != b->open || a->order!=b->order || !a->facts.same(b->facts) || !a->numeric.same(b->numeric) ||
        a->types != b->types || a->expandedTypes != b->expandedTypes || a->members.size() != b->members.size() || a->choices.size()!=b->choices.size() ||
        !sameDefinition(a->repeated, b->repeated)) return false;
    for(std::size_t i=0;i<a->choices.size();++i) {
        const auto& left=a->choices[i];const auto& right=b->choices[i];
        if(left.source!=right.source || left.alternatives.size()!=right.alternatives.size()) return false;
        for(std::size_t j=0;j<left.alternatives.size();++j)
            if(!sameDefinition(left.alternatives[j],right.alternatives[j])) return false;
    }
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
    Place unmeasuredRoot_, fillRoot_;
    bool typedDescription(const Place& target,const DefinitionSpec& requirement) const {
        if(!target->node || !requirement->list || requirement->repeated ||
           !access_->physicalList(target->node) || access_->openTail(target->node) ||
           !access_->hasTypeRequirements(target->node)) return false;
        for(const auto& member:requirement->members) if(member->label.empty()) return false;
        for(auto member=POVDescription(target->node).member();member;member=POVDescription(member).adjacent())
            if(access_->subList(member) || !access_->hasTypeRequirements(member)) return false;
        // A complete ordered layout can disambiguate repeated field types by
        // position. Missing positions still require unique nominal evidence.
        if(requirement->order==uOrdered && target->node->pItem->value->items.size()==requirement->members.size()) {
            std::set<std::string> labels;
            for(const auto& member:requirement->members) if(!labels.insert(member->label).second) return false;
        }
        return true;
    }
    bool fillDescription(const Place& target,const DefinitionSpec& requirement) const {
        return target==fillRoot_ && typedDescription(target,requirement);
    }
    void prepareRepeated(const Place& target, const DefinitionSpec& requirement, GraphTypeFacts actual) {
        auto access = access_;
        bool adopt = !target->node || access->adoptableList(target->node);
        bool open = adopt || access->openTail(target->node);
        std::vector<POVNode> members;
        if (!adopt) {
            if ((requirement->order==uOrdered && !access->definitionOrderedCandidate(target->node)) ||
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
        if ((!target->node || access->definitionOrderedCandidate(target->node)) &&
            (!target->node || !access->definitionOrderKnown(target->node))) {
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
    void prepareRecord(const Place& target,const DefinitionSpec& requirement,const GraphTypeFacts& actual,bool lazy=false) {
        auto access=access_;
        // Unnamed/duplicate descriptions need richer disambiguating predicates.
        for(std::size_t i=0;i<requirement->members.size();++i) {
            const auto& member=requirement->members[i];
            if(member->label.empty()) {supported=false;return;}
            for(std::size_t j=0;j<i;++j) {
                ++work.visits;
                if(compareNominal(member->label,member->types,requirement->members[j]->label,requirement->members[j]->types)==ScalarMatch::yes) {
                    supported=false;return;
                }
            }
        }
        bool adopt=!target->node || access->adoptableList(target->node);
        std::vector<POVNode> present;
        if(!adopt) {
            if(!access->physicalList(target->node) ||
               (target->node->pItem->orderMode!=uUnknown && target->node->pItem->orderMode!=requirement->order)) {supported=false;return;}
            for(auto member=POVDescription(target->node).member();member;member=POVDescription(member).adjacent()) {
                ++work.visits;
                if(access->subList(member)) {supported=false;return;}
                present.push_back(member);
            }
        }
        if(requirement->facts.hasSize && requirement->facts.size<static_cast<std::int64_t>(present.size()))
            throw DefinitionConflict("definition members exceed explicit extent");
        MemberCorrespondence mapping(present.size(),requirement->members.size(),
            [](std::size_t,std::size_t){return false;},[&](std::size_t a,std::size_t b) {
                const auto& member=requirement->members[b];auto names=access->typeNames(present[a]);
                std::vector<std::string> types(names.begin(),names.end());
                return compareNominal(member->label,member->types,types.empty() ? "" : types.front(),types);
            },work);
        if(!mapping.unambiguous || (!adopt && !lazy && target!=fillRoot_ && !mapping.missing.empty())) {supported=false;return;}
        // Ordered typed descriptions can omit fields, but cannot reorder them
        // or guess where an extra field belongs. Reuse the same nominal proof.
        if(requirement->order==uOrdered) {
            if(!lazy && mapping.pairs.size()!=present.size()) {supported=false;return;}
            for(std::size_t i=1;i<mapping.pairs.size();++i)
                if(mapping.pairs[i-1].first>=mapping.pairs[i].first) throw DefinitionConflict("typed fields contradict definition order");
        }
        auto count=static_cast<std::int64_t>(present.size()+(lazy ? 0 : mapping.missing.size()));
        if((actual.hasSize && actual.size<count) || (requirement->facts.hasSize && requirement->facts.size<count))
            throw DefinitionConflict("definition parts exceed explicit extent");
        if(adopt) writes.push_back([access,target,requirement] {
            bool changed=access->adoptDefinitionList(target->node);
            changed=access->setListOrder(target->node,static_cast<OrderMode>(requirement->order)) || changed;
            SetFlagBit(target->node->pItem->value->flags,infonState::tailUnfinished,false);
            return changed;
        });
        if(!adopt && target->node->pItem->orderMode==uUnknown) writes.push_back([access,target,requirement] {
            return access->setListOrder(target->node,static_cast<OrderMode>(requirement->order));
        });
        if(!adopt && !actual.hasSize && access->typeFacts(target->node).hasSize) {
            ++work.creations;
            writes.push_back([access,target] {return access->invalidateCountedSize(target->node);});
        }
        if(!requirement->facts.accepts(actual)) writes.push_back([access,target,requirement] {
            return access->applyTypeFacts(target->node,requirement->facts);
        });
        for(const auto& pair:mapping.pairs) prepare(place(present[pair.first]),requirement->members[pair.second]);
        if(!lazy) for(auto index:mapping.missing) {
            auto member=std::make_shared<Slot>();auto spec=requirement->members[index];
            work.creations=addCost(work.creations,2);
            POVNode before;
            if(requirement->order==uOrdered) for(const auto& pair:mapping.pairs)
                if(pair.second>index) {before=present[pair.first];break;}
            writes.push_back([access,target,member,spec,before] {member->node=spec->appendMember(*access,target->node,before);return true;});
            prepare(member,spec);
        }
    }
    void prepare(const Place& target, const DefinitionSpec& sourceRequirement) {
        if (!supported) return;
        auto requirement=sourceRequirement;
        bool lazy=fillRoot_ && target!=fillRoot_ && typedDescription(target,requirement);
        if(lazy && !requirement->facts.explicitSize) {
            requirement=std::make_shared<DefinitionRequirement>(*requirement);
            requirement->facts.hasSize=false;
        }
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
            if(!actual.explicitSize && ((requirement->list && requirement->order==uUnordered && target->node->pItem->orderMode==uUnknown) ||
                                       (fillDescription(target,requirement) && requirement->facts.hasSize && actual.hasSize && actual.size!=requirement->facts.size))) actual.hasSize=false;
            auto found = proposed_.find(target->node->pItem);
            if (found != proposed_.end()) actual = found->second;
        }
        // A choice constrains the result, but does not authorize choosing an
        // arbitrary value. An accepted correspondence can already satisfy one
        // branch; otherwise leave the alternatives available to graph search.
        for(const auto& choice:requirement->choices) {
            bool accepted=false,pending=!target->node;
            if(target->node) for(const auto& alternative:choice.alternatives) {
                try {
                    ProteusDefinitionPlan check(access_,target->node,alternative);
                    work.visits=addCost(work.visits,check.work.visits);
                    if(check.dataSatisfied()) {accepted=true;break;}
                    pending=true;
                } catch(const DefinitionConflict&) {}
            }
            if(!accepted && !pending) throw DefinitionConflict("no compatible definition alternative");
            if(!accepted) {supported=false;return;}
        }
        auto combined = actual;
        if (!combined.combine(requirement->facts))
            throw DefinitionConflict("incompatible definition requirements");
        if (target->node) proposed_[target->node->pItem] = combined;
        // Cache the full meaning, not the subset demanded by this application.
        // A later explicit normalization of this child must still see its parts.
        bindings.push_back({target, sourceRequirement});
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
        if (requirement->order==uUnordered || lazy || fillDescription(target,requirement)) {prepareRecord(target,requirement,actual,lazy);return;}
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
                    member->node = spec->appendMember(*access,owner->node); return true;
                });
                prepare(member, spec);
            }
            // Closing a zero-width gap is a real write; an already closed
            // physical list does not produce a fill in the alignment plan.
            auto extent=owner==unmeasuredRoot_ ? -1 : fill.extent;
            writes.push_back([access, owner, fill, extent] {
                return access->finishTail(owner->node, fill.close, extent);
            });
        }
    }
public:
    ProteusDefinitionPlan(std::shared_ptr<ProteusGraphAccess> access, const POVNode& target, const DefinitionSpec& requirement,
                          std::map<POVNode, std::int64_t> extents = {}, bool measureRoot=true, bool fillParts=false)
        : access_(std::move(access)), extents_(std::move(extents)) {
        if(!measureRoot) unmeasuredRoot_=place(target);
        if(fillParts) fillRoot_=place(target);
        prepare(place(target), requirement);
    }
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
