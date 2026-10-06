#pragma once

#include <deque>

namespace proteus4 {

class NonPersistentNavigation : public std::logic_error {
public: NonPersistentNavigation():std::logic_error("navigation producer is not incremental") {}
};

// Retained deductions, not a second model graph. Each entry is one demand and
// the support links established by evaluating it. A publication queues only
// the entries which read it. Call under the graph context lock.
template<class Node> class PersistentTraversal {
public:
    using Q=Query<Node>;
    using ID=std::size_t;
    using Counters=DeductionStats;
private:
    struct Entry {
        Q query;
        std::set<ID> parents,children;
        std::map<Q,Refinement<Node>> requirements;
        DependencyProof::Ptr evidence;
        bool active=false,queued=false,deferred=false;
        std::size_t evaluations=0;
    };
    // Completion evidence must not retain query nodes: a model can hold its
    // own completion proof. Keeping the traversal there would create a cycle.
    struct Witness {
        DependencyKey changed=std::make_shared<Dependency>();
        std::map<ID,DependencyProof::Ptr> proofs;
    };
    struct State {
        std::vector<Entry> entries;
        std::map<Q,ID> index;
        std::deque<ID> ready;
        std::function<void()> wake;
        std::shared_ptr<Witness> witness=std::make_shared<Witness>();
        Counters counters;
        std::size_t deferred=0;
        void queue(ID id) {
            auto& e=entries[id];
            if(!e.active || e.queued) return;
            e.queued=true;ready.push_back(id);
        }
    };
    std::shared_ptr<State> state_=std::make_shared<State>();
    DependencyKey broad_,untracked_;
    std::function<void(const Q&,bool)> contribution_;
    std::vector<Refinement<Node>> requirements_;
    ID entry(const Q& q) {
        auto& s=*state_;auto found=s.index.find(q);
        if(found!=s.index.end()) return found->second;
        ID id=s.entries.size();s.index.emplace(q,id);s.entries.push_back({q});return id;
    }
    void requirement(ID owner,Refinement<Node> r,bool withdraw) {
        r.owner=owner+1;r.withdraw=withdraw;requirements_.push_back(std::move(r));
    }
    void activate(ID id) {
        auto& s=*state_;auto& e=s.entries[id];
        if(e.active) return;
        e.active=true;++s.counters.activations;s.deferred+=e.deferred;
        // A previously disconnected deduction may now have different guards.
        s.queue(id);
    }
    // Deletion is limited to the descendants of removed supports. Surviving
    // incoming support re-establishes reachability, including through cycles.
    void prune(const std::set<ID>& roots) {
        if(roots.empty()) return;
        auto& s=*state_;std::set<ID> affected;std::vector<ID> todo(roots.begin(),roots.end());
        while(!todo.empty()) {
            auto id=todo.back();todo.pop_back();
            if(!s.entries[id].active || !affected.insert(id).second) continue;
            ++s.counters.pruneVisits;
            for(auto child:s.entries[id].children) todo.push_back(child);
        }
        std::set<ID> supported;
        for(auto id:affected) {
            if(id==0 || std::any_of(s.entries[id].parents.begin(),s.entries[id].parents.end(),[&](ID parent) {
                return s.entries[parent].active && !affected.count(parent);
            })) {supported.insert(id);todo.push_back(id);}
        }
        while(!todo.empty()) {
            auto id=todo.back();todo.pop_back();
            for(auto child:s.entries[id].children)
                if(affected.count(child) && supported.insert(child).second) todo.push_back(child);
        }
        for(auto id:affected) if(!supported.count(id)) {
            auto& e=s.entries[id];e.active=false;s.deferred-=e.deferred;++s.counters.withdrawals;s.witness->proofs.erase(id);e.evidence.reset();
            contribution_(e.query,false);
            for(const auto& r:e.requirements) requirement(id,r.second,true);
            e.requirements.clear();
        }
    }
public:
    PersistentTraversal(Q root,std::function<void(const Q&,bool)> contribution)
        :contribution_(std::move(contribution)) {state_->witness->changed->evidence=state_->witness;activate(entry(root));}
    void connect(std::function<void()> wake,DependencyKey broad,DependencyKey untracked) {
        state_->wake=std::move(wake);broad_=std::move(broad);untracked_=std::move(untracked);
    }
    // Completion proofs retain only the witness, so their dependencies
    // remain live without retaining the caller or its query nodes.
    void observe() const {ReadScope::observe(state_->witness->changed);}
    bool ready() const {return !state_->ready.empty();}
    bool deferred() const {return state_->deferred!=0;}
    Counters counters() const {auto c=state_->counters;c.records=state_->entries.size();return c;}
    std::size_t size() const {return state_->entries.size();}
    std::vector<std::pair<Q,std::size_t>> visits() const {
        std::vector<std::pair<Q,std::size_t>> result;
        for(const auto& e:state_->entries) result.emplace_back(e.query,e.evaluations);
        return result;
    }
    std::vector<Refinement<Node>> takeRequirements() {auto out=std::move(requirements_);requirements_.clear();return out;}
    // Pure navigation edges name their destination explicitly. Materializing
    // operations remain scheduler work, never run while enumerating edges.
    template<class Expand> bool advance(Expand expand) {
        auto& s=*state_;
        if(s.ready.empty()) return false;
        ID id=s.ready.front();s.ready.pop_front();s.entries[id].queued=false;
        if(!s.entries[id].active) return true;
        auto q=s.entries[id].query;Expansion<Node> out;std::set<DependencyKey> reads{broad_};
        {
            ReadScope capture([&](const DependencyKey& key){reads.insert(key ? key : untracked_);});
            out=expand(q);
            for(const auto& edge:out.edges) if(!edge.destination)
                throw NonPersistentNavigation();
            contribution_(q,true);
        }
        ++s.counters.evaluations;++s.entries[id].evaluations;
        std::set<ID> children;
        for(const auto& edge:out.edges) if(edge.destination->node) children.insert(entry(*edge.destination));
        auto previous=s.entries[id].children;std::set<ID> removed;
        // Add support before removing old support, so a converging continuation
        // stays active when another route supplies the same result.
        for(auto child:children) if(!previous.count(child)) {
            s.entries[child].parents.insert(id);++s.counters.supportAdds;
        }
        for(auto child:children) activate(child);
        s.entries[id].children=std::move(children);
        for(auto child:previous) if(!s.entries[id].children.count(child)) {
            s.entries[child].parents.erase(id);removed.insert(child);++s.counters.supportRemoves;
        }
        auto& e=s.entries[id];s.deferred-=e.deferred;e.deferred=out.deferred;s.deferred+=e.deferred;
        std::map<Q,Refinement<Node>> desired;
        for(auto r:out.refinements) {
            auto old=desired.find(r.query);
            if(old==desired.end()) desired.emplace(r.query,r);
            else old->second.alternative &= r.alternative;
        }
        for(const auto& r:e.requirements) if(!desired.count(r.first)) requirement(id,r.second,true);
        for(const auto& r:desired) {
            auto old=e.requirements.find(r.first);
            if(old==e.requirements.end() || old->second.alternative!=r.second.alternative) requirement(id,r.second,false);
        }
        e.requirements=std::move(desired);
        std::weak_ptr<State> weak=state_;
        e.evidence=DependencyProof::make(reads);
        s.witness->proofs[id]=e.evidence;
        std::weak_ptr<Witness> witness=s.witness;
        e.evidence->onInvalidated([weak,witness,id] {
            if(auto proof=witness.lock()) proof->changed->publish();
            if(auto state=weak.lock();state && state->entries[id].active) {
                ++state->counters.notifications;state->queue(id);
                if(state->wake) state->wake();
            }
        });
        prune(removed);
        return true;
    }
};

} // namespace proteus4
