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
        return {{"Set supplied record value",[access,target,source,adopt,required,missing,plans,paired]() -> SetResult {
            WriteScope identities;
            auto footprint=[&] {
                auto view=readView(target);
                return ReadScope::changedObjects({target.get(),view.get(),view->infSize.get(),view->value.get()});
            };
            auto changes=footprint();
            auto tail=POVDescription(target).member(true);
            if(tail && !missing.empty()) changes.include(ReadScope::changedObjects({POVDescription::positionKey(tail)}));
            bool changed=false;
            if(adopt) {
                changed=access->adoptDefinitionList(target);
                readView(target)->orderMode=uUnordered;
                SetFlagBit(readView(target)->value->flags,infonState::tailUnfinished,
                    getFlagBit(readView(source)->value->flags,infonState::tailUnfinished));
                // Unknown result types acquire the supplied nominal identity;
                // a written target type keeps its own name.
                if(!readView(target)->type) {
                    readView(target)->type=readView(source)->type;
                    readView(target)->addedTypes=readView(source)->addedTypes;
                }
            }
            changed=access->applyTypeFacts(target,required) || changed;
            for(const auto& plan:plans) {
                auto result=plan->applyChanges();changed=result.changed || changed;
                if(result.changed) changes.include(result.changes.value_or(Changes{}));
            }
            for(const auto& pair:paired) {
                // Importing a named occurrence may also replace its view;
                // that separate operation retains its conservative footprint.
                bool named=access->bind(pair.first);named=access->bind(pair.second) || named;
                if(named) changes=Changes{};
                access->registry->recordIdentity(pair.first,pair.second,iekListCorrespondence,0);changed=true;
            }
            for(const auto& member:missing) {
                if(access->bind(member)) changes=Changes{};
                access->appendTailMember(target,member);changed=true;
            }
            changes.include(footprint());changes.include(identities.changes());
            return {changed,changes};
        },work}};
    }
    bool satisfied() const override {return verified_;}
    bool symbolic() const override {return !verified_;}
};
