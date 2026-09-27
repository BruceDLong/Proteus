#pragma once

// Copy a supplied description into an unknown value using ordinary independent
// member views and identities. Replay follows those same identities as source
// parts arrive; it never pairs independently supplied fields by position/name.
class ProteusRecordGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    bool verified_=false;
public:
    ProteusRecordGoal(Query<POVNode> query,std::shared_ptr<ProteusGraphAccess> access,
                      std::shared_ptr<ProteusDefinitions> definitions)
        :GoalSpec<POVNode>(query),access_(std::move(access)),definitions_(std::move(definitions)) {}
    void beginPass() override {verified_=false;}
    std::vector<SetCommand> found(const Visit<POVNode>&) override {
        auto target=request.node,source=request.related;
        PayloadReadiness readiness(access_,definitions_->payload(source),definitions_);
        if(!readiness.ready) return {};
        Work work=readiness.work;
        std::vector<POVNode> left,right,missing;
        std::vector<std::pair<POVNode,POVNode>> paired;
        bool adopt=access_->adoptableList(target);
        if(!RecordStructure::members(*access_,source,right,work) ||
           (!adopt && !RecordStructure::members(*access_,target,left,work))) return {};
        auto actual=access_->typeFacts(target),required=access_->typeFacts(source),combined=actual;
        // This is an asserted relationship, not nominal field selection.
        // Different written names do not by themselves contradict equal data.
        if(!combined.combine(required))
            throw DefinitionConflict("incompatible unordered value constraints");
        if(combined.hasSize && (combined.size<0 || combined.size.getDenominator()!=1 || combined.size<static_cast<std::int64_t>(right.size())))
            throw DefinitionConflict("unordered members exceed explicit extent");
        if(!RecordStructure::pairs(*access_,left,right,paired,missing,work)) return {};
        for(const auto& member:missing) if(!access_->canMakeMember(member)) return {};
        bool facts=!required.accepts(actual);
        verified_=!adopt && !facts && missing.empty();
        if(verified_) return {};
        work.creations=addCost(work.creations,2*missing.size()+static_cast<unsigned>(adopt));
        auto access=access_;
        return {{"Set supplied record value",[access,target,source,adopt,required,missing] {
            bool changed=false;
            if(adopt) {
                changed=access->adoptDefinitionList(target);
                target->pItem->orderMode=uUnordered;
                SetFlagBit(target->pItem->value->flags,infonState::tailUnfinished,
                    getFlagBit(source->pItem->value->flags,infonState::tailUnfinished));
                // Unknown result types acquire the supplied nominal identity;
                // a written target type keeps its own name.
                if(!target->pItem->type) {
                    target->pItem->type=source->pItem->type;
                    target->pItem->addedTypes=source->pItem->addedTypes;
                }
            }
            changed=access->applyTypeFacts(target,required) || changed;
            for(const auto& member:missing) {access->bind(member);access->appendTailMember(target,member);changed=true;}
            return changed;
        },work}};
    }
    bool satisfied() const override {return verified_;}
    bool symbolic() const override {return !verified_;}
};
