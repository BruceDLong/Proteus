#pragma once

// Scalar relationship plans interpret qualified endpoints independently of
// their payloads. The graph exposes expressions/refinements; this finite proof
// never hides a recursive normalization or stores another adjacency graph.
class ScalarRelation {
public:
    NumericView result;
    bool text=false, copiedText=false;
    Work work{0,0};
    bool supported=false,accepted=false;
    ScalarRelation(ProteusGraphAccess& access,const POVNode& target,const POVNode& source) {
        auto actual=access.typeFacts(target);
        if(source->looseType() && actual.valid && (actual.overlay<0 || actual.overlay==infonState::STR) &&
           access.plain(source,false,false,true,false) && source->pItem->value->sizeMode!=fromContext &&
           !access.hasTypeRequirements(target) && !access.hasTypeRequirements(source) &&
           POVDescription(source,access.registry.get()).relationships().empty()) {
            auto supplied=access.predicateFacts(source,true);
            if(supplied.valid && supplied.overlay==infonState::STR) {
                supported=text=true;work.visits=2;
                auto match=matchText(actual,supplied,source->looseSize());
                result.facts=match.result;accepted=match.verdict==ScalarMatch::yes;
                copiedText=accepted && !actual.hasValue;return;
            }
        }
        NumericRangeAccess reader(access);
        auto left=reader.read(target),right=reader.read(source,false,true);
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
        auto match=matchNumeric(left,right,source->looseSize(),source->looseType());
        if(match.verdict==ScalarMatch::no && !source->looseType())
            throw GoalRejected("incompatible numeric range constraints");
        result=match.result;accepted=match.verdict==ScalarMatch::yes;
        if(accepted && !left.bounds.same(result.bounds)) work.creations=result.bounds.point ? 1 : 5;
    }
};

class ProteusScalarRelationGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusCompletion> completion_;
    bool verified_=false;
public:
    ProteusScalarRelationGoal(Query<POVNode> query,std::shared_ptr<ProteusGraphAccess> access,
                       std::shared_ptr<ProteusCompletion> completion)
        :GoalSpec<POVNode>(query),access_(std::move(access)),completion_(std::move(completion)) {}
    void beginPass() override {verified_=false;}
    std::vector<SetCommand> found(const Visit<POVNode>&) override {
        ScalarRelation plan(*access_,request.node,request.related);
        if(!plan.accepted) return {};
        auto access=access_;auto target=request.node;auto result=plan.result;bool text=plan.text,copiedText=plan.copiedText;
        return {{text ? "Set string relationship evidence" : "Set numeric range evidence",[this,access,target,result,text,copiedText] {
            bool changed=!text && (result.expression || result.bounds.point) ? NumericRangeAccess::write(*access,target,result) :
                access->applyTypeFacts(target,result.facts);
            if(copiedText) {
                // A copied prefix has a counted width only when that width is
                // proved. Unknown endpoints must remain visibly unknown.
                auto mode=result.facts.hasSize ? fromCount : fromGiven;
                if(target->pItem->value->sizeMode!=mode) {target->pItem->value->sizeMode=mode;changed=true;}
            }
            verified_=true;return changed;
        },plan.work}};
    }
    bool satisfied() const override {return verified_;}
    bool symbolic() const override {return !verified_;}
    void settled(bool resolved) override {if(resolved) completion_->represented(request.node,request.related);}
};
