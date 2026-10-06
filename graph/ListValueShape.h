#pragma once

// Acquiring an unknown list value first exposes its finite shape. Member
// matching remains SequenceCorrespondence's job, including nested values and
// loose qualifiers. A colon supplies no previously unknown root extent.
class ProteusListValueGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
public:
    ProteusListValueGoal(Query<POVNode> query,std::shared_ptr<ProteusGraphAccess> access,
                         std::shared_ptr<ProteusDefinitions> definitions)
        :GoalSpec<POVNode>(query),access_(std::move(access)),definitions_(std::move(definitions)) {}
    std::optional<Expansion<POVNode>> expand(const Visit<POVNode>&) override {
        Expansion<POVNode> out;
        if(definitions_->required(request.node) && !definitions_->saved(request.node))
            out.refinements.push_back({{request.node,compileTypeRequirements,0},{0,0},"establish target shape constraints",false});
        return out;
    }
    std::vector<SetCommand> found(const Visit<POVNode>&) override {
        if(!access_->adoptableList(request.node)) return {};
        // The target's declared structure determines its members. In loose
        // correspondence the RHS may contain flattened units, so copying its
        // cardinality before establishing that structure is not sound.
        if(definitions_->required(request.node)) {
            auto meaning=definitions_->saved(request.node);
            if(!meaning || meaning->requirement->list) return {};
        }
        auto source=definitions_->payload(request.related);
        PayloadReadiness ready(access_,source,definitions_);
        if(!ready.ready || !access_->closedPhysicalList(source) || !access_->orderedList(source)) return {};
        SpanLayout<POVNode> layout;Cost visits=ready.work.visits;
        if(!readListLayout(*access_,source,layout,visits,[&](const auto& at) {
            return at.phase!=ListPhase::enter || (access_->closedPhysicalList(at.node) && access_->orderedList(at.node));
        },[](const POVNode& node){return node;})) return {};
        if(!layout.solve()) return {};
        for(const auto& segment:layout.segments) if(segment.gap) return {};
        auto count=static_cast<std::int64_t>(layout.segments.size());
        bool partial=request.related->looseSize();
        auto facts=access_->typeFacts(request.node);
        if(facts.hasSize) {
            if(facts.size<0 || facts.size.getDenominator()!=1 || facts.size>count || (!partial && facts.size!=count)) return {};
            count=facts.size.getNumerator().get_si();
        }
        auto requirement=std::make_shared<DefinitionRequirement>();requirement->list=true;
        requirement->facts.valid=true;requirement->facts.overlay=infonState::LST;
        requirement->facts.timelike=getFlagBit(readView(source)->value->flags,infonState::timelike);
        requirement->facts.hasSize=!partial || facts.hasSize;requirement->facts.size=count;
        for(std::int64_t i=0;i<count;++i) {
            auto member=std::make_shared<DefinitionRequirement>();member->facts.valid=true;
            requirement->members.push_back(member);
        }
        auto plan=std::make_shared<ProteusDefinitionPlan>(access_,request.node,requirement,std::map<POVNode,std::int64_t>{},!partial || facts.hasSize);
        if(!plan->supported) return {};
        plan->work.visits=addCost(plan->work.visits,visits);
        auto target=request.node;
        return {{"Set acquired list shape",[plan,target,partial] {
            bool changed=plan->apply();
            if(partial) readView(target)->value->sizeMode=fromGiven;
            return changed;
        },plan->work}};
    }
    bool satisfied() const override {return access_->literalList(request.node);}
    bool symbolic() const override {return !satisfied();}
};
