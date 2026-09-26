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
    // Type words and explicit alias links are sufficient for nominal selection.
    // Reading them must not force unrelated members of a definition to resolve.
    std::vector<std::string> nominalNames(std::vector<std::string> names, bool& complete) {
        std::set<POVNode> seen;
        std::vector<POVNode> pending;
        std::size_t next=0;
        while (next<names.size() || !pending.empty()) {
            if (pending.empty()) {
                const auto name=names[next++];++work.visits;
                if (auto body=definitions_->lookup(name)) pending.push_back(body);
                else if (definitions_->declared(name)) complete=false;
                continue;
            }
            auto node=pending.back();pending.pop_back();
            if (!seen.insert(node).second) continue;
            ++work.visits;
            if (!access_->plain(node,false,false,false,false)) {complete=false;continue;}
            for (const auto& name:definitions_->names(node))
                if (std::find(names.begin(),names.end(),name)==names.end()) names.push_back(name);
            for (const auto& rhs:POVDescription(node).work()) pending.push_back(rhs);
        }
        return names;
    }
    ScalarMatch nominal(const DefinitionSpec& required,const DefinitionSpec& actual) {
        auto result=compareNominal(required->label,required->types,actual->label,actual->types);
        if (result!=ScalarMatch::no) return result;
        bool complete=true;
        auto left=nominalNames(required->types,complete),right=nominalNames(actual->types,complete);
        result=compareNominal(required->label,left,actual->label,right);
        return result==ScalarMatch::no && !complete ? ScalarMatch::unknown : result;
    }
    DefinitionSpec read(const POVNode& node, bool predicate, std::vector<POVNode> context,
                        bool scalarOnly=false, bool useMeaning=false) {
        ++work.visits;
        if (active_.count(node->pItem)) return {};
        auto specification=std::make_shared<DefinitionRequirement>();
        specification->types=definitions_->names(node);
        if (!specification->types.empty()) specification->label=specification->types.front();
        specification->facts=predicate ? access_->predicateFacts(node) : access_->typeFacts(node);
        auto refinementContext=context;refinementContext.insert(refinementContext.begin(),node);
        if (!predicate && !access_->literalList(node) &&
            (!access_->hasTypeRequirements(node) || !POVDescription(node).work().empty()))
            out_.refinements.push_back({{node,sizeAndValue,0,{},refinementContext},{1,0},"refine field candidate",true});
        if (!readable(node,predicate) || !specification->facts.valid) return {};
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
            if (meaning->requirement->list && std::any_of(specification->members.begin(),specification->members.end(),
                    [](const auto& member){return !member;})) return {};
            try {specification=combineDefinitions(specification,meaning->requirement);}
            catch(const DefinitionConflict&) {conflict_=true;return {};}
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
            auto match=compareScalar(pair.first->facts,pair.second->facts);
            if (typeMatch==ScalarMatch::no || match==ScalarMatch::no) return ScalarMatch::no;
            unknown |= typeMatch==ScalarMatch::unknown || match==ScalarMatch::unknown;
            if (!pair.first->list) continue;
            // Open and unordered structures need their own correspondence
            // proof. Never turn missing positions into successful omission.
            if (!pair.second->list || pair.first->open || pair.second->open) {unknown=true;continue;}
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
        if(getFlagBit(predicate->pItem->flags,infonView::invertMatch) && result!=ScalarMatch::unknown)
            result=result==ScalarMatch::yes ? ScalarMatch::no : ScalarMatch::yes;
        return result;
    }
};
