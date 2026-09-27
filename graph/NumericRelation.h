#pragma once

// Range work is interpreted at the relationship boundary. The graph still
// exposes operand expressions and RHS refinements; no recursive normalizer is
// hidden inside this proof or its Set.
class NumericRelation {
public:
    NumericView left,right,result;
    Work work{0,0};
    bool supported=false,accepted=false;
    NumericRelation(ProteusGraphAccess& access,const POVNode& target,const POVNode& source) {
        NumericRangeAccess reader(access);
        left=reader.read(target);right=reader.read(source,false,true);
        work=reader.work;
        if(access.unknownOverlay(target) && access.plain(target,false,false,false,false) && right.bounds.valid) {
            left.bounds=NumericBounds::any();left.constraint=true;
            left.facts=access.typeFacts(target);left.facts.overlay=infonState::NUM;
        }
        supported=left.bounds.valid && right.bounds.valid &&
            ((source->looseType() && left.constraint) ||
             (!source->looseType() && (left.expression || right.expression)));
        if(!supported) return;
        // Type/application obligations and RHS work must first be proved by
        // the normal graph, not ignored by this scalar projection.
        if(access.hasTypeRequirements(target) || access.hasTypeRequirements(source) ||
           !POVDescription(source,access.registry.get()).relationships().empty()) return;
        result=left;
        if(source->looseType()) {
            // Range-valued matching is deliberately pending until its output
            // information contract is agreed. Literal containment is definite.
            if(!right.bounds.point || !left.bounds.contains(right.bounds.low)) return;
            result.bounds=right.bounds;
            if(!result.facts.hasSize) {result.facts.hasSize=right.facts.hasSize;result.facts.size=right.facts.size;}
            result.facts.explicitSize=right.facts.explicitSize;
        } else {
            auto a=left.facts,b=right.facts;a.hasValue=b.hasValue=false;
            if(!a.combine(b)) throw std::runtime_error("incompatible numeric range sizes");
            result.facts=a;result.bounds=left.bounds.intersect(right.bounds);
            if(result.bounds.empty) throw std::runtime_error("disjoint numeric range constraints");
        }
        accepted=result.bounds.valid && !result.bounds.empty;
        if(accepted && !left.bounds.same(result.bounds)) work.creations=result.bounds.point ? 1 : 5;
    }
};

class ProteusNumericGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusCompletion> completion_;
    bool verified_=false;
public:
    ProteusNumericGoal(Query<POVNode> query,std::shared_ptr<ProteusGraphAccess> access,
                       std::shared_ptr<ProteusCompletion> completion)
        :GoalSpec<POVNode>(query),access_(std::move(access)),completion_(std::move(completion)) {}
    void beginPass() override {verified_=false;}
    std::vector<SetCommand> found(const Visit<POVNode>&) override {
        NumericRelation plan(*access_,request.node,request.related);
        if(!plan.accepted) return {};
        auto access=access_;auto target=request.node;auto result=plan.result;
        return {{"Set numeric range evidence",[this,access,target,result] {
            bool changed=NumericRangeAccess::write(*access,target,result);
            verified_=true;return changed;
        },plan.work}};
    }
    bool satisfied() const override {return verified_;}
    bool symbolic() const override {return !verified_;}
    void settled(bool resolved) override {if(resolved) completion_->represented(request.node,request.related);}
};
