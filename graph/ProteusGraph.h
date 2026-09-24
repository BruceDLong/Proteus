#pragma once

// Included after the CodeDog declarations. Domain knowledge is implemented in
// ProteusGraphAccess (GraphFoundation.Lib.dog); this layer adapts its results to
// generic edges and GoalSpec-generated commands.
namespace proteus4 {
using POVNode = std::shared_ptr<POV>;
enum Demand { itemFound = 0, sizeOnly = 1, valueOnly = 2, sizeAndValue = 3,
              itemAt = 4, physicalItemAt = 5,
              listContents = 6, listMember = 7, listEnd = 8,
              listCorrespondence = 9, correspondingMember = 10, correspondenceEnd = 11 };

// Completion flags are derived metadata. Keep weak witnesses independently
// of handles so later information invalidates even a released search result.
// Context-wide invalidation matches the foundation's existing revision policy.
class ProteusCompletion {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::vector<std::weak_ptr<POV>> marked_;
public:
    explicit ProteusCompletion(std::shared_ptr<ProteusGraphAccess> access)
        : access_(std::move(access)) {}
    void mark(const POVNode& node) {
        marked_.push_back(node);
        access_->setNormed(node, true);
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
    }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        access_->bind(visit.query.node);
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
        return {{"Set requested size/value", [access, target, source, demand] {
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

// A relational goal checks the complete paired walk before a Set publishes
// member identities. Child goals then use ordinary registry/scalar traversal.
class ProteusCorrespondenceGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    GraphListCorrespondence evidence_;
    bool verified_ = false;
public:
    ProteusCorrespondenceGoal(Query<POVNode> query, std::shared_ptr<ProteusGraphAccess> access)
        : GoalSpec<POVNode>(query), access_(std::move(access)) {}
    void beginPass() override { evidence_.reset(); verified_ = false; }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        if (visit.query.kind == correspondingMember)
            evidence_.add(visit.query.node, visit.query.related);
        if (visit.query.kind != correspondenceEnd) return {};
        verified_ = access_->correspondenceShapeAgrees(request.node, request.related, evidence_);
        if (!verified_) return {};
        auto access = access_;
        auto left = request.node, right = request.related;
        auto evidence = evidence_;
        int demand = static_cast<int>(request.argument);
        return {{"Set list member identities", [access, left, right, evidence, demand] {
            return access->applyCorrespondence(left, right, evidence, demand);
        }}};
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
    std::shared_ptr<GoalSpec<POVNode>> goal(const Query<POVNode>& query) override {
        if (query.kind == listCorrespondence)
            return std::make_shared<ProteusCorrespondenceGoal>(query, access_);
        return std::make_shared<ProteusGoal>(query, access_, completion_);
    }
    Expansion<POVNode> expand(const Query<POVNode>& query) override {
        Expansion<POVNode> result;
        auto node = query.node;
        if (query.kind == itemFound || query.kind == listEnd || query.kind == correspondenceEnd)
            return result;
        if (query.kind == listCorrespondence || query.kind == correspondingMember) {
            auto right = query.related;
            if (query.kind == listCorrespondence) {
                if (!access_->correspondingLists(node, right)) { result.deferred = true; return result; }
            } else if (!access_->correspondingMembers(node, right)) {
                result.deferred = true; return result;
            }
            auto leftNext = query.kind == listCorrespondence ? access_->first(node) : access_->next(node);
            auto rightNext = query.kind == listCorrespondence ? access_->first(right) : access_->next(right);
            if (bool(leftNext) != bool(rightNext)) { result.deferred = true; return result; }
            if (!leftNext) {
                result.edges.push_back(direct("end of corresponding members",
                    {node, correspondenceEnd, query.argument, right}));
            } else if (access_->correspondingMembers(leftNext, rightNext)) {
                Query<POVNode> next{leftNext, correspondingMember, query.argument, rightNext};
                Cost creations = access_->hasDirectIdentity(leftNext, rightNext) ? 0 : 1;
                result.edges.push_back({"corresponding list members", {1, creations}, false,
                                        [next] { return next; }});
            } else result.deferred = true;
            return result;
        }
        if (query.kind == listContents) {
            if (!access_->physicalList(node)) { result.deferred = true; return result; }
            auto first = access_->first(node);
            result.edges.push_back(direct(first ? "first list member" : "empty list end",
                {first ? first : node, first ? listMember : listEnd, query.argument}));
            return result;
        }
        if (query.kind == listMember) {
            // Normalization requires the member's complete result. A size-only
            // count needs just an embedded span's extent; nested values count 1.
            if (query.argument == sizeAndValue || access_->subList(node))
                result.refinements.push_back({{node, static_cast<int>(query.argument), 0},
                    {0, 0}, "resolve list member", false});
            auto next = access_->next(node);
            result.edges.push_back(direct(next ? "next list member" : "end of list members",
                {next ? next : node, next ? listMember : listEnd, query.argument}));
            return result;
        }
        if (query.kind >= sizeOnly && query.kind <= sizeAndValue) {
            result.deferred = !access_->supports(node, query.kind);
            if (access_->physicalList(node)) {
                if (query.kind != sizeOnly || !(access_->known(node) & sizeOnly))
                    result.refinements.push_back({{node, listContents, query.kind}, {0, 0},
                        "inspect list contents", query.kind == sizeOnly});
            }
            auto relationship = [&](const POVNode& source, const char* label) {
                if (!access_->relationSupported(node, source, query.kind)) {
                    result.deferred = true; return;
                }
                result.edges.push_back(direct(label, {source, query.kind, 0}));
                if (query.kind != sizeOnly && access_->literalList(node))
                    result.refinements.push_back({{node, listCorrespondence, query.kind, source},
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
