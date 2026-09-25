#pragma once
#include "SpanAlignment.h"

// Included after the CodeDog declarations. Domain knowledge is implemented in
// ProteusGraphAccess (GraphFoundation.Lib.dog); this layer adapts its results to
// generic edges and GoalSpec-generated commands.
namespace proteus4 {
using POVNode = std::shared_ptr<POV>;
enum Demand { itemFound = 0, sizeOnly = 1, valueOnly = 2, sizeAndValue = 3,
              itemAt = 4, physicalItemAt = 5,
              listContents = 6, listMember = 7, listEnd = 8,
              listCorrespondence = 9, correspondenceEnter = 10, correspondenceEnd = 11,
              correspondenceMember = 12, correspondenceLeave = 13,
              concatField = 14, concatMember = 15, concatEnd = 16,
              selectIntersection = 17, selectionMember = 18, selectionFound = 19,
              selectionExtentMember = 20, selectionExtentEnd = 21 };

// Completion flags are derived metadata. Keep weak witnesses independently
// of handles so later information invalidates even a released search result.
// Context-wide invalidation matches the foundation's existing revision policy.
class ProteusCompletion {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::vector<std::weak_ptr<POV>> marked_;
    struct Reply { std::uint64_t origin, order, position; POVNode node; };
    std::vector<Reply> effects_;
public:
    explicit ProteusCompletion(std::shared_ptr<ProteusGraphAccess> access)
        : access_(std::move(access)) {}
    void mark(const POVNode& node) {
        marked_.push_back(node);
        access_->setNormed(node, true);
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
    }
};

class ProteusGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusCompletion> completion_;
    POVNode selected_;
    bool deferred_ = false, reachedEnd_ = false;
    GraphListMeasure measure_;
public:
    ProteusGoal(Query<POVNode> query, std::shared_ptr<ProteusGraphAccess> access,
                std::shared_ptr<ProteusCompletion> completion)
        : GoalSpec<POVNode>(query), access_(std::move(access)),
          completion_(std::move(completion)) {}
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
        if (access_->inputPending(visit.query.node) && request.kind != listContents) return {};
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
        if (demand != sizeOnly && access_->literalList(target)) return {};
        // Work is conjunctive. A route is evidence only if each intermediate
        // constraint agrees with the finding. It is not an alternative branch.
        for (const auto& step : visit.path) {
            int verdict = access_->compatible(step.node, source, demand);
            if (verdict < 0) throw std::runtime_error("incompatible scalar constraints");
            if (!verdict) { deferred_ = true; return {}; }
        }
        auto access = access_;
        return {{"Set requested size/value/type", [access, target, source, demand] {
            return access->apply(target, source, demand);
        }}};
    }
    bool satisfied() const override {
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
    SpanLayout<POVNode> layouts_[2];
    std::vector<std::size_t> starts_[2];
    bool verified_ = false;
public:
    ProteusCorrespondenceGoal(Query<POVNode> query, std::shared_ptr<ProteusGraphAccess> access)
        : GoalSpec<POVNode>(query), access_(std::move(access)) {}
    void beginPass() override {
        layouts_[0] = {}; layouts_[1] = {};
        starts_[0].clear(); starts_[1].clear(); verified_ = false;
    }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        const auto& q = visit.query;
        if (q.kind == listCorrespondence) return {};
        auto side = static_cast<std::size_t>(q.argument);
        if (side > 1) return {};
        auto& layout = layouts_[side];
        if (q.kind == correspondenceEnter) starts_[side].push_back(layout.segments.size());
        if (q.kind == correspondenceMember && !access_->subList(q.node))
            layout.segments.push_back({q.node, false, 1});
        if (q.kind == correspondenceLeave) {
            if (access_->openTail(q.node)) layout.segments.push_back({q.node, true, -1});
            auto extent = access_->listExtent(q.node);
            if (extent == -2) layout.valid = false;
            layout.equations.push_back({starts_[side].back(), layout.segments.size(), extent, q.node});
            starts_[side].pop_back();
        }
        if (q.kind != correspondenceEnd) return {};
        auto plan = alignSpans(layouts_[0], layouts_[1]);
        if (!plan.valid) return {};
        // All mutation preconditions are checked before the batch is offered.
        // Unsupported new member semantics keep the entire plan symbolic.
        for (std::size_t i = 0; i < plan.fills.size(); ++i) {
            const auto& fill = plan.fills[i];
            for (const auto& source : fill.sources)
                if (!access_->canMakeMember(source)) return {};
            for (std::size_t j = 0; j < i; ++j)
                if (access_->sameInfon(fill.owner, plan.fills[j].owner)) return {};
        }
        verified_ = plan.complete;
        Work work{0, 0};
        for (const auto& pair : plan.identities)
            if (!access_->hasDirectIdentity(pair.first, pair.second)) ++work.creations;
        for (const auto& fill : plan.fills) work.creations += 2 * fill.sources.size();
        auto access = access_;
        auto left = request.node, right = request.related;
        int demand = static_cast<int>(request.argument);
        return {{"Set span alignment", [access, left, right, plan, demand] {
            access->bind(left); access->bind(right);
            for (const auto& pair : plan.identities) { access->bind(pair.first); access->bind(pair.second); }
            for (const auto& fill : plan.fills) {
                access->bind(fill.owner);
                for (const auto& source : fill.sources) access->bind(source);
            }
            bool changed = access->apply(left, right, demand);
            for (const auto& pair : plan.identities) {
                if (!access->hasDirectIdentity(pair.first, pair.second)) {
                    access->registry->recordIdentity(pair.first, pair.second, iekListCorrespondence, 0);
                    changed = true;
                }
            }
            for (const auto& fill : plan.fills) {
                for (const auto& source : fill.sources) {
                    access->appendTailMember(fill.owner, source); changed = true;
                }
                changed |= access->finishTail(fill.owner, fill.close, fill.extent);
            }
            return changed;
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

class ProteusAccess : public GraphAccess<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusCompletion> completion_;
    static Edge<POVNode> direct(std::string label, Query<POVNode> query,
                                bool alternative = false, Cost remainingVisits = 0) {
        return {std::move(label), {1, 0}, alternative, [query] { return query; },
                {remainingVisits, 0}};
    }
public:
    explicit ProteusAccess(std::shared_ptr<ProteusGraphAccess> access)
        : access_(std::move(access)),
          completion_(std::make_shared<ProteusCompletion>(access_)) {}
    void invalidateCompletion() override { completion_->invalidate(); }
    std::vector<std::function<void()>> takeEffects() override { return completion_->takeEffects(); }
    std::shared_ptr<GoalSpec<POVNode>> goal(const Query<POVNode>& query) override {
        if (query.kind == listCorrespondence)
            return std::make_shared<ProteusCorrespondenceGoal>(query, access_);
        if (query.kind == concatField)
            return std::make_shared<ProteusConcatGoal>(query, access_);
        if (query.kind == selectIntersection)
            return std::make_shared<ProteusSelectionGoal>(query, access_);
        return std::make_shared<ProteusGoal>(query, access_, completion_);
    }
    Expansion<POVNode> expand(const Query<POVNode>& query) override {
        Expansion<POVNode> result;
        auto node = query.node;
        if ((query.kind == itemAt || query.kind == physicalItemAt) && access_->inputPending(node)) {
            result.deferred = true; return result;
        }
        if (query.kind == itemFound || query.kind == listEnd || query.kind == correspondenceEnd || query.kind == concatEnd || query.kind == selectionFound)
            return result;
        if (query.kind == selectIntersection) {
            if (!access_->canSelect(node)) { result.deferred = true; return result; }
            auto first = access_->selectionShapeFirst(node);
            result.edges.push_back(direct("inspect intersection extent",
                {first ? first : node, first ? selectionExtentMember : selectionExtentEnd, 0, node}));
            return result;
        }
        if (query.kind == selectionExtentMember) {
            auto next = access_->next(node);
            result.edges.push_back(direct("next intersection extent position",
                {next ? next : query.related, next ? selectionExtentMember : selectionExtentEnd, 0, query.related}));
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
            if (auto child = access_->selectionChild(node, query.argument)) {
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
                while (!(next = access_->selectionNeighbour(cursor, query.argument)) && !context.empty()) {
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
            auto first = access_->concatFirst(node, query.argument);
            result.edges.push_back(direct("first concatenation operand",
                {first ? first : node, first ? concatMember : concatEnd, query.argument, node}));
            return result;
        }
        if (query.kind == concatMember) {
            result.refinements.push_back({{node, valueOnly, 0}, {0, 0}, "resolve operand value", false});
            if (access_->concatNeedsRadix(node))
                result.refinements.push_back({{node, sizeOnly, 0}, {0, 0}, "resolve numeric radix", false});
            auto next = access_->next(node);
            result.edges.push_back(direct("next concatenation operand",
                {next ? next : query.related, next ? concatMember : concatEnd, query.argument, query.related}));
            return result;
        }
        if (query.kind == listCorrespondence) {
            if (!access_->correspondingLists(node, query.related)) { result.deferred = true; return result; }
            result.edges.push_back(direct("inspect left span layout", {node, correspondenceEnter, 0, query.related}));
            return result;
        }
        if (query.kind == correspondenceEnter || query.kind == correspondenceMember ||
            query.kind == correspondenceLeave) {
            auto context = query.context;
            auto continuation = [&](const POVNode& current) {
                if (auto next = access_->next(current))
                    return Query<POVNode>{next, correspondenceMember, query.argument, query.related, context};
                auto owner = context.back(); context.pop_back();
                return Query<POVNode>{owner, correspondenceLeave, query.argument, query.related, context};
            };
            if (query.kind == correspondenceEnter) {
                if (!access_->physicalList(node) || !access_->orderedList(node)) {
                    result.deferred = true; return result;
                }
                for (const auto& ancestor : context) if (access_->sameInfon(node, ancestor)) {
                    result.deferred = true; return result;
                }
                if (auto first = access_->first(node)) {
                    context.push_back(node);
                    result.edges.push_back(direct("first span member",
                        {first, correspondenceMember, query.argument, query.related, context}));
                } else result.edges.push_back(direct("empty span boundary",
                    {node, correspondenceLeave, query.argument, query.related, context}));
            } else if (query.kind == correspondenceMember) {
                if (access_->subList(node))
                    result.edges.push_back(direct("enter embedded span",
                        {node, correspondenceEnter, query.argument, query.related, context}));
                else if (access_->correspondingMembers(node, node))
                    result.edges.push_back(direct("next span position", continuation(node)));
                else result.deferred = true;
            } else if (!context.empty()) {
                result.edges.push_back(direct("leave embedded span", continuation(node)));
            } else if (!query.argument) {
                result.edges.push_back(direct("inspect right span layout",
                    {query.related, correspondenceEnter, 1, node}));
            } else result.edges.push_back(direct("span layouts available",
                {node, correspondenceEnd, 1, query.related}));
            return result;
        }
        if (query.kind == listContents) {
            if (!access_->physicalList(node) && !access_->growingList(node)) { result.deferred = true; return result; }
            auto first = access_->first(node);
            result.edges.push_back(direct(first ? "first list member" : "empty list end",
                {first ? first : node, first ? listMember : listEnd, query.argument}));
            return result;
        }
        if (query.kind == listMember) {
            // Normalization requires the member's complete result. A size-only
            // count needs just an embedded span's extent; nested values count 1.
            if (query.argument != sizeOnly || access_->subList(node))
                result.refinements.push_back({{node, static_cast<int>(query.argument), 0},
                    {0, 0}, "resolve list member", false});
            auto next = access_->next(node);
            result.edges.push_back(direct(next ? "next list member" : "end of list members",
                {next ? next : node, next ? listMember : listEnd, query.argument}));
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
            result.deferred = !access_->supports(node, query.kind);
            if (access_->canSelect(node))
                result.refinements.push_back({{node, selectIntersection, 0}, {0, 0}, "find intersection result", false});
            if (access_->hasConcat(node, 1))
                result.refinements.push_back({{node, concatField, 1}, {0, 0}, "resolve size expression", false});
            if (access_->hasConcat(node, 2))
                result.refinements.push_back({{node, concatField, 2}, {0, 0}, "resolve value expression", false});
            if (access_->physicalList(node)) {
                if (query.kind != sizeOnly || !(access_->known(node) & sizeOnly))
                    result.refinements.push_back({{node, listContents, query.kind}, {0, 0},
                        "inspect list contents", query.kind == sizeOnly});
            }
            // Keep the original demand endpoint across unresolved RHSs, so
            // a discovered list supplies correspondence directly to that goal.
            auto origin = query.related ? query.related : node;
            auto relationship = [&](const POVNode& source, const char* label) {
                if (access_->inputPending(source)) {
                    result.refinements.push_back({{source, query.kind, 0}, {1, 0}, "await RHS input", false});
                    result.deferred = true; return;
                }
                if (!access_->relationSupported(node, source, query.kind)) {
                    if (access_->canSelect(source))
                        result.refinements.push_back({{source, selectIntersection, 0}, {1, 0}, "find RHS intersection result", false});
                    if (access_->hasConcat(source, 1) || access_->hasConcat(source, 2))
                        result.refinements.push_back({{source, query.kind, 0}, {1, 0}, "resolve expression RHS", false});
                    result.deferred = true; return;
                }
                result.edges.push_back(direct(label, {source, query.kind, 0, origin}));
                if (query.kind != sizeOnly && access_->correspondingLists(origin, source))
                    result.refinements.push_back({{origin, listCorrespondence, query.kind, source},
                        {1, 0}, "match list RHS members", false});
            };
            for (auto source : access_->workSources(node)) relationship(source, "work-list constraint");
            for (auto source : access_->identitySources(node)) relationship(source, "registry identity");
            return result;
        }
        if (query.kind == itemAt) {
            if (query.argument < 0 || !access_->orderedList(node)) {
                return result;
            }
            if (access_->sparse(node)) {
                if (query.argument >= access_->extent(node)) {
                    return result;
                }
                auto access = access_;
                // The virtual edge is visible before even a single implicit
                // occurrence is created. Its cost is independent of the index.
                result.edges.push_back({"sparse occurrence", {1, 1}, true,
                    [access, node, query] {
                        auto existing = access->existingSparseItem(node, query.argument);
                        auto item = access->sparseItem(node, query.argument);
                        return Arrival<POVNode>{{item, itemFound, 0}, !existing && bool(item)};
                    }});
            } else {
                auto first = access_->first(node);
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
            for (auto source : access_->identitySources(node))
                result.edges.push_back(direct("identity indexed access",
                    {source, itemAt, query.argument}, true));
            return result;
        }
        if (query.kind == physicalItemAt) {
            // Nested/list-span traversal is deliberately not guessed here.
            if (access_->subList(node)) return result;
            if (query.argument == 0)
                result.edges.push_back(direct("selected physical item", {node, itemFound, 0}, true));
            else if (auto next = access_->next(node))
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
