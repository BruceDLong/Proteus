#pragma once

// Call/return sharing for one correspondence pass. These are recognition
// states and proof links, not copies of the POV graph. Input positions, local
// list counts and root/member matching policy distinguish calls; callers do
// not. Each new caller joins both existing and subsequently discovered results.
// The ordinary scheduler prices every transition and owns all execution.
class SequenceChart {
    using Q = Query<POVNode>;
    using ID = std::size_t;
    static constexpr ID none = std::numeric_limits<ID>::max();
    struct Support {
        ID previous=none, child=none, waiter=none;
        bool operator<(const Support& b) const {
            return std::tie(previous,child,waiter)<std::tie(b.previous,b.child,b.waiter);
        }
    };
    struct State {
        ID call;
        Q query;
        std::vector<Support> proofs;
        std::set<Support> distinct;
        Cost cost=0;
    };
    struct Call { Q entry; bool nested; ID start; std::vector<ID> waiters, results; };
    struct Waiter { ID previous, child; Q entry; };
    SequenceCorrespondence matcher_;
    std::vector<State> states_;
    std::vector<Call> calls_;
    std::vector<Waiter> waiters_;
    std::map<std::pair<ID,Q>,ID> stateIndex_;
    std::map<Q,ID> callIndex_;
    std::map<std::pair<ID,Q>,ID> waiterIndex_;
    std::vector<ID> complete_;

    static ID inputEnd(const Q& q) { return 2+q.coordinates[0]; }
    static bool matched(const Q& q) {
        return q.kind==sequenceMatched || q.kind==sequenceAtomMatched || q.kind==sequenceChoiceMatched;
    }
    // A call starts with just its input cursor. Its parent list/choice frame
    // lives in the waiter. This is the stack-independent recognition key.
    static Q local(Q q) {
        q.context.resize(inputEnd(q));q.coordinates.resize(3);return q;
    }
    // Restore the caller only for returning a result or compiling a proof.
    // Keep the child's end input cursor, which may have crossed embedded spans.
    static Q attach(Q q,const Q& entry) {
        if(q.coordinates.empty()) return q;
        q.context.insert(q.context.begin()+inputEnd(q),entry.context.begin()+inputEnd(entry),entry.context.end());
        q.coordinates.insert(q.coordinates.begin()+3,entry.coordinates.begin()+3,entry.coordinates.end());
        return q;
    }
    Q stateQuery(ID id) const {
        const auto& q=states_[id].query;
        return {q.node,sequenceChartState,static_cast<std::int64_t>(id),q.related,q.context,
                {q.kind,q.argument}};
    }
    Q waiterQuery(ID id) const {
        const auto& q=waiters_[id].entry;
        return {q.node,sequenceChartCall,static_cast<std::int64_t>(id),{},q.context,{q.argument}};
    }
    ID state(ID call,Q q,Support support) {
        auto key=std::make_pair(call,q);
        auto found=stateIndex_.find(key);
        ID id;
        if(found==stateIndex_.end()) {
            id=states_.size();stateIndex_.emplace(std::move(key),id);
            states_.push_back({call,std::move(q),{},{},0});
        } else id=found->second;
        if(states_[id].distinct.insert(support).second) states_[id].proofs.push_back(support);
        return id;
    }
    ID call(Q entry,bool nested) {
        auto found=callIndex_.find(entry);
        if(found!=callIndex_.end()) return found->second;
        auto id=calls_.size();callIndex_.emplace(entry,id);
        calls_.push_back({entry,nested,none,{},{}});
        calls_[id].start=state(id,std::move(entry),{});return id;
    }
    ID waiter(ID previous,Q entry) {
        auto key=std::make_pair(previous,entry);
        auto found=waiterIndex_.find(key);
        if(found!=waiterIndex_.end()) return found->second;
        auto child=call(local(entry),true);
        auto id=waiters_.size();waiterIndex_.emplace(std::move(key),id);
        waiters_.push_back({previous,child,std::move(entry)});
        calls_[child].waiters.push_back(id);return id;
    }
    static Edge<POVNode> edge(const char* label,Q q) {
        return {label,{1,0},true,[q]{return q;}};
    }
    // Return a shared child's result through the same semantic continuation
    // that an unshared walk would use. No tentative result mutates an infon.
    void resume(Expansion<POVNode>& out,ID waiterID,ID result) {
        auto waiter=waiters_[waiterID];
        auto parent=states_[waiter.previous].call;
        auto expansion=matcher_.expand(attach(states_[result].query,waiter.entry),calls_[parent].nested);
        for(auto& step:expansion.edges) {
            auto follow=step.follow;
            step.follow=[this,follow,parent,waiter,waiterID,result] {
                auto arrival=follow();
                if(!arrival.graphChanged)
                    arrival.query=stateQuery(state(parent,std::move(arrival.query),{waiter.previous,result,waiterID}));
                return arrival;
            };
            out.edges.push_back(std::move(step));
        }
    }

    // Enumerate proof choices only when an output is needed; recognition never
    // expands their Cartesian product. Stop at the first publishable result.
    // An active-state guard rejects cyclic proofs without rejecting the call.
    bool paths(ID id,const Q* entry,std::vector<Q>& path,std::set<ID>& active,
               const std::function<bool()>& emit) const {
        if(!active.insert(id).second) return false;
        const auto& s=states_[id];
        auto size=path.size();
        // A finished subtree may be used again by a later occurrence (notably
        // finite zero-width repetition). Only unfinished ancestry is cyclic.
        auto deliver=[&] { active.erase(id);auto done=emit();active.insert(id);return done; };
        for(const auto& proof:s.proofs) {
            auto append=[&] {
                path.push_back(entry ? attach(s.query,*entry) : s.query);
                auto done=deliver();path.resize(size);return done;
            };
            auto afterPrevious=[&] {
                // A predecessor has just filled the prefix. Only remove this
                // state's contribution when trying another packed alternative.
                auto prefixSize=path.size();
                auto appendResult=[&] {
                    path.push_back(entry ? attach(s.query,*entry) : s.query);
                    auto done=deliver();path.resize(prefixSize);return done;
                };
                if(proof.child==none) return appendResult();
                auto childEntry=waiters_[proof.waiter].entry;
                if(entry) childEntry=attach(std::move(childEntry),*entry);
                return paths(proof.child,&childEntry,path,active,appendResult);
            };
            bool done=proof.previous==none ? append() : paths(proof.previous,entry,path,active,afterPrevious);
            path.resize(size);
            if(done) {active.erase(id);return true;}
        }
        active.erase(id);return false;
    }
public:
    SequenceChart(std::shared_ptr<ProteusGraphAccess> access,
                  std::shared_ptr<ProteusDefinitions> definitions,Q request)
        : matcher_(std::move(access),std::move(definitions)) {
        call(std::move(request),false);
    }
    Expansion<POVNode> expand(const Visit<POVNode>& visit) {
        const auto& q=visit.query;
        Expansion<POVNode> out;
        if(q.kind==sequenceCorrespondence) {
            out.edges.push_back(edge("start shared sequence",stateQuery(calls_[0].start)));return out;
        }
        if(q.kind==sequenceChartCall) {
            auto waiter=waiters_.at(q.argument);
            out.edges.push_back(edge("share matching subproblem",stateQuery(calls_[waiter.child].start)));
            // A caller can arrive after the result, or before it. Both joins
            // use the same pair, and the scheduler suppresses duplicate states.
            for(auto result:calls_[waiter.child].results) resume(out,q.argument,result);
            return out;
        }
        auto id=static_cast<ID>(q.argument);
        auto owner=states_.at(id).call;auto cursor=states_[id].query;states_[id].cost=visit.cost;
        if(cursor.kind==sequenceComplete) {
            if(std::find(complete_.begin(),complete_.end(),id)==complete_.end()) complete_.push_back(id);
            return out;
        }
        if(calls_[owner].nested && matched(cursor)) {
            auto& results=calls_[owner].results;
            if(std::find(results.begin(),results.end(),id)==results.end()) results.push_back(id);
            for(auto waiter:calls_[owner].waiters) resume(out,waiter,id);
            return out;
        }
        out=matcher_.expand(cursor,calls_[owner].nested);
        for(auto& step:out.edges) {
            auto follow=step.follow;
            step.follow=[this,follow,id,owner,kind=cursor.kind] {
                auto arrival=follow();
                if(arrival.graphChanged) return arrival;
                bool child=arrival.query.kind==sequenceEnter &&
                    (kind==sequenceEnter || kind==sequenceBoundary);
                arrival.query=child ? waiterQuery(waiter(id,std::move(arrival.query))) :
                    stateQuery(state(owner,std::move(arrival.query),{id,none,none}));
                return arrival;
            };
        }
        return out;
    }
    bool finding(const Visit<POVNode>& visit,const std::function<bool(Visit<POVNode>)>& accept) const {
        if(visit.query.kind!=sequenceChartState) return false;
        auto id=static_cast<ID>(visit.query.argument);
        if(states_.at(id).query.kind!=sequenceComplete) return false;
        std::vector<Q> path;std::set<ID> active;
        return paths(id,nullptr,path,active,[&] {return accept({states_[id].query,path,visit.cost});});
    }
    bool proofs(const std::function<bool(Visit<POVNode>)>& accept) const {
        for(auto id:complete_) {
            std::vector<Q> path;std::set<ID> active;
            if(paths(id,nullptr,path,active,[&] {return accept({states_[id].query,path,states_[id].cost});})) return true;
        }
        return false;
    }
};
