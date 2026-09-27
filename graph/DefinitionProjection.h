#pragma once

// A finite read-only plan over declaration POVs. Only the matched member's
// requirements are compiled; the parent body is neither applied nor normalized.
class ProteusProjectionGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::map<POVNode,ProteusDefinitions::Certificate> descriptions_;
    bool pending_ = false;
    bool layout(const POVNode& node, SpanLayout<POVNode>& result, Work& work) {
        return readListLayout(*access_,node,result,work.visits,[&](const auto& at) {
            return at.phase!=ListPhase::enter || (access_->physicalList(at.node) && !access_->memberTemplate(at.node));
        },[](const POVNode& member){return member;});
    }
public:
    ProteusProjectionGoal(Query<POVNode> query,std::shared_ptr<ProteusGraphAccess> access,
                          std::shared_ptr<ProteusDefinitions> definitions)
        :GoalSpec<POVNode>(query),access_(std::move(access)),definitions_(std::move(definitions)) {}
    void beginPass() override {pending_=false;}
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        if(visit.query.kind!=projectField) return {};
        auto target=request.node,owner=request.related;
        Work work{0,0};Expansion<POVNode> unused;
        FieldPredicate matcher(access_,definitions_,unused);
        SpanLayout<POVNode> instance;
        if(!layout(owner,instance,work)) {pending_=true;return {};}
        // Repeated type words correspond in their existing ordered sequence.
        // An unordered duplicate has no distinguished occurrence to project.
        std::size_t ordinal=0,total=0;bool present=false;
        for(const auto& segment:instance.segments) {
            if(segment.gap) {if(!present && owner->pItem->orderMode!=uUnordered && segment.width!=0) pending_=true;continue;}
            auto match=matcher.matchType(target,segment.node);
            if(match==ScalarMatch::unknown && !present) pending_=true;
            if(segment.node==target) {present=true;ordinal=total;}
            if(match==ScalarMatch::yes) ++total;
        }
        if(!present || pending_ || (owner->pItem->orderMode==uUnordered && total!=1)) {pending_=true;return {};}
        std::vector<POVNode> sources;
        auto addMeanings=[&](const POVNode& node) {
            for(const auto& name:definitions_->names(node)) {
                ++work.visits;
                if(auto body=definitions_->lookup(name)) sources.push_back(body);
                else if(definitions_->declared(name)) pending_=true;
            }
        };
        addMeanings(owner);
        std::set<POVNode> seen;
        GraphTypeFacts facts;facts.valid=true;
        for(std::size_t i=0;i<sources.size();++i) {
            auto body=sources[i];if(!seen.insert(body).second) continue;
            ++work.visits;
            if(!access_->plain(body,false,false,false,false)) {pending_=true;continue;}
            addMeanings(body);
            for(const auto& rhs:POVDescription(body).work()) sources.push_back(rhs);
            if(!access_->literalList(body)) continue;
            SpanLayout<POVNode> meaning;
            if(!layout(body,meaning,work)) {pending_=true;continue;}
            std::vector<POVNode> candidates;bool ordered=body->pItem->orderMode!=uUnordered;
            for(const auto& segment:meaning.segments) {
                if(segment.gap) {if(ordered && segment.width!=0) {pending_=true;break;}continue;}
                auto match=matcher.matchType(target,segment.node);
                if(match==ScalarMatch::unknown) {pending_=true;if(ordered) break;}
                if(match==ScalarMatch::yes) candidates.push_back(segment.node);
                if(ordered && candidates.size()>ordinal) break;
            }
            if(candidates.empty()) continue;
            if((!ordered && candidates.size()!=1) || (ordered && candidates.size()<=ordinal)) {pending_=true;continue;}
            auto source=candidates[ordered ? ordinal : 0];
            auto& proof=descriptions_[source];
            if(!proof || !definitions_->current(proof)) proof=definitions_->describe(source,work);
            if(!proof || proof->requirement->list || !proof->requirement->choices.empty()) {pending_=true;continue;}
            if(!facts.combine(proof->requirement->facts)) throw DefinitionConflict("incompatible projected field requirements");
        }
        work.visits=addCost(work.visits,matcher.work.visits);
        if(pending_) return {};
        auto actual=access_->typeFacts(target),combined=actual;
        if(!((facts.hasSize && !actual.hasSize) || (facts.hasValue && !actual.hasValue) ||
             (facts.overlay>=0 && actual.overlay<0))) return {};
        if(!combined.combine(facts)) throw DefinitionConflict("incompatible projected field facts");
        if(facts.accepts(actual)) return {};
        auto access=access_;
        return {{"Set field facts from parent definition",[access,target,facts] {return access->applyTypeFacts(target,facts);},work}};
    }
    bool satisfied() const override {return !pending_;}
    bool symbolic() const override {return pending_;}
};
