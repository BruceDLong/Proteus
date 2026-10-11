#pragma once

// A list walk owns positions and count evidence; member values are independent
// required goals. Retain both across arrivals instead of replaying the prefix.
class ProteusListContentsGoal : public GoalSpec<POVNode> {
    using Q=Query<POVNode>;
    std::shared_ptr<ProteusGraphAccess> access_;
    std::function<Expansion<POVNode>(const Q&)> expand_;
    std::map<Q,GraphListMeasure> members_;
    std::set<Q> ends_;
    GraphListMeasure measure_;
    std::size_t unknown_=0;
    PersistentTraversal<POVNode> traversal_;
    void contribute(const Q& q,bool active) {
        if(q.kind==listEnd) {
            if(active) ends_.insert(q);else ends_.erase(q);
        }
        if(q.kind!=listMember) return;
        auto previous=members_.find(q);
        if(previous!=members_.end()) {
            measure_.count=measure_.count-previous->second.count;
            unknown_-=!previous->second.known;
            members_.erase(previous);
        }
        if(active) {
            GraphListMeasure member;member.add(q.node);
            measure_.count+=member.count;unknown_+=!member.known;
            members_.emplace(q,std::move(member));
        }
        measure_.known=unknown_==0;
    }
public:
    ProteusListContentsGoal(Q query,std::shared_ptr<ProteusGraphAccess> access,
                           std::function<Expansion<POVNode>(const Q&)> expand)
        :GoalSpec<POVNode>(query),access_(std::move(access)),expand_(std::move(expand)),
         traversal_(query,[this](const Q& q,bool active){contribute(q,active);}) {}
    bool persistent() const override {return true;}
    bool retainPath() const override {return false;}
    DeductionStats deductionStats() const override {return traversal_.counters();}
    std::vector<std::pair<Q,std::size_t>> deductionVisits() const override {return traversal_.visits();}
    void connect(std::function<void()> wake,DependencyKey broad,DependencyKey untracked) override {
        traversal_.connect(std::move(wake),std::move(broad),std::move(untracked));
    }
    std::vector<SetCommand> found(const Visit<POVNode>&) override {return {};}
    std::optional<Expansion<POVNode>> expand(const Visit<POVNode>&) override {
        traversal_.advance([this](const Q& q) {
            if(q.kind==listContents) {
                access_->bind(q.node);
                auto error=access_->inputError(q.node);
                if(!error.empty()) throw std::runtime_error(error);
            }
            return expand_(q);
        });
        Expansion<POVNode> out;out.refinements=traversal_.takeRequirements();
        out.deferred=traversal_.deferred();
        if(traversal_.ready()) {
            auto q=request;out.edges.push_back({"continue list deduction",{0,0},false,[q]{return q;}});
        }
        return out;
    }
    CommandBatch<POVNode> finishPass() override {
        if(traversal_.ready() || ends_.empty() || !access_->canSetCountedSize(request.node,measure_)) return {};
        auto access=access_;auto target=request.node;auto measure=measure_;
        return {{request,{},0},{{"Set counted list size",[access,target,measure] {
            auto view=readView(target);
            bool changed=access->applyCountedSize(target,measure);
            return SetResult{changed,changed ? ReadScope::changedObjects({
                target.get(),view.get(),view->infSize.get()}) : Changes::only({})};
        }}}};
    }
    bool satisfied() const override {
        traversal_.observe();
        return !traversal_.ready() && !ends_.empty() && access_->listExtentAgrees(request.node,measure_);
    }
    bool symbolic() const override {return !satisfied();}
};
