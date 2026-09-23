#include "GraphSearch.h"
#include <atomic>
#include <cassert>
#include <iostream>

using namespace proteus4;
struct TestNode;
using Node = std::shared_ptr<TestNode>;
struct TestNode {
    int value = -1, size = -1;
    std::vector<Node> rhs;
    Node prerequisite, afterRefinement;
    Node reveals;
    bool endless = false;
};
static Node node(int value = -1, int size = -1) {
    auto n = std::make_shared<TestNode>(); n->value = value; n->size = size; return n;
}

class TestGoal : public GoalSpec<Node> {
public:
    explicit TestGoal(Query<Node> q) : GoalSpec<Node>(q) {}
    bool satisfied() const override {
        return request.kind == 2 ? request.node->size >= 0 : request.node->value >= 0;
    }
    bool symbolic() const override { return !satisfied(); }
    std::vector<SetCommand> found(const Visit<Node>& visit) override {
        Node target = request.node, source = visit.query.node;
        if (target == source) return {};
        int kind = request.kind;
        auto field = [kind](const Node& n) { return kind == 2 ? n->size : n->value; };
        int value = field(source);
        if (value < 0) return {};
        for (const auto& step : visit.path)
            if (field(step.node) >= 0 && field(step.node) != value)
                throw std::runtime_error("constraint conflict");
        return {{kind == 2 ? "Set size" : "Set value", [target, value, kind] {
            int& destination = kind == 2 ? target->size : target->value;
            if (destination == value) return false;
            destination = value; return true;
        }}};
    }
};

class TestAccess : public GraphAccess<Node> {
public:
    std::atomic<int> expanded{0}, expensive{0}, cheap{0};
    Node costRoot;
    Node costResult = node(42);
    std::shared_ptr<GoalSpec<Node>> goal(const Query<Node>& query) override {
        return std::make_shared<TestGoal>(query);
    }
    Expansion<Node> expand(const Query<Node>& query) override {
        ++expanded;
        Expansion<Node> result;
        auto n = query.node;
        if (n == costRoot) {
            result.edges.push_back({"expensive materialization", {1000000, 1000000}, true,
                [this, query] { ++expensive; return Query<Node>{costResult, query.kind, 0}; }});
            result.edges.push_back({"sparse shortcut", {1, 1}, true,
                [this, query] { ++cheap; return Query<Node>{costResult, query.kind, 0}; }});
            return result;
        }
        for (auto rhs : n->rhs)
            result.edges.push_back({"identity", {1, 0}, false,
                [rhs, query] { return Query<Node>{rhs, query.kind, 0}; }});
        if (n->reveals && n->rhs.empty()) {
            result.edges.push_back({"discover new adjacency", {1, 0}, false, [n, query] {
                n->rhs.push_back(n->reveals);
                return Arrival<Node>{query, true};
            }});
        }
        if (n->prerequisite && query.kind == 1) {
            if (n->prerequisite->size < 0)
                result.refinements.push_back({{n->prerequisite, 2, 0}, {1, 0}, "required size"});
            else {
                Node source = n->afterRefinement;
                result.edges.push_back({"edge exposed by Set", {1, 0}, false,
                    [source] { return Query<Node>{source, 1, 0}; }});
            }
        }
        if (n->endless) {
            // A large virtual space, created one bounded step at a time.
            result.edges.push_back({"virtual continuation", {1, 0}, false, [query] {
                auto next = node(); next->endless = true;
                return Query<Node>{next, query.kind, 0};
            }});
        }
        return result;
    }
};

static bool saw(const NormalizationHandle<Node>& h, const std::string& label) {
    for (const auto& step : h.steps()) if (step.action == label) return true;
    return false;
}

int main() {
    int cases = 0;
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(), middle = node(), leaf = node(7);
        root->rhs = {middle}; middle->rhs = {root, leaf};
        auto h = manager.normalize({root, 1, 0}); h.waitForCompletion();
        assert(root->value == 7 && middle->value == -1);
        assert(h.status().fullyNormed && !h.status().symbolicValues && !h.status().failed);
        assert(root->rhs.size() == 1 && saw(h, "Set value"));
        manager.update([&] { middle->rhs.clear(); }); // release the fixture's owning cycle
        ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(); a->costRoot = root;
        auto h = manager.normalize({root, 1, 0}); h.waitForCompletion();
        assert(root->value == 42 && a->cheap == 1 && a->expensive == 0);
        assert(saw(h, "sparse shortcut")); ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(); a->costRoot = root;
        CostPolicy policy; policy.maximumRoute = 4;
        auto h = manager.normalize({root, 1, 0}, policy); h.waitForCompletion();
        assert(root->value == -1 && a->cheap == 0 && a->expensive == 0);
        assert(h.status().fullyNormed && h.status().symbolicValues && h.status().withSubscribers);
        assert(!h.status().cancelled && !h.status().failed);
        h.cancel(); h.waitForCompletion();
        assert(h.status().cancelled && !h.status().withSubscribers && !h.status().fullyNormed);
        ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(); a->costRoot = root;
        CostPolicy policy; policy.maximumRoute = 5;
        auto h = manager.normalize({root, 1, 0}, policy); h.waitForCompletion();
        assert(root->value == 42 && a->expensive == 0 && a->cheap == 1);
        assert(!h.status().symbolicValues && !h.status().withSubscribers);
        ++cases;
    }
    {
        // A route threshold does not consume a cumulative execution budget.
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(); for (int i = 0; i < 200; ++i) root->rhs.push_back(node(8));
        CostPolicy policy; policy.maximumRoute = 1;
        auto h = manager.normalize({root, 1, 0}, policy); h.waitForCompletion();
        assert(root->value == 8 && a->expanded >= 201 && !h.status().symbolicValues);
        ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(), dependency = node(), sizeSource = node(-1, 100);
        root->prerequisite = dependency; root->afterRefinement = node(12);
        dependency->rhs = {sizeSource};
        auto h = manager.normalize({root, 1, 0}); h.waitForCompletion();
        assert(dependency->size == 100 && root->value == 12);
        assert(saw(h, "subgoal: required size") && saw(h, "edge exposed by Set"));
        assert(!h.status().failed && !h.status().symbolicValues); ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(), missing = node(); root->rhs = {missing};
        auto h = manager.normalize({root, 1, 0}); h.waitForCompletion();
        assert(h.status().fullyNormed && h.status().withSubscribers);
        manager.update([&] { missing->rhs.push_back(node(19)); });
        h.waitForCompletion();
        assert(root->value == 19 && !h.status().withSubscribers);
        assert(saw(h, "subscribe") && saw(h, "Set value")); ++cases;
    }
    {
        // Race publication with expansion/subscription installation. No sleep
        // or assumption about which thread runs first is needed.
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        for (int i = 0; i < 100; ++i) {
            auto root = node(), missing = node(); root->rhs = {missing};
            auto h = manager.normalize({root, 1, 0});
            std::thread publisher([&] { manager.update([&] { missing->value = i; }); });
            publisher.join(); h.waitForCompletion();
            assert(root->value == i && !h.status().failed);
        }
        ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto cancelledRoot = node(), other = node(), shared = node();
        cancelledRoot->rhs = {shared}; other->rhs = {shared};
        auto first = manager.normalize({cancelledRoot, 1, 0});
        auto second = manager.normalize({other, 1, 0});
        first.waitForCompletion(); second.waitForCompletion(); first.cancel();
        manager.update([&] { shared->value = 23; });
        first.waitForCompletion(); second.waitForCompletion();
        assert(first.status().cancelled && cancelledRoot->value == -1);
        assert(other->value == 23 && !second.status().cancelled); ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(); root->endless = true;
        auto h = manager.normalize({root, 1, 0});
        while (a->expanded < 5) std::this_thread::yield();
        h.cancel(); h.waitForCompletion();
        assert(h.status().cancelled && !h.status().fullyNormed);
        ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(); root->rhs = {node(2), node(3)};
        auto h = manager.normalize({root, 1, 0}); h.waitForCompletion();
        assert(h.status().failed && !h.status().fullyNormed);
        assert(root->rhs.size() == 2 && root->value == 2);
        ++cases;
    }
    {
        NormalizationHandle<Node> surviving;
        {
            auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
            surviving = manager.normalize({node(), 1, 0}); surviving.waitForCompletion();
        }
        surviving.waitForCompletion(); assert(surviving.status().cancelled);
        ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(); root->prerequisite = node(); root->rhs = {node(31)};
        auto h = manager.normalize({root, 1, 0}); h.waitForCompletion();
        assert(root->value == 31 && root->prerequisite->size == -1);
        assert(!h.status().symbolicValues && !h.status().withSubscribers);
        ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(), virtualTail = node(); virtualTail->endless = true;
        root->rhs = {node(37), virtualTail};
        auto h = manager.normalize({root, 1, 0});
        while (!manager.read([&] { return root->value == 37; })) std::this_thread::yield();
        h.cancel(); h.waitForCompletion();
        assert(root->value == 37 && h.status().cancelled && saw(h, "Set value"));
        ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(); root->reveals = node(43);
        auto h = manager.normalize({root, 1, 0}); h.waitForCompletion();
        assert(root->value == 43 && h.status().fullyNormed);
        assert(saw(h, "materialize: discover new adjacency"));
        ++cases;
    }
    {
        CostPolicy policy;
        assert(policy.estimate({std::numeric_limits<Cost>::max(), 1}) ==
               std::numeric_limits<Cost>::max());
        assert(addCost(std::numeric_limits<Cost>::max(), 1) == std::numeric_limits<Cost>::max());
        ++cases;
    }
    std::cout << "Graph search: " << cases << "/" << cases << " scenarios passed\n";
}
