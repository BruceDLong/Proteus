#pragma once

// Domain-independent scheduling and traversal. Node is an opaque domain handle;
// no shadow graph is constructed. All callbacks run under the context mutex and
// must be bounded, nonblocking operations. I/O is represented by subscriptions.
#include "GraphDependencies.h"
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <optional>
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
    // Coordinates of a domain cursor (for example counts at nested list
    // boundaries). These distinguish search states, not semantic graph nodes.
    std::vector<std::int64_t> coordinates{};
    bool operator<(const Query& other) const {
        return std::tie(node, kind, argument, related, context, coordinates) <
               std::tie(other.node, other.kind, other.argument, other.related, other.context, other.coordinates);
    }
};

template<class Node> struct Visit {
    Query<Node> query;
    std::vector<Query<Node>> path;
    Cost cost = 0;
};

// A compound operation can report its actual effects after execution. Plain
// boolean producers keep the command's declared (by default broad) footprint.
struct SetResult {
    bool changed;
    std::optional<Changes> changes;
    SetResult(bool value):changed(value) {}
    SetResult(bool value, Changes effects):changed(value),changes(std::move(effects)) {}
    operator bool() const {return changed;}
};

struct SetCommand {
    std::string description;
    // Validate the entire batch before its first write. true means graph change.
    std::function<SetResult()> apply;
    Work work{0, 0};
    Changes changes;
};

// A proved contradiction belongs to one goal, not to the source node or the
// entire search. Required parents inherit rejection; exploratory parents do
// not. Other exceptions still report engine/input failures for the session.
class GoalRejected : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// A collected result retains the finding's route cost and trace when its Set
// must wait until the current frontier has been examined.
template<class Node> struct CommandBatch {
    Visit<Node> finding;
    std::vector<SetCommand> commands;
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
    // Called once after this pass's queued work, before completion metadata.
    // Return priced Sets; do not mutate graph state from this callback.
    virtual CommandBatch<Node> finishPass() { return {}; }
    // Only alternative routes may be pruned after satisfaction. Constraints
    // remain searchable even when every requested field is already concrete.
    virtual bool satisfied() const = 0;
    virtual bool symbolic() const = 0;
    virtual Node result() const { return request.node; }
};

template<class Node> struct Arrival {
    Query<Node> query;
    bool graphChanged = false;
    Changes changes;
    Arrival(Query<Node> q) : query(std::move(q)) {}
    Arrival(Query<Node> q, bool changed, Changes footprint = {})
        : query(std::move(q)), graphChanged(changed), changes(std::move(footprint)) {}
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
    // A tracked adapter must invalidate completion evidence with the same
    // precision, or include the resulting metadata changes in its footprint.
    virtual void invalidateCompletion(const Changes&) { invalidateCompletion(); }
    // Opt in only when all semantic callbacks report their reads. Mixed
    // implementations must observe({}) in helpers with unknown read sets.
    virtual bool tracksDependencies() const { return false; }
    virtual void observeTask(const Query<Node>&) {
        if(!tracksDependencies()) ReadScope::observe({});
    }
    // Explicit queries may refresh validation caches. This hook must not alter
    // semantic information, adjacency, or another search's completion state.
    virtual void beginSearch(const Query<Node>&) {}
    // Captured under the context lock; delivered outside it. Effects must not
    // wait for this worker, but may publish new information.
    virtual std::vector<std::function<void()>> takeEffects() { return {}; }
    virtual Expansion<Node> expand(const Query<Node>&) = 0;
    // Construct a specification, not a snapshot of mutable facts. Read those
    // facts in beginPass/found/finishPass/completion callbacks on each pass.
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

template<class Node> struct SearchSession : std::enable_shared_from_this<SearchSession<Node>> {
    struct Pending {
        std::size_t task = 0;
        Cost cost = 0;
        Cost estimate = 0;
        std::uint64_t sequence = 0;
        Edge<Node> edge;
        std::vector<Query<Node>> path;
        std::uint64_t generation = 0;
    };
    struct Task {
        std::shared_ptr<GoalSpec<Node>> goal;
        Cost baseCost = 0;
        std::map<Query<Node>, Cost> visited;
        struct Link {std::size_t child;Cost cost;};
        std::vector<std::size_t> required;
        std::vector<Link> children;
        bool deferred = false;
        std::uint64_t generation = 0;
        bool finishedPass = false;
        std::string rejection;
        bool active = false, dirty = true;
        std::set<DependencyKey> dependencies;
        std::shared_ptr<Dependency::Wake> wake;
        std::vector<Pending> parked;
    };
    struct Later {
        bool operator()(const Pending& a, const Pending& b) const {
            return std::tie(a.estimate, a.sequence) > std::tie(b.estimate, b.sequence);
        }
    };
    std::vector<Task> tasks;
    std::map<Query<Node>,std::size_t> taskIndex;
    std::priority_queue<Pending, std::vector<Pending>, Later> frontier;
    std::shared_ptr<GoalSpec<Node>> root;
    CostPolicy policy;
    Status status;
    std::vector<Step<Node>> steps;
    bool quiescent = false, terminal = false, reconcile = false, prune = false;
    std::set<std::size_t> invalidated, rejected;
    bool watch = true;
    std::uint64_t sequence = 0;
};

template<class Node> struct Context {
    std::mutex mutex, joinMutex;
    std::condition_variable changed;
    std::atomic<std::size_t> waitingPublications{0};
    std::uint64_t publicationTurn=0; // protected by mutex
    std::thread worker;
    bool stopping = false, dispatching = false;
    std::uint64_t revision = 0;
    std::size_t cursor = 0;
    std::shared_ptr<GraphAccess<Node>> access;
    std::vector<std::weak_ptr<SearchSession<Node>>> sessions;

    DependencyKey untracked = std::make_shared<Dependency>();

    static void wakeTask(const std::shared_ptr<SearchSession<Node>>& session, std::size_t index) {
        if(session->terminal) return;
        auto& task=session->tasks[index];
        bool newlyDirty=!task.dirty;
        if(newlyDirty) {task.dirty=true;++task.generation;}
        session->invalidated.insert(index);
        if(task.active) {
            session->prune |= !task.children.empty();
            session->reconcile = true;
            session->quiescent = false;
            session->status.fullyNormed = false;
            session->status.withSubscribers = false;
        }
        // A required child's rejection is also evidence held by its parents.
        // Ordinary completion is recomputed by settle; a propagated rejection
        // must be reopened when that child's evidence changes.
        if(newlyDirty) for(auto parent:session->rejected) {
            const auto& candidate=session->tasks[parent];
            if(!candidate.rejection.empty() &&
               std::find(candidate.required.begin(),candidate.required.end(),index)!=candidate.required.end())
                wakeTask(session,parent);
        }
    }
    void observe(SearchSession<Node>& session, std::size_t index, const DependencyKey& key) {
        auto token=key ? key : untracked;
        auto& task=session.tasks[index];
        if(task.dependencies.insert(token).second) token->subscribe(task.wake);
    }
    void graphChanged(const Changes& changes = {}) {
        if(changes.empty()) return;
        access->invalidateCompletion(changes);
        ++revision;
        if(changes.all) {
            for (auto& weak : sessions) if (auto session = weak.lock())
                for(std::size_t i=0;i<session->tasks.size();++i) wakeTask(session,i);
        } else {
            // Indexed by the evidence actually read. Unrelated sessions retain
            // their frontier, visited queries, accumulated findings and costs.
            untracked->publish();
            for(const auto& key:changes.keys) key->publish();
        }
        changed.notify_all();
    }
};

// A producer owns the same lock as search while editing shared nodes. It
// releases the lock around input waits and publishes only explicit milestones.
template<class Node> class Publication {
    std::shared_ptr<Context<Node>> context_;
    std::unique_lock<std::mutex> lock_;
    Changes changes_ = Changes::only({});
public:
    explicit Publication(std::shared_ptr<Context<Node>> context)
        : context_(std::move(context)), lock_(context_->mutex,std::defer_lock) {
        resume();
        if (context_->stopping) throw std::logic_error("graph context is closed");
    }
    void changed(const Changes& changes = {}) { changes_.include(changes); }
    void flush() {
        if (!changes_.empty()) {
            auto changes=std::move(changes_);changes_=Changes::only({});
            context_->graphChanged(changes);
        }
    }
    void suspend() { flush(); lock_.unlock(); }
    void resume() {
        ++context_->waitingPublications;
        lock_.lock();
        --context_->waitingPublications;
        ++context_->publicationTurn;
        context_->changed.notify_all();
    }
    ~Publication() { if (lock_.owns_lock()) flush(); }
};

template<class Node> class InputPause {
    std::shared_ptr<Publication<Node>> publication_;
public:
    explicit InputPause(std::shared_ptr<Publication<Node>> publication)
        : publication_(std::move(publication)) {
        if (publication_) publication_->suspend();
    }
    ~InputPause() { if (publication_) publication_->resume(); }
};

template<class Node> class NormalizationHandle {
    std::shared_ptr<Context<Node>> context_;
    std::shared_ptr<SearchSession<Node>> session_;
public:
    NormalizationHandle() = default;
    NormalizationHandle(std::shared_ptr<Context<Node>> c,
                        std::shared_ptr<SearchSession<Node>> s)
        : context_(std::move(c)), session_(std::move(s)) {}
    template<class Predicate> void waitUntil(Predicate ready) const {
        if (!session_) return;
        std::unique_lock<std::mutex> lock(context_->mutex);
        context_->changed.wait(lock, [&] {
            return !context_->dispatching &&
                (session_->terminal || (session_->quiescent && ready()));
        });
    }
    void waitForCompletion() const { waitUntil([] { return true; }); }
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
    bool active() const {
        if (!session_) return false;
        std::lock_guard<std::mutex> lock(context_->mutex);
        return !session_->terminal;
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
        if (!s.tasks[task].rejection.empty() ||
            (edge.alternative && s.tasks[task].goal->satisfied())) return;
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
    static void enqueueCommands(Session& s, std::size_t task, const Visit<Node>& finding,
                                std::vector<SetCommand> commands) {
        for (auto& command : commands) {
            Edge<Node> operation{command.description, command.work, false,
                [query=finding.query, apply=std::move(command.apply), changes=std::move(command.changes)] {
                    auto result=apply();return Arrival<Node>{query,result.changed,result.changes.value_or(changes)};
                }};
            operation.command = true;
            enqueue(s, task, finding.cost, std::move(operation), finding.path);
        }
    }
    static ReadScope observeTask(Context<Node>& c, Session& s, std::size_t index) {
        return ReadScope([&c,&s,index](const DependencyKey& key) {c.observe(s,index,key);});
    }
    static void resetTask(Context<Node>& c, Session& s, std::size_t index) {
        auto& task=s.tasks[index];
        s.rejected.erase(index);
        task.dirty=false;task.visited.clear();task.required.clear();task.children.clear();
        task.deferred=false;task.finishedPass=false;task.rejection.clear();task.parked.clear();
        std::weak_ptr<Session> weak=s.weak_from_this();
        task.wake=std::make_shared<Dependency::Wake>([weak,index] {
            if(auto current=weak.lock()) Context<Node>::wakeTask(current,index);
        });
        task.dependencies.clear();
        auto reads=observeTask(c,s,index);c.access->observeTask(task.goal->request);
        task.goal->beginPass();
        auto query=task.goal->request;
        enqueue(s,index,task.baseCost,{"visit",{0,0},false,[query] {return query;}},{});
        s.steps.push_back({index ? "resume subgoal" : "search",query,task.baseCost,c.revision});
    }
    // A dirty parent's old outgoing demands cannot authorize work. Clean shared
    // children remain reusable if another current route reaches them. Parked
    // work retains its price and sequence; abandoned branches cannot write.
    static void activate(Context<Node>& c, Session& s, std::size_t first, Cost cost) {
        std::vector<typename Session::Task::Link> pending{{first,cost}};
        while(!pending.empty()) {
            auto link=pending.back();pending.pop_back();auto index=link.child;
            auto& task=s.tasks[index];
            if(!task.dirty && !task.rejection.empty()) {task.active=true;continue;}
            if((!task.active && task.baseCost!=link.cost) || (task.active && link.cost<task.baseCost)) {
                // A removed cheap route cannot subsidize later expensive work.
                task.baseCost=link.cost;task.dirty=true;++task.generation;
                if(task.active) {s.reconcile=true;s.prune=true;}
            }
            if(task.active && !task.dirty) continue;
            task.active=true;
            if(task.dirty) resetTask(c,s,index);
            else {
                for(auto& item:task.parked) if(item.generation==task.generation) s.frontier.push(std::move(item));
                task.parked.clear();
                pending.insert(pending.end(),task.children.begin(),task.children.end());
            }
        }
    }
    static void reconcileTasks(Context<Node>& c, Session& s) {
        // Ignore outgoing demands of dirty tasks. Reprice shared goals from
        // surviving routes before scheduling any work; otherwise removal of a
        // cheap parent could leave a stale discount on an expensive route.
        std::map<std::size_t,Cost> reached;
        bool repriced;
        do {
            reached.clear();reached[0]=0;
            std::queue<std::size_t> pending;pending.push(0);
            while(!pending.empty()) {
                auto index=pending.front();pending.pop();
                if(s.tasks[index].dirty) continue;
                for(const auto& link:s.tasks[index].children) {
                    auto old=reached.find(link.child);
                    if(old==reached.end() || link.cost<old->second) {
                        reached[link.child]=link.cost;pending.push(link.child);
                    }
                }
            }
            repriced=false;
            for(const auto& entry:reached) {
                auto& task=s.tasks[entry.first];
                if(task.baseCost==entry.second || (!task.dirty && !task.rejection.empty())) continue;
                task.baseCost=entry.second;task.dirty=true;++task.generation;
                repriced |= !task.children.empty();
            }
        } while(repriced);
        for(std::size_t i=0;i<s.tasks.size();++i) {
            auto& task=s.tasks[i];task.active=reached.count(i);
            if(!task.active) continue;
            if(task.dirty) resetTask(c,s,i);
            else {
                for(auto& item:task.parked) if(item.generation==task.generation) s.frontier.push(std::move(item));
                task.parked.clear();
            }
        }
    }
    static std::size_t startTask(Context<Node>& c, Session& s, std::shared_ptr<GoalSpec<Node>> goal, Cost cost) {
        const auto query=goal->request;
        auto existing=s.taskIndex.find(query);
        std::size_t index;
        if(existing==s.taskIndex.end()) {
            index=s.tasks.size();s.taskIndex.emplace(query,index);
            typename Session::Task task;task.goal=std::move(goal);task.baseCost=cost;
            s.tasks.push_back(std::move(task));
        } else index=existing->second;
        activate(c,s,index,cost);
        return index;
    }
    static void reject(Session& s, std::size_t task, const std::string& reason, Cost cost) {
        std::vector<std::size_t> pending{task};
        for (std::size_t next=0;next<pending.size();++next) {
            auto index=pending[next];auto& rejected=s.tasks[index];
            if (!rejected.rejection.empty()) continue;
            rejected.rejection=reason.empty() ? "goal rejected" : reason;
            rejected.finishedPass=true;
            s.rejected.insert(index);
            ++rejected.generation; // discard this goal's queued visits and Sets
            s.steps.push_back({"reject: " + rejected.rejection,rejected.goal->request,cost,s.status.revision});
            for (std::size_t parent=0;parent<s.tasks.size();++parent) {
                if(!s.tasks[parent].active) continue;
                const auto& required=s.tasks[parent].required;
                if (std::find(required.begin(),required.end(),index)!=required.end()) pending.push_back(parent);
            }
        }
        if (!s.tasks[0].rejection.empty()) {
            s.status.failed=true;s.status.fullyNormed=false;s.status.withSubscribers=false;
            s.status.error=s.tasks[0].rejection;s.terminal=true;s.frontier={};
        }
    }
    static void settle(Context<Node>& c, Session& s) {
        // Resolve the dependency DAG from its leaves. Cycles cannot establish
        // their own completion; they stay symbolic until other evidence breaks
        // the dependency. No recursive calls or domain-specific fields here.
        std::vector<std::vector<std::size_t>> parents(s.tasks.size());
        std::vector<std::size_t> remaining(s.tasks.size());
        std::vector<bool> resolved(s.tasks.size(), false), local(s.tasks.size());
        std::queue<std::size_t> ready;
        for (std::size_t i = 0; i < s.tasks.size(); ++i) {
            const auto& task = s.tasks[i];
            if(!task.active) continue;
            auto reads=observeTask(c,s,i);c.access->observeTask(task.goal->request);
            local[i] = task.rejection.empty() && !task.deferred && !task.goal->symbolic();
            remaining[i] = task.required.size();
            for (auto child : task.required) parents[child].push_back(i);
            if (local[i] && !remaining[i]) ready.push(i);
        }
        while (!ready.empty()) {
            auto i = ready.front(); ready.pop();
            resolved[i] = true;
            {auto reads=observeTask(c,s,i);c.access->observeTask(s.tasks[i].goal->request);
             s.tasks[i].goal->settled(true);}
            for (auto parent : parents[i])
                if (!--remaining[parent] && local[parent]) ready.push(parent);
        }
        for (std::size_t i = 0; i < s.tasks.size(); ++i)
            if (s.tasks[i].active && !resolved[i] && s.tasks[i].rejection.empty()) {
                auto reads=observeTask(c,s,i);c.access->observeTask(s.tasks[i].goal->request);
                s.tasks[i].goal->settled(false);
            }
        s.status.symbolicValues = !resolved[0];
    }
    static void advance(Context<Node>& c, Session& s) {
        s.status.revision=c.revision;
        if(s.tasks.empty()) startTask(c,s,s.root,0);
        if(s.reconcile) {
            s.reconcile=false;
            if(s.prune) {
                s.prune=false;
                reconcileTasks(c,s);
            } else for(auto index:s.invalidated)
                if(s.tasks[index].active && s.tasks[index].dirty) resetTask(c,s,index);
            s.invalidated.clear();
        }
        if (s.frontier.empty()) {
            for (std::size_t i=0;i<s.tasks.size();++i) {
                auto& task=s.tasks[i];
                if (!task.active || task.finishedPass) continue;
                task.finishedPass=true;
                auto reads=observeTask(c,s,i);c.access->observeTask(task.goal->request);
                try {
                    auto batch=task.goal->finishPass();
                    enqueueCommands(s,i,batch.finding,std::move(batch.commands));
                } catch (const GoalRejected& error) {
                    reject(s,i,error.what(),task.baseCost);
                    if (s.terminal) { c.changed.notify_all();return; }
                }
            }
            if (!s.frontier.empty()) return;
            settle(c,s);
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
        auto& task=s.tasks[pending.task];
        if (pending.generation != task.generation) return;
        if(!task.active) {task.parked.push_back(std::move(pending));return;}
        auto reads=observeTask(c,s,pending.task);c.access->observeTask(task.goal->request);
        try { follow(c,s,pending); }
        catch (const GoalRejected& error) {
            reject(s,pending.task,error.what(),pending.cost);
            if (s.terminal) c.changed.notify_all();
        }
    }
    static void follow(Context<Node>& c, Session& s, typename Session::Pending& pending) {
        auto goal = s.tasks[pending.task].goal;
        if (pending.edge.alternative && goal->satisfied()) return;
        auto arrival = pending.edge.follow();
        auto query = arrival.query;
        if (arrival.graphChanged) {
            s.steps.push_back({pending.edge.command ? pending.edge.description : "materialize: " + pending.edge.description,
                               query, pending.cost, c.revision});
            c.graphChanged(arrival.changes);
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
        enqueueCommands(s,pending.task,visit,goal->found(visit));
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
            auto child = startTask(c,s,c.access->goal(refinement.query),cost);
            auto& children=s.tasks[pending.task].children;
            auto link=std::find_if(children.begin(),children.end(),[child](const auto& item){return item.child==child;});
            if(link==children.end()) children.push_back({child,cost});
            else link->cost=std::min(link->cost,cost);
            if (!refinement.alternative) {
                auto& required = s.tasks[pending.task].required;
                if (std::find(required.begin(), required.end(), child) == required.end())
                    required.push_back(child);
                if (!s.tasks[child].rejection.empty()) {
                    reject(s,pending.task,s.tasks[child].rejection,pending.cost);
                    return;
                }
            }
        }
    }

    static void run(Context<Node>* c) {
        std::unique_lock<std::mutex> lock(c->mutex);
        while (!c->stopping) {
            // Repeated search passes must not win every mutex reacquisition
            // while a producer is waiting to supply their missing information.
            // Give one producer a turn, then take another bounded search step;
            // a continuous stream of producers must not starve search either.
            if(c->waitingPublications.load()) {
                auto turn=c->publicationTurn;
                c->changed.wait(lock,[&] {
                    return c->stopping || !c->waitingPublications.load() || c->publicationTurn!=turn;
                });
                if(c->stopping) break;
            }
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
            auto effects = c->access->takeEffects();
            c->dispatching = !effects.empty();
            lock.unlock();
            std::string effectError;
            for (auto& effect : effects) {
                try { effect(); }
                catch (const std::exception& error) { effectError = error.what(); }
                catch (...) { effectError = "completion effect failed"; }
            }
            std::this_thread::yield();
            lock.lock();
            c->dispatching = false;
            if (!effectError.empty()) {
                next->terminal = true; next->status.failed = true;
                next->status.fullyNormed = false; next->status.withSubscribers = false;
                next->status.error = effectError;
            }
            c->changed.notify_all();
            // Cancellation/publication enter between bounded steps and during
            // effects, never during a Set batch or semantic graph access.
            next.reset();
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
        context_->access->beginSearch(query);
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
    std::shared_ptr<Publication<Node>> beginPublication() {
        if (!context_) throw std::logic_error("initialize graph context first");
        return std::make_shared<Publication<Node>>(context_);
    }
    std::uint64_t revision() const {
        if(!context_) return 0;
        std::lock_guard<std::mutex> lock(context_->mutex);
        return context_->revision;
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
    // A caller that can distinguish an idempotent operation from a semantic
    // write must not publish merely because it acquired the writer lock.
    template<class Function> bool updateIfChanged(Function action, Changes changes = {}) {
        if (!context_) throw std::logic_error("initialize graph context first");
        std::lock_guard<std::mutex> lock(context_->mutex);
        if (context_->stopping) throw std::logic_error("graph context is closed");
        bool changed=action();
        if(changed) context_->graphChanged(changes);
        return changed;
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
