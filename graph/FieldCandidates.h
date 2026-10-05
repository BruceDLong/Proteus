#pragma once

// Retained comparisons subscribe independently to the facts they inspect.
// The enclosing path rechecks invalid evidence and reuses its other answers;
// its ordinary traversal, refinement and publication rules stay unchanged.
class FieldCandidates {
public:
    struct Answer {
        DependencyKey changed=std::make_shared<Dependency>();
        DependencyProof::Ptr evidence;
        ScalarMatch match=ScalarMatch::unknown;
        Work work{0,0};
        std::vector<Refinement<POVNode>> refinements;
        bool ready=false;
    };
    using Answers=std::set<std::shared_ptr<Answer>>;
    // The path owns answers across retries; the context only indexes them.
    // Scope also pins its search epoch while newer handles are submitted.
    class Scope {
        Scope* previous_;
    public:
        Answers& answers;std::int64_t epoch;
        Scope(Answers& held,std::int64_t generation):previous_(scope_),answers(held),epoch(generation) {scope_=this;}
        ~Scope() {scope_=previous_;}
        Scope(const Scope&)=delete;
        Scope& operator=(const Scope&)=delete;
    };
private:
    inline static thread_local Scope* scope_=nullptr;
    std::map<Query<POVNode>,std::weak_ptr<Answer>> answers_;
    std::size_t cleanupAt_=64;
    std::int64_t epoch_=0;
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    DependencyKey modelFacts_;
public:
    FieldCandidates(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions,DependencyKey modelFacts)
        :access_(std::move(access)),definitions_(std::move(definitions)),modelFacts_(std::move(modelFacts)) {}
    void beginSearch() {++epoch_;}
    std::int64_t epoch() const {return epoch_;}
    struct Assessment {
        ScalarMatch match;Work work;DependencyProof::Ptr evidence;
        std::vector<Refinement<POVNode>> refinements;
    };
    Assessment assess(const Query<POVNode>& query) {
        Expansion<POVNode> out;std::set<DependencyKey> reads{modelFacts_};ScalarMatch match;Work work;
        {
            ReadScope capture([&](const DependencyKey& key){reads.insert(key);});
            FieldPredicate predicate(access_,definitions_,out);
            match=predicate.match(query.related,query.node,query.context);work=predicate.work;
        }
        if(reads.count(DependencyKey{})) throw std::logic_error("untracked field comparison");
        return {match,work,DependencyProof::make(reads),std::move(out.refinements)};
    }
    static void retain(const std::shared_ptr<Answer>& answer,const Assessment& result) {
        answer->evidence=result.evidence;answer->match=result.match;answer->work=result.work;
        answer->refinements=result.refinements;answer->ready=true;
        std::weak_ptr<Answer> weak=answer;
        std::weak_ptr<DependencyProof> expected=result.evidence;
        result.evidence->onInvalidated([weak,expected] {
            if(auto current=weak.lock();current && current->ready && current->evidence==expected.lock()) {
                current->ready=false;current->changed->publish();
            }
        });
    }
    Query<POVNode> request(const POVNode& predicate,const POVNode& candidate,const std::vector<POVNode>& owners) const {
        return {candidate,selectField,scope_ ? scope_->epoch : epoch_,predicate,owners};
    }
    std::shared_ptr<Answer> answer(const Query<POVNode>& request) {
        auto found=answers_.find(request);
        if(found!=answers_.end()) if(auto answer=found->second.lock()) return answer;
        if(answers_.size()>=cleanupAt_) {
            for(auto it=answers_.begin();it!=answers_.end();)
                if(it->second.expired()) it=answers_.erase(it);else ++it;
            cleanupAt_=std::max<std::size_t>(64,2*answers_.size());
        }
        auto answer=std::make_shared<Answer>();answer->changed->evidence=answer;
        answers_[request]=answer;return answer;
    }
    ScalarMatch match(const POVNode& predicate,const POVNode& candidate,const std::vector<POVNode>& owners,
                      Expansion<POVNode>& out,Cost& visits) {
        auto query=request(predicate,candidate,owners);auto result=answer(query);
        if(scope_) scope_->answers.insert(result);
        // Re-evaluate only invalid evidence. The enclosing path already owns
        // the work and the required refinements; no extra scheduler goal is needed.
        if(!result->ready) retain(result,assess(query));
        ReadScope::observe(result->changed);
        out.refinements.insert(out.refinements.end(),result->refinements.begin(),result->refinements.end());
        visits=addCost(visits,result->work.visits);
        return result->match;
    }
};
