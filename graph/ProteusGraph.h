#pragma once
#include "ListStructure.h"
#include <set>

// Included after the CodeDog declarations. Domain knowledge is implemented in
// ProteusGraphAccess (GraphFoundation.Lib.dog); this layer adapts its results to
// generic edges and GoalSpec-generated commands.
namespace proteus4 {
using POVNode = std::shared_ptr<POV>;
#include "POVDescription.h"
enum Demand { itemFound = 0, sizeOnly = 1, valueOnly = 2, sizeAndValue = 3,
              itemAt = 4, physicalItemAt = 5,
              listContents = 6, listMember = 7, listEnd = 8,
              listCorrespondence = 9, correspondenceEnter = 10, correspondenceEnd = 11,
              correspondenceMember = 12, correspondenceLeave = 13,
              concatField = 14, concatMember = 15, concatEnd = 16,
              selectIntersection = 17, selectionMember = 18, selectionFound = 19,
              selectionExtentMember = 20, selectionExtentEnd = 21,
              typeRequirements = 22, definitionEnter = 23, definitionLeave = 24, definitionEnd = 25, assessRepresentation = 26, groupedList = 27,
              selectRelative = 28, relativeItem = 29 };

inline ListPosition<POVNode> listPosition(const Query<POVNode>& query) {
    auto phase = query.kind == correspondenceEnter ? ListPhase::enter :
                 query.kind == correspondenceMember ? ListPhase::member : ListPhase::leave;
    return {query.node, phase, query.context};
}

#include "DefinitionApplications.h"
#include "RHSRepresentation.h"
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

// A representation proof is narrower than normalization: unspecified fields
// need not become known. It changes only revision-scoped printer metadata.
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
        verified_ = plan.complete && !access_->inputPending(request.node) && !access_->inputPending(request.related);
        Work work{0, 0};
        for (const auto& pair : plan.identities)
            if (!access_->hasDirectIdentity(pair.first, pair.second)) ++work.creations;
        for (const auto& fill : plan.fills) work.creations += 2 * fill.sources.size();
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
        if (visit.query.kind != selectionFound) return {};
        if (!access_->selectionExtentAgrees(request.node, measure_)) return {};
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
    struct Binding { POVNode anchor, selected; std::weak_ptr<POV> expression; };
    using Key = std::weak_ptr<infonView>;
    std::map<Key, Binding, std::owner_less<Key>> bindings_;
public:
    POVNode anchor(const POVNode& node, const POVNode& proposed) const {
        auto found = bindings_.find(node->pItem);
        if (found == bindings_.end()) return proposed ? proposed : node;
        if (node != found->second.expression.lock() || (proposed && proposed != found->second.anchor))
            throw std::runtime_error("relative cursor used in different occurrence contexts");
        return found->second.anchor;
    }
    bool bind(const POVNode& node, const POVNode& anchor, const POVNode& selected) {
        auto found = bindings_.find(node->pItem);
        if (found != bindings_.end()) {
            if (found->second.anchor != anchor || found->second.selected != selected)
                throw std::runtime_error("relative position changed after binding");
            return false;
        }
        bindings_.emplace(node->pItem, Binding{anchor, selected, node}); return true;
    }
};

class ProteusRelativeGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusRelativePositions> positions_;
    bool verified_ = false;
public:
    ProteusRelativeGoal(Query<POVNode> query, std::shared_ptr<ProteusGraphAccess> access,
                        std::shared_ptr<ProteusRelativePositions> positions)
        : GoalSpec<POVNode>(query), access_(std::move(access)), positions_(std::move(positions)) {}
    void beginPass() override { verified_ = false; }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        if (visit.query.kind != itemFound) return {};
        verified_ = true;
        auto access = access_; auto positions = positions_;
        auto owner = request.node, anchor = request.related, selected = visit.query.node;
        return {{"Set relative position identity", [access, positions, owner, anchor, selected] {
            bool changed = positions->bind(owner, anchor, selected);
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
        if (query.kind == selectRelative)
            return std::make_shared<ProteusRelativeGoal>(query, access_, positions_);
        if (query.kind == groupedList)
            return std::make_shared<ProteusGroupedListGoal>(query, access_, definitions_, completion_);
        if (query.kind == assessRepresentation)
            return std::make_shared<ProteusRepresentationGoal>(query, access_, completion_);
        if (query.kind == typeRequirements)
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
        bool pending = evidence_->pending(query.node);
        auto result = expandSemantic(query);
        result.deferred = result.deferred || pending;
        return result;
    }
private:
    Expansion<POVNode> expandSemantic(const Query<POVNode>& query) {
        Expansion<POVNode> result;
        auto node = query.node;
        POVDescription description(node, access_->registry.get());
        if (query.kind == assessRepresentation || query.kind == groupedList) return result;
        if (query.kind == typeRequirements) {
            if (!definitions_->saved(node))
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
            result.edges.push_back(direct("relative occurrence", {query.related, relativeItem, query.argument}));
            return result;
        }
        if (query.kind == relativeItem) {
            // Physical links and virtual coordinates describe the same relation.
            // Embedded spans need a logical boundary walk, so remain visible as
            // deferred demands until that route is supported.
            if (auto owner = definitions_->retainedOccurrenceOwner(node)) {
                auto index = description.occurrenceIndex();
                auto extent = access_->listExtent(owner);
                if (index >= 0 && index < extent &&
                    (query.argument < 0 ? index > 0 : index < extent - 1))
                    result.edges.push_back(direct("relative sparse coordinate", {owner, itemAt, index + query.argument}));
            } else if (auto neighbor = description.adjacent(query.argument < 0)) {
                if (!access_->subList(node) && !access_->subList(neighbor))
                    result.edges.push_back(direct("adjacent occurrence", {neighbor, itemFound, 0}));
                else result.deferred = true;
            } else result.deferred = true;
            return result;
        }
        if (query.kind == selectIntersection) {
            if (!access_->canSelect(node)) { result.deferred = true; return result; }
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
            if (!access_->physicalList(node) && !access_->growingList(node)) { result.deferred = true; return result; }
            result.edges.push_back(sequence(description.member(),
                {node, listMember, query.argument}, {node, listEnd, query.argument}));
            return result;
        }
        if (query.kind == listMember) {
            // Normalization requires the member's complete result. A size-only
            // count needs just an embedded span's extent; nested values count 1.
            if (query.argument != sizeOnly || access_->subList(node))
                result.refinements.push_back({{node, static_cast<int>(query.argument), 0},
                    {0, 0}, "resolve list member", false});
            result.edges.push_back(sequence(description.adjacent(), query, {node, listEnd, query.argument}));
            return result;
        }
        if (query.kind >= sizeOnly && query.kind <= sizeAndValue) {
            if (access_->inputPending(node)) {
                result.deferred = true;
                if (access_->growingList(node))
                    result.refinements.push_back({{node, listContents, query.kind}, {0, 0},
                        "visit published stream members", false});
                return result;
            }
            if (access_->isDefinition(node)) {
                result.deferred = !definitions_->registered(node); return result;
            }
            result.deferred = !access_->supports(node, query.kind);
            if (auto direction = access_->relativeDirection(node)) {
                auto anchor = positions_->anchor(node, query.context.empty() ? POVNode{} : query.context.front());
                result.refinements.push_back({{node, selectRelative, direction, anchor}, {0, 0},
                    "resolve relative position", false});
            }
            if (definitions_->required(node))
                result.refinements.push_back({{node, typeRequirements, 0}, {1, 0}, "validate type and member requirements", false});
            if (access_->canSelect(node))
                result.refinements.push_back({{node, selectIntersection, 0}, {0, 0}, "find intersection result", false});
            if (access_->hasConcat(node, 1))
                result.refinements.push_back({{node, concatField, 1, {}, query.context}, {0, 0}, "resolve size expression", false});
            if (access_->hasConcat(node, 2))
                result.refinements.push_back({{node, concatField, 2, {}, query.context}, {0, 0}, "resolve value expression", false});
            if (access_->physicalList(node)) {
                if (query.kind != sizeOnly || !(access_->known(node) & sizeOnly))
                    result.refinements.push_back({{node, listContents, query.kind}, {0, 0},
                        "inspect list contents", query.kind == sizeOnly});
            }
            if (query.kind != sizeOnly && access_->templatePrefixList(node)) {
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
                if (access_->inputPending(source)) {
                    result.refinements.push_back({{source, query.kind, 0, {}, context}, {1, 0}, "await RHS input", false});
                    result.deferred = true;
                    if (!access_->prefixList(source)) return;
                }
                if (!access_->relationSupported(node, source, query.kind)) {
                    if (definitions_->canInherit(origin, source))
                        result.edges.push_back(direct("checked type evidence", {source, query.kind, 0, origin}));
                    if (access_->canSelect(source))
                        result.refinements.push_back({{source, selectIntersection, 0}, {1, 0}, "find RHS intersection result", false});
                    if (access_->relativeDirection(source) || access_->hasConcat(source, 1) || access_->hasConcat(source, 2))
                        result.refinements.push_back({{source, query.kind, 0, {}, context}, {1, 0}, "resolve expression RHS", false});
                    result.deferred = true; return;
                }
                result.edges.push_back(direct(label, {source, query.kind, 0, origin, context}));
                if (query.kind != sizeOnly && access_->correspondingLists(origin, source))
                    result.refinements.push_back({{origin, listCorrespondence, query.kind, source},
                        {1, 0}, "match list RHS members", false});
            };
            for (const auto& relation : description.relationships()) {
                auto source = relation.target;
                if (relation.isWork() && query.kind != sizeOnly && access_->wholeLooseRHS(source)) {
                    result.refinements.push_back({{node, groupedList, 0, source}, {1, 0},
                        "map repeated groups to source spans", false});
                    continue;
                }
                relationship(source, relation.isWork() ? "work-list constraint" : "registry identity", relation.isWork());
                if (relation.isWork() && query.kind == sizeAndValue)
                    result.refinements.push_back({{node, assessRepresentation, 0, source}, {0, 0},
                        "assess represented RHS", false});
            }
            return result;
        }
        if (query.kind == itemAt) {
            // Type information may expose order, extent and a virtual sparse
            // route. Advertise that refinement before rejecting unknown shape.
            if (definitions_->required(node))
                result.refinements.push_back({{node, typeRequirements, 0}, {1, 0}, "expose indexed list shape", true});
            if (query.argument < 0 || !access_->orderedList(node)) {
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
                        return Arrival<POVNode>{{item, itemFound, 0}, !existing && bool(item)};
                    }});
            } else {
                auto first = description.member();
                if (first)
                    result.edges.push_back(direct("first physical item",
                        {first, physicalItemAt, query.argument}, true,
                        static_cast<Cost>(query.argument) + 1));
                if (!(access_->known(node) & sizeOnly)) {
                    // An advertised refinement opportunity: size may expose a
                    // sparse edge. The manager starts a child goal and retries
                    // this query when its Set changes the graph.
                    result.refinements.push_back({{node, sizeOnly, 0}, {1, 0},
                                                   "find size to expose indexed access"});
                }
            }
            // Registered equal lists may supply another route to this item.
            for (const auto& relation : description.relationships(false))
                result.edges.push_back(direct("identity indexed access",
                    {relation.target, itemAt, query.argument}, true));
            return result;
        }
        if (query.kind == physicalItemAt) {
            // Nested/list-span traversal is deliberately not guessed here.
            if (access_->subList(node)) return result;
            if (query.argument == 0)
                result.edges.push_back(direct("selected physical item", {node, itemFound, 0}, true));
            else if (auto next = description.adjacent())
                result.edges.push_back(direct("next physical item",
                    {next, physicalItemAt, query.argument - 1}, true,
                    static_cast<Cost>(query.argument)));
            return result;
        }
        result.deferred = true;
        return result;
    }
};
} // namespace proteus4
