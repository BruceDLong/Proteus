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

// Work annotations remain available for diagnostics and a future cost policy.
// Scheduling is FIFO; these compatibility fields do not limit or rank work.
struct Work { Cost visits = 1; Cost creations = 0; };
struct CostPolicy {
    Cost visit = 1, creation = 4;
    Cost maximumRoute = std::numeric_limits<Cost>::max();
    Cost estimate(Work) const { return 0; }
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
    // Retain evidence established by this atomic Set after old evidence has
    // been invalidated. Runs under the context lock, before any next search
    // step; it must not change model data or perform another search.
    std::function<void()> retainEvidence;
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

// A collected result retains the finding and trace when its Set
// must wait until the current frontier has been examined.
template<class Node> struct CommandBatch {
    Visit<Node> finding;
    std::vector<SetCommand> commands;
};


template<class Node> struct Arrival {
    Query<Node> query;
    bool graphChanged = false;
    Changes changes;
    std::function<void()> retainEvidence;
    Arrival(Query<Node> q) : query(std::move(q)) {}
    Arrival(Query<Node> q, bool changed, Changes footprint = {})
        : query(std::move(q)), graphChanged(changed), changes(std::move(footprint)) {}
};

template<class Node> struct Edge {
    std::string description;
    Work work;
    bool alternative = false;
    // Deferred until dequeued. Enumerating an edge must not perform its
    // expensive materialization. The returned query can carry a new demand.
    std::function<Arrival<Node>()> follow;
    // Reserved work annotation for a future policy; ignored by FIFO scheduling.
    Work remaining{0, 0};
    bool command = false;
    // A pure navigation edge. Its callback is not needed to discover its target.
    std::optional<Query<Node>> destination;
};

template<class Node> struct Refinement {
    Query<Node> query;
    Work work;
    std::string description;
    // false makes completion of this subgoal a requirement of its parent.
    bool alternative = true;
    bool pruneWhenSatisfied = true; // optional discovery may still expose missing parts
    std::size_t owner = 0; // retained deduction that owns this demand
    bool withdraw = false;
};

template<class Node> struct Expansion {
    std::vector<Edge<Node>> edges;
    std::vector<Refinement<Node>> refinements;
    bool deferred = false;
};

struct DeductionStats {
    std::size_t evaluations=0, activations=0, withdrawals=0, supportAdds=0,
                supportRemoves=0, pruneVisits=0, notifications=0, records=0;
    DeductionStats& operator+=(const DeductionStats& b) {
        evaluations+=b.evaluations;activations+=b.activations;withdrawals+=b.withdrawals;
        supportAdds+=b.supportAdds;supportRemoves+=b.supportRemoves;pruneVisits+=b.pruneVisits;
        notifications+=b.notifications;records+=b.records;return *this;
    }
};

template<class Node> class GoalSpec {
public:
    Query<Node> request;
    explicit GoalSpec(Query<Node> q) : request(std::move(q)) {}
    virtual ~GoalSpec() = default;
    virtual void beginPass() {}
    // Opt in only with independently subscribed, retained deductions. The
    // runtime resumes this producer without clearing its progress or demands.
    virtual bool persistent() const { return false; }
    virtual DeductionStats deductionStats() const { return {}; }
    virtual std::vector<std::pair<Query<Node>,std::size_t>> deductionVisits() const {return {};}
    virtual void connect(std::function<void()>,DependencyKey,DependencyKey) {}
    // A goal may maintain a shared traversal chart for its demand. Its edges
    // still run on this scheduler, with the same lifecycle rules.
    // nullopt delegates ordinary edge access to the domain adapter.
    virtual std::optional<Expansion<Node>> expand(const Visit<Node>&) { return std::nullopt; }
    virtual bool retainPath() const { return true; }
    // Publish completion metadata only, after this goal and its required
    // subgoals have been examined. Independent children can complete separately.
    // This callback must not change semantic information or graph adjacency.
    virtual void settled(bool fullyResolved) { (void)fullyResolved; }
    virtual void settled(bool fullyResolved, const DependencyProof::Ptr&) {settled(fullyResolved);}
    virtual std::vector<SetCommand> found(const Visit<Node>&) = 0;
    // Called once after this pass's queued work, before completion metadata.
    // Return priced Sets; do not mutate graph state from this callback.
    virtual CommandBatch<Node> finishPass() { return {}; }
    // Guarded publications wait for required subgoals, including their own
    // prerequisites. A visited frontier alone does not prove a guard.
    virtual bool waitForRequirements() const { return false; }
    // Only alternative routes may be pruned after satisfaction. Constraints
    // remain searchable even when every requested field is already concrete.
    virtual bool satisfied() const = 0;
    virtual bool symbolic() const = 0;
    virtual Node result() const { return request.node; }
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
        std::set<std::size_t> required;
        std::map<std::size_t,Link> children;
        std::map<std::pair<std::size_t,std::size_t>,bool> demands;
        std::map<std::size_t,std::pair<std::size_t,std::size_t>> references;
        bool started=false;
        bool deferred = false;
        std::uint64_t generation = 0;
        bool finishedPass = false;
        std::string rejection;
        bool active = false, dirty = true;
        std::set<DependencyKey> dependencies;
        std::vector<std::shared_ptr<void>> evidence;
        std::shared_ptr<Dependency::Wake> wake;
        std::vector<Pending> parked;
    };
    std::vector<Task> tasks;
    std::map<Query<Node>,std::size_t> taskIndex;
    std::queue<Pending> frontier;
    std::shared_ptr<GoalSpec<Node>> root;
    CostPolicy policy;
    Status status;
    std::vector<Step<Node>> steps;
    bool quiescent = false, terminal = false, reconcile = false, prune = false;
    std::set<std::size_t> invalidated, rejected;
    bool watch = true;
    std::uint64_t sequence = 0;
    // A shared producer retains one task table. Each caller still owns an
    // ordinary session/handle, with independent cancellation and completion.
    bool shared = false;
    Query<Node> shareGroup;
    std::vector<std::weak_ptr<SearchSession>> clients;
    std::shared_ptr<SearchSession> producer;
    std::size_t rootTask = 0;
    std::atomic<bool> clientsChanged{false};
    ~SearchSession() {
        // Destruction may run under the context lock. Defer pruning to the
        // next worker step; no locking or model writes belong in this path.
        if(producer) producer->clientsChanged.store(true);
    }
};

template<class Node> struct Context {
    std::mutex mutex, joinMutex;
    std::condition_variable changed;
    std::atomic<std::size_t> waitingAccess{0};
    std::uint64_t accessTurn=0; // protected by mutex
    std::thread worker;
    bool stopping = false, dispatching = false;
    // Notification only: observing this counter never grants access to nodes.
    // Actual model reads still acquire mutex through GraphManager::read().
    std::atomic<std::uint64_t> revision{0};
    std::size_t cursor = 0;
    std::shared_ptr<GraphAccess<Node>> access;
    std::vector<std::weak_ptr<SearchSession<Node>>> sessions;

    ObjectDependencies objectDependencies;
    DependencyKey untracked = std::make_shared<Dependency>();
    DependencyKey broadChanges = std::make_shared<Dependency>();

    // Announce before competing for the mutex so the worker hands a turn to
    // external callers between steps. Reads, writes, cancellation and parser
    // publications use the same boundary; no model operation is interrupted.
    std::unique_lock<std::mutex> acquire() {
        std::unique_lock<std::mutex> lock(mutex,std::defer_lock);
        ++waitingAccess;
        try { lock.lock(); }
        catch (...) { --waitingAccess;changed.notify_all();throw; }
        --waitingAccess;
        ++accessTurn;
        changed.notify_all();
        return lock;
    }

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
            for(auto& weak:session->clients) if(auto client=weak.lock();client && !client->terminal) {
                client->quiescent=false;client->status.fullyNormed=false;
                client->status.withSubscribers=false;
            }
        }
        // A required child's rejection is also evidence held by its parents.
        // Ordinary completion is recomputed by settle; a propagated rejection
        // must be reopened when that child's evidence changes.
        if(newlyDirty) for(auto parent:session->rejected) {
            const auto& candidate=session->tasks[parent];
            if(!candidate.rejection.empty() &&
               candidate.required.count(index))
                wakeTask(session,parent);
        }
    }
    void observe(SearchSession<Node>& session, std::size_t index, const DependencyKey& key) {
        auto token=key ? key : untracked;
        auto& task=session.tasks[index];
        if(task.dependencies.insert(token).second) {
            token->subscribe(task.wake);
            if(auto evidence=token->evidence.lock()) task.evidence.push_back(std::move(evidence));
        }
    }
    void graphChanged(const Changes& changes = {}) {
        if(changes.empty()) return;
        access->invalidateCompletion(changes);
        ++revision;
        if(changes.all) {
            // Cached evidence can survive its handle. Broad writes must also
            // invalidate those proofs, not just the currently running tasks.
            broadChanges->publish();untracked->publish();
            for (auto& weak : sessions) if (auto session = weak.lock(); session && !session->terminal)
                for(std::size_t i=0;i<session->tasks.size();++i) wakeTask(session,i);
        } else {
            // Indexed by the evidence actually read. Unrelated sessions retain
            // their frontier, visited queries and accumulated findings.
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
    std::chrono::steady_clock::time_point yieldAt_;
public:
    explicit Publication(std::shared_ptr<Context<Node>> context)
        : context_(std::move(context)) {
        resume();
        if (context_->stopping) throw std::logic_error("graph context is closed");
    }
    // A producer may batch input operations proved nonblocking, but must
    // periodically publish and yield even when a large document is buffered.
    bool inputYieldDue() const { return std::chrono::steady_clock::now() >= yieldAt_; }
    void changed(const Changes& changes = {}) { changes_.include(changes); }
    void flush() {
        if (!changes_.empty()) {
            auto changes=std::move(changes_);changes_=Changes::only({});
            context_->graphChanged(changes);
        }
    }
    void suspend() { flush(); lock_.unlock(); }
    void resume() {
        lock_=context_->acquire();
        yieldAt_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(2);
        context_->changed.notify_all();
    }
    ~Publication() { if (lock_.owns_lock()) flush(); }
};

template<class Node> class InputPause {
    std::shared_ptr<Publication<Node>> publication_;
public:
    // The caller must prove the whole operation cannot wait for input before
    // passing false. Unknown/open input keeps the immediate-release behavior.
    explicit InputPause(std::shared_ptr<Publication<Node>> publication, bool mayBlock=true)
        : publication_(publication && (mayBlock || publication->inputYieldDue()) ? std::move(publication) : nullptr) {
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
        auto lock=context_->acquire();
        context_->changed.wait(lock, [&] {
            return !context_->dispatching &&
                (session_->terminal || (session_->quiescent && ready()));
        });
    }
    void waitForCompletion() const { waitUntil([] { return true; }); }
    void cancel() const {
        if (!session_) return;
        auto lock=context_->acquire();
        if (session_->terminal) return;
        session_->terminal = true;
        session_->status.cancelled = true;
        session_->status.fullyNormed = false;
        session_->status.withSubscribers = false;
        session_->frontier = {};
        if(auto owner=session_->producer) {
            owner->reconcile=owner->prune=true;owner->quiescent=false;
        }
        (session_->producer ? session_->producer : session_)->steps.push_back({"cancel", session_->root->request, 0,
                                  context_->revision});
        context_->changed.notify_all();
    }
    bool active() const {
        if (!session_) return false;
        auto lock=context_->acquire();
        return !session_->terminal;
    }
    Status status() const {
        if (!session_) return {};
        auto lock=context_->acquire();
        return session_->status;
    }
    std::vector<Step<Node>> steps() const {
        if (!session_) return {};
        auto lock=context_->acquire();
        return session_->producer ? session_->producer->steps : session_->steps;
    }
    std::map<Query<Node>,std::size_t> deductionVisits() const {
        if(!session_) return {};
        auto lock=context_->acquire();std::map<Query<Node>,std::size_t> result;
        for(const auto& task:(session_->producer ? session_->producer : session_)->tasks)
            for(const auto& entry:task.goal->deductionVisits()) result[entry.first]+=entry.second;
        return result;
    }
    DeductionStats deductionStats() const {
        if(!session_) return {};
        auto lock=context_->acquire();DeductionStats result;
        for(const auto& task:(session_->producer ? session_->producer : session_)->tasks) result+=task.goal->deductionStats();
        return result;
    }
    Node result() const {
        if (!session_) return {};
        auto lock=context_->acquire();
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
        (void)base;
        s.frontier.push({task, 0, 0, s.sequence++, std::move(edge),
                         std::move(path), s.tasks[task].generation});
    }

    static void enqueueCommands(Session& s, std::size_t task, const Visit<Node>& finding,
                                std::vector<SetCommand> commands) {
        for (auto& command : commands) {
            Edge<Node> operation{command.description, command.work, false,
                [query=finding.query, apply=std::move(command.apply), changes=std::move(command.changes)] {
                    auto result=apply();
                    Arrival<Node> arrival{query,result.changed,result.changes.value_or(changes)};
                    arrival.retainEvidence=std::move(result.retainEvidence);return arrival;
                }};
            operation.command = true;
            enqueue(s, task, finding.cost, std::move(operation), finding.path);
        }
    }
    static ReadScope observeTask(Context<Node>& c, Session& s, std::size_t index) {
        return ReadScope([&c,&s,index](const DependencyKey& key) {c.observe(s,index,key);},&c.objectDependencies);
    }
    static void resetTask(Context<Node>& c, Session& s, std::size_t index) {
        auto& task=s.tasks[index];
        s.rejected.erase(index);
        task.dirty=false;
        // Refresh only the lifecycle/publication read scope. Semantic reads
        // and continuations belong to the retained producer, not this scope.
        task.dependencies.clear();task.evidence.clear();
        std::weak_ptr<Session> weak=s.weak_from_this();
        task.wake=std::make_shared<Dependency::Wake>([weak,index] {
            if(auto current=weak.lock()) Context<Node>::wakeTask(current,index);
        });
        if(task.started && task.goal->persistent()) {
            task.finishedPass=false;task.deferred=false;task.rejection.clear();
            auto query=task.goal->request;
            enqueue(s,index,0,{"continue deductions",{0,0},false,[query] {return query;}},{});
            s.steps.push_back({"continue deductions",query,0,c.revision});
            return;
        }
        task.visited.clear();task.required.clear();task.children.clear();task.demands.clear();task.references.clear();
        task.deferred=false;task.finishedPass=false;task.rejection.clear();task.parked.clear();
        if(!task.started) {task.goal->connect(*task.wake,c.broadChanges,c.untracked);task.started=true;}
        auto reads=observeTask(c,s,index);c.access->observeTask(task.goal->request);
        task.goal->beginPass();
        auto query=task.goal->request;
        enqueue(s,index,task.baseCost,{"visit",{0,0},false,[query] {return query;}},{});
        s.steps.push_back({index ? "resume subgoal" : "search",query,task.baseCost,c.revision});
    }
    // A dirty parent's old outgoing demands cannot authorize work. Clean shared
    // children remain reusable if another current route reaches them. Parked
    // work retains its sequence; abandoned branches cannot write.
    static void activate(Context<Node>& c, Session& s, std::size_t first, Cost cost) {
        std::vector<typename Session::Task::Link> pending{{first,cost}};
        while(!pending.empty()) {
            auto link=pending.back();pending.pop_back();auto index=link.child;
            auto& task=s.tasks[index];
            if(!task.dirty && !task.rejection.empty()) {task.active=true;continue;}
            if(task.active && !task.dirty) continue;
            task.active=true;
            if(task.dirty) resetTask(c,s,index);
            else {
                for(auto& item:task.parked) if(item.generation==task.generation) s.frontier.push(std::move(item));
                task.parked.clear();
            }
            // A resumed persistent producer keeps its demands. Reactivate
            // their closure even when an ancestor temporarily detached it.
            for(const auto& child:task.children) pending.push_back(child.second);
        }
    }
    static void reconcileTasks(Context<Node>& c, Session& s) {
        // Retain shared children reachable from current demands. Scheduling
        // order has no bearing on reachability or semantic completion.
        std::set<std::size_t> reached;
        if(s.shared) {
            for(auto& weak:s.clients) if(auto client=weak.lock();client && !client->terminal)
                reached.insert(client->rootTask);
        } else reached.insert(0);
        std::queue<std::size_t> pending;for(auto index:reached) pending.push(index);
        while(!pending.empty()) {
            auto index=pending.front();pending.pop();
            if(s.tasks[index].dirty && !s.tasks[index].goal->persistent()) continue;
            for(const auto& link:s.tasks[index].children)
                if(reached.insert(link.second.child).second) pending.push(link.second.child);
        }
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
        if(s.shared) {
            for(auto& weak:s.clients) if(auto client=weak.lock();client && !client->terminal &&
                !s.tasks[client->rootTask].rejection.empty()) {
                client->status.failed=true;client->status.fullyNormed=false;
                client->status.withSubscribers=false;client->terminal=true;
                client->status.error=s.tasks[client->rootTask].rejection;
                s.reconcile=s.prune=true;
            }
        } else if (!s.tasks[0].rejection.empty()) {
            s.status.failed=true;s.status.fullyNormed=false;s.status.withSubscribers=false;
            s.status.error=s.tasks[0].rejection;s.terminal=true;s.frontier={};
        }
    }
    static std::vector<bool> settle(Context<Node>& c, Session& s, bool publish = true) {
        // Resolve the dependency DAG from its leaves. Cycles cannot establish
        // their own completion; they stay symbolic until other evidence breaks
        // the dependency. No recursive calls or domain-specific fields here.
        std::vector<std::vector<std::size_t>> parents(s.tasks.size());
        std::vector<std::size_t> remaining(s.tasks.size());
        std::vector<bool> resolved(s.tasks.size(), false), local(s.tasks.size());
        std::vector<DependencyProof::Ptr> proofs(s.tasks.size());
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
            if(publish) {auto reads=observeTask(c,s,i);c.access->observeTask(s.tasks[i].goal->request);
             auto& task=s.tasks[i];std::vector<DependencyProof::Ptr> children;
             for(auto child:task.required) children.push_back(proofs[child]);
             auto proof=DependencyProof::make(task.dependencies,std::move(children));
             proof->observe({c.broadChanges});
             task.goal->settled(true,proof);
             proof->observe(task.dependencies);proofs[i]=std::move(proof);}
            for (auto parent : parents[i])
                if (!--remaining[parent] && local[parent]) ready.push(parent);
        }
        if(publish) for (std::size_t i = 0; i < s.tasks.size(); ++i)
            if (s.tasks[i].active && !resolved[i] && s.tasks[i].rejection.empty()) {
                auto reads=observeTask(c,s,i);c.access->observeTask(s.tasks[i].goal->request);
                s.tasks[i].goal->settled(false,{});
            }
        if(publish) s.status.symbolicValues = !resolved[0];
        return resolved;
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
            // Ordinary collected results get the first publication opportunity.
            // Then resolve prerequisite closure for guarded Sets. Completion
            // metadata is published only after both phases have become quiet.
            for(int guarded=0;guarded!=2;++guarded) {
                bool progressed;
                do {
                    progressed=false;
                    std::vector<bool> resolved;
                    if(guarded) {
                        if(std::none_of(s.tasks.begin(),s.tasks.end(),[](const auto& task) {
                            return task.active && !task.finishedPass && task.goal->waitForRequirements();
                        })) break;
                        resolved=settle(c,s,false);
                    }
                    for(std::size_t i=0;i<s.tasks.size();++i) {
                        auto& task=s.tasks[i];
                        if(!task.active || task.finishedPass || task.goal->waitForRequirements()!=bool(guarded)) continue;
                        if(guarded && (task.deferred || !task.rejection.empty() ||
                           std::any_of(task.required.begin(),task.required.end(),[&](auto child){return !resolved[child];}))) continue;
                        task.finishedPass=true;progressed=true;
                        auto reads=observeTask(c,s,i);c.access->observeTask(task.goal->request);
                        try {
                            auto batch=task.goal->finishPass();
                            enqueueCommands(s,i,batch.finding,std::move(batch.commands));
                        } catch(const GoalRejected& error) {
                            reject(s,i,error.what(),task.baseCost);
                            if(s.terminal) {c.changed.notify_all();return;}
                        }
                    }
                    if(!s.frontier.empty()) return;
                } while(guarded && progressed);
            }
            auto resolved=settle(c,s);
            if(s.shared) {
                s.status.symbolicValues=false;
                for(auto& weak:s.clients) if(auto client=weak.lock();client && !client->terminal) {
                    client->status.revision=c.revision;
                    client->status.fullyNormed=true;
                    client->status.symbolicValues=!resolved[client->rootTask];
                    client->status.withSubscribers=client->watch && client->status.symbolicValues;
                    client->quiescent=true;client->terminal=!client->status.withSubscribers;
                    s.status.symbolicValues |= client->status.symbolicValues;
                }
                reconcileTasks(c,s); // completed callers no longer authorize work
            }
            s.status.fullyNormed = true;
            s.status.withSubscribers = s.watch && s.status.symbolicValues;
            s.quiescent = true;
            s.terminal = !s.shared && !s.status.withSubscribers;
            s.steps.push_back({s.status.withSubscribers ? "subscribe" : "settle",
                               s.root->request, 0, c.revision});
            c.changed.notify_all();
            return;
        }
        auto pending = s.frontier.front();
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
    static bool requirementsReady(Context<Node>& c, Session& s, std::size_t consumer) {
        // Recheck only this prerequisite DAG at the execution boundary. Scoped
        // writes can reopen a child without invalidating its consumer's reads.
        std::vector<std::pair<std::size_t,bool>> pending;
        std::map<std::size_t,int> state;
        for(auto child:s.tasks[consumer].required) pending.push_back({child,false});
        while(!pending.empty()) {
            auto [index,leaving]=pending.back();pending.pop_back();
            if(leaving) {state[index]=2;continue;}
            if(state[index]==2) continue;
            if(state[index]==1) return false;
            const auto& task=s.tasks[index];
            if(!task.active || task.dirty || !task.finishedPass || task.deferred || !task.rejection.empty()) return false;
            {auto reads=observeTask(c,s,index);c.access->observeTask(task.goal->request);
             if(task.goal->symbolic()) return false;}
            state[index]=1;pending.push_back({index,true});
            for(auto child:task.required) pending.push_back({child,false});
        }
        return true;
    }
    static void follow(Context<Node>& c, Session& s, typename Session::Pending& pending) {
        auto goal = s.tasks[pending.task].goal;
        if(pending.edge.command && goal->waitForRequirements() && !requirementsReady(c,s,pending.task)) {
            s.tasks[pending.task].finishedPass=false;
            s.steps.push_back({"wait for changed prerequisites",goal->request,pending.cost,c.revision});
            return;
        }
        if (pending.edge.alternative && goal->satisfied()) return;
        auto arrival = pending.edge.follow();
        auto query = arrival.query;
        if (arrival.graphChanged) {
            s.steps.push_back({pending.edge.command ? pending.edge.description : "materialize: " + pending.edge.description,
                               query, pending.cost, c.revision});
            c.graphChanged(arrival.changes);
            if(arrival.retainEvidence) arrival.retainEvidence();
            return;
        }
        if(arrival.retainEvidence) arrival.retainEvidence();
        if (pending.edge.command) return; // an unchanged Set has no new finding
        if (!query.node) { s.tasks[pending.task].deferred = true; return; }
        auto& visited = s.tasks[pending.task].visited;
        auto previous = visited.find(query);
        if (!goal->persistent()) {
            if (previous != visited.end()) return;
            visited[query] = 0;
        }
        if (goal->retainPath()) pending.path.push_back(query);
        s.steps.push_back({pending.edge.description, query, pending.cost, c.revision});
        Visit<Node> visit{query, pending.path, pending.cost};
        enqueueCommands(s,pending.task,visit,goal->found(visit));
        auto local = goal->expand(visit);
        auto expansion = local ? std::move(*local) : c.access->expand(query);
        if(goal->persistent()) s.tasks[pending.task].deferred=expansion.deferred;
        else s.tasks[pending.task].deferred |= expansion.deferred;
        for (auto& edge : expansion.edges)
            enqueue(s, pending.task, pending.cost, std::move(edge), pending.path);
        for (const auto& refinement : expansion.refinements) {
            if (!refinement.withdraw && refinement.alternative && refinement.pruneWhenSatisfied && goal->satisfied()) continue;
            Cost cost = 0;
            const auto existing=s.taskIndex.find(refinement.query);
            if(refinement.withdraw && existing==s.taskIndex.end()) continue;
            // Construct only on a query-index miss. Reused children still need
            // activation; ownership and required/exploratory accounting follow below.
            std::size_t child;
            if(existing!=s.taskIndex.end()) {
                child=existing->second;
                if(!refinement.withdraw) activate(c,s,child,cost);
            } else child=startTask(c,s,c.access->goal(refinement.query),cost);
            auto& parent=s.tasks[pending.task];
            auto key=std::make_pair(refinement.owner,child);
            auto old=parent.demands.find(key);
            // Legacy owner zero accumulates demands for a whole pass. An
            // optional route must not downgrade a required route already seen.
            bool required=!refinement.alternative || (refinement.owner==0 &&
                old!=parent.demands.end() && old->second);
            if(old!=parent.demands.end()) {
                auto& refs=parent.references[child];--refs.first;if(old->second) --refs.second;
                parent.demands.erase(old);
            }
            if(!refinement.withdraw) {
                parent.demands[key]=required;
                auto& refs=parent.references[child];++refs.first;if(required) ++refs.second;
            }
            auto refs=parent.references[child];
            if(refs.first) parent.children[child]={child,0};
            else {parent.children.erase(child);parent.references.erase(child);s.reconcile=s.prune=true;}
            if(refs.second) parent.required.insert(child);else parent.required.erase(child);
            s.steps.push_back({(refinement.withdraw ? "release subgoal: " : "subgoal: ")+refinement.description,
                               refinement.query,0,c.revision});
            if(refs.second && !s.tasks[child].rejection.empty()) {
                reject(s,pending.task,s.tasks[child].rejection,0);return;
            }
        }
    }

    static void run(Context<Node>* c) {
        std::unique_lock<std::mutex> lock(c->mutex);
        while (!c->stopping) {
            // Hand a waiting caller a turn before the next search step.
            // Unlock/yield/relock alone can repeatedly outrun a GUI reader or
            // producer. One completed acquisition lets search progress again
            // even while callers continue to arrive.
            if(c->waitingAccess.load()) {
                auto turn=c->accessTurn;
                c->changed.wait(lock,[&] {
                    return c->stopping || !c->waitingAccess.load() || c->accessTurn!=turn;
                });
                if(c->stopping) break;
            }
            std::shared_ptr<Session> next;
            auto count = c->sessions.size();
            for (std::size_t i = 0; i < count; ++i) {
                c->cursor %= count;
                auto candidate = c->sessions[c->cursor++].lock();
                if(candidate && candidate->shared && candidate->clientsChanged.exchange(false)) {
                    candidate->reconcile=candidate->prune=true;candidate->quiescent=false;
                }
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
                for(auto& weak:next->clients) if(auto client=weak.lock();client && !client->terminal) {
                    client->status=next->status;client->terminal=true;
                }
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
                for(auto& weak:next->clients) if(auto client=weak.lock();client && !client->terminal) {
                    client->status=next->status;client->terminal=true;
                }
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
        auto lock=context_->acquire();
        if (context_->stopping) throw std::logic_error("graph context is closed");
        if (!query.node) throw std::invalid_argument("normalize requires a node");
        ReadScope objects({},&context_->objectDependencies);
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
    // The caller supplies a semantic sharing group, excluding only demand
    // strength (such as output depth). Occurrence and cursor context remain
    // part of that key. Live handles retain progress; no global strong cache.
    NormalizationHandle<Node> normalizeShared(Query<Node> query, Query<Node> group,
                                              bool watch = true) {
        if(!context_) throw std::logic_error("initialize graph context first");
        auto lock=context_->acquire();
        if(context_->stopping) throw std::logic_error("graph context is closed");
        if(!query.node) throw std::invalid_argument("normalize requires a node");
        ReadScope objects({},&context_->objectDependencies);
        context_->access->beginSearch(query);
        std::shared_ptr<Session> owner;
        auto& sessions=context_->sessions;
        sessions.erase(std::remove_if(sessions.begin(),sessions.end(),
            [](const auto& weak){return weak.expired();}),sessions.end());
        for(auto& weak:sessions) if(auto candidate=weak.lock();candidate && candidate->shared &&
            !candidate->terminal && !(group<candidate->shareGroup) && !(candidate->shareGroup<group)) {
            owner=candidate;break;
        }
        if(!owner) {
            owner=std::make_shared<Session>();owner->shared=true;owner->shareGroup=std::move(group);
            owner->root=context_->access->goal(query);sessions.push_back(owner);
        }
        auto client=std::make_shared<Session>();
        client->producer=owner;client->watch=watch;
        client->rootTask=startTask(*context_,*owner,context_->access->goal(query),0);
        client->root=owner->tasks[client->rootTask].goal;
        owner->clients.erase(std::remove_if(owner->clients.begin(),owner->clients.end(),
            [](const auto& weak){return weak.expired();}),owner->clients.end());
        owner->clients.push_back(client);
        owner->quiescent=false;owner->reconcile=owner->prune=true;
        // A cached rejection is still specific to this request, not its peers.
        const auto& rejection=owner->tasks[client->rootTask].rejection;
        if(!rejection.empty()) {
            client->status.failed=true;client->status.error=rejection;client->terminal=true;
        }
        context_->changed.notify_all();
        return {context_,std::move(client)};
    }
    std::shared_ptr<Publication<Node>> beginPublication() {
        if (!context_) throw std::logic_error("initialize graph context first");
        return std::make_shared<Publication<Node>>(context_);
    }
    std::uint64_t revision() const {
        // Polling for change must not block a GUI behind ongoing search.
        // The counter is only a hint to acquire a fresh model snapshot/read.
        return context_ ? context_->revision.load(std::memory_order_relaxed) : 0;
    }
    // Writers use this boundary so publication and subscription installation
    // cannot miss each other. Readers of a live result use read() as well.
    template<class Function> auto read(Function action) const -> decltype(action()) {
        if (!context_) throw std::logic_error("initialize graph context first");
        auto lock=context_->acquire();
        ReadScope objects({},&context_->objectDependencies);
        return action();
    }
    template<class Function> void update(Function action) {
        if (!context_) throw std::logic_error("initialize graph context first");
        auto lock=context_->acquire();
        if (context_->stopping) throw std::logic_error("graph context is closed");
        ReadScope objects({},&context_->objectDependencies);
        action();
        context_->graphChanged();
    }
    // A caller that can distinguish an idempotent operation from a semantic
    // write must not publish merely because it acquired the writer lock.
    template<class Function> bool updateIfChanged(Function action, Changes changes = {}) {
        if (!context_) throw std::logic_error("initialize graph context first");
        auto lock=context_->acquire();
        if (context_->stopping) throw std::logic_error("graph context is closed");
        ReadScope objects({},&context_->objectDependencies);
        bool changed=action();
        if(changed) context_->graphChanged(changes);
        return changed;
    }
    void close() {
        if (!context_) return;
        std::lock_guard<std::mutex> joinLock(context_->joinMutex);
        {
            auto lock=context_->acquire();
            context_->stopping = true;
            for (auto& weak : context_->sessions) if (auto s = weak.lock()) {
                if (!s->terminal) {
                    s->terminal = true;
                    s->status.cancelled = true;
                    s->status.fullyNormed = false;
                    s->status.withSubscribers = false;
                    s->frontier = {};
                    for(auto& weak:s->clients) if(auto client=weak.lock();client && !client->terminal) {
                        client->terminal=true;client->status.cancelled=true;
                        client->status.fullyNormed=false;client->status.withSubscribers=false;
                    }
                }
            }
            context_->changed.notify_all();
        }
        if (context_->worker.joinable()) context_->worker.join();
    }
};

} // namespace proteus4

#include "PersistentTraversal.h"
