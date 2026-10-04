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
    std::vector<Refinement<POVNode>> requestedFacts_;
    std::map<DefinitionSpec,NumericView> numbers_;
    bool readable(const POVNode& node, bool predicate) {
        if (!access_->plainStructure(node,false,false,false,false,false,false) ||
            (!predicate && getFlagBit(readView(node)->flags,infonView::invertMatch))) return false;
        for (const auto& relation:POVDescription(node,access_->registry.get()).relationships()) {
            if (access_->sameInfon(node,relation.target)) continue;
            if(!RHSRepresentation::expressed(access_,node,relation.target,definitions_,work)) return false;
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
                        bool scalarOnly=false, bool useMeaning=false, const DefinitionSpec& demand={}) {
        ++work.visits;
        if (active_.count(readView(node))) return {};
        auto specification=std::make_shared<DefinitionRequirement>();
        specification->types=definitions_->names(node);
        specification->captureLabel(node);
        specification->facts=definitions_->facts(node,predicate);
        auto refinementContext=context;refinementContext.insert(refinementContext.begin(),node);
        if(!predicate && WordForms::collectionType(*access_,node)) {
            out_.refinements.push_back({{node,valueOnly,0,{},refinementContext},{1,0},"derive collection type",true});
            return {};
        }
        if(predicate && specification->facts.valid && specification->facts.overlay<0 &&
           WordForms::collectionPredicate(node,*definitions_)) specification->facts.overlay=infonState::LST;
        const bool canRead=readable(node,predicate);
        const bool needsFact=!predicate && demand &&
            (demand->list || (demand->facts.hasValue && !specification->facts.hasValue) ||
             (demand->facts.hasSize && !specification->facts.hasSize) ||
             (demand->facts.overlay>=0 && specification->facts.overlay<0));
        const bool structure=demand && (demand->list || demand->facts.overlay==infonState::LST);
        Refinement<POVNode> refinement{{node,structure ? valueOnly : sizeAndValue,
            structure ? exposeStructure : lazyParts,{},refinementContext},{1,0},"refine field candidate",true};
        if ((!predicate || !getFlagBit(readView(node)->flags,infonView::invertMatch)) &&
            (!canRead || (!access_->literalList(node) &&
             (!access_->hasTypeRequirements(node) || !POVDescription(node).work().empty()))))
            out_.refinements.push_back(refinement);
        else if(needsFact) requestedFacts_.push_back(refinement);
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
            getModeBits(readView(node)->value->flags,infonState::evalMode)==infonState::emLiteral &&
            !access_->literalList(node)) return {};
        // An empty open ordered body constrains the list header/extent only.
        // It adds no member predicates and need not expand a sparse candidate.
        if (predicate && listHeaderOnly(*access_,node)) scalarOnly=true;
        if (!scalarOnly && predicate && access_->repeatedDefinitionList(node) && specification->facts.hasSize) {
            // A fixed repeated predicate describes every candidate position;
            // it is not an instruction to allocate that many pattern nodes.
            active_.insert(readView(node));
            specification->list=true;specification->open=true;
            specification->repeated=read(definitions_->repeatedBody(node),true,context,false,useMeaning);
            active_.erase(readView(node));
            if(!specification->repeated) return {};
        } else if (!scalarOnly && RecordStructure::record(*access_,node)) {
            std::vector<POVNode> members;
            if(!RecordStructure::members(*access_,node,members,work)) return {};
            active_.insert(readView(node));
            specification->list=true;specification->order=uUnordered;
            specification->open=!access_->closedPhysicalList(node);
            auto owners=context;owners.insert(owners.begin(),node);
            for(const auto& member:members) {
                auto header=std::make_shared<DefinitionRequirement>();
                header->types=definitions_->names(member);header->captureLabel(member);
                header->facts.valid=true;
                // A partial description only asks about its own typed parts.
                // Do not normalize unrelated geometry, state, or unknown data.
                bool relevant=predicate || !demand || !demand->list || demand->order!=uUnordered;
                DefinitionSpec childDemand;
                if(!relevant) for(const auto& required:demand->members) {
                    if(!required || nominal(required,header)!=ScalarMatch::no) {
                        relevant=true;childDemand=required;break;
                    }
                }
                specification->members.push_back(relevant ?
                    read(member,predicate,owners,false,useMeaning,childDemand) : header);
            }
            active_.erase(readView(node));
        } else if (!scalarOnly && access_->literalList(node)) {
            active_.insert(readView(node));
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
                    if (position.gap || getFlagBit(readView(position.node)->flags,infonView::invertMatch)) {available=false;break;}
                    auto owners=context;
                    const auto& local=locations.at(position.node);owners.insert(owners.end(),local.begin(),local.end());
                    auto child=read(position.node,predicate,std::move(owners),false,useMeaning);
                    specification->members.push_back(child);
                }
            }
            active_.erase(readView(node));
            if (!available) return {};
        }
        if (meaning) {
            if(meaning->requirement->hasExpressions() || !meaning->requirement->choices.empty()) return {};
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
            if(pair.first->repeated && pair.first->members.empty() && pair.first->facts.hasSize &&
               pair.second->list && !pair.second->open && pair.second->order==uOrdered) {
                for(const auto& member:pair.second->members) pending.push_back({pair.first->repeated,member});
                continue;
            }
            if(pair.first->order==uUnordered && pair.second->list) {
                // Conjoin the requested parts, not every part of the candidate.
                // Unique nominal correspondence avoids permutation search and
                // leaves ambiguous/unknown occurrences unresolved.
                MemberCorrespondence mapping(pair.second->members.size(),pair.first->members.size(),
                    [](std::size_t,std::size_t){return false;},[&](std::size_t a,std::size_t b) {
                        const auto& wanted=pair.first->members[b];const auto& present=pair.second->members[a];
                        return wanted && present ? nominal(wanted,present) : ScalarMatch::unknown;
                    },work);
                if(!mapping.unambiguous) {unknown=true;continue;}
                if(!mapping.missing.empty()) {
                    if(!pair.second->open) return ScalarMatch::no;
                    unknown=true;
                }
                for(const auto& member:mapping.pairs)
                    pending.push_back({pair.first->members[member.second],pair.second->members[member.first]});
                continue;
            }
            // Positional patterns still require a closed ordered structure.
            // Never turn missing positions into successful omission.
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
        if(required && !required->list && required->facts.valid && required->facts.overlay<0 &&
           !required->facts.hasSize && !required->facts.hasValue && !required->label.empty()) {
            // A bare type selection asks for nominal identity, not the entire
            // value behind that member's outstanding equality constraints.
            auto kind=matchType(predicate,candidate);
            if(kind!=ScalarMatch::unknown)
                return getFlagBit(readView(predicate)->flags,infonView::invertMatch) ? complement(kind) : kind;
        }
        auto actual=read(candidate,false,owners,required && !required->list,false,required);
        auto result=conflict_ ? ScalarMatch::unknown : compare(required,actual);
        // First answer the written query from explicit facts. Only a missing
        // fact needs definition evidence; selecting a typed object is not a
        // request to validate its entire definition.
        if (result==ScalarMatch::unknown && required && !conflict_) {
            actual=read(candidate,false,owners,!required->list,true,required);
            result=conflict_ ? ScalarMatch::unknown : compare(required,actual);
        }
        // A known definition can also disprove an untyped candidate (for
        // example a string cannot satisfy a numeric type). Descriptive facts
        // alone never establish the missing nominal identity.
        if (result==ScalarMatch::unknown && required && actual && !conflict_) {
            auto exclusion=read(predicate,true,{},false,true);
            if (!conflict_ && compare(exclusion,actual)==ScalarMatch::no) result=ScalarMatch::no;
        }
        // A read-only definition proof may already answer the query. Only an
        // undecided comparison needs to materialize its requested facts.
        if(result==ScalarMatch::unknown)
            out_.refinements.insert(out_.refinements.end(),requestedFacts_.begin(),requestedFacts_.end());
        if(getFlagBit(readView(predicate)->flags,infonView::invertMatch)) result=complement(result);
        return result;
    }
};
