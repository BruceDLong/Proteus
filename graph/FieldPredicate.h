#pragma once

// Read-only finite structural comparison. The temporary requirements are proof
// data over existing members, not new POVs or type-application RHSs. Unsupported
// relations remain unknown and advertise refinement through the normal graph.
class FieldPredicate {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    Expansion<POVNode>& out_;
    std::set<std::shared_ptr<infonView>> active_;
    bool conflict_=false;
    std::map<DefinitionSpec,NumericView> numbers_;
    bool readable(const POVNode& node, bool predicate) {
        if (!access_->plainStructure(node,false,false,false,false,false,false) ||
            (!predicate && getFlagBit(node->pItem->flags,infonView::invertMatch))) return false;
        for (const auto& relation:POVDescription(node,access_->registry.get()).relationships()) {
            if (access_->sameInfon(node,relation.target)) continue;
            RHSRepresentation proof(access_,node,relation.target,definitions_);
            work.visits=addCost(work.visits,proof.work.visits);
            if (!proof.represented) return false;
        }
        return true;
    }
    ScalarMatch nominal(const DefinitionSpec& required,const DefinitionSpec& actual) {
        auto result=compareNominal(required->label,required->types,actual->label,actual->types);
        if (result!=ScalarMatch::no) return result;
        bool complete=true;
        auto left=definitions_->nominalNames(required->types,complete,work),right=definitions_->nominalNames(actual->types,complete,work);
        result=compareNominal(required->label,left,actual->label,right);
        return result==ScalarMatch::no && !complete ? ScalarMatch::unknown : result;
    }
    DefinitionSpec read(const POVNode& node, bool predicate, std::vector<POVNode> context,
                        bool scalarOnly=false, bool useMeaning=false) {
        ++work.visits;
        if (active_.count(node->pItem)) return {};
        auto specification=std::make_shared<DefinitionRequirement>();
        specification->types=definitions_->names(node);
        specification->captureLabel(node);
        specification->facts=predicate ? access_->predicateFacts(node,false) : access_->typeFacts(node);
        auto refinementContext=context;refinementContext.insert(refinementContext.begin(),node);
        if(!predicate && WordForms::collectionType(*access_,node)) {
            out_.refinements.push_back({{node,valueOnly,0,{},refinementContext},{1,0},"derive collection type",true});
            return {};
        }
        if(predicate && specification->facts.valid && specification->facts.overlay<0 &&
           WordForms::collectionPredicate(node,*definitions_)) specification->facts.overlay=infonState::LST;
        const bool canRead=readable(node,predicate);
        if ((!predicate || !getFlagBit(node->pItem->flags,infonView::invertMatch)) && !access_->literalList(node) &&
            (!access_->hasTypeRequirements(node) || !POVDescription(node).work().empty() || !canRead))
            out_.refinements.push_back({{node,sizeAndValue,0,{},refinementContext},{1,0},"refine field candidate",true});
        if (!canRead) return {};
        NumericRangeAccess numeric(*access_);auto number=numeric.read(node,predicate);
        work.visits=addCost(work.visits,numeric.work.visits);
        if(access_->hasTypeRequirements(node)) number.constraint=false;
        if(number.bounds.valid) {numbers_[specification]=number;specification->facts=number.facts;}
        if (!specification->facts.valid) return {};
        ProteusDefinitions::Certificate meaning;
        if (useMeaning && access_->hasTypeRequirements(node) && !definitions_->optional(node)) {
            meaning=definitions_->saved(node);
            if (!meaning) {
                out_.refinements.push_back({{node,compileTypeRequirements,0},{1,0},"read field type meaning",true});
                return {};
            }
        }
        // A complemented literal list cannot use the positive-only list
        // accessors. Keep it unresolved rather than comparing just its header.
        if (specification->facts.overlay==infonState::LST &&
            getModeBits(node->pItem->value->flags,infonState::evalMode)==infonState::emLiteral &&
            !access_->literalList(node)) return {};
        // An empty open ordered body constrains the list header/extent only.
        // It adds no member predicates and need not expand a sparse candidate.
        if (predicate && listHeaderOnly(*access_,node)) scalarOnly=true;
        if (!scalarOnly && access_->literalList(node)) {
            active_.insert(node->pItem);
            SpanLayout<POVNode> layout;
            std::map<POVNode,std::vector<POVNode>> locations;
            bool available=readListLayout(*access_,node,layout,work.visits,[&](const auto& at) {
                if (at.phase==ListPhase::member) locations[at.node]=at.owners;
                return at.phase!=ListPhase::enter || (access_->closedPhysicalList(at.node) &&
                    access_->definitionOrderedCandidate(at.node) && readable(at.node,predicate) &&
                    (at.node==node || !access_->hasTypeRequirements(at.node)));
            },[](const POVNode& member){return member;});
            if (available) {
                specification->list=true;
                GraphTypeFacts count;count.valid=true;count.hasSize=true;count.size=static_cast<std::int64_t>(layout.segments.size());
                if (!specification->facts.combine(count)) conflict_=true;
                for (const auto& position:layout.segments) {
                    if (position.gap || getFlagBit(position.node->pItem->flags,infonView::invertMatch)) {available=false;break;}
                    auto owners=context;
                    const auto& local=locations.at(position.node);owners.insert(owners.end(),local.begin(),local.end());
                    auto child=read(position.node,predicate,std::move(owners),false,useMeaning);
                    specification->members.push_back(child);
                }
            }
            active_.erase(node->pItem);
            if (!available) return {};
        }
        if (meaning) {
            if(!meaning->requirement->choices.empty()) return {};
            if (meaning->requirement->list && std::any_of(specification->members.begin(),specification->members.end(),
                    [](const auto& member){return !member;})) return {};
            try {specification=combineDefinitions(specification,meaning->requirement,work);}
            catch(const DefinitionConflict&) {conflict_=true;return {};}
            catch(const DefinitionPending&) {return {};}
        }
        return specification;
    }
    ScalarMatch compare(const DefinitionSpec& required,const DefinitionSpec& actual) {
        if (!required || !actual) return ScalarMatch::unknown;
        std::vector<std::pair<DefinitionSpec,DefinitionSpec>> pending{{required,actual}};
        bool unknown=false;
        while (!pending.empty()) {
            auto pair=pending.back();pending.pop_back();++work.visits;
            if (!pair.first || !pair.second) {unknown=true;continue;}
            auto typeMatch=nominal(pair.first,pair.second);
            auto requiredNumber=numbers_.find(pair.first),actualNumber=numbers_.find(pair.second);
            bool range=requiredNumber!=numbers_.end() && requiredNumber->second.constraint;
            auto match=range ? compareNumeric(requiredNumber->second,actualNumber==numbers_.end() ? NumericView{} : actualNumber->second,pair.second->facts) :
                              compareScalar(pair.first->facts,pair.second->facts);
            if (typeMatch==ScalarMatch::no || match==ScalarMatch::no) return ScalarMatch::no;
            unknown |= typeMatch==ScalarMatch::unknown || match==ScalarMatch::unknown;
            if (!pair.first->list) continue;
            // Open and unordered structures need their own correspondence
            // proof. Never turn missing positions into successful omission.
            if (!pair.second->list || pair.first->order!=uOrdered || pair.second->order!=uOrdered || pair.first->open || pair.second->open) {unknown=true;continue;}
            if (pair.first->members.size()!=pair.second->members.size()) return ScalarMatch::no;
            for (std::size_t i=0;i<pair.first->members.size();++i)
                pending.push_back({pair.first->members[i],pair.second->members[i]});
        }
        return unknown ? ScalarMatch::unknown : ScalarMatch::yes;
    }
public:
    Work work{0,0};
    FieldPredicate(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions,
                   Expansion<POVNode>& expansion)
        :access_(std::move(access)),definitions_(std::move(definitions)),out_(expansion) {}
    // Field correspondence uses nominal identity without asking an unrelated
    // definition member to normalize or testing the destination's unknown value.
    ScalarMatch matchType(const POVNode& predicate,const POVNode& candidate) {
        auto spec=[&](const POVNode& node) {
            auto result=std::make_shared<DefinitionRequirement>();
            result->types=definitions_->names(node);
            result->captureLabel(node);
            return result;
        };
        return nominal(spec(predicate),spec(candidate));
    }
    ScalarMatch match(const POVNode& predicate,const POVNode& candidate,const std::vector<POVNode>& owners) {
        auto required=read(predicate,true,{});
        auto actual=read(candidate,false,owners,required && !required->list);
        auto result=conflict_ ? ScalarMatch::unknown : compare(required,actual);
        // First answer the written query from explicit facts. Only a missing
        // fact needs definition evidence; selecting a typed object is not a
        // request to validate its entire definition.
        if (result==ScalarMatch::unknown && required && !conflict_) {
            actual=read(candidate,false,owners,!required->list,true);
            result=conflict_ ? ScalarMatch::unknown : compare(required,actual);
        }
        // A known definition can also disprove an untyped candidate (for
        // example a string cannot satisfy a numeric type). Descriptive facts
        // alone never establish the missing nominal identity.
        if (result==ScalarMatch::unknown && required && actual && !conflict_) {
            auto exclusion=read(predicate,true,{},false,true);
            if (!conflict_ && compare(exclusion,actual)==ScalarMatch::no) result=ScalarMatch::no;
        }
        if(getFlagBit(predicate->pItem->flags,infonView::invertMatch)) result=complement(result);
        return result;
    }
};
