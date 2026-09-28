#pragma once

// Copy a supplied description into an unknown value using ordinary independent
// member views and identities. Replay follows those same identities as source
// parts arrive. Independent typed fields use unique nominal correspondence;
// ambiguous matches wait for more distinguishing information.
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
        MemberCorrespondence mapping(left.size(),right.size(),[&](std::size_t a,std::size_t b) {
            return access_->sameInfon(left[a],right[b]) || access_->hasDirectIdentity(left[a],right[b]);
        },[&](std::size_t a,std::size_t b) {
            if(!access_->hasTypeRequirements(right[b])) return ScalarMatch::unknown;
            return compareNominal(*definitions_,right[b],left[a],work);
        },work);
        if(!mapping.unambiguous) return {};
        for(auto index:mapping.missing) missing.push_back(right[index]);
        if(combined.hasSize && combined.size<static_cast<std::int64_t>(left.size()+missing.size()))
            throw DefinitionConflict("unordered members exceed explicit extent");
        std::vector<std::shared_ptr<ProteusDefinitionPlan>> plans;
        for(const auto& pair:mapping.pairs) {
            auto lhs=left[pair.first],rhs=right[pair.second];
            if(access_->sameInfon(lhs,rhs) || access_->hasDirectIdentity(lhs,rhs)) continue;
            auto meaning=definitions_->describe(rhs,work);
            if(!meaning) return {};
            auto plan=std::make_shared<ProteusDefinitionPlan>(access_,lhs,meaning->requirement);
            work.visits=addCost(work.visits,plan->work.visits);
            work.creations=addCost(work.creations,plan->work.creations+1);
            if(!plan->supported) return {};
            plans.push_back(plan);paired.push_back({lhs,rhs});
        }
        for(const auto& member:missing) if(!access_->canMakeMember(member)) return {};
        bool facts=!required.accepts(actual);
        verified_=!adopt && !facts && missing.empty() && paired.empty();
        if(verified_) return {};
        work.creations=addCost(work.creations,2*missing.size()+static_cast<unsigned>(adopt));
        auto access=access_;
        return {{"Set supplied record value",[access,target,source,adopt,required,missing,plans,paired] {
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
            for(const auto& plan:plans) changed=plan->apply() || changed;
            for(const auto& pair:paired) {
                access->bind(pair.first);access->bind(pair.second);
                access->registry->recordIdentity(pair.first,pair.second,iekListCorrespondence,0);changed=true;
            }
            for(const auto& member:missing) {access->bind(member);access->appendTailMember(target,member);changed=true;}
            return changed;
        },work}};
    }
    bool satisfied() const override {return verified_;}
    bool symbolic() const override {return !verified_;}
};
