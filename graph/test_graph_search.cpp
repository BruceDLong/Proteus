#include "GraphSearch.h"
#include <atomic>
#include <cassert>
#include <iostream>
#include <future>
#include <chrono>

using namespace proteus4;
struct TestNode;
using Node = std::shared_ptr<TestNode>;
struct TestNode {
    int value = -1, size = -1;
    DependencyKey facts = std::make_shared<Dependency>();
    Cost setCreations = 0;
    std::vector<Node> rhs;
    Node prerequisite, afterRefinement;
    Node reveals;
    bool endless = false, complete = false;
    std::vector<std::pair<Node, Cost>> required, exploratory;
};
static Node node(int value = -1, int size = -1) {
    auto n = std::make_shared<TestNode>(); n->value = value; n->size = size; return n;
}

class TestGoal : public GoalSpec<Node> {
public:
    explicit TestGoal(Query<Node> q) : GoalSpec<Node>(q) {}
    void beginPass() override { request.node->complete = false; }
    void settled(bool complete) override {
        if (complete)
            for (const auto& child : request.node->required) assert(child.first->complete);
        request.node->complete = complete;
    }
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
                throw GoalRejected("constraint conflict");
        if (field(target) == value) return {};
        return {{kind == 2 ? "Set size" : "Set value", [target, value, kind] {
            int& destination = kind == 2 ? target->size : target->value;
            if (destination == value) return false;
            destination = value; return true;
        }, {0, source->setCreations}}};
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
        for (const auto& child : n->required)
            result.refinements.push_back({{child.first, 1, 0}, {child.second, 0},
                                          "required member", false});
        for (const auto& child : n->exploratory)
            result.refinements.push_back({{child.first, 1, 0}, {child.second, 0},
                                          "exploratory member", true});
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

// A result assembled across cheap and slow routes must publish one complete
// batch, rather than repeatedly restarting at a cheaper partial result.
class CollectedGoal : public GoalSpec<Node> {
    int sum_=0;
    Visit<Node> last_;
    std::shared_ptr<int> publications_;
public:
    CollectedGoal(Query<Node> q,std::shared_ptr<int> publications)
        :GoalSpec<Node>(q),publications_(std::move(publications)) {}
    void beginPass() override {sum_=0;last_={request,{},0};}
    bool satisfied() const override {return false;}
    bool symbolic() const override {return request.node->value!=sum_;}
    std::vector<SetCommand> found(const Visit<Node>& visit) override {
        if(visit.query.node!=request.node && visit.query.node->value>=0) {
            sum_+=visit.query.node->value;last_=visit;
        }
        return {};
    }
    CommandBatch<Node> finishPass() override {
        if(request.node->value==sum_) return {};
        auto target=request.node;auto publications=publications_;int total=sum_;
        return {last_,{{"Set collected result",[target,publications,total] {
            target->value=total;++*publications;return true;
        },{1,1}}}};
    }
};
class CollectedAccess : public TestAccess {
public:
    std::shared_ptr<int> publications=std::make_shared<int>(0);
    std::shared_ptr<GoalSpec<Node>> goal(const Query<Node>& query) override {
        return std::make_shared<CollectedGoal>(query,publications);
    }
};

// The scheduler knows none of these fields. This adapter reports every read,
// including failed lookups, and scopes writes to the same evidence tokens.
class TrackedGoal : public TestGoal {
public:
    using TestGoal::TestGoal;
    bool satisfied() const override {
        ReadScope::observe(request.node->facts);return TestGoal::satisfied();
    }
    std::vector<SetCommand> found(const Visit<Node>& visit) override {
        ReadScope::observe(request.node->facts);
        for(const auto& step:visit.path) ReadScope::observe(step.node->facts);
        auto commands=TestGoal::found(visit);
        for(auto& command:commands) command.changes=Changes::only({request.node->facts});
        return commands;
    }
};
class TrackedAccess : public TestAccess {
public:
    bool tracksDependencies() const override {return true;}
    std::shared_ptr<GoalSpec<Node>> goal(const Query<Node>& query) override {
        return std::make_shared<TrackedGoal>(query);
    }
    Expansion<Node> expand(const Query<Node>& query) override {
        ReadScope::observe(query.node->facts);
        if(query.node->prerequisite) ReadScope::observe(query.node->prerequisite->facts);
        return TestAccess::expand(query);
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
    {
        // One missing child cannot prevent an independent child completing.
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(1), ready = node(2), missing = node();
        root->required = {{ready, 1}, {missing, 1}};
        auto h = manager.normalize({root, 1, 0}); h.waitForCompletion();
        assert(!root->complete && ready->complete && !missing->complete);
        assert(h.status().symbolicValues && h.status().withSubscribers);
        manager.update([&] { missing->value = 3; }); h.waitForCompletion();
        assert(root->complete && missing->complete && !h.status().symbolicValues);
        ++cases;
    }
    {
        // Shared dependencies need dependency order, not reverse creation order.
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(1), left = node(2), right = node(3), leaf = node(4);
        root->required = {{left, 1}, {right, 1}};
        left->required = {{leaf, 1}};
        right->required = {{left, 1}, {leaf, 1}};
        auto h = manager.normalize({root, 1, 0}); h.waitForCompletion();
        assert(root->complete && left->complete && right->complete && leaf->complete);
        assert(!h.status().symbolicValues); ++cases;
    }
    {
        // A cycle is not its own proof. An independent leaf still completes.
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(1), cyclic = node(2), leaf = node(3);
        root->required = {{cyclic, 1}, {leaf, 1}}; cyclic->required = {{root, 1}};
        auto h = manager.normalize({root, 1, 0}); h.waitForCompletion();
        assert(!root->complete && !cyclic->complete && leaf->complete);
        assert(h.status().symbolicValues && !h.status().failed);
        h.cancel(); manager.update([&] { cyclic->required.clear(); }); ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(1), child = node(2); root->required = {{child, 1}};
        CostPolicy policy; policy.maximumRoute = 0;
        auto h = manager.normalize({root, 1, 0}, policy); h.waitForCompletion();
        assert(!root->complete && !child->complete && h.status().symbolicValues);
        ++cases;
    }
    {
        // An initially excluded shared task must be retried through its cheaper
        // route; reaching it twice is still one completion obligation.
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(1), child = node(2), bridge = node(3);
        root->required = {{child, 10}, {bridge, 1}}; bridge->required = {{child, 1}};
        CostPolicy policy; policy.maximumRoute = 3;
        auto h = manager.normalize({root, 1, 0}, policy); h.waitForCompletion();
        assert(root->complete && child->complete && bridge->complete);
        assert(!h.status().symbolicValues); ++cases;
    }
    {
        // Cancellation covers child goals and prevents a later publication
        // from writing through the cancelled parent's search.
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(1), child = node(), source = node();
        root->required = {{child, 1}}; child->rhs = {source};
        auto h = manager.normalize({root, 1, 0}); h.waitForCompletion(); h.cancel();
        manager.update([&] { source->value = 4; }); h.waitForCompletion();
        assert(h.status().cancelled && child->value == -1 && !root->complete);
        ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(), source = node(9); source->setCreations = 2; root->rhs = {source};
        CostPolicy policy; policy.maximumRoute = 2;
        auto h = manager.normalize({root, 1, 0}, policy); h.waitForCompletion();
        assert(root->value == -1 && h.status().symbolicValues && saw(h, "exclude: Set value")); ++cases;
    }
    {
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(), costly = node(9), cheap = node(9);
        costly->setCreations = 100; root->rhs = {costly, cheap};
        auto h = manager.normalize({root, 1, 0}); h.waitForCompletion();
        assert(root->value == 9 && !h.status().symbolicValues);
        int writes = 0;
        for (const auto& step : h.steps()) if (step.action == "Set value") {
            assert(step.cost == 1); ++writes;
        }
        assert(writes == 1); ++cases;
    }
    {
        // Parser ownership blocks mutation/search, but an input pause publishes
        // completed information and lets subscribed work run without lost wakes.
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto root = node(); auto h = manager.normalize({root, 1, 0}); h.waitForCompletion();
        auto publication = manager.beginPublication();
        root->rhs.push_back(node(51)); publication->changed();
        {
            InputPause<Node> pause(publication);
            h.waitForCompletion();
            assert(root->value == 51 && !h.status().symbolicValues);
        }
        publication.reset(); ++cases;
    }
    {
        // Quiescence can precede producer EOF; cancellation releases the wait.
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto h = manager.normalize({node(), 1, 0}); h.waitForCompletion();
        std::atomic<bool> entered{false};
        auto waiter = std::async(std::launch::async, [&] {
            entered = true; h.waitUntil([] { return false; });
        });
        while (!entered) std::this_thread::yield();
        assert(waiter.wait_for(std::chrono::milliseconds(20)) == std::future_status::timeout);
        h.cancel();
        assert(waiter.wait_for(std::chrono::seconds(2)) == std::future_status::ready);
        waiter.get(); ++cases;
    }
    {
        // EOF publication wakes an input-lifetime wait even if the result stays symbolic.
        auto a = std::make_shared<TestAccess>(); GraphManager<Node> manager(a);
        auto h = manager.normalize({node(), 1, 0}); h.waitForCompletion();
        bool eof = false;
        auto waiter = std::async(std::launch::async, [&] { h.waitUntil([&] { return eof; }); });
        assert(waiter.wait_for(std::chrono::milliseconds(20)) == std::future_status::timeout);
        manager.update([&] { eof = true; });
        assert(waiter.wait_for(std::chrono::seconds(2)) == std::future_status::ready);
        waiter.get(); assert(h.status().withSubscribers); ++cases;
    }
    {
        // Effects run after settlement, outside the context lock, and finish
        // before waitForCompletion returns. They may publish other information.
        struct Effects : TestAccess {
            Node watched;
            bool sent = false;
            std::function<void()> effect;
            std::vector<std::function<void()>> takeEffects() override {
                if (!sent && watched && watched->complete) { sent = true; return {effect}; }
                return {};
            }
        };
        auto a = std::make_shared<Effects>(); GraphManager<Node> manager(a);
        auto target = node(); auto waiting = manager.normalize({target, 1, 0}); waiting.waitForCompletion();
        a->watched = node(61);
        int deliveries = 0;
        a->effect = [&] { manager.update([&] { target->value = 62; ++deliveries; }); };
        auto h = manager.normalize({a->watched, 1, 0}); h.waitForCompletion(); waiting.waitForCompletion();
        assert(deliveries == 1 && target->value == 62 && !waiting.status().symbolicValues);
        ++cases;
    }
    {
        auto access=std::make_shared<CollectedAccess>();GraphManager<Node> manager(access);
        auto root=node(99),slow=node();root->rhs={node(2),slow};
        for(int i=0;i<30;++i) {auto next=node();slow->rhs={next};slow=next;}
        slow->rhs={node(3)};
        auto result=manager.normalize({root,1,0});result.waitForCompletion();
        assert(root->value==5 && *access->publications==1 && !result.status().symbolicValues);
        auto replay=manager.normalize({root,1,0});replay.waitForCompletion();
        assert(root->value==5 && *access->publications==1 && !replay.status().failed);
        ++cases;
    }
    {
        // Deferred publication keeps the winning route's cost. Exclusion
        // must not requeue the same end-of-pass command forever.
        auto access=std::make_shared<CollectedAccess>();GraphManager<Node> manager(access);
        auto root=node();root->rhs={node(7)};
        CostPolicy cost;cost.maximumRoute=5; // route 1 + Set (1 visit, 1 creation) = 6
        auto result=manager.normalize({root,1,0},cost);result.waitForCompletion();
        assert(root->value==-1 && *access->publications==0 && result.status().withSubscribers);
        assert(saw(result,"exclude: Set collected result"));result.cancel();++cases;
    }
    {
        // Rejecting one exploratory comparison must leave the same source
        // available to a slower successful route in this search.
        auto access=std::make_shared<TestAccess>();GraphManager<Node> manager(access);
        auto root=node(),bad=node(8),source=node(9),cursor=root;
        bad->rhs={source};root->exploratory={{bad,0}};
        for(int i=0;i<8;++i) {auto next=node();cursor->rhs={next};cursor=next;}
        cursor->rhs={source};
        auto result=manager.normalize({root,1,0});result.waitForCompletion();
        assert(!result.status().failed && !result.status().symbolicValues && root->value==9);
        assert(bad->value==8 && bad->rhs.front()==source && source->value==9);
        ++cases;
    }
    {
        // A failed required descendant rejects its exploratory parent only.
        auto access=std::make_shared<TestAccess>();GraphManager<Node> manager(access);
        auto root=node(),attempt=node(1),bad=node(8),source=node(9);
        root->exploratory={{attempt,0}};attempt->required={{bad,0}};bad->rhs={source};
        auto result=manager.normalize({root,1,0});result.waitForCompletion();
        assert(!result.status().failed && result.status().withSubscribers);
        assert(!bad->complete && !attempt->complete && saw(result,"reject: constraint conflict"));
        manager.update([&]{root->rhs={source};});result.waitForCompletion();
        assert(!result.status().failed && root->value==9 && bad->value==8);
        ++cases;
    }
    {
        // Once the same rejected goal becomes required, failure reaches the
        // root even if that dependency was discovered after the rejection.
        auto access=std::make_shared<TestAccess>();GraphManager<Node> manager(access);
        auto root=node(),later=node(1),bad=node(8),source=node(9);
        root->exploratory={{bad,0}};root->required={{later,10}};
        later->required={{bad,0}};bad->rhs={source};
        auto result=manager.normalize({root,1,0});result.waitForCompletion();
        assert(result.status().failed && !result.status().withSubscribers && !root->complete);
        assert(bad->rhs.front()==source && source->value==9);++cases;
    }
    {
        // Rejection is revision-scoped. A subscribed search retries after new
        // evidence, but it never spins on an unchanged failed comparison.
        auto access=std::make_shared<TestAccess>();GraphManager<Node> manager(access);
        auto root=node(),bad=node(8),source=node(9);root->exploratory={{bad,0}};bad->rhs={source};
        auto result=manager.normalize({root,1,0});result.waitForCompletion();
        int rejections=0;for(const auto& step:result.steps()) if(step.action=="reject: constraint conflict") ++rejections;
        assert(rejections==1 && result.status().withSubscribers);
        manager.update([&]{bad->value=9;});result.waitForCompletion();
        assert(!result.status().failed && bad->complete && result.status().withSubscribers);
        result.cancel();manager.update([&]{root->rhs={source};});result.waitForCompletion();
        assert(result.status().cancelled && root->value==-1);++cases;
    }
    {
        // Real engine faults are not silently treated as a losing candidate.
        struct BrokenAccess : TestAccess {
            Node broken;
            Expansion<Node> expand(const Query<Node>& q) override {
                if(q.node==broken) throw std::logic_error("broken invariant");
                return TestAccess::expand(q);
            }
        };
        auto access=std::make_shared<BrokenAccess>();GraphManager<Node> manager(access);
        auto root=node();access->broken=node();root->exploratory={{access->broken,0}};
        auto result=manager.normalize({root,1,0});result.waitForCompletion();
        assert(result.status().failed && result.status().error=="broken invariant");++cases;
    }
    {
        // Rejection during expansion invalidates Sets already queued by found.
        struct RejectingAccess : TestAccess {
            Node source;
            Expansion<Node> expand(const Query<Node>& q) override {
                if(q.node==source) throw GoalRejected("rejected after finding");
                return TestAccess::expand(q);
            }
        };
        auto access=std::make_shared<RejectingAccess>();GraphManager<Node> manager(access);
        auto root=node(),candidate=node();access->source=node(9);candidate->rhs={access->source};
        root->exploratory={{candidate,0}};
        auto result=manager.normalize({root,1,0});result.waitForCompletion();
        assert(!result.status().failed && result.status().withSubscribers && candidate->value==-1);
        assert(!saw(result,"Set value"));result.cancel();++cases;
    }
    for(bool inSet:{false,true}) {
        // Both delayed validation and an end-of-pass proof reject locally.
        struct RejectingGoal : TestGoal {
            bool inSet;
            RejectingGoal(Query<Node> q,bool where):TestGoal(q),inSet(where) {}
            std::vector<SetCommand> found(const Visit<Node>&) override {
                if(!inSet) return {};
                return {{"rejecting Set",[]{throw GoalRejected("rejected Set");return false;}}};
            }
            CommandBatch<Node> finishPass() override {throw GoalRejected("rejected finish");}
        };
        struct RejectingAccess : TestAccess {
            Node rejected;bool inSet=false;
            std::shared_ptr<GoalSpec<Node>> goal(const Query<Node>& q) override {
                if(q.node==rejected) return std::make_shared<RejectingGoal>(q,inSet);
                return TestAccess::goal(q);
            }
        };
        auto access=std::make_shared<RejectingAccess>();GraphManager<Node> manager(access);
        auto root=node();access->rejected=node();access->inSet=inSet;root->exploratory={{access->rejected,0}};
        auto result=manager.normalize({root,1,0});result.waitForCompletion();
        assert(!result.status().failed && result.status().withSubscribers && !access->rejected->complete);
        assert(saw(result,inSet ? "reject: rejected Set" : "reject: rejected finish"));result.cancel();++cases;
    }
    {
        // A busy search shares its context with a producer repeatedly publishing
        // prefixes. Neither side may require the other to finish its lifetime.
        auto access=std::make_shared<TestAccess>();GraphManager<Node> manager(access);
        auto root=node();root->endless=true;
        auto result=manager.normalize({root,1,0});
        auto producer=std::async(std::launch::async,[&] {
            for(int i=0;i<1000;++i) {
                auto publication=manager.beginPublication();
                root->size=i;publication->changed();
            }
        });
        bool responsive=producer.wait_for(std::chrono::seconds(2))==std::future_status::ready;
        result.cancel();producer.get();
        assert(responsive && root->size==999 && result.status().cancelled);++cases;
    }
    {
        // Many independent subscriptions must not wake for one local write.
        auto a=std::make_shared<TrackedAccess>();GraphManager<Node> manager(a);
        std::vector<Node> nodes;std::vector<NormalizationHandle<Node>> handles;
        for(int i=0;i<128;++i) {nodes.push_back(node());handles.push_back(manager.normalize({nodes.back(),1}));}
        for(auto& h:handles) h.waitForCompletion();
        int before=a->expanded;auto rev=manager.revision();
        assert(!manager.updateIfChanged([] {return false;}));
        assert(manager.revision()==rev && a->expanded==before);
        for(int i=0;i<128;++i) {
            manager.updateIfChanged([&] {nodes[i]->value=i;return true;},Changes::only({nodes[i]->facts}));
            handles[i].waitForCompletion();
            assert(!handles[i].status().symbolicValues && nodes[i]->value==i);
        }
        assert(a->expanded==before+128);
        std::cout<<"Independent subscriptions: 128 writes, "<<a->expanded-before<<" additional expansions\n";
        ++cases;
    }
    {
        // Readiness/negative evidence is subscribed even when no RHS exists.
        auto a=std::make_shared<TrackedAccess>();GraphManager<Node> manager(a);
        auto root=node(),source=node(7),unrelated=node();
        auto h=manager.normalize({root,1});h.waitForCompletion();auto steps=h.steps().size();
        manager.updateIfChanged([&] {unrelated->value=3;return true;},Changes::only({unrelated->facts}));
        h.waitForCompletion();assert(h.steps().size()==steps && h.status().withSubscribers);
        {auto publication=manager.beginPublication();root->rhs={source};publication->changed(Changes::only({root->facts}));}
        h.waitForCompletion();assert(root->value==7 && !h.status().failed && !h.status().symbolicValues);
        // Completed handles retain the explicit re-normalization contract.
        steps=h.steps().size();manager.updateIfChanged([&] {source->value=8;return true;},Changes::only({source->facts}));
        assert(h.steps().size()==steps);
        auto again=manager.normalize({root,1});again.waitForCompletion();assert(again.status().failed);++cases;
    }
    {
        // A followed transitive edge subscribes its actual endpoint. Removing
        // an edge also retires obsolete reads when the search rebuilds.
        auto a=std::make_shared<TrackedAccess>();GraphManager<Node> manager(a);
        auto root=node(),middle=node(),leaf=node();root->rhs={middle};middle->rhs={leaf};
        auto h=manager.normalize({root,1});h.waitForCompletion();
        manager.updateIfChanged([&] {middle->rhs.clear();return true;},Changes::only({middle->facts}));h.waitForCompletion();
        int before=a->expanded;
        manager.updateIfChanged([&] {leaf->value=9;return true;},Changes::only({leaf->facts}));h.waitForCompletion();
        assert(a->expanded==before && root->value==-1 && h.status().withSubscribers);
        manager.updateIfChanged([&] {middle->rhs={leaf};return true;},Changes::only({middle->facts}));h.waitForCompletion();
        assert(root->value==9 && !h.status().failed);++cases;
    }
    {
        // Collected findings reset rather than appending stale evidence when
        // one contributor changes under a scoped publication.
        struct WaitingSum : CollectedGoal {
            using CollectedGoal::CollectedGoal;
            bool symbolic() const override {return true;}
            std::vector<SetCommand> found(const Visit<Node>& visit) override {
                ReadScope::observe(visit.query.node->facts);
                return CollectedGoal::found(visit);
            }
            CommandBatch<Node> finishPass() override {
                ReadScope::observe(request.node->facts);
                auto batch=CollectedGoal::finishPass();
                for(auto& command:batch.commands) command.changes=Changes::only({request.node->facts});
                return batch;
            }
        };
        struct SumAccess : TrackedAccess {
            std::shared_ptr<int> publications=std::make_shared<int>(0);
            std::shared_ptr<GoalSpec<Node>> goal(const Query<Node>& q) override {
                return std::make_shared<WaitingSum>(q,publications);
            }
        };
        auto a=std::make_shared<SumAccess>();GraphManager<Node> manager(a);
        auto root=node(),left=node(2),right=node(3);root->rhs={left,right};
        auto h=manager.normalize({root,1});h.waitForCompletion();assert(root->value==5 && h.status().withSubscribers);
        manager.updateIfChanged([&] {right->value=4;return true;},Changes::only({right->facts}));h.waitForCompletion();
        assert(root->value==6 && !h.status().failed && *a->publications==2);h.cancel();++cases;
    }
    {
        // Broad fallback still wakes tracked readers. One publication may
        // contain both scoped and unclassified writes.
        auto a=std::make_shared<TrackedAccess>();GraphManager<Node> manager(a);
        auto root=node(),other=node();auto h=manager.normalize({root,1});h.waitForCompletion();
        {auto p=manager.beginPublication();other->value=2;p->changed(Changes::only({other->facts}));root->value=8;p->changed();}
        h.waitForCompletion();assert(root->value==8 && !h.status().symbolicValues);++cases;
    }
    {
        // Shared required children remain one subgoal per request, with its
        // dependency retained by both paths through the refinement graph.
        auto a=std::make_shared<TrackedAccess>();GraphManager<Node> manager(a);
        auto root=node(1),left=node(1),right=node(1),child=node();
        root->required={{left,0},{right,0}};left->required={{child,0}};right->required={{child,0}};
        auto h=manager.normalize({root,1});h.waitForCompletion();assert(h.status().withSubscribers);
        manager.updateIfChanged([&] {child->value=1;return true;},Changes::only({child->facts}));h.waitForCompletion();
        assert(!h.status().symbolicValues && child->complete && left->complete && right->complete);++cases;
    }
    {
        // Scope does not weaken lock handoff: a producer racing a read/subscribe
        // step must either be observed initially or wake the waiting search.
        for(int i=0;i<100;++i) {
            auto a=std::make_shared<TrackedAccess>();GraphManager<Node> manager(a);auto root=node();
            auto h=manager.normalize({root,1});
            auto writer=std::async(std::launch::async,[&] {
                auto p=manager.beginPublication();root->value=7;p->changed(Changes::only({root->facts}));
            });
            writer.get();h.waitForCompletion();assert(!h.status().symbolicValues && root->value==7);
        }
        ++cases;
    }
    {
        // Cancellation releases execution even if its dependencies later change.
        auto a=std::make_shared<TrackedAccess>();GraphManager<Node> manager(a);auto root=node();
        auto h=manager.normalize({root,1});h.waitForCompletion();h.cancel();auto before=h.steps().size();
        manager.updateIfChanged([&] {root->value=1;return true;},Changes::only({root->facts}));
        assert(h.status().cancelled && h.steps().size()==before);++cases;
    }
    {
        // Partly converted readers can explicitly retain a broad dependency.
        struct MixedAccess : TrackedAccess {
            Expansion<Node> expand(const Query<Node>& q) override {
                ReadScope::observe({});return TrackedAccess::expand(q);
            }
        };
        auto a=std::make_shared<MixedAccess>();GraphManager<Node> manager(a);
        auto root=node(),other=node();auto h=manager.normalize({root,1});h.waitForCompletion();
        int before=a->expanded;
        manager.updateIfChanged([&] {other->value=1;return true;},Changes::only({other->facts}));h.waitForCompletion();
        assert(a->expanded==before+1 && h.status().withSubscribers);h.cancel();++cases;
    }
    {
        // Empty footprints and untouched publication batches are not revisions.
        auto a=std::make_shared<TrackedAccess>();GraphManager<Node> manager(a);auto root=node();
        auto h=manager.normalize({root,1});h.waitForCompletion();auto revision=manager.revision();auto count=h.steps().size();
        {auto p=manager.beginPublication();p->changed(Changes::only({}));}
        bool metadata=false;
        manager.updateIfChanged([&] {metadata=true;return true;},Changes::only({}));h.waitForCompletion();
        assert(metadata && manager.revision()==revision && h.steps().size()==count);h.cancel();++cases;
    }
    {
        // A publication between visits must preserve an unrelated active
        // frontier, not just avoid waking an already quiescent subscription.
        struct PublishingGoal : TrackedGoal {
            Node unrelated;bool offered=false;
            PublishingGoal(Query<Node> q,Node other):TrackedGoal(q),unrelated(std::move(other)) {}
            std::vector<SetCommand> found(const Visit<Node>& visit) override {
                auto commands=TrackedGoal::found(visit);
                if(!offered) {
                    offered=true;auto other=unrelated;
                    commands.push_back({"publish independent fact",[other] {other->value=9;return true;},
                        {0,0},Changes::only({other->facts})});
                }
                return commands;
            }
        };
        struct PublishingAccess : TrackedAccess {
            Node other=node();
            std::shared_ptr<GoalSpec<Node>> goal(const Query<Node>& q) override {
                return std::make_shared<PublishingGoal>(q,other);
            }
        };
        auto a=std::make_shared<PublishingAccess>();GraphManager<Node> manager(a);
        auto root=node(7),middle=node(7),last=node(7);root->rhs={middle};middle->rhs={last};
        auto h=manager.normalize({root,1});h.waitForCompletion();int passes=0;
        for(const auto& step:h.steps()) passes+=step.action=="search";
        assert(!h.status().failed && passes==1 && a->expanded==3 && a->other->value==9);++cases;
    }
    {
        // Retaining a dependency token does not retain the model/search it
        // once notified. A later write cleans up the expired weak observer.
        auto a=std::make_shared<TrackedAccess>();GraphManager<Node> manager(a);
        auto root=node();auto token=root->facts;std::weak_ptr<TestNode> weak=root;
        {auto h=manager.normalize({root,1});h.waitForCompletion();}
        root.reset();
        auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(2);
        while(!weak.expired() && std::chrono::steady_clock::now()<deadline) std::this_thread::yield();
        assert(weak.expired());
        manager.updateIfChanged([] {return true;},Changes::only({token}));++cases;
    }
    {
        // One normalization with many independent members used to rebuild its
        // root and every earlier member for each Set. Local writes now retry
        // just the member that consumed that fact.
        for(int count:{16,32,64,128}) {
            auto a=std::make_shared<TrackedAccess>();GraphManager<Node> manager(a);auto root=node(1);
            for(int i=0;i<count;++i) {auto member=node();member->rhs={node(i)};root->required.push_back({member,0});}
            auto h=manager.normalize({root,1});h.waitForCompletion();
            assert(!h.status().failed && !h.status().symbolicValues && a->expanded==4*count+1);
            for(int i=0;i<count;++i) assert(root->required[i].first->value==i);
            std::cout<<"Shared search: "<<count<<" writes, "<<a->expanded<<" expansions\n";
        }
        ++cases;
    }
    {
        // A delayed Set on a removed route must be parked. If that exact
        // demand becomes relevant again, its unchanged work can resume.
        struct RemovingGoal : TrackedGoal {
            Node root, waiting;std::shared_ptr<bool> removed;
            RemovingGoal(Query<Node> q,Node r,Node w,std::shared_ptr<bool> done)
                :TrackedGoal(q),root(r),waiting(w),removed(done) {}
            std::vector<SetCommand> found(const Visit<Node>& visit) override {
                auto commands=TrackedGoal::found(visit);
                if(request.node==root && !*removed) {
                    auto r=root,w=waiting;auto done=removed;
                    commands.push_back({"remove candidate",[r,w,done] {
                        *done=true;r->required={{w,0}};
                        return SetResult{true,Changes::only({r->facts})};
                    },{5,0}});
                }
                return commands;
            }
        };
        struct RemovingAccess : TrackedAccess {
            Node root,waiting;std::shared_ptr<bool> removed=std::make_shared<bool>(false);
            std::shared_ptr<GoalSpec<Node>> goal(const Query<Node>& q) override {
                return std::make_shared<RemovingGoal>(q,root,waiting,removed);
            }
        };
        auto a=std::make_shared<RemovingAccess>();GraphManager<Node> manager(a);
        auto branch=node(),source=node(9);source->setCreations=100;branch->rhs={source};
        a->root=node(1);a->waiting=node();a->root->required={{branch,0},{a->waiting,0}};
        auto h=manager.normalize({a->root,1});h.waitForCompletion();
        assert(h.status().withSubscribers && *a->removed && branch->value==-1);
        manager.updateIfChanged([&] {a->root->required.push_back({branch,0});return true;},Changes::only({a->root->facts}));
        h.waitForCompletion();assert(branch->value==9 && h.status().withSubscribers && !h.status().failed);h.cancel();++cases;
    }
    {
        // A cached subgoal cannot retain the price of a route that disappeared.
        auto a=std::make_shared<TrackedAccess>();GraphManager<Node> manager(a);
        auto root=node(1),cheap=node(1),expensive=node(1),shared=node();
        cheap->required={{shared,1}};expensive->required={{shared,10}};
        root->required={{cheap,0},{expensive,0}};CostPolicy policy;policy.maximumRoute=5;
        auto h=manager.normalize({root,1},policy);h.waitForCompletion();
        manager.updateIfChanged([&] {root->required={{expensive,0}};return true;},Changes::only({root->facts}));h.waitForCompletion();
        manager.updateIfChanged([&] {shared->rhs={node(7)};return true;},Changes::only({shared->facts}));h.waitForCompletion();
        assert(shared->value==-1 && h.status().withSubscribers);
        manager.updateIfChanged([&] {root->required.push_back({cheap,0});return true;},Changes::only({root->facts}));h.waitForCompletion();
        assert(shared->value==7 && !h.status().failed && !h.status().symbolicValues);++cases;
    }
    {
        // A failed required child rejects its exploratory parent. Repairing
        // that child must reopen the parent's derived rejection as well.
        auto a=std::make_shared<TrackedAccess>();GraphManager<Node> manager(a);
        auto root=node(),branch=node(3),bad=node();bad->rhs={node(1),node(2)};
        root->exploratory={{branch,0}};branch->required={{bad,0}};
        auto h=manager.normalize({root,1});h.waitForCompletion();
        assert(!h.status().failed && !branch->complete);
        manager.updateIfChanged([&] {bad->rhs.resize(1);return true;},Changes::only({bad->facts}));h.waitForCompletion();
        assert(!h.status().failed && branch->complete && bad->complete);h.cancel();++cases;
    }
    {
        struct EvidenceGoal : TrackedGoal {
            std::shared_ptr<std::map<Node,DependencyProof::Ptr>> saved;
            DependencyKey finalRead;
            EvidenceGoal(Query<Node> q,decltype(saved) p,DependencyKey k):TrackedGoal(q),saved(p),finalRead(k) {}
            void settled(bool complete,const DependencyProof::Ptr& proof) override {
                TrackedGoal::settled(complete);if(!complete) return;
                ReadScope::observe(finalRead); // includes reads made during settlement
                (*saved)[request.node]=proof;std::weak_ptr<TestNode> weak=request.node;
                proof->onInvalidated([weak] {if(auto node=weak.lock()) node->complete=false;});
            }
        };
        struct EvidenceAccess : TrackedAccess {
            std::shared_ptr<std::map<Node,DependencyProof::Ptr>> saved=std::make_shared<std::map<Node,DependencyProof::Ptr>>();
            DependencyKey finalRead=std::make_shared<Dependency>();
            std::shared_ptr<GoalSpec<Node>> goal(const Query<Node>& q) override {
                return std::make_shared<EvidenceGoal>(q,saved,finalRead);
            }
        };
        auto access=std::make_shared<EvidenceAccess>();GraphManager<Node> manager(access);
        auto root=node(1),child=node(2);root->required={{child,0}};
        auto normalize=[&] {auto h=manager.normalize({root,1});h.waitForCompletion();assert(root->complete);};
        normalize();auto rootProof=access->saved->at(root);auto childProof=access->saved->at(child);
        access->saved->erase(child);childProof.reset(); // parent retains required evidence after all handles retire
        manager.updateIfChanged([] {return true;},Changes::only({node()->facts}));assert(rootProof->valid() && root->complete);
        manager.updateIfChanged([] {return true;},Changes::only({child->facts}));
        assert(!rootProof->valid() && !root->complete && !child->complete);++cases;
        normalize();rootProof=access->saved->at(root);
        manager.updateIfChanged([] {return true;},Changes::only({access->finalRead}));
        assert(!rootProof->valid() && !root->complete);++cases;
        normalize();rootProof=access->saved->at(root);
        manager.update([] {});assert(!rootProof->valid() && !root->complete);
        std::weak_ptr<DependencyProof> weak=rootProof;rootProof.reset();access->saved->clear();assert(weak.expired());++cases;
    }
    {
        // A cache replacement must not disconnect an in-flight reader from
        // the certificate it actually used, or from that certificate's inputs.
        struct Cache {
            DependencyProof::Ptr current,result;
            Node root;
        };
        struct CacheGoal : TrackedGoal {
            std::shared_ptr<Cache> cache;
            CacheGoal(Query<Node> q,std::shared_ptr<Cache> c):TrackedGoal(q),cache(c) {}
            std::vector<SetCommand> found(const Visit<Node>& visit) override {
                if(request.node==cache->root && visit.query.node==request.node) assert(cache->current->valid());
                return TrackedGoal::found(visit);
            }
            void settled(bool complete,const DependencyProof::Ptr& proof) override {
                TrackedGoal::settled(complete);
                if(complete && request.node==cache->root) cache->result=proof;
            }
        };
        struct CacheAccess : TrackedAccess {
            std::shared_ptr<Cache> cache=std::make_shared<Cache>();
            std::shared_ptr<GoalSpec<Node>> goal(const Query<Node>& q) override {return std::make_shared<CacheGoal>(q,cache);}
        };
        auto access=std::make_shared<CacheAccess>();GraphManager<Node> manager(access);
        auto input=std::make_shared<Dependency>(),other=std::make_shared<Dependency>();
        access->cache->current=DependencyProof::make({input});
        std::weak_ptr<DependencyProof> original=access->cache->current;
        auto root=node(1),waiting=node();root->required={{waiting,0}};access->cache->root=root;
        {
            auto h=manager.normalize({root,1});h.waitForCompletion();assert(h.status().withSubscribers);
            manager.read([&] {access->cache->current=DependencyProof::make({other});});
            assert(!original.expired());
            manager.updateIfChanged([&] {waiting->value=2;return true;},Changes::only({waiting->facts}));
            h.waitForCompletion();assert(!h.status().symbolicValues && access->cache->result->valid());
        }
        assert(!original.expired());
        manager.updateIfChanged([] {return true;},Changes::only({input}));
        assert(!access->cache->result->valid());
        access->cache->result.reset();assert(original.expired());++cases;
    }
    std::cout << "Graph search: " << cases << "/" << cases << " scenarios passed\n";
}
