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
    bool independentTemporalValue = false;
    // Contextual expressions remain graph constraints in each instance. Their
    // declaration scopes identify the instance that %self must refer to.
    struct Expression {POVNode source, scope;};
    std::vector<Expression> expressions;
    std::vector<POVNode> selfScopes;
    NumericBounds numeric; // optional symbolic value constraint, not infSize
    // An open requirement may carry a proved prefix with an unresolved tail.
    // Embedded virtual definitions remain a separate obligation; a written
    // prefix never allocates the unresolved positions of its tail.
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
    bool hasExpressions() const {
        // Compiled definitions share subrequirements. Walk the DAG, not every
        // path through it; keep visits local so later edits remain observable.
        std::vector<const DefinitionRequirement*> pending{this};
        std::set<const DefinitionRequirement*> seen;
        while(!pending.empty()) {
            auto node=pending.back();pending.pop_back();
            if(!node || !seen.insert(node).second) continue;
            if(!node->expressions.empty()) return true;
            if(node->repeated) pending.push_back(node->repeated.get());
            for(const auto& member:node->members) pending.push_back(member.get());
            for(const auto& choice:node->choices) for(const auto& alternative:choice.alternatives)
                pending.push_back(alternative.get());
        }
        return false;
    }
    void captureLabel(const POVNode& node) {
        writtenType=readView(node)->type;
        if(!writtenType && !readView(node)->addedTypes.empty()) writtenType=readView(node)->addedTypes.front();
        if(writtenType) label=writtenType->typeID();
    }
    POVNode appendMember(ProteusGraphAccess& access,const POVNode& owner,const POVNode& before={}) const {
        auto member=access.insertDefinitionMember(owner,writtenType ? "" : label,before);
        // Keep spelling/model metadata separate from the normalized comparison
        // key, and never share mutable WordUse state with the declaration.
        if(writtenType) readView(member)->type=std::make_shared<WordUse>(*writtenType);
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

// Conjunction preserves the compiled DAG. Cache ordered pairs only within
// this operation: left/right retain their spelling and provenance precedence.
using DefinitionPairs = std::map<std::pair<DefinitionSpec,DefinitionSpec>,DefinitionSpec>;
inline DefinitionSpec combineDefinitions(const DefinitionSpec& left, const DefinitionSpec& right, Work& work,
                                         DefinitionPairs& combined) {
    ++work.visits;
    auto [entry,inserted]=combined.emplace(std::make_pair(left,right),DefinitionSpec{});
    if(!inserted) {
        if(!entry->second) throw DefinitionPending{};
        return entry->second;
    }
    auto result = std::make_shared<DefinitionRequirement>(*left);
    result->independentTemporalValue |= right->independentTemporalValue;
    for(const auto& scope:right->selfScopes)
        if(std::find(result->selfScopes.begin(),result->selfScopes.end(),scope)==result->selfScopes.end()) result->selfScopes.push_back(scope);
    for(const auto& expression:right->expressions)
        if(std::none_of(result->expressions.begin(),result->expressions.end(),[&](const auto& old) {
            return old.source==expression.source && old.scope==expression.scope;
        })) result->expressions.push_back(expression);
    if (result->facts.pendingNumericValue(right->facts)) throw DefinitionPending{};
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
            for(const auto& pair:mapping.pairs) result->members[pair.first]=combineDefinitions(result->members[pair.first],right->members[pair.second],work,combined);
            for(auto member:mapping.missing) result->members.push_back(right->members[member]);
        } else {
            // Closed ends must agree; open bodies contribute only their known
            // prefix. Conjoin common positions before considering the tail.
            const auto leftCount=result->members.size(), rightCount=right->members.size();
            if ((!result->open && rightCount>leftCount) || (!right->open && leftCount>rightCount))
                throw DefinitionConflict("incompatible definition lengths");
            for (std::size_t i=0;i<std::min(leftCount,rightCount);++i)
                result->members[i]=combineDefinitions(result->members[i],right->members[i],work,combined);
            result->members.insert(result->members.end(),right->members.begin()+std::min(leftCount,rightCount),right->members.end());
            if (result->repeated && right->repeated)
                result->repeated=combineDefinitions(result->repeated,right->repeated,work,combined);
            else if (right->repeated) {
                result->repeated=right->repeated;result->templateSource=right->templateSource;
            }
            if(result->repeated) for(auto& member:result->members)
                member=combineDefinitions(member,result->repeated,work,combined);
            result->open=result->open && right->open;
            // Closing the extent does not remove the per-member constraint.
            // Retain it so a later finite prefix can fill an existing sparse instance.
        }
    } else if (right->list) {
        result->list = true; result->order=right->order; result->open = right->open; result->members = right->members;
        result->repeated = right->repeated; result->templateSource = right->templateSource;
    }
    if (result->list && !result->memberLayout().valid)
        throw DefinitionConflict("incompatible definition extent");
    entry->second=result;
    return result;
}

inline DefinitionSpec combineDefinitions(const DefinitionSpec& left, const DefinitionSpec& right, Work& work) {
    DefinitionPairs combined;
    return combineDefinitions(left,right,work,combined);
}

// Bind only expressions written in this declaration. A nested named meaning
// has already established its own scope and keeps that binding.
inline DefinitionSpec scopeDefinition(const DefinitionSpec& source,const POVNode& scope,Work& work,
                                      std::map<DefinitionSpec,DefinitionSpec>& scoped) {
    ++work.visits;
    auto [entry,inserted]=scoped.emplace(source,DefinitionSpec{});
    if(!inserted) {
        if(!entry->second) throw DefinitionPending{};
        return entry->second;
    }
    auto result=source;
    auto edit=[&] {if(result==source) result=std::make_shared<DefinitionRequirement>(*source);};
    for(std::size_t i=0;i<source->expressions.size();++i) if(!source->expressions[i].scope) {
        edit();result->expressions[i].scope=scope;
    }
    for(std::size_t i=0;i<source->members.size();++i) {
        auto member=scopeDefinition(source->members[i],scope,work,scoped);
        if(member!=source->members[i]) {edit();result->members[i]=member;}
    }
    if(source->repeated) {
        auto repeated=scopeDefinition(source->repeated,scope,work,scoped);
        if(repeated!=source->repeated) {edit();result->repeated=repeated;}
    }
    entry->second=result;
    return result;
}

inline DefinitionSpec scopeDefinition(const DefinitionSpec& source,const POVNode& scope,Work& work) {
    // An unchanged result proves that this subtree needs no rebinding in this
    // pass. Both unchanged and copied subtrees remain shared by all parents.
    std::map<DefinitionSpec,DefinitionSpec> scoped;
    return scopeDefinition(source,scope,work,scoped);
}

inline bool sameDefinition(const DefinitionSpec& left, const DefinitionSpec& right) {
    if(left==right) return true;
    // One node can be compared with several distinct counterparts. Deduplicate
    // pairs, preserving structural equality regardless of how sharing differs.
    using Pair=std::pair<DefinitionSpec,DefinitionSpec>;
    std::vector<Pair> pending{{left,right}};
    std::set<Pair> seen;
    while(!pending.empty()) {
        auto [a,b]=pending.back();pending.pop_back();
        if(a==b) continue;
        if(!a || !b) return false;
        if(!seen.insert({a,b}).second) continue;
        if(a->list != b->list || a->open != b->open || a->order!=b->order || !a->facts.same(b->facts) || !a->numeric.same(b->numeric) ||
           a->independentTemporalValue != b->independentTemporalValue ||
           a->types != b->types || a->expandedTypes != b->expandedTypes || a->members.size() != b->members.size() || a->choices.size()!=b->choices.size() || a->selfScopes!=b->selfScopes || a->expressions.size()!=b->expressions.size()) return false;
        for(std::size_t i=0;i<a->expressions.size();++i)
            if(a->expressions[i].source!=b->expressions[i].source || a->expressions[i].scope!=b->expressions[i].scope) return false;
        pending.push_back({a->repeated,b->repeated});
        for(std::size_t i=0;i<a->choices.size();++i) {
            const auto& x=a->choices[i];const auto& y=b->choices[i];
            if(x.source!=y.source || x.alternatives.size()!=y.alternatives.size()) return false;
            for(std::size_t j=0;j<x.alternatives.size();++j)
                pending.push_back({x.alternatives[j],y.alternatives[j]});
        }
        for(std::size_t i=0;i<a->members.size();++i)
            pending.push_back({a->members[i],b->members[i]});
    }
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
    enum class Check { active, pending, accepted, rejected };
    struct PlanningPass {
        std::map<std::pair<POVNode,DefinitionSpec>,Check> alternatives;
    };
    std::shared_ptr<PlanningPass> pass_;
    Check checkAlternative(const POVNode& target,const DefinitionSpec& requirement) {
        ++work.visits;
        if(!pass_) pass_=std::make_shared<PlanningPass>();
        auto [entry,inserted]=pass_->alternatives.emplace(std::make_pair(target,requirement),Check::active);
        if(!inserted) return entry->second;
        try {
            ProteusDefinitionPlan check(access_,target,requirement,pass_);
            work.visits=addCost(work.visits,check.work.visits);
            entry->second=check.dataSatisfied() ? Check::accepted : Check::pending;
        } catch(const DefinitionConflict&) {entry->second=Check::rejected;}
        return entry->second;
    }
    ProteusDefinitionPlan(std::shared_ptr<ProteusGraphAccess> access,const POVNode& target,
                          const DefinitionSpec& requirement,std::shared_ptr<PlanningPass> pass)
        :access_(std::move(access)),pass_(std::move(pass)) {
        prepare(place(target),requirement);
    }
    std::map<POVNode,Place> selfScopes_;
    bool pendingExpressions_=false;
    // Registry-changing operations retain the model-wide fallback until
    // their registry reads and writes have their own dependency keys.
    bool scopedWrites_=true;
    Changes writeFootprint() const {
        auto changes=Changes::only({});
        auto position=[&](const POVNode& node) {
            if(node) changes.include(ReadScope::changedObjects({POVDescription::positionKey(node)}));
        };
        auto destination=[&](const POVNode& node) {
            if(!node) return;
            auto view=readView(node);
            changes.include(ReadScope::changedObjects({node.get(),view.get(),view->infSize.get(),view->value.get()}));
            // Filling a gap can replace a tail link or insert before a mapped
            // member. Position-only readers need those existing links too.
            position(node);position(node->prev);position(node->next);
            position(view->value->items.head);position(view->value->items.tail);
        };
        // Layout places include embedded owners; bindings also include slots
        // whose POVs will be allocated by the plan itself.
        for(const auto& entry:places_) destination(entry.second->node);
        for(const auto& binding:bindings) destination(binding.first->node);
        return changes;
    }
    bool prepareExpressions(const Place& target,const DefinitionSpec& requirement) {
        bool direct=false;
        for(const auto& expression:requirement->expressions) {
            auto binding=selfScopes_.find(expression.scope);
            auto self=binding==selfScopes_.end() ? Place{} : binding->second;
            auto matches=[&](const POVNode& node) {
                // Path lowering retains the original expression and may move
                // result constraints into a choice predicate. Those are still
                // the installed RHS, not permission to add another copy.
                std::vector<std::shared_ptr<infonView>> pending{readView(node)};
                std::set<std::shared_ptr<infonView>> seen;
                while(!pending.empty()) {
                    auto infon=pending.back();pending.pop_back();
                    if(!infon || !seen.insert(infon).second) continue;
                    ++work.visits;
                    if(infon->copySource==readView(expression.source) && infon->thisPtr &&
                       (!self || (self->node && infon->thisPtr==readView(self->node)))) return true;
                    if(auto state=infon->pathSelection) {
                        pending.push_back(state->expression);
                        if(state->predicate) pending.push_back(readView(state->predicate));
                    }
                    for(const auto& rhs:infon->wrkList) pending.push_back(readView(rhs));
                }
                return false;
            };
            POVNode existing;
            if(target->node) {
                if(matches(target->node)) {existing=target->node;direct=true;}
                else for(const auto& relation:POVDescription(target->node,access_->registry.get()).relationships())
                    if(matches(relation.target)) {existing=relation.target;break;}
            }
            if(existing) {
                auto proof=readView(existing)->normalizationEvidence;
                pendingExpressions_ |= (proof && !proof->valid()) || !getFlagBit(readView(existing)->flags,infonView::isNormed);
                continue;
            }
            if(!self) {supported=false;return false;}
            // Copying an expression may bind named occurrences in the registry.
            scopedWrites_=false;
            auto access=access_;work.creations=addCost(work.creations,2);
            writes.push_back([access,target,self,expression] {
                auto rhs=std::make_shared<POV>(923);readView(rhs)=std::make_shared<infonView>();
                std::vector<infonView*> copied;
                readView(expression.source)->_copyAsTypeTo(readView(rhs),true,true,readView(self->node),&copied);
                readView(rhs)->copySource=readView(expression.source);
                SetModeBits(readView(rhs)->flags,infonView::copySourceType,infonView::cstDefinition);
                access->bind(rhs);readView(target->node)->addPOVToWrkList(rhs);
                SetFlagBit(readView(target->node)->flags,infonView::isNormed,false);++target->node->workRevision;
                return true;
            });
        }
        return direct;
    }
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
        if(requirement->order==uOrdered && readView(target->node)->value->items.size()==requirement->members.size()) {
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
        // An enclosing repetition constrains virtual members too. Conjoin its
        // template on this instance, never mutate a shared declaration or wait
        // until every virtual position has been enumerated.
        if(target->node && open && requirement->templateSource) {
            auto existing=access->memberTemplate(target->node);
            auto supplied=readView(requirement->templateSource);
            if(existing && existing!=supplied && !existing->hasRHS(supplied)) {
                auto view=std::make_shared<POV>(923);readView(view)=existing;
                auto facts=access->typeFacts(view);auto combined=facts;
                ++work.visits;
                if(facts.valid && !combined.combine(requirement->repeated->facts))
                    throw DefinitionConflict("incompatible repeated member template");
                auto sameWord=existing->type && supplied->type && existing->type->typeID()==supplied->type->typeID();
                auto sameLiteral=access->plain(view,false,false,false,false) && access->plain(requirement->templateSource,false,false,false,false) &&
                    existing->wrkList.empty() && supplied->wrkList.empty() && !access->literalList(view) &&
                    !access->literalList(requirement->templateSource) && facts.same(access->typeFacts(requirement->templateSource)) &&
                    existing->addedTypes.empty() && supplied->addedTypes.empty() &&
                    ((!existing->type && !supplied->type) || sameWord);
                if(!sameLiteral) {
                    if(supplied->identityName) scopedWrites_=false;
                    work.creations=addCost(work.creations,2);
                    writes.push_back([access,target,existing,supplied] {
                        auto combined=std::make_shared<infonView>(*existing);
                        auto rhs=std::make_shared<POV>(923);readView(rhs)=supplied;access->bind(rhs);
                        combined->addPOVToWrkList(rhs);readView(target->node)->value->listSpec=combined;
                        SetFlagBit(readView(target->node)->flags,infonView::isNormed,false);++target->node->workRevision;
                        return true;
                    });
                }
            }
        }
        std::vector<POVNode> members;
        if (!adopt) {
            if ((requirement->order==uOrdered && !access->definitionOrderedCandidate(target->node)) ||
                (!access->physicalList(target->node) && !access->templatePrefixList(target->node))) {
                supported = false; return;
            }
            bool embedded=false;
            for(auto item=access->first(target->node,false);item;item=access->next(item,false)) embedded|=access->subList(item);
            if(embedded && requirement->members.empty()) {
                SpanLayout<POVNode> structure;
                if(!readListLayout(*access,target->node,structure,work.visits,[&](const auto& at) {
                    return at.phase!=ListPhase::enter || ((access->physicalList(at.node) || access->templatePrefixList(at.node)) && access->definitionOrderedCandidate(at.node));
                },[](const POVNode& node){return node;}) || !structure.solve()) {supported=false;return;}
                auto repeated=std::make_shared<DefinitionRequirement>(*requirement);
                repeated->facts.hasSize=false;repeated->facts.explicitSize=false;
                repeated->label.clear();repeated->writtenType.reset();repeated->types.clear();repeated->expandedTypes.clear();
                for(const auto& segment:structure.segments) {
                    if(segment.gap && segment.node==target->node) continue;
                    prepare(place(segment.node),segment.gap ? repeated : requirement->repeated);
                }
                if(!requirement->facts.accepts(actual)) writes.push_back([access,target,requirement] {
                    return access->applyTypeFacts(target->node,requirement->facts);
                });
                if(!access->definitionOrderKnown(target->node)) {
                    ++orderWrites;writes.push_back([access,target]{return access->setDefinitionOrder(target->node);});
                }
                return;
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
        if(finalFacts.hasSize && finalFacts.size<static_cast<std::int64_t>(requirement->members.size()))
            throw DefinitionConflict("definition prefix exceeds instance extent");
        if(!open && members.size()<requirement->members.size())
            throw DefinitionConflict("definition prefix exceeds closed instance");
        // Only written positions are added. Reuse an already demanded POV
        // when its position becomes concrete; no new virtual tail is walked.
        std::set<POVNode> prepared;
        auto demanded=target->node ? access->demandedOccurrences(target->node) : std::vector<POVNode>{};
        for(std::size_t i=0;i<std::max(members.size(),requirement->members.size());++i) {
            auto spec=i<requirement->members.size() ? requirement->members[i] : requirement->repeated;
            Place member;
            if(i<members.size()) member=place(members[i]);
            else {
                POVNode existing;
                for(const auto& occurrence:demanded)
                    if(POVDescription(occurrence).occurrenceIndex()==static_cast<std::int64_t>(i)) {
                        // Distinct views need a joint identity plan, as in
                        // grouped correspondence; never discard one silently.
                        if(existing && existing!=occurrence) {supported=false;return;}
                        existing=occurrence;
                    }
                member=existing ? place(existing) : std::make_shared<Slot>();
                if(existing) scopedWrites_=false; // repetition materialization updates registry identity
                work.creations=addCost(work.creations,existing ? 0 : 2);
                writes.push_back([access,target,member,spec,existing] {
                    member->node=existing ? access->appendOccurrence(target->node,existing) : spec->appendMember(*access,target->node);
                    return true;
                });
            }
            if(member->node) prepared.insert(member->node);
            prepare(member,spec);
        }
        for(const auto& member:demanded) if(!prepared.count(member)) {
            auto index=POVDescription(member).occurrenceIndex();
            prepare(place(member),index>=0 && static_cast<std::size_t>(index)<requirement->members.size() ?
                requirement->members[index] : requirement->repeated);
        }
        auto count=static_cast<std::int64_t>(std::max(members.size(),requirement->members.size()));
        if(open && finalFacts.hasSize && finalFacts.size==count) writes.push_back([access,target,count] {
            return access->finishTail(target->node,true,count);
        });
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
               (readView(target->node)->orderMode!=uUnknown && readView(target->node)->orderMode!=requirement->order)) {supported=false;return;}
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
            SetFlagBit(readView(target->node)->value->flags,infonState::tailUnfinished,false);
            return changed;
        });
        if(!adopt && readView(target->node)->orderMode==uUnknown) writes.push_back([access,target,requirement] {
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
        struct RestoreScopes {
            std::map<POVNode,Place>& scopes;
            std::map<POVNode,Place> old;
            ~RestoreScopes(){scopes=std::move(old);}
        } restore{selfScopes_,selfScopes_};
        for(const auto& scope:requirement->selfScopes) selfScopes_[scope]=target;
        if(requirement->independentTemporalValue &&
           (!target->node || !POVDescription(target->node).independentTemporalValue())) {
            writes.push_back([target] {
                SetFlagBit(readView(target->node)->flags,infonView::independentTemporalValue,true);
                SetFlagBit(readView(target->node)->flags,infonView::isNormed,false);
                ++target->node->workRevision;return true;
            });
        }
        bool directExpression=prepareExpressions(target,requirement);
        if(!supported) return;
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
                auto previous=proposedNumbers_.find(readView(target->node));
                if(previous!=proposedNumbers_.end()) number.bounds=previous->second;
            }
            // A bound expression may now be an unresolved choice. Keep its
            // complete requirement pending, while independent sibling facts
            // can still be published. This never certifies the symbolic value.
            if(!actual.valid && directExpression) {
                pendingExpressions_=true;
                bindings.push_back({target,sourceRequirement});
                return;
            }
            if (!actual.valid) { supported = false; return; }
            if(!actual.explicitSize && ((requirement->list && requirement->order==uUnordered && readView(target->node)->orderMode==uUnknown) ||
                                       (fillDescription(target,requirement) && requirement->facts.hasSize && actual.hasSize && actual.size!=requirement->facts.size))) actual.hasSize=false;
            auto found = proposed_.find(readView(target->node));
            if (found != proposed_.end()) actual = found->second;
        }
        // A choice constrains the result, but does not authorize choosing an
        // arbitrary value. An accepted correspondence can already satisfy one
        // branch; otherwise leave the alternatives available to graph search.
        for(const auto& choice:requirement->choices) {
            bool accepted=false,pending=!target->node;
            if(target->node) for(const auto& alternative:choice.alternatives) {
                auto checked=checkAlternative(target->node,alternative);
                if(checked==Check::accepted) {accepted=true;break;}
                pending |= checked!=Check::rejected;
            }
            if(!accepted && !pending) throw DefinitionConflict("no compatible definition alternative");
            if(!accepted) {supported=false;return;}
        }
        auto combined = actual;
        if (combined.pendingNumericValue(requirement->facts)) { supported=false; return; }
        if (!combined.combine(requirement->facts))
            throw DefinitionConflict("incompatible definition requirements");
        if (target->node) proposed_[readView(target->node)] = combined;
        // Cache the full meaning, not the subset demanded by this application.
        // A later explicit normalization of this child must still see its parts.
        bindings.push_back({target, sourceRequirement});
        if (!requirement->list) {
            if(requirement->numeric.valid) {
                NumericView result;result.facts=combined;result.expression=true;
                result.bounds=number.bounds.valid ? number.bounds.intersect(requirement->numeric) : requirement->numeric;
                if(result.bounds.empty) throw DefinitionConflict("incompatible numeric value requirements");
                if(target->node) proposedNumbers_[readView(target->node)]=result.bounds;
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
            auto infon = readView(target->node);
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
        // Retain commands and bindings, never a history of attempted POVs.
        // A later construction reads current values and completion evidence.
        pass_.reset();
    }
    // Deferred allocation clients can prepare a member before the priced Set
    // creates its POV. All other slots and writes remain instance-local.
    void bindTarget(const POVNode& node) { place({})->node = node; }
    bool satisfied() const { return supported && writes.empty() && !pendingExpressions_; }
    bool dataSatisfied() const { return supported && writes.size() == orderWrites && !pendingExpressions_; }
    SetResult applyChanges() {
        if(!supported || writes.empty()) return false;
        if(!scopedWrites_) return apply();
        auto changes=writeFootprint();
        bool changed=apply();
        if(changed) changes.include(writeFootprint());
        return {changed,changed ? changes : Changes::only({})};
    }
    bool apply() {
        if (!supported) return false;
        bool changed = false;
        for (const auto& write : writes) changed |= write();
        return changed;
    }
};
