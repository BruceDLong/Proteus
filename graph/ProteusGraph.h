#pragma once
#include "ListStructure.h"
#include "NumericRange.h"
#include <set>

// Included after the CodeDog declarations. Domain knowledge is implemented in
// ProteusGraphAccess (GraphFoundation.Lib.dog); this layer adapts its results to
// generic edges and GoalSpec-generated commands.
namespace proteus4 {
using POVNode = std::shared_ptr<POV>;
#include "POVDescription.h"
#include "NumericRangeAccess.h"
enum Demand { itemFound = 0, sizeOnly = 1, valueOnly = 2, sizeAndValue = 3,
              itemAt = 4, physicalItemAt = 5,
              listContents = 6, listMember = 7, listEnd = 8,
              listCorrespondence = 9, correspondenceEnter = 10, correspondenceEnd = 11,
              correspondenceMember = 12, correspondenceLeave = 13,
              concatField = 14, concatMember = 15, concatEnd = 16,
              selectIntersection = 17, selectionMember = 18, selectionFound = 19,
              selectionExtentMember = 20, selectionExtentEnd = 21,
              typeRequirements = 22, definitionEnter = 23, definitionLeave = 24, definitionEnd = 25, assessRepresentation = 26, groupedList = 27,
              selectRelative = 28, relativeItem = 29, relativeMember = 30, relativeBoundary = 31,
              sequenceCorrespondence = 32, sequenceEnter = 33, sequenceBoundary = 34, sequenceMatched = 35, sequenceComplete = 36, relationshipConstraints = 37, correspondenceInput = 38, sequenceAtomMatched = 39, compileTypeRequirements = 40, selectField = 41, fieldPosition = 42, pathMatched = 43, pathUnknown = 44, pathEnd = 45, pathAlternative = 46, pathAlternativesEnd = 47, pathAlternativeUnknown = 48, selectPosition = 49, pathIndexStep = 50, positionStart = 51, pathSliceStart = 52, pathSliceCandidate = 53, selectSelf = 54, pathSelfFound = 55, filterChoice = 56, choicePosition = 57, choiceMatched = 58, choiceUnknown = 59, choiceEnd = 60, choicePending = 61, projectField = 62, numericCorrespondence = 63 };

inline ListPosition<POVNode> listPosition(const Query<POVNode>& query) {
    auto phase = query.kind == correspondenceEnter ? ListPhase::enter :
                 query.kind == correspondenceMember ? ListPhase::member : ListPhase::leave;
    return {query.node, phase, query.context};
}

#include "DefinitionApplications.h"
#include "RHSRepresentation.h"
#include "ScalarPredicate.h"
#include "FieldPredicate.h"
#include "PathSelection.h"
#include "DefinitionProjection.h"
#include "IntersectionInput.h"
#include "GroupedListPlan.h"

// Early correspondence is conditional on the parser retaining the ordered
// list body. Weak provenance witnesses neither copy nodes nor own a second
// graph. A later comma/postfix reinterpretation fails dependent searches;
// partial Sets remain inspectable, as with other failed constraints.
class ProteusStreamEvidence {
    struct Guard {
        std::weak_ptr<POV> source;
        std::weak_ptr<infonState> body;
    };
    using Key = std::weak_ptr<infonView>;
    std::map<Key, std::vector<Guard>, std::owner_less<Key>> guards_;
    std::shared_ptr<ProteusGraphAccess> access_;
    bool add(const POVNode& target, const Guard& guard) {
        auto& list = guards_[target->pItem];
        for (const auto& existing : list)
            if (!existing.source.owner_before(guard.source) && !guard.source.owner_before(existing.source))
                return false;
        list.push_back(guard); return true;
    }
public:
    explicit ProteusStreamEvidence(std::shared_ptr<ProteusGraphAccess> access) : access_(std::move(access)) {}
    bool inherit(const POVNode& target, const POVNode& source) {
        bool changed = false;
        if (access_->prefixList(source)) changed |= add(target, {source, source->pItem->value});
        auto found = guards_.find(source->pItem);
        if (found != guards_.end()) {
            auto guards = found->second;
            for (const auto& guard : guards) changed |= add(target, guard);
        }
        return changed;
    }
    void prune() {
        for (auto it = guards_.begin(); it != guards_.end();)
            if (it->first.expired()) it = guards_.erase(it);
            else ++it;
    }
    bool pending(const POVNode& node) {
        auto found = guards_.find(node->pItem);
        if (found == guards_.end()) return false;
        auto& guards = found->second;
        for (auto it = guards.begin(); it != guards.end();) {
            auto source = it->source.lock(); auto body = it->body.lock();
            if (!source || !body || source->pItem->value != body ||
                !access_->spanList(source) || !access_->orderedList(source))
                throw std::runtime_error("stream prefix assumption changed: ordered list body required");
            auto error = access_->inputError(source);
            if (!error.empty()) throw std::runtime_error(error);
            if (!access_->inputPending(source)) it = guards.erase(it);
            else ++it;
        }
        bool pending = !guards.empty();
        if (!pending) guards_.erase(found);
        return pending;
    }
};

// Completion flags are derived metadata. Keep weak witnesses independently
// of handles so later information invalidates even a released search result.
// Context-wide invalidation matches the foundation's existing revision policy.
class ProteusCompletion {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::vector<std::weak_ptr<POV>> marked_;
    std::vector<std::weak_ptr<GraphRHSProof>> represented_;
    struct Reply { std::uint64_t origin, order, position; POVNode node; };
    std::vector<Reply> effects_;
public:
    explicit ProteusCompletion(std::shared_ptr<ProteusGraphAccess> access)
        : access_(std::move(access)) {}
    void mark(const POVNode& node) {
        marked_.push_back(node);
        access_->setNormed(node, true);
    }
    void represented(const POVNode& node, const POVNode& rhs) {
        if (node->pItem->workIsRepresented(rhs)) return;
        auto& records = node->pItem->representedWork;
        records.erase(std::remove_if(records.begin(), records.end(),
            [](const auto& record) { return !record->valid; }), records.end());
        auto proof = std::make_shared<GraphRHSProof>(); proof->rhs = rhs;
        records.push_back(proof); represented_.push_back(proof);
    }
    void representedTemplate(const POVNode& node, const POVNode& source) {
        auto proof = std::make_shared<GraphRHSProof>(); proof->rhs = source;
        node->pItem->representedTemplate = proof; represented_.push_back(proof);
    }
    void reply(const POVNode& node) {
        if (!access_->replyReady(node)) return;
        auto origin = access_->replyOrigin(node);
        access_->claimReply(node);
        if (origin) effects_.push_back({origin, access_->replyOrder(node), access_->replyPosition(node), node});
    }
    std::vector<std::function<void()>> takeEffects() {
        // Claim all settled markers before rendering any containing list. Then
        // deliver in marker order, independently of goal completion order.
        std::stable_sort(effects_.begin(), effects_.end(), [](const Reply& a, const Reply& b) {
            return std::tie(a.origin, a.order) < std::tie(b.origin, b.order);
        });
        std::vector<std::function<void()>> result;
        auto core = access_->replyCore;
        for (const auto& reply : effects_) {
            auto origin = reply.origin, position = reply.position;
            auto message = access_->replyText(reply.node);
            result.push_back([core, origin, position, message] {
                core->submitProteusResult(origin, position, message);
            });
        }
        effects_.clear(); return result;
    }
    void invalidate() {
        for (auto& weak : marked_)
            if (auto node = weak.lock()) access_->setNormed(node, false);
        marked_.clear();
        for (const auto& weak : represented_)
            if (auto proof = weak.lock()) proof->valid = false;
        represented_.clear();
    }
};

// Partial representation supplies printer metadata. A closed strict equality
// can instead leave pending work through a priced Set that retains its identity.
class ProteusRepresentationGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusCompletion> completion_;
    bool checked_ = false, represented_ = false;
public:
    ProteusRepresentationGoal(Query<POVNode> query, std::shared_ptr<ProteusGraphAccess> access,
                              std::shared_ptr<ProteusCompletion> completion)
        : GoalSpec<POVNode>(query), access_(std::move(access)), completion_(std::move(completion)) {}
    void beginPass() override { checked_ = false; represented_ = false; }
    std::vector<SetCommand> found(const Visit<POVNode>&) override {
        RHSRepresentation plan(access_, request.node, request.related);
        if(plan.complete && request.node->pItem->workListContainsPOV(request.related)) {
            auto access=access_;auto owner=request.node,source=request.related;
            ++plan.work.creations;
            return {{"Consolidate RHS equality",[this,access,owner,source] {
                access->bind(owner);access->bind(source);
                auto relation=access->registry->recordIdentity(owner,source,iekAssertion,0);
                if(relation) {
                    std::lock_guard<std::mutex> lock(access->registry->registryMutex);
                    auto& storedOwner=relation->leftPOV==owner ? relation->leftWorkOwner : relation->rightWorkOwner;
                    // Reusing a POV after replacing its view must not overwrite
                    // a constraint still owned by the earlier shared view.
                    if(storedOwner && storedOwner!=owner->pItem) {
                        checked_=represented_=true;return false;
                    }
                    storedOwner=owner->pItem;
                }
                owner->pItem->discardWrkListPOV(source);
                ++owner->workRevision;SetFlagBit(owner->pItem->flags,infonView::isNormed,false);
                return true;
            },plan.work}};
        }
        return {{"Check RHS representation", [this, represented = plan.represented] {
            checked_ = true; represented_ = represented;
            return false; // proof metadata only: no semantic revision
        }, plan.work}};
    }
    bool satisfied() const override { return checked_; }
    bool symbolic() const override { return !checked_; }
    void settled(bool fullyResolved) override {
        if (fullyResolved && represented_) completion_->represented(request.node, request.related);
    }
};

class ProteusGroupedListGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::shared_ptr<ProteusCompletion> completion_;
    bool verified_ = false, represented_ = false;
    POVNode template_;
public:
    ProteusGroupedListGoal(Query<POVNode> query, std::shared_ptr<ProteusGraphAccess> access,
                          std::shared_ptr<ProteusDefinitions> definitions,
                          std::shared_ptr<ProteusCompletion> completion)
        : GoalSpec<POVNode>(query), access_(std::move(access)), definitions_(std::move(definitions)),
          completion_(std::move(completion)) {}
    void beginPass() override { verified_ = represented_ = false; template_.reset(); }
    std::vector<SetCommand> found(const Visit<POVNode>&) override {
        auto proof = definitions_->saved(request.node);
        if (!proof) return {};
        auto plan = std::make_shared<GroupedListPlan>(access_, request.node, request.related, proof->requirement);
        if (!plan->supported) return {};
        verified_ = plan->satisfied(); represented_ = plan->represented;
        template_ = proof->requirement->templateSource;
        return {{"Set grouped list correspondence", [plan] { return plan->apply(); }, plan->work}};
    }
    bool satisfied() const override { return verified_; }
    bool symbolic() const override { return !verified_; }
    void settled(bool fullyResolved) override {
        if (fullyResolved && template_) completion_->representedTemplate(request.node, template_);
        if (fullyResolved && represented_) completion_->represented(request.node, request.related);
    }
};

#include "NumericRelation.h"
#include "SequenceCorrespondence.h"

class ProteusGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusCompletion> completion_;
    std::shared_ptr<ProteusStreamEvidence> evidence_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    POVNode selected_;
    bool deferred_ = false, reachedEnd_ = false;
    GraphListMeasure measure_;
public:
    ProteusGoal(Query<POVNode> query, std::shared_ptr<ProteusGraphAccess> access,
                std::shared_ptr<ProteusCompletion> completion, std::shared_ptr<ProteusStreamEvidence> evidence,
                std::shared_ptr<ProteusDefinitions> definitions)
        : GoalSpec<POVNode>(query), access_(std::move(access)),
          completion_(std::move(completion)), evidence_(std::move(evidence)), definitions_(std::move(definitions)) {}
    void beginPass() override {
        deferred_ = false;
        reachedEnd_ = false;
        measure_.reset();
        if (request.kind == sizeAndValue) access_->setNormed(request.node, false);
    }
    void settled(bool fullyResolved) override {
        if (request.kind == sizeAndValue && fullyResolved && satisfied())
            completion_->mark(request.node);
        if (request.kind == sizeAndValue) completion_->reply(request.node);
    }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        access_->bind(visit.query.node);
        auto error = access_->inputError(visit.query.node);
        if (!error.empty()) throw std::runtime_error(error);
        if (access_->inputPending(visit.query.node) && request.kind != listContents &&
            !access_->prefixList(visit.query.node)) return {};
        if (request.kind >= sizeOnly && request.kind <= sizeAndValue &&
            visit.query.node == request.node && PathExpression::ready(*access_,request.node)) {
            auto access=access_;auto node=request.node;
            return {{"Prepare path selection",[access,node] {return PathExpression::prepare(*access,node);},PathExpression::work(*access,node)}};
        }
        if (request.kind >= sizeOnly && request.kind <= sizeAndValue &&
            visit.query.node == request.node && access_->hasInnerSource(request.node)) {
            auto access = access_; auto owner = request.node;auto definitions=definitions_;
            return {{"Set inner source constraint", [access, owner, definitions] {
                bool changed=access->applyInnerSource(owner);
                if(auto body=definitions->selectionBody(owner)) changed=IntersectionInput::prepare(*access,body) || changed;
                return changed;
            }, {1, 14}}};
        }
        if (request.kind >= sizeOnly && request.kind <= sizeAndValue &&
            visit.query.node==request.node && IntersectionInput::ready(*access_,request.node)) {
            auto access=access_;auto owner=request.node;
            return {{"Prepare end-relative input window",[access,owner] {return IntersectionInput::prepare(*access,owner);},
                {owner->pItem->wrkList.size(),12*owner->pItem->wrkList.size()}}};
        }
        if ((request.kind==relationshipConstraints || (request.kind>=sizeOnly && request.kind<=sizeAndValue)) && visit.query.node==request.node) {
            auto constraints=PathExpression::choiceConstraints(request.node);
            if(!constraints.empty()) {
                auto access=access_;auto node=request.node;
                return {{"Set choice constraints",[access,node,constraints] {return PathExpression::constrainChoice(*access,node,constraints);},{constraints.size(),0}}};
            }
            if(auto state=request.node->pItem->pathSelection;state && PathExpression::needsUpdates(state)) {
                auto access=access_;
                return {{"Set selected source constraints",[access,state] {return PathExpression::applyUpdates(*access,state);},{state->updates.size(),0}}};
            }
        }
        if (request.kind != listContents && access_->isDefinition(visit.query.node)) {
            if (visit.query.node != request.node || request.kind != sizeAndValue ||
                !access_->definitionReady(request.node) || definitions_->registered(request.node)) return {};
            auto definitions = definitions_; auto node = request.node;
            return {{"Register definition", [definitions, node] { return definitions->publish(node); }, {0, 1}}};
        }
        if (request.kind == listContents) {
            if (visit.query.kind == listMember) measure_.add(visit.query.node);
            if (visit.query.kind != listEnd) return {};
            reachedEnd_ = true;
            auto access = access_;
            auto target = request.node;
            auto measure = measure_;
            return {{"Set counted list size", [access, target, measure] {
                return access->applyCountedSize(target, measure);
            }}};
        }
        if (request.kind == itemAt) {
            if (visit.query.kind == itemFound && !selected_) {
                auto selected = visit.query.node;
                return {{"Set selected POV", [this, selected] {
                    selected_ = selected; return true;
                }}};
            }
            return {};
        }
        // Local scalar normalization precedes comparison, just as Proteus3's
        // rangeRules does. Canonicalization must publish a revision even when
        // only the rational representation changes.
        auto numeric = access_->planNumericLiteral(visit.query.node);
        if (numeric.changed) {
            auto access = access_; auto node = visit.query.node;
            return {{"Set normalized numeric literal", [access, node, numeric] {
                return access->applyNumericLiteral(node, numeric);
            }}};
        }
        auto source = visit.query.node;
        auto target = request.node;
        int demand = request.kind;
        if (source == target || demand < sizeOnly || demand > sizeAndValue) return {};
        // List values are related by member correspondence, not a shallow
        // overlay copy. That goal also validates shape before copying size.
        if (demand != sizeOnly && access_->literalList(target)) {
            if (!definitions_->canInherit(target, source)) return {};
            auto definitions = definitions_;
            return {{"Reuse checked type requirements", [definitions, target, source] {
                return definitions->inherit(target, source);
            }}};
        }
        // Work is conjunctive. A route is evidence only if each intermediate
        // constraint agrees with the finding. It is not an alternative branch.
        for (const auto& step : visit.path) {
            int verdict = access_->compatible(step.node, source, demand);
            if (verdict < 0) throw std::runtime_error("incompatible scalar constraints");
            if (!verdict) { deferred_ = true; return {}; }
        }
        auto access = access_;
        auto evidence = evidence_; auto definitions = definitions_;
        return {{"Set requested size/value/type", [access, evidence, definitions, target, source, demand] {
            bool changed = access->apply(target, source, demand);
            if (changed) evidence->inherit(target, source);
            return definitions->inherit(target, source) || changed;
        }}};
    }
    bool satisfied() const override {
        if (request.kind == relationshipConstraints) return true;
        if (request.kind == sizeAndValue && access_->isDefinition(request.node))
            return definitions_->registered(request.node);
        if (request.kind == listContents)
            return reachedEnd_ && access_->listExtentAgrees(request.node, measure_);
        if (request.kind == itemAt) return bool(selected_);
        return (access_->known(request.node) & request.kind) == request.kind;
    }
    bool symbolic() const override { return deferred_ || !satisfied(); }
    POVNode result() const override {
        return request.kind == itemAt ? selected_ : request.node;
    }
};

// The graph walk collects only loaded positions and symbolic gaps. The pure
// layout planner derives anchored matches and finite tail fills; a priced Set
// publishes that plan before the search can settle.
class ProteusCorrespondenceGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusStreamEvidence> evidence_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    ListLayoutBuilder<POVNode> layouts_[2];
    bool verified_ = false;
public:
    ProteusCorrespondenceGoal(Query<POVNode> query, std::shared_ptr<ProteusGraphAccess> access,
                             std::shared_ptr<ProteusStreamEvidence> evidence, std::shared_ptr<ProteusDefinitions> definitions)
        : GoalSpec<POVNode>(query), access_(std::move(access)), evidence_(std::move(evidence)), definitions_(std::move(definitions)) {}
    void beginPass() override {
        layouts_[0] = {}; layouts_[1] = {};
        verified_ = false;
    }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        const auto& q = visit.query;
        if (q.kind == listCorrespondence) return {};
        auto side = static_cast<std::size_t>(q.argument);
        if (side > 1) return {};
        if (q.kind != correspondenceEnd)
            layouts_[side].record(listPosition(q), *access_, [](const POVNode& member) { return member; });
        if (q.kind != correspondenceEnd) return {};
        auto plan = alignSpans(layouts_[0].layout, layouts_[1].layout);
        if (!plan.valid) return {};
        // Alignment is symmetric, but input ownership is not. Keep useful
        // matches while withholding every fill/closure of a parser-owned tail.
        plan.fills.erase(std::remove_if(plan.fills.begin(), plan.fills.end(), [&](const auto& fill) {
            if (!access_->inputPending(fill.owner)) return false;
            plan.complete = false; return true;
        }), plan.fills.end());
        // All mutation preconditions are checked before the batch is offered.
        // Unsupported new member semantics keep the entire plan symbolic.
        for (std::size_t i = 0; i < plan.fills.size(); ++i) {
            const auto& fill = plan.fills[i];
            // A finite RHS can close a pristine compressed span. Partial prefix
            // fills and remapping existing implicit occurrences need a richer
            // correspondence plan; neither may silently lose virtual edges.
            if (access_->memberTemplate(fill.owner) && !fill.close) return {};
            for (const auto& source : fill.sources)
                if (!access_->canMakeMember(source)) return {};
            for (std::size_t j = 0; j < i; ++j)
                if (access_->sameInfon(fill.owner, plan.fills[j].owner)) return {};
        }
        // Splices preserve unknown inner spans as embedded identity views.
        // Their source nodes denote complete spans, not tails of loaded owners.
        for(const auto& splice:plan.splices) {
            for(const auto& fill:plan.fills)
                if(access_->sameInfon(splice.owner,fill.owner)) return {};
            if(access_->inputPending(splice.owner) || access_->memberTemplate(splice.owner)) return {};
            for(const auto& source:splice.sources)
                if(!access_->canMakeMember(source.node) || access_->inputPending(source.node) ||
                   access_->sameInfon(splice.owner,source.node) ||
                   (source.gap && access_->memberTemplate(source.node))) return {};
        }
        Work work{0, 0};
        plan.crossings.erase(std::remove_if(plan.crossings.begin(),plan.crossings.end(),[&](const auto& part) {
            for(const auto& owner:{part.left,part.right}) {
                if(access_->inputPending(owner) || access_->memberTemplate(owner)) return true;
                for(const auto& fill:plan.fills) if(access_->sameInfon(owner,fill.owner)) return true;
            }
            if(access_->sameInfon(part.left,part.right)) return true;
            for(const auto& group:{part.prefix,part.suffix}) for(const auto& source:group)
                if(!access_->canMakeMember(source) || access_->inputPending(source) ||
                   access_->sameInfon(source,part.left) || access_->sameInfon(source,part.right)) return true;
            return !crossingHasMiddle(part,[&](const POVNode& a,const POVNode& b) {
                ++work.visits;auto left=access_->typeFacts(a),right=access_->typeFacts(b);
                return left.valid && right.valid && !left.combine(right);
            });
        }),plan.crossings.end());
        verified_ = plan.complete && !access_->inputPending(request.node) && !access_->inputPending(request.related);
        for(const auto& part:plan.crossings) work.creations+=2*(part.prefix.size()+part.suffix.size()+3);
        for (const auto& pair : plan.identities)
            if (!access_->hasDirectIdentity(pair.first, pair.second)) ++work.creations;
        for (const auto& fill : plan.fills) work.creations += 2 * fill.sources.size();
        for(const auto& splice:plan.splices) work.creations+=2*splice.sources.size();
        auto access = access_;
        auto left = request.node, right = request.related;
        int demand = static_cast<int>(request.argument);
        auto evidence = evidence_; auto definitions = definitions_;
        return {{"Set span alignment", [access, evidence, definitions, left, right, plan, demand] {
            access->bind(left); access->bind(right);
            for (const auto& pair : plan.identities) { access->bind(pair.first); access->bind(pair.second); }
            for (const auto& fill : plan.fills) {
                access->bind(fill.owner);
                for (const auto& source : fill.sources) access->bind(source);
            }
            for(const auto& splice:plan.splices) {
                access->bind(splice.owner);
                for(const auto& source:splice.sources) access->bind(source.node);
            }
            for(const auto& part:plan.crossings) {
                access->bind(part.left);access->bind(part.right);
                for(const auto& group:{part.prefix,part.suffix}) for(const auto& source:group) access->bind(source);
            }
            bool changed = access->apply(left, right, demand);
            if (changed) evidence->inherit(left, right);
            auto inherit = [&](const POVNode& member) {
                bool added = evidence->inherit(member, left);
                return evidence->inherit(member, right) || added;
            };
            for (const auto& pair : plan.identities) {
                changed |= inherit(pair.first); changed |= inherit(pair.second);
                if (!access->hasDirectIdentity(pair.first, pair.second)) {
                    access->registry->recordIdentity(pair.first, pair.second, iekListCorrespondence, 0);
                    changed = true;
                }
            }
            for (const auto& fill : plan.fills) {
                for (const auto& source : fill.sources) {
                    auto member = access->appendTailMember(fill.owner, source);
                    inherit(member); evidence->inherit(member, source); changed = true;
                }
                changed |= access->finishTail(fill.owner, fill.close, fill.extent);
            }
            for(const auto& splice:plan.splices) {
                for(const auto& source:splice.sources) {
                    auto member=access->appendTailMember(splice.owner,source.node);
                    SetFlagBit(member->flags,POV::isSubItm,source.gap);
                    inherit(member);evidence->inherit(member,source.node);changed=true;
                }
                changed=access->finishTail(splice.owner,true,-1) || changed;
            }
            for(const auto& part:plan.crossings) {
                auto middle=std::make_shared<POV>(904);middle->pItem=std::make_shared<infonView>();
                auto item=middle->pItem;
                SetModeBits(item->flags,infonView::viewMode,infonView::vmOverlay);
                SetModeBits(item->flags,infonView::itmMode,infonView::mItem);
                SetModeBits(item->value->flags,infonState::overlayType,infonState::LST);
                SetModeBits(item->value->flags,infonState::evalMode,infonState::emLiteral);
                SetFlagBit(item->value->flags,infonState::tailUnfinished,part.middle!=0);
                SetModeBits(item->infSize->flags,infonState::overlayType,infonState::NUM);
                SetModeBits(item->infSize->flags,infonState::evalMode,(part.middle<0 ? infonState::emUnknown : infonState::emLiteral));
                if(part.middle>=0) {item->infSize->num=part.middle;item->value->sizeMode=fromGiven;}
                access->bind(middle);
                auto append=[&](const POVNode& owner,const POVNode& source,bool embedded) {
                    auto member=access->appendTailMember(owner,source);SetFlagBit(member->flags,POV::isSubItm,embedded);
                    inherit(member);evidence->inherit(member,source);
                };
                append(part.left,middle,true);
                for(const auto& source:part.suffix) append(part.left,source,false);
                for(const auto& source:part.prefix) append(part.right,source,false);
                append(part.right,middle,true);
                access->finishTail(part.left,true,-1);access->finishTail(part.right,true,-1);changed=true;
            }
            return definitions->inherit(left, right) || changed;
        }, work}};
    }
    bool satisfied() const override { return verified_; }
    bool symbolic() const override { return !verified_; }
};

// A field expression walks its existing operands. The same goal handles the
// value slot and the size slot, without creating a proxy POV for either field.
class ProteusConcatGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    GraphConcatTerms terms_;
    bool verified_ = false;
public:
    ProteusConcatGoal(Query<POVNode> query, std::shared_ptr<ProteusGraphAccess> access)
        : GoalSpec<POVNode>(query), access_(std::move(access)) {}
    void beginPass() override { terms_.items.clear(); verified_ = false; }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        if (visit.query.kind == concatMember) terms_.items.push_back(visit.query.node);
        if (visit.query.kind != concatEnd) return {};
        auto plan = access_->planConcat(request.node, request.argument, terms_);
        if (!plan.valid) return {};
        verified_ = true;
        auto access = access_; auto owner = request.node; auto slot = request.argument;
        auto terms = terms_;
        return {{"Set concatenation result", [access, owner, slot, terms] {
            return access->applyConcat(owner, slot, terms);
        }, {plan.visits, plan.creations}}};
    }
    bool satisfied() const override { return verified_; }
    bool symbolic() const override { return !verified_; }
};

class ProteusSelectionGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    bool verified_ = false;
    GraphListMeasure measure_;
public:
    ProteusSelectionGoal(Query<POVNode> query, std::shared_ptr<ProteusGraphAccess> access)
        : GoalSpec<POVNode>(query), access_(std::move(access)) {}
    void beginPass() override { verified_ = false; measure_.reset(); }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        if (visit.query.kind == selectionExtentMember) measure_.add(visit.query.node);
        const bool indexed = access_->selectionIndex(request.node) >= 0;
        if (visit.query.kind != (indexed ? itemFound : selectionFound)) return {};
        if (!indexed && !access_->selectionExtentAgrees(request.node, measure_)) return {};
        auto selected = visit.query.node;
        if (!access_->selectionAgrees(request.node, selected)) return {};
        verified_ = true;
        auto access = access_; auto owner = request.node;
        auto creation = access->hasDirectIdentity(owner, selected) ? 0u : 1u;
        return {{"Set selected result identity", [access, owner, selected] {
            return access->applySelection(owner, selected);
        }, {0, creation}}};
    }
    bool satisfied() const override { return verified_; }
    bool symbolic() const override { return !verified_; }
};

// Contextual reference evidence, not additional adjacency. Keep the syntax's
// consuming occurrence after lowering the cursor, including inverse identity
// visits. One shared cursor expression cannot silently acquire two contexts.
class ProteusRelativePositions {
    struct Binding { std::vector<POVNode> context; POVNode selected; std::weak_ptr<POV> expression; };
    using Key = std::weak_ptr<infonView>;
    std::map<Key, Binding, std::owner_less<Key>> bindings_;
public:
    std::vector<POVNode> context(const POVNode& node, const std::vector<POVNode>& proposed, IdentityRegistry* registry) const {
        auto found = bindings_.find(node->pItem);
        if (found == bindings_.end()) return proposed.empty() ? std::vector<POVNode>{node} : proposed;
        const auto& saved = found->second.context;
        // Result aliases carry the same view and an explicit correspondence
        // to its syntax occurrence. An identity visit supplies no new cursor
        // context; reuse that occurrence's binding, never the display list.
        if(proposed.empty() && node!=found->second.expression.lock())
            for(const auto& relation:POVDescription(node,registry).relationships(false))
                if(relation.identity && relation.identity->evidenceKind==iekListCorrespondence &&
                   relation.target==found->second.expression.lock() && relation.target->pItem==node->pItem) return saved;
        if (node != found->second.expression.lock() || (!proposed.empty() && proposed.front() != saved.front()) ||
            (proposed.size() > 1 && saved.size() > 1 && proposed != saved))
            throw std::runtime_error("relative cursor used in different occurrence contexts");
        return proposed.size() > saved.size() ? proposed : saved;
    }
    bool bind(const POVNode& node, const std::vector<POVNode>& context, const POVNode& selected) {
        auto found = bindings_.find(node->pItem);
        if (found != bindings_.end()) {
            if (found->second.context.front() != context.front() || found->second.selected != selected)
                throw std::runtime_error("relative position changed after binding");
            if (context.size() > found->second.context.size()) { found->second.context = context; return true; }
            return false;
        }
        bindings_.emplace(node->pItem, Binding{context, selected, node}); return true;
    }
};

class ProteusRelativeGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusRelativePositions> positions_;
    std::shared_ptr<ProteusStreamEvidence> evidence_;
    bool verified_ = false;
public:
    ProteusRelativeGoal(Query<POVNode> query, std::shared_ptr<ProteusGraphAccess> access,
                        std::shared_ptr<ProteusRelativePositions> positions,
                        std::shared_ptr<ProteusStreamEvidence> evidence)
        : GoalSpec<POVNode>(query), access_(std::move(access)), positions_(std::move(positions)), evidence_(std::move(evidence)) {}
    void beginPass() override { verified_ = false; }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        if (visit.query.kind != itemFound) return {};
        verified_ = true;
        auto access = access_; auto positions = positions_; auto evidence = evidence_;
        auto owner = request.node, selected = visit.query.node;
        auto context = request.context; context.insert(context.begin(), request.related);
        return {{"Set relative position identity", [access, positions, evidence, owner, context, selected] {
            bool changed = positions->bind(owner, context, selected);
            for (std::size_t i=1; i<context.size(); ++i) changed |= evidence->inherit(owner, context[i]);
            return access->applyRelative(owner, selected) || changed;
        }, {0, access->hasDirectIdentity(owner, selected) ? 0u : 1u}}};
    }
    bool satisfied() const override { return verified_; }
    bool symbolic() const override { return !verified_; }
};

class ProteusAccess : public GraphAccess<POVNode> {
    std::shared_ptr<ProteusRelativePositions> positions_ = std::make_shared<ProteusRelativePositions>();
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusCompletion> completion_;
    std::shared_ptr<ProteusStreamEvidence> evidence_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    static Edge<POVNode> direct(std::string label, Query<POVNode> query,
                                bool alternative = false, Cost remainingVisits = 0) {
        return {std::move(label), {1, 0}, alternative, [query] { return query; },
                {remainingVisits, 0}};
    }
    static Edge<POVNode> sequence(const POVNode& endpoint, Query<POVNode> member, Query<POVNode> end) {
        if (!endpoint) return direct("list boundary", std::move(end));
        member.node = endpoint;
        return direct("list position", std::move(member));
    }
public:
    explicit ProteusAccess(std::shared_ptr<ProteusGraphAccess> access)
        : access_(std::move(access)),
          completion_(std::make_shared<ProteusCompletion>(access_)),
          evidence_(std::make_shared<ProteusStreamEvidence>(access_)),
          definitions_(std::make_shared<ProteusDefinitions>(access_)) {}
    void invalidateCompletion() override { completion_->invalidate(); evidence_->prune(); definitions_->prune(); }
    std::vector<std::function<void()>> takeEffects() override { return completion_->takeEffects(); }
    std::shared_ptr<GoalSpec<POVNode>> goal(const Query<POVNode>& query) override {
        if (query.kind == numericCorrespondence)
            return std::make_shared<ProteusNumericGoal>(query,access_,completion_);
        if (query.kind == projectField)
            return std::make_shared<ProteusProjectionGoal>(query, access_, definitions_);
        if (query.kind == selectField || query.kind == selectPosition || query.kind == selectSelf || query.kind == filterChoice)
            return std::make_shared<ProteusPathGoal>(query, access_, definitions_);
        if (query.kind == sequenceCorrespondence)
            return std::make_shared<ProteusSequenceGoal>(query, access_, definitions_, completion_);
        if (query.kind == selectRelative)
            return std::make_shared<ProteusRelativeGoal>(query, access_, positions_, evidence_);
        if (query.kind == groupedList)
            return std::make_shared<ProteusGroupedListGoal>(query, access_, definitions_, completion_);
        if (query.kind == assessRepresentation)
            return std::make_shared<ProteusRepresentationGoal>(query, access_, completion_);
        if (query.kind == typeRequirements || query.kind == compileTypeRequirements)
            return std::make_shared<ProteusTypeGoal>(query, definitions_);
        if (query.kind == listCorrespondence)
            return std::make_shared<ProteusCorrespondenceGoal>(query, access_, evidence_, definitions_);
        if (query.kind == concatField)
            return std::make_shared<ProteusConcatGoal>(query, access_);
        if (query.kind == selectIntersection)
            return std::make_shared<ProteusSelectionGoal>(query, access_);
        return std::make_shared<ProteusGoal>(query, access_, completion_, evidence_, definitions_);
    }
    Expansion<POVNode> expand(const Query<POVNode>& query) override {
        if (query.kind >= selectField && query.kind <= choicePending)
            return PathSelection(access_, definitions_, [this](const Query<POVNode>& q) {return expandSemantic(q);}).expand(query);
        if ((query.kind >= sequenceCorrespondence && query.kind <= sequenceComplete) || query.kind == correspondenceInput || query.kind == sequenceAtomMatched)
            return SequenceCorrespondence(access_, definitions_).expand(query);
        bool pending = evidence_->pending(query.node);
        auto result = expandSemantic(query);
        result.deferred = result.deferred || pending;
        return result;
    }
private:
    void relativeStep(Expansion<POVNode>& result, const Query<POVNode>& query, ListPhase phase,
                      Cost inspection = 0) {
        if (phase == ListPhase::leave && query.node == query.related) { result.deferred = true; return; }
        auto at = ListPosition<POVNode>{query.node, phase, query.context};
        auto next = phase == ListPhase::enter ? nextListPosition(*access_, std::move(at), true, query.argument < 0) :
                                               adjacentListPosition(*access_, std::move(at), true, query.argument < 0);
        if (next.phase == ListPhase::done || next.phase == ListPhase::cycle) { result.deferred = true; return; }
        auto kind = next.phase == ListPhase::member ? relativeMember : relativeBoundary;
        auto edge = direct("logical neighbor step", {next.node, kind, query.argument, query.related, next.owners});
        edge.work.visits += inspection; result.edges.push_back(std::move(edge));
    }
    void boundaryNeeds(Expansion<POVNode>& result, const POVNode& node, const ListBoundary<POVNode>& boundary) {
        for (const auto& missing : boundary.missingExtents)
            result.refinements.push_back({{missing, sizeOnly, 0}, {boundary.visits, 0}, "find embedded extent", true});
        if (boundary.extent < 0)
            result.refinements.push_back({{node, sizeOnly, 0}, {boundary.visits, 0}, "find span extent", true});
    }
    Expansion<POVNode> expandSemantic(const Query<POVNode>& query) {
        Expansion<POVNode> result;
        auto node = query.node;
        POVDescription description(node, access_->registry.get());
        if (query.kind == assessRepresentation || query.kind == groupedList || query.kind == projectField || query.kind == numericCorrespondence) return result;
        if (query.kind == typeRequirements || query.kind == compileTypeRequirements) {
            if (definitions_->required(node) && !definitions_->saved(node))
                result.edges.push_back(direct("inspect type definitions", {node, definitionEnter, 0, node}));
            return result;
        }
        if (query.kind == definitionEnd) return result;
        if (query.kind == definitionEnter || query.kind == definitionLeave) {
            auto context = query.context;
            if (query.kind == definitionEnter) {
                for (const auto& ancestor : context) if (access_->sameInfon(ancestor, node)) {
                    result.deferred = true; return result;
                }
                std::vector<POVNode> children;
                if (!definitions_->children(node, query.related, children)) {
                    result.deferred = true; return result;
                }
                if (!children.empty()) {
                    context.push_back(node);
                    result.edges.push_back(direct("definition constraint", {children.front(), definitionEnter, 0, query.related, context}));
                } else result.edges.push_back(direct("definition boundary", {node, definitionLeave, 0, query.related, context}));
            } else if (context.empty()) {
                result.edges.push_back(direct("type requirements available", {query.related, definitionEnd, 0}));
            } else {
                auto parent = context.back(); context.pop_back();
                std::vector<POVNode> children;
                if (!definitions_->children(parent, query.related, children)) { result.deferred = true; return result; }
                auto current = std::find(children.begin(), children.end(), node);
                if (current != children.end() && ++current != children.end()) {
                    context.push_back(parent);
                    result.edges.push_back(direct("next definition constraint", {*current, definitionEnter, 0, query.related, context}));
                } else result.edges.push_back(direct("leave definition", {parent, definitionLeave, 0, query.related, context}));
            }
            return result;
        }
        if ((query.kind == itemAt || query.kind == physicalItemAt) && access_->inputPending(node)) {
            result.deferred = true; return result;
        }
        if (query.kind == itemFound || query.kind == listEnd || query.kind == correspondenceEnd || query.kind == concatEnd || query.kind == selectionFound)
            return result;
        if (query.kind == selectRelative) {
            if (!query.context.empty() && !access_->definitionOrderedCandidate(query.context.back())) {
                result.deferred = true; return result;
            }
            result.edges.push_back(direct("relative occurrence", {query.related, relativeItem, query.argument, query.context.empty() ? POVNode{} : query.context.front(), query.context}));
            return result;
        }
        if (query.kind == relativeItem) {
            if (auto owner = definitions_->retainedOccurrenceOwner(node)) {
                auto index = description.occurrenceIndex();
                auto extent = access_->listExtent(owner);
                if (index >= 0 && index < extent &&
                    (query.argument < 0 ? index > 0 : index < extent - 1))
                    result.edges.push_back(direct("relative sparse coordinate", {owner, itemAt, index + query.argument}));
            } else relativeStep(result, query, ListPhase::leave);
            return result;
        }
        if (query.kind == relativeMember || query.kind == relativeBoundary) {
            if (query.kind == relativeMember && !access_->subList(node)) {
                result.edges.push_back(direct("adjacent occurrence", {node, itemFound, 0})); return result;
            }
            if (!access_->navigationList(node)) {
                result.refinements.push_back({{node, valueOnly, 0}, {1, 0}, "resolve navigation span", true});
                result.deferred = true; return result;
            }
            // The known beginning of a span and a closed end need no extent
            // scan. Open ends require evidence about the remaining positions.
            if (!access_->openTail(node) || (query.kind == relativeMember && query.argument > 0 && description.member()) ||
                (query.kind == relativeBoundary && query.argument < 0)) {
                relativeStep(result, query, query.kind == relativeMember ? ListPhase::enter : ListPhase::leave);
                return result;
            }
            auto boundary = listBoundary(*access_, node);
            if (!boundary.valid) { result.deferred = true; return result; }
            boundaryNeeds(result, node, boundary);
            if (boundary.tail == 0) {
                relativeStep(result, query, query.kind == relativeMember ? ListPhase::enter : ListPhase::leave, boundary.visits);
            } else {
                auto index = query.kind == relativeBoundary ? boundary.prefix :
                             query.argument < 0 ? boundary.extent - 1 : 0;
                if (index >= 0 && (boundary.extent < 0 || index < boundary.extent)) {
                    auto edge = direct("open span position", {node, itemAt, index}, true);
                    edge.work.visits += boundary.visits; result.edges.push_back(std::move(edge));
                }
                result.refinements.push_back({{node, valueOnly, 0}, {boundary.visits, 0}, "resolve open span boundary", true});
            }
            return result;
        }
        if (query.kind == selectIntersection) {
            if (!access_->canSelect(node)) { result.deferred = true; return result; }
            if (auto index = access_->selectionIndex(node); index >= 0) {
                result.edges.push_back(direct("intersection prefix endpoint", {definitions_->selectionBody(node), itemAt, index}));
                return result;
            }
            result.edges.push_back(sequence(POVDescription::boundary(access_->selectionSource(node)->value),
                {node, selectionExtentMember, 0, node}, {node, selectionExtentEnd, 0, node}));
            return result;
        }
        if (query.kind == selectionExtentMember) {
            result.edges.push_back(sequence(description.adjacent(), query,
                {query.related, selectionExtentEnd, 0, query.related}));
            return result;
        }
        if (query.kind == selectionExtentEnd) {
            auto owner = query.related;
            auto selected = access_->selectionStart(owner);
            if (!selected) { result.deferred = true; return result; }
            result.edges.push_back(direct("intersection result position",
                {selected, access_->selectionIsMarked(owner) ? selectionFound : selectionMember,
                 access_->selectionIsLast(owner) ? 1 : 0, owner}));
            return result;
        }
        if (query.kind == selectionMember) {
            if (!access_->subList(node)) {
                result.edges.push_back(direct("selected intersection member",
                    {node, selectionFound, 0, query.related}));
                return result;
            }
            if (access_->selectionCycle(query.related, node)) { result.deferred = true; return result; }
            for (const auto& ancestor : query.context)
                if (access_->sameInfon(node, ancestor)) { result.deferred = true; return result; }
            if (!access_->selectionSpan(node, query.argument)) {
                result.refinements.push_back({{node, valueOnly, 0}, {0, 0}, "resolve selection span", false});
                result.deferred = true; return result;
            }
            auto context = query.context;
            if (auto child = description.member(query.argument != 0)) {
                context.push_back(node);
                result.edges.push_back(direct("enter result span",
                    {child, selectionMember, query.argument, query.related, context}));
            } else {
                if (access_->openTail(node)) {
                    result.refinements.push_back({{node, valueOnly, 0}, {0, 0}, "resolve empty open span", false});
                    result.deferred = true; return result;
                }
                auto cursor = node;
                POVNode next;
                while (!(next = POVDescription(cursor).adjacent(query.argument != 0)) && !context.empty()) {
                    cursor = context.back(); context.pop_back();
                }
                if (next) result.edges.push_back(direct("skip empty result span",
                    {next, selectionMember, query.argument, query.related, context}));
                else result.deferred = true;
            }
            return result;
        }
        if (query.kind == concatField) {
            if (!access_->hasConcat(node, query.argument)) { result.deferred = true; return result; }
            result.edges.push_back(sequence(POVDescription::boundary(access_->concatSource(node, query.argument)),
                {node, concatMember, query.argument, node, query.context}, {node, concatEnd, query.argument, node}));
            return result;
        }
        if (query.kind == concatMember) {
            auto context = query.context.empty() ? std::vector<POVNode>{query.related} : query.context;
            result.refinements.push_back({{node, valueOnly, 0, {}, context}, {0, 0}, "resolve operand value", false});
            if (access_->concatNeedsRadix(node))
                result.refinements.push_back({{node, sizeOnly, 0, {}, context}, {0, 0}, "resolve numeric radix", false});
            result.edges.push_back(sequence(description.adjacent(), query,
                {query.related, concatEnd, query.argument, query.related}));
            return result;
        }
        if (query.kind == listCorrespondence) {
            if (!access_->correspondingLists(node, query.related)) { result.deferred = true; return result; }
            result.edges.push_back(direct("inspect left span layout", {node, correspondenceEnter, 0, query.related}));
            return result;
        }
        if (query.kind == correspondenceEnter || query.kind == correspondenceMember ||
            query.kind == correspondenceLeave) {
            auto at = listPosition(query);
            if (!listPositionSupported(*access_, at) || (at.phase == ListPhase::enter &&
                (!access_->spanList(node) || !access_->orderedList(node)))) {
                result.deferred = true; return result;
            }
            auto next = nextListPosition(*access_, std::move(at), true);
            if (next.phase == ListPhase::cycle) result.deferred = true;
            else if (next.phase != ListPhase::done) {
                auto kind = next.phase == ListPhase::enter ? correspondenceEnter :
                            next.phase == ListPhase::member ? correspondenceMember : correspondenceLeave;
                result.edges.push_back(direct("next span position",
                    {next.node, kind, query.argument, query.related, std::move(next.owners)}));
            } else if (!query.argument) {
                result.edges.push_back(direct("inspect right span layout",
                    {query.related, correspondenceEnter, 1, node}));
            } else result.edges.push_back(direct("span layouts available",
                {node, correspondenceEnd, 1, query.related}));
            return result;
        }
        if (query.kind == listContents) {
            for (const auto& ancestor : query.context)
                if (access_->sameInfon(ancestor, node)) { result.deferred = true; return result; }
            if (!access_->physicalList(node) && !access_->growingList(node)) { result.deferred = true; return result; }
            auto owners = query.context; owners.push_back(node);
            result.edges.push_back(sequence(description.member(),
                {node, listMember, query.argument, {}, owners}, {node, listEnd, query.argument}));
            return result;
        }
        if (query.kind == listMember) {
            // Normalization requires the member's complete result. A size-only
            // count needs just an embedded span's extent; nested values count 1.
            if (query.argument != sizeOnly || access_->subList(node)) {
                auto context = query.context; context.insert(context.begin(), node);
                if(query.argument==relationshipConstraints)
                    result.edges.push_back(direct("check member constraints",{node,relationshipConstraints,0,{},context}));
                else result.refinements.push_back({{node, static_cast<int>(query.argument), 0, {}, context},
                    {0, 0}, "resolve list member", false});
            }
            result.edges.push_back(sequence(description.adjacent(), query, {node, listEnd, query.argument}));
            return result;
        }
        if ((query.kind >= sizeOnly && query.kind <= sizeAndValue) || query.kind == relationshipConstraints) {
            const bool constraintsOnly = query.kind == relationshipConstraints;
            const int demand = constraintsOnly ? sizeAndValue : query.kind;
            // Do not compare noncanonical scalar evidence while its local Set
            // is pending; that publication will expose the same edges again.
            if (access_->planNumericLiteral(node).changed) { result.deferred = true; return result; }
            auto owners = access_->subList(node) ? query.context : std::vector<POVNode>{};
            if (!owners.empty()) owners.erase(owners.begin());
            if (access_->inputPending(node)) {
                result.deferred = true;
                if (access_->growingList(node))
                    result.refinements.push_back({{node, listContents, demand, {}, owners}, {0, 0},
                        "visit published stream members", false});
                return result;
            }
            if (access_->isDefinition(node)) {
                result.deferred = !definitions_->registered(node); return result;
            }
            bool pendingUpdate=false,selectedView=false;
            if(auto field=node->pItem->pathSelection) {
                result.refinements.push_back({pathRequest(node),{0,0},"select path result",false});
                pendingUpdate=!PathExpression::updatesReady(field);
                if(!constraintsOnly) for(const auto& effect:field->effects)
                    result.refinements.push_back({{effect,sizeAndValue,0},{1,0},"complete retained path",false});
                if(!field->result) return result;
                selectedView=!field->pending && field->matches.size()==1 &&
                    node->pItem->value==field->result->pItem->value && node->pItem->infSize==field->result->pItem->infSize;
                // Normalizing a choice also demands its available candidates.
                // They are alternatives, not identities of the choice root.
                if(field->pending || field->matches.size()!=1)
                    for(const auto& match:field->matches)
                        result.refinements.push_back({{match,demand,0,{},PathExpression::location(field,match)},{1,0},"normalize path alternative",false});
            }
            result.deferred = pendingUpdate || !access_->supports(node, demand);
            if (auto direction = access_->relativeDirection(node); direction && !selectedView) {
                auto context = positions_->context(node, query.context, access_->registry.get());
                auto anchor = context.front(); context.erase(context.begin());
                result.refinements.push_back({{node, selectRelative, direction, anchor, context}, {0, 0},
                    "resolve relative position", false});
            }
            // A member's missing facts can be supplied by its containing type
            // without requiring the entire parent's definition to normalize.
            if (!constraintsOnly && !node->pItem->pathSelection && access_->hasTypeRequirements(node) &&
                access_->scalarCandidate(node) && (access_->known(node)&demand)!=demand &&
                query.context.size()>1 && query.context.front()==node) {
                // Embedded spans preserve membership in their containing list;
                // an ordinary nested list is the boundary of this relation.
                for(auto at=query.context.rbegin();at!=query.context.rend()-1;++at) {
                    auto parent=*at;
                    if(access_->hasTypeRequirements(parent))
                        result.refinements.push_back({{node,projectField,demand,parent},{1,0},"find field facts in parent definition",true});
                    if(!access_->subList(parent)) break;
                }
            }
            if (definitions_->required(node) && !definitions_->describesParts(node))
                result.refinements.push_back({{node, typeRequirements, 0}, {1, 0}, "validate type and member requirements", false});
            if (access_->hasInnerSource(node)) { result.deferred = true; return result; }
            if (access_->canSelect(node)) {
                auto body = definitions_->selectionBody(node);
                if (!POVDescription(body).work().empty()) {
                    if(constraintsOnly) result.edges.push_back(direct("inner source constraints",{body,relationshipConstraints,0}));
                    else result.refinements.push_back({{body, relationshipConstraints, 0}, {1, 0}, "prove inner source constraints", false});
                    result.refinements.push_back({{body, sizeAndValue, 0}, {1, 0}, "refine inner members", true});
                }
                result.refinements.push_back({{node, selectIntersection, 0}, {0, 0}, "find intersection result", false});
            }
            if (access_->hasConcat(node, 1))
                result.refinements.push_back({{node, concatField, 1, {}, query.context}, {0, 0}, "resolve size expression", false});
            if (access_->hasConcat(node, 2))
                result.refinements.push_back({{node, concatField, 2, {}, query.context}, {0, 0}, "resolve value expression", false});
            if (!constraintsOnly && !selectedView && access_->physicalList(node)) {
                if (demand != sizeOnly || !(access_->known(node) & sizeOnly))
                    result.refinements.push_back({{node, listContents, demand, {}, owners}, {0, 0},
                        "inspect list contents", demand == sizeOnly});
            }
            if (!constraintsOnly && !selectedView && demand != sizeOnly && access_->templatePrefixList(node)) {
                for (auto member = description.member(); member; member = POVDescription(member).adjacent())
                    result.refinements.push_back({{member, sizeAndValue, 0}, {1, 0}, "normalize template prefix member", false});
                for (auto member : access_->demandedOccurrences(node))
                    result.refinements.push_back({{member, sizeAndValue, 0}, {1, 0}, "normalize demanded occurrence", false});
            }
            // Keep the original demand endpoint across unresolved RHSs, so
            // a discovered list supplies correspondence directly to that goal.
            auto origin = query.related ? query.related : node;
            auto relationship = [&](const POVNode& source, const char* label, bool isWork) {
                auto context = isWork ? (query.context.empty() ? std::vector<POVNode>{node} : query.context)
                                      : std::vector<POVNode>{};
                if(!isWork) if(auto field=node->pItem->pathSelection)
                    if(field->result==source && !field->pending && field->matches.size()==1) context=PathExpression::location(field,source);
                if (access_->inputPending(source)) {
                    result.refinements.push_back({{source, demand, 0, {}, context}, {1, 0}, "await RHS input", false});
                    result.deferred = true;
                    if (!access_->prefixList(source)) return;
                }
                if (!access_->relationSupported(node, source, demand)) {
                    if (definitions_->canInherit(origin, source))
                        result.edges.push_back(direct("checked type evidence", {source, demand, 0, origin}));
                    if (access_->canSelect(source))
                        result.refinements.push_back({{source, selectIntersection, 0}, {1, 0}, "find RHS intersection result", false});
                    if (PathExpression::ready(*access_,source) || source->pItem->pathSelection || access_->hasInnerSource(source) || access_->relativeDirection(source) || access_->hasConcat(source, 1) || access_->hasConcat(source, 2))
                        result.refinements.push_back({{source, demand, 0, {}, context}, {1, 0}, "resolve expression RHS", false});
                    result.deferred = true; return;
                }
                if(constraintsOnly) {
                    auto verdict=access_->compatible(node,source,sizeAndValue);
                    if(verdict<0) throw std::runtime_error("incompatible scalar constraints");
                    if(!verdict) result.deferred=true;
                }
                result.edges.push_back(direct(label, {source, query.kind, 0, origin, context}));
                if (demand != sizeOnly && access_->correspondingLists(origin, source)) {
                    result.refinements.push_back({{origin, listCorrespondence, demand, source},
                        {1, 0}, "match list RHS members", false});
                    // Equality aligns members whose constraints must also
                    // agree. Grammar correspondence proves its own predicates
                    // and must not reinterpret them as asserted member values.
                    if(constraintsOnly)
                        result.edges.push_back(direct("check aligned member constraints",{origin,listContents,relationshipConstraints}));
                }
            };
            for (const auto& relation : description.relationships()) {
                auto source = relation.target;
                // A selected view already shares these exact state slots.
                // Validate its source directly; positional correspondence would
                // invent work merely because the view headers are distinct.
                if (relation.derived && node->pItem->value==source->pItem->value &&
                    node->pItem->infSize==source->pItem->infSize) {
                    if(constraintsOnly) result.edges.push_back(direct("selected source constraints",{source,relationshipConstraints,0}));
                    else result.refinements.push_back({{source,query.kind,0,{},PathExpression::location(node->pItem->pathSelection,source)},{0,0},"normalize selected source",false});
                    continue;
                }
                // Expose endpoint expressions before deciding which kind of
                // correspondence their resulting payload can participate in.
                if (relation.isWork() && source->looseType())
                    for (int slot : {1, 2}) if (access_->hasConcat(source, slot))
                        result.refinements.push_back({{source, concatField, slot}, {1, 0},
                            "resolve qualified RHS expression", false});
                if (demand != sizeOnly && NumericRelation(*access_,node,source).supported) {
                    result.refinements.push_back({{node,numericCorrespondence,0,source},{1,0},"match numeric range",false});
                    // Source facts may themselves arrive through ordinary work.
                    if(!source->looseType()) result.refinements.push_back({{source,sizeAndValue,0},{1,0},"refine numeric RHS",true});
                    continue;
                }
                if (relation.isWork() && demand != sizeOnly &&
                    SequenceCorrespondence::relationship(*access_, node, source)) {
                    result.refinements.push_back({{node, sequenceCorrespondence, 0, source}, {1, 0},
                        "match sequence correspondence", false});
                    continue;
                }
                if (relation.isWork() && demand != sizeOnly && access_->wholeLooseRHS(source)) {
                    result.refinements.push_back({{node, groupedList, 0, source}, {1, 0},
                        "map repeated groups to source spans", false});
                    continue;
                }
                relationship(source, relation.description(), relation.isWork());
                if (relation.isWork() && !relation.consolidated && demand == sizeAndValue)
                    result.refinements.push_back({{node, assessRepresentation, 0, source}, {0, 0},
                        "assess represented RHS", false});
            }
            return result;
        }
        if (query.kind == itemAt) {
            for(const auto& owner:query.context)
                if(access_->sameInfon(owner,node)) {result.deferred=true;return result;}
            // Type information may expose order, extent and a virtual sparse
            // route. Advertise that refinement before rejecting unknown shape.
            if (definitions_->required(node))
                result.refinements.push_back({{node, typeRequirements, 0}, {1, 0}, "expose indexed list shape", true});
            if (query.argument < 0) return result;
            // Shape may itself be an expression or an unresolved RHS. Expose
            // these routes before testing whether direct navigation is ready.
            if (!(access_->known(node) & sizeOnly))
                result.refinements.push_back({{node, sizeOnly, 0}, {1, 0}, "find size to expose indexed access", true});
            if (!access_->navigationList(node)) {
                result.refinements.push_back({{node, valueOnly, 0}, {1, 0}, "find value to expose indexed access", true});
                return result;
            }
            if (access_->sparse(node) && access_->listExtent(node) >= 0) {
                if (query.argument >= access_->extent(node)) {
                    return result;
                }
                auto access = access_; auto definitions = definitions_;
                // The virtual edge is visible before even a single implicit
                // occurrence is created. Its cost is independent of the index.
                result.edges.push_back({"sparse occurrence", {1, 1}, true,
                    [access, definitions, node, query] {
                        definitions->retainOccurrenceOwner(node);
                        auto existing = access->existingSparseItem(node, query.argument);
                        auto item = access->sparseItem(node, query.argument);
                        auto owners=query.context;owners.push_back(node);
                        return Arrival<POVNode>{{item, itemFound, 0, {}, owners}, !existing && bool(item)};
                    }});
            } else {
                auto first = description.member();
                if (first) {
                    auto owners=query.context;owners.push_back(node);
                    result.edges.push_back(direct("first physical item",
                        {first, physicalItemAt, query.argument, {}, owners}, true));
                }
            }
            // Registered equal lists may supply another route to this item.
            for (const auto& relation : description.relationships(false))
                result.edges.push_back(direct("identity indexed access",
                    {relation.target, itemAt, query.argument, {}, query.context}, true));
            return result;
        }
        if (query.kind == physicalItemAt) {
            std::int64_t remaining = query.argument;
            Cost inspection = 0;
            if (access_->subList(node)) {
                if (!access_->navigationList(node)) {
                    result.refinements.push_back({{node, valueOnly, 0}, {1, 0}, "resolve indexed span", true});
                    result.deferred = true; return result;
                }
                auto boundary = listBoundary(*access_, node); inspection = boundary.visits;
                if (!boundary.valid) { result.deferred = true; return result; }
                boundaryNeeds(result, node, boundary);
                if (boundary.extent < 0 || remaining < boundary.extent) {
                    auto edge = direct("indexed embedded position", {node, itemAt, remaining, {}, query.context}, true);
                    edge.work.visits += inspection; result.edges.push_back(std::move(edge));
                    return result;
                }
                remaining -= boundary.extent;
            } else if (remaining == 0) {
                result.edges.push_back(direct("selected physical item", {node, itemFound, 0, {}, query.context}, true)); return result;
            } else --remaining;
            auto next = nextListPosition(*access_, ListPosition<POVNode>{node, ListPhase::leave, query.context}, true);
            if (next.phase == ListPhase::member) {
                auto edge = direct("next logical item", {next.node, physicalItemAt, remaining, {}, next.owners}, true);
                edge.work.visits += inspection; result.edges.push_back(std::move(edge));
            }
            return result;
        }
        result.deferred = true;
        return result;
    }
};
} // namespace proteus4
