#pragma once

// Domain-independent scheduling and traversal. Node is an opaque domain handle;
// no shadow graph is constructed. All callbacks run under the context mutex and
// must be bounded, nonblocking operations. I/O is represented by subscriptions.
#include <algorithm>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

namespace proteus4 {

using Cost = std::uint64_t;
inline Cost addCost(Cost a, Cost b) {
    return b > std::numeric_limits<Cost>::max() - a
        ? std::numeric_limits<Cost>::max() : a + b;
}

// Structural estimates, not prices supplied by a semantic feature.
struct Work { Cost visits = 1; Cost creations = 0; };
struct CostPolicy {
    Cost visit = 1, creation = 4;
    Cost maximumRoute = std::numeric_limits<Cost>::max();
    Cost estimate(Work work) const {
        auto product = [](Cost a, Cost b) {
            return b && a > std::numeric_limits<Cost>::max() / b
                ? std::numeric_limits<Cost>::max() : a * b;
        };
        return addCost(product(visit, work.visits), product(creation, work.creations));
    }
};

template<class Node> struct Query {
    Node node{};
    int kind = 0;
    std::int64_t argument = 0;
    // Optional second endpoint for a relational demand. Both endpoints remain
    // opaque to the scheduler and participate in goal/visit identity.
    Node related{};
    // Domain traversal context (for example, return points through embedded
    // spans). These are existing nodes, not replacement graph vertices.
    std::vector<Node> context{};
    bool operator<(const Query& other) const {
        return std::tie(node, kind, argument, related, context) <
               std::tie(other.node, other.kind, other.argument, other.related, other.context);
    }
};

template<class Node> struct Visit {
    Query<Node> query;
    std::vector<Query<Node>> path;
    Cost cost = 0;
};

struct SetCommand {
    std::string description;
    // Validate the entire batch before its first write. true means graph change.
    std::function<bool()> apply;
    Work work{0, 0};
};

template<class Node> class GoalSpec {
public:
    Query<Node> request;
    explicit GoalSpec(Query<Node> q) : request(std::move(q)) {}
    virtual ~GoalSpec() = default;
    virtual void beginPass() {}
    // Publish completion metadata only, after this goal and its required
    // subgoals have been examined. Independent children can complete separately.
    // This callback must not change semantic information or graph adjacency.
    virtual void settled(bool fullyResolved) { (void)fullyResolved; }
    virtual std::vector<SetCommand> found(const Visit<Node>&) = 0;
    // Only alternative routes may be pruned after satisfaction. Constraints
    // remain searchable even when every requested field is already concrete.
    virtual bool satisfied() const = 0;
    virtual bool symbolic() const = 0;
    virtual Node result() const { return request.node; }
};

template<class Node> struct Arrival {
    Query<Node> query;
    bool graphChanged = false;
    Arrival(Query<Node> q) : query(std::move(q)) {}
    Arrival(Query<Node> q, bool changed) : query(std::move(q)), graphChanged(changed) {}
};

template<class Node> struct Edge {
    std::string description;
    Work work;
    bool alternative = false;
    // Deferred until selected by cost. Enumerating an edge must not perform its
    // expensive materialization. The returned query can carry a new demand.
    std::function<Arrival<Node>()> follow;
    // Lower bound on work after this step (e.g. a remaining list walk). This
    // ranks routes before expansion, without charging the estimate twice.
    Work remaining{0, 0};
    bool command = false;
};

template<class Node> struct Refinement {
    Query<Node> query;
    Work work;
    std::string description;
    // false makes completion of this subgoal a requirement of its parent.
    bool alternative = true;
};

template<class Node> struct Expansion {
    std::vector<Edge<Node>> edges;
    std::vector<Refinement<Node>> refinements;
    bool deferred = false;
};

template<class Node> class GraphAccess {
public:
    virtual ~GraphAccess() = default;
    // Called under the context lock for every semantic publication, including
    // Sets and lazy materialization. Retired sessions may have left metadata.
    virtual void invalidateCompletion() {}
    virtual Expansion<Node> expand(const Query<Node>&) = 0;
    virtual std::shared_ptr<GoalSpec<Node>> goal(const Query<Node>&) = 0;
};

struct Status {
    bool fullyNormed = false;
    bool withSubscribers = false;
    bool symbolicValues = true;
    bool cancelled = false;
    bool failed = false;
    std::uint64_t revision = 0;
    std::string error;
};

template<class Node> struct Step {
    std::string action;
    Query<Node> query;
    Cost cost = 0;
    std::uint64_t revision = 0;
};

template<class Node> struct SearchSession {
    struct Task {
        std::shared_ptr<GoalSpec<Node>> goal;
        Cost baseCost = 0;
        std::map<Query<Node>, Cost> visited;
        std::vector<std::size_t> required;
        bool deferred = false;
        std::uint64_t generation = 0;
    };
    struct Pending {
        std::size_t task = 0;
        Cost cost = 0;
        Cost estimate = 0;
        std::uint64_t sequence = 0;
        Edge<Node> edge;
        std::vector<Query<Node>> path;
        std::uint64_t generation = 0;
    };
    struct Later {
        bool operator()(const Pending& a, const Pending& b) const {
            return std::tie(a.estimate, a.sequence) > std::tie(b.estimate, b.sequence);
        }
    };
    std::vector<Task> tasks;
    std::priority_queue<Pending, std::vector<Pending>, Later> frontier;
    std::shared_ptr<GoalSpec<Node>> root;
    CostPolicy policy;
    Status status;
    std::vector<Step<Node>> steps;
    bool quiescent = false, terminal = false, restart = true;
    bool watch = true;
    std::uint64_t sequence = 0;
};

template<class Node> struct Context {
    std::mutex mutex, joinMutex;
    std::condition_variable changed;
    std::thread worker;
    bool stopping = false;
    std::uint64_t revision = 0;
    std::size_t cursor = 0;
    std::shared_ptr<GraphAccess<Node>> access;
    std::vector<std::weak_ptr<SearchSession<Node>>> sessions;

    void graphChanged() {
        access->invalidateCompletion();
        ++revision;
        // Conservative context-wide invalidation first. A dependency index can
        // narrow this later without changing the subscribe/publish contract.
        for (auto& weak : sessions) if (auto session = weak.lock()) {
            if (!session->terminal &&
                (!session->quiescent || session->status.withSubscribers)) {
                session->restart = true;
                session->quiescent = false;
                session->status.fullyNormed = false;
                session->status.withSubscribers = false;
            }
        }
        changed.notify_all();
    }
};

template<class Node> class NormalizationHandle {
    std::shared_ptr<Context<Node>> context_;
    std::shared_ptr<SearchSession<Node>> session_;
public:
    NormalizationHandle() = default;
    NormalizationHandle(std::shared_ptr<Context<Node>> c,
                        std::shared_ptr<SearchSession<Node>> s)
        : context_(std::move(c)), session_(std::move(s)) {}
    void waitForCompletion() const {
        if (!session_) return;
        std::unique_lock<std::mutex> lock(context_->mutex);
        context_->changed.wait(lock, [&] {
            return session_->quiescent || session_->terminal;
        });
    }
    void cancel() const {
        if (!session_) return;
        std::lock_guard<std::mutex> lock(context_->mutex);
        if (session_->terminal) return;
        session_->terminal = true;
        session_->status.cancelled = true;
        session_->status.fullyNormed = false;
        session_->status.withSubscribers = false;
        session_->frontier = {};
        session_->steps.push_back({"cancel", session_->root->request, 0,
                                  context_->revision});
        context_->changed.notify_all();
    }
    Status status() const {
        if (!session_) return {};
        std::lock_guard<std::mutex> lock(context_->mutex);
        return session_->status;
    }
    std::vector<Step<Node>> steps() const {
        if (!session_) return {};
        std::lock_guard<std::mutex> lock(context_->mutex);
        return session_->steps;
    }
    Node result() const {
        if (!session_) return {};
        std::lock_guard<std::mutex> lock(context_->mutex);
        return session_->root->result();
    }
};

template<class Node> class GraphManager {
    using Session = SearchSession<Node>;
    std::shared_ptr<Context<Node>> context_;

    static void enqueue(Session& s, std::size_t task, Cost base,
                        Edge<Node> edge, std::vector<Query<Node>> path) {
        if (edge.alternative && s.tasks[task].goal->satisfied()) return;
        Cost cost = addCost(base, s.policy.estimate(edge.work));
        Cost estimate = addCost(cost, s.policy.estimate(edge.remaining));
        if (estimate > s.policy.maximumRoute) {
            s.tasks[task].deferred = true;
            s.steps.push_back({"exclude: " + edge.description,
                              s.tasks[task].goal->request, estimate, s.status.revision});
            return;
        }
        s.frontier.push({task, cost, estimate, s.sequence++, std::move(edge),
                         std::move(path), s.tasks[task].generation});
    }
    static std::size_t startTask(Session& s, std::shared_ptr<GoalSpec<Node>> goal, Cost cost) {
        auto query = goal->request;
        auto index = s.tasks.size();
        for (std::size_t i = 0; i < s.tasks.size(); ++i) {
            auto& task = s.tasks[i];
            const auto& existing = task.goal->request;
            if (!(query < existing) && !(existing < query)) {
                if (task.baseCost <= cost) return i;
                // A shared subgoal may first be advertised by a costly route.
                // Retry at the cheaper price and discard its old queued steps.
                index = i;
                task.baseCost = cost;
                task.visited.clear();
                task.required.clear();
                task.deferred = false;
                ++task.generation;
                goal = task.goal;
                break;
            }
        }
        goal->beginPass();
        if (index == s.tasks.size()) s.tasks.push_back({std::move(goal), cost, {}, {}, false, 0});
        enqueue(s, index, cost, {"visit", {0, 0}, false,
                                 [query] { return query; }}, {});
        return index;
    }
    static void settle(Session& s) {
        // Resolve the dependency DAG from its leaves. Cycles cannot establish
        // their own completion; they stay symbolic until other evidence breaks
        // the dependency. No recursive calls or domain-specific fields here.
        std::vector<std::vector<std::size_t>> parents(s.tasks.size());
        std::vector<std::size_t> remaining(s.tasks.size());
        std::vector<bool> resolved(s.tasks.size(), false), local(s.tasks.size());
        std::queue<std::size_t> ready;
        for (std::size_t i = 0; i < s.tasks.size(); ++i) {
            const auto& task = s.tasks[i];
            local[i] = !task.deferred && !task.goal->symbolic();
            remaining[i] = task.required.size();
            for (auto child : task.required) parents[child].push_back(i);
            if (local[i] && !remaining[i]) ready.push(i);
        }
        while (!ready.empty()) {
            auto i = ready.front(); ready.pop();
            resolved[i] = true;
            s.tasks[i].goal->settled(true);
            for (auto parent : parents[i])
                if (!--remaining[parent] && local[parent]) ready.push(parent);
        }
        for (std::size_t i = 0; i < s.tasks.size(); ++i)
            if (!resolved[i]) s.tasks[i].goal->settled(false);
        s.status.symbolicValues = !resolved[0];
    }
    static void advance(Context<Node>& c, Session& s) {
        if (s.restart) {
            s.restart = false;
            s.frontier = {};
            s.tasks.clear();
            s.status.revision = c.revision;
            startTask(s, s.root, 0);
            s.steps.push_back({"search", s.root->request, 0, c.revision});
        }
        if (s.frontier.empty()) {
            settle(s);
            s.status.fullyNormed = true;
            s.status.withSubscribers = s.watch && s.status.symbolicValues;
            s.quiescent = true;
            s.terminal = !s.status.withSubscribers;
            s.steps.push_back({s.status.withSubscribers ? "subscribe" : "settle",
                               s.root->request, 0, c.revision});
            c.changed.notify_all();
            return;
        }
        auto pending = s.frontier.top();
        s.frontier.pop();
        if (pending.generation != s.tasks[pending.task].generation) return;
        auto goal = s.tasks[pending.task].goal;
        if (pending.edge.alternative && goal->satisfied()) return;
        auto arrival = pending.edge.follow();
        auto query = arrival.query;
        if (arrival.graphChanged) {
            s.steps.push_back({pending.edge.command ? pending.edge.description : "materialize: " + pending.edge.description,
                               query, pending.cost, c.revision});
            c.graphChanged();
            return;
        }
        if (pending.edge.command) return; // an unchanged Set has no new finding
        if (!query.node) { s.tasks[pending.task].deferred = true; return; }
        auto& visited = s.tasks[pending.task].visited;
        auto previous = visited.find(query);
        if (previous != visited.end() && previous->second <= pending.cost) return;
        visited[query] = pending.cost;
        pending.path.push_back(query);
        s.steps.push_back({pending.edge.description, query, pending.cost, c.revision});
        Visit<Node> visit{query, pending.path, pending.cost};
        for (auto& command : goal->found(visit)) {
            Edge<Node> operation{command.description, command.work, false,
                [query, apply = std::move(command.apply)] { return Arrival<Node>{query, apply()}; }};
            operation.command = true;
            enqueue(s, pending.task, pending.cost, std::move(operation), pending.path);
        }
        auto expansion = c.access->expand(query);
        s.tasks[pending.task].deferred |= expansion.deferred;
        for (auto& edge : expansion.edges)
            enqueue(s, pending.task, pending.cost, std::move(edge), pending.path);
        for (const auto& refinement : expansion.refinements) {
            if (refinement.alternative && goal->satisfied()) continue;
            Cost cost = addCost(pending.cost, s.policy.estimate(refinement.work));
            if (cost > s.policy.maximumRoute && refinement.alternative) {
                s.steps.push_back({"exclude: " + refinement.description,
                                   refinement.query, cost, c.revision});
                continue;
            }
            // Keep even an excluded required query as an obligation. Another
            // route may reach that same subgoal cheaply enough to satisfy it.
            s.steps.push_back({"subgoal: " + refinement.description,
                               refinement.query, cost, c.revision});
            auto child = startTask(s, c.access->goal(refinement.query), cost);
            if (!refinement.alternative) {
                auto& required = s.tasks[pending.task].required;
                if (std::find(required.begin(), required.end(), child) == required.end())
                    required.push_back(child);
            }
        }
    }
    static void run(Context<Node>* c) {
        std::unique_lock<std::mutex> lock(c->mutex);
        while (!c->stopping) {
            std::shared_ptr<Session> next;
            auto count = c->sessions.size();
            for (std::size_t i = 0; i < count; ++i) {
                c->cursor %= count;
                auto candidate = c->sessions[c->cursor++].lock();
                if (candidate && !candidate->quiescent && !candidate->terminal) {
                    next = std::move(candidate); break;
                }
            }
            if (!next) { c->changed.wait(lock); continue; }
            try { advance(*c, *next); }
            catch (const std::exception& error) {
                next->terminal = true;
                next->status.failed = true;
                next->status.fullyNormed = false;
                next->status.withSubscribers = false;
                next->status.error = error.what();
                next->steps.push_back({"failed: " + next->status.error,
                                       next->root->request, 0, c->revision});
                c->changed.notify_all();
            }
            // Cancellation and external publication can enter between bounded
            // steps, never during a Set batch or a graph callback.
            next.reset();
            lock.unlock();
            std::this_thread::yield();
            lock.lock();
        }
    }
public:
    GraphManager() = default;
    explicit GraphManager(std::shared_ptr<GraphAccess<Node>> access) { init(std::move(access)); }
    GraphManager(const GraphManager&) = delete;
    GraphManager& operator=(const GraphManager&) = delete;
    ~GraphManager() { close(); }
    void init(std::shared_ptr<GraphAccess<Node>> access) {
        if (context_) throw std::logic_error("graph context already initialized");
        if (!access) throw std::invalid_argument("graph access required");
        context_ = std::make_shared<Context<Node>>();
        context_->access = std::move(access);
        context_->worker = std::thread(run, context_.get());
    }
    NormalizationHandle<Node> normalize(Query<Node> query, CostPolicy policy = {},
                                        bool watch = true) {
        if (!context_) throw std::logic_error("initialize graph context first");
        std::lock_guard<std::mutex> lock(context_->mutex);
        if (context_->stopping) throw std::logic_error("graph context is closed");
        if (!query.node) throw std::invalid_argument("normalize requires a node");
        auto session = std::make_shared<Session>();
        session->root = context_->access->goal(query);
        session->policy = policy;
        session->watch = watch;
        auto& sessions = context_->sessions;
        sessions.erase(std::remove_if(sessions.begin(), sessions.end(),
                       [](const std::weak_ptr<Session>& weak) { return weak.expired(); }), sessions.end());
        sessions.push_back(session);
        context_->changed.notify_all();
        return {context_, std::move(session)};
    }
    // Writers use this boundary so publication and subscription installation
    // cannot miss each other. Readers of a live result use read() as well.
    template<class Function> auto read(Function action) const -> decltype(action()) {
        if (!context_) throw std::logic_error("initialize graph context first");
        std::lock_guard<std::mutex> lock(context_->mutex);
        return action();
    }
    template<class Function> void update(Function action) {
        if (!context_) throw std::logic_error("initialize graph context first");
        std::lock_guard<std::mutex> lock(context_->mutex);
        if (context_->stopping) throw std::logic_error("graph context is closed");
        action();
        context_->graphChanged();
    }
    void close() {
        if (!context_) return;
        std::lock_guard<std::mutex> joinLock(context_->joinMutex);
        {
            std::lock_guard<std::mutex> lock(context_->mutex);
            context_->stopping = true;
            for (auto& weak : context_->sessions) if (auto s = weak.lock()) {
                if (!s->terminal) {
                    s->terminal = true;
                    s->status.cancelled = true;
                    s->status.fullyNormed = false;
                    s->status.withSubscribers = false;
                    s->frontier = {};
                }
            }
            context_->changed.notify_all();
        }
        if (context_->worker.joinable()) context_->worker.join();
    }
};

} // namespace proteus4
