#pragma once

// Included after the CodeDog declarations. Domain knowledge is implemented in
// ProteusGraphAccess (GraphFoundation.Lib.dog); this layer adapts its results to
// generic edges and GoalSpec-generated commands.
namespace proteus4 {
using POVNode = std::shared_ptr<POV>;
enum Demand { itemFound = 0, sizeOnly = 1, valueOnly = 2, sizeAndValue = 3,
              itemAt = 4, physicalItemAt = 5 };

class ProteusGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    POVNode selected_;
    bool deferred_ = false;
public:
    ProteusGoal(Query<POVNode> query, std::shared_ptr<ProteusGraphAccess> access)
        : GoalSpec<POVNode>(query), access_(std::move(access)) {}
    void beginPass() override { deferred_ = false; }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        access_->bind(visit.query.node);
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
        if (request.kind == itemAt) return bool(selected_);
        return (access_->known(request.node) & request.kind) == request.kind;
    }
    bool symbolic() const override { return deferred_ || !satisfied(); }
    POVNode result() const override {
        return request.kind == itemAt ? selected_ : request.node;
    }
};

class ProteusAccess : public GraphAccess<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    static Edge<POVNode> direct(std::string label, Query<POVNode> query,
                                bool alternative = false, Cost remainingVisits = 0) {
        return {std::move(label), {1, 0}, alternative, [query] { return query; },
                {remainingVisits, 0}};
    }
public:
    explicit ProteusAccess(std::shared_ptr<ProteusGraphAccess> access)
        : access_(std::move(access)) {}
    std::shared_ptr<GoalSpec<POVNode>> goal(const Query<POVNode>& query) override {
        return std::make_shared<ProteusGoal>(query, access_);
    }
    Expansion<POVNode> expand(const Query<POVNode>& query) override {
        Expansion<POVNode> result;
        auto node = query.node;
        if (query.kind == itemFound) return result;
        if (query.kind >= sizeOnly && query.kind <= sizeAndValue) {
            result.deferred = !access_->supports(node, query.kind);
            for (auto source : access_->workSources(node)) {
                if (access_->relationSupported(node, source, query.kind))
                    result.edges.push_back(direct("work-list constraint", {source, query.kind, 0}));
                else result.deferred = true;
            }
            for (auto source : access_->identitySources(node)) {
                if (access_->relationSupported(node, source, query.kind))
                    result.edges.push_back(direct("registry identity", {source, query.kind, 0}));
                else result.deferred = true;
            }
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
