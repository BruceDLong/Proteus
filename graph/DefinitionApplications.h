#pragma once
#include "DefinitionPlan.h"
#include "UnitMeasure.h"

// Context-owned catalogue and reusable type evidence. Bodies are read through
// existing POVs. Application never normalizes or specializes the shared body.
class ProteusDefinitions {
public:
    struct Definition { POVNode declaration, body; };
    struct Binding { std::string name; POVNode body; };
    struct Witness {
        POVNode node;
        std::shared_ptr<infonView> infon;
        GraphTypeFacts facts;
        std::vector<std::string> names;
        std::vector<POVNode> work, members;
        bool list = false, embedded = false, open = false;
        int order = uUnknown;
        POVNode memberTemplate, choiceBody;
        // Immutable source snapshot shared by every proof compiled from it.
        // Validation belongs to the snapshot, not each enclosing certificate.
        mutable std::uint64_t checkedEpoch=0;
        mutable bool checkedCurrent=false;
        mutable std::set<DependencyKey> reads;
    };
    struct Evidence {
        std::vector<Binding> bindings;
        std::map<POVNode, std::shared_ptr<const Witness>> witnesses;
        std::map<POVNode, DefinitionSpec> compiled;
        // Cached validation re-observes one shared dependency group, including
        // header-alias reads made while checking the source witnesses.
        DependencyProof::Ptr reads;
        std::uint64_t checkedEpoch=0;
        bool checkedCurrent=false;
    };
    struct Proof {
        std::shared_ptr<Evidence> evidence = std::make_shared<Evidence>();
        DefinitionSpec requirement;
        bool compilable = true;
        Work composition{0,0};
    };
    using Certificate = std::shared_ptr<Proof>;
    // Derived unit views keep identities to the original members. Cache only
    // the result of an explicit window demand, never an expanded unit domain.
    using UnitKey=std::tuple<POVNode,std::string,std::int64_t,std::int64_t,bool>;
    struct UnitView {
        DefinitionSpec shape;
        std::vector<std::tuple<POVNode,bool,std::int64_t,std::int64_t>> pieces;
        POVNode result;
    };
    std::map<UnitKey,UnitView> unitViews;
    // A virtual unit run is a coordinate relationship, not an identity between
    // containers with different cardinalities. Indexed access exposes it before
    // allocating ordinary repetition occurrences.
    struct UnitSpan {POVNode source; DefinitionSpec member; std::int64_t start,count;};
    std::map<POVNode,UnitSpan> unitSpans;
    const UnitSpan* unitSpan(POVNode node) const {
        std::set<POVNode> seen;
        while(node && seen.insert(node).second) {
            if(auto found=unitSpans.find(node);found!=unitSpans.end()) return &found->second;
            node=readView(node)->selectedResult;
        }
        return nullptr;
    }

private:
    std::shared_ptr<ProteusGraphAccess> access_;
    DependencyKey modelFacts_;
    std::map<std::string, std::vector<Definition>> catalogue_;
    mutable std::map<std::string,DependencyKey> catalogueKeys_;
    DependencyKey catalogueChanges_ = std::make_shared<Dependency>();
    DependencyKey catalogueKey(const std::string& name) const {
        auto& key=catalogueKeys_[name];if(!key) key=std::make_shared<Dependency>();
        ReadScope::observe(key);return key;
    }
    // Read-only declaration compilation survives unrelated instance writes.
    // Every reuse validates the source witnesses and vocabulary bindings.
    std::set<POVNode> declarationBodies_;
    std::map<POVNode,Certificate> compiledBodies_;
    std::uint64_t validationEpoch_=1;
    struct CatalogueResolution {bool declared=false; POVNode body;};
    mutable std::map<std::string,CatalogueResolution> resolutions_;
    const CatalogueResolution& resolveCatalogue(const std::string& name) const {
        catalogueKey(name); // A cached absence is still an observed name.
        auto found=resolutions_.find(name);
        if(found!=resolutions_.end()) return found->second;
        CatalogueResolution result;bool ambiguous=false;
        auto entries=catalogue_.find(name);
        if(entries!=catalogue_.end()) for(const auto& entry:entries->second) {
            if(!access_->definitionApplies(entry.declaration)) continue;
            ambiguous |= result.declared || !access_->definitionSelectable(entry.declaration) ||
                access_->definitionName(entry.declaration)!=name || !registered(entry.declaration);
            result.declared=true;result.body=entry.body;
        }
        if(ambiguous) result.body.reset();
        return resolutions_.emplace(name,std::move(result)).first->second;
    }
    // Successful instance checks remain true until the next publication.
    // Do not cache a pending check: settling a child can establish completion.
    mutable std::set<std::tuple<POVNode,DefinitionSpec,bool>> accepted_;
    // Stable read-only POVs for existing template/owner infons. This exposes
    // stored relations to the walk; it does not clone or normalize their data.
    mutable std::map<std::shared_ptr<infonView>, POVNode> views_;
    mutable POVNode unspecifiedMember_;
    // Registry occurrence backlinks are non-owning. Retain each demanded
    // source for this context so a selected item can outlive its lookup handle.
    std::set<POVNode> occurrenceOwners_;
    POVNode view(const std::shared_ptr<infonView>& infon) const {
        if (!infon) return {};
        auto& result = views_[infon];
        if (!result) result = access_->requirementView(infon);
        return result;
    }
    POVNode memberTemplate(const POVNode& node) const { return view(access_->memberTemplate(node)); }
    POVNode occurrenceOwner(const POVNode& node) const { return view(access_->occurrenceOwner(node)); }
    bool listRequirement(const POVNode& node) const {
        return access_->positionalDefinitionList(node) || access_->repeatedDefinitionList(node) || access_->recordDefinitionList(node);
    }
    // Compiled declaration expressions retain symbolic scopes; instance binding
    // happens in the plan. An additional POV must not repeat application.
    using Key = std::weak_ptr<infonView>;
    std::map<Key, std::vector<Certificate>, std::owner_less<Key>> evidence_;
    mutable std::map<Key, DependencyKey, std::owner_less<Key>> evidenceKeys_;
    DependencyKey evidenceKey(const POVNode& node) const {
        auto& key=evidenceKeys_[readView(node)];
        if(!key) key=std::make_shared<Dependency>();
        ReadScope::observe(key);return key;
    }
    std::vector<POVNode> members(const POVNode& node) const {
        if(auto body=choiceBody(node)) {
            std::vector<POVNode> result;
            for(auto child=POVDescription(body).member();child;child=POVDescription(child).adjacent()) {
                if(access_->subList(child)) return {}; // embedded alternatives need a logical choice walk
                result.push_back(child);
            }
            return result;
        }
        auto raw = access_->definitionMembers(node); return {raw.begin(), raw.end()};
    }
    std::vector<POVNode> work(const POVNode& node) const {
        return POVDescription(node).work();
    }
public:
    explicit ProteusDefinitions(std::shared_ptr<ProteusGraphAccess> access, DependencyKey modelFacts = {})
        : access_(std::move(access)),modelFacts_(std::move(modelFacts)) {}
    Changes modelChanges() const {return Changes::only({modelFacts_});}
    POVNode choiceBody(const POVNode& node) const {
        auto item=readView(node);
        if(!item->isOrLike() || !access_->plain(node,true,false,false,false) || !access_->literalSelectionBody(item->innr) ||
           item->innr->partRef || item->innr->funcArgs || !item->innr->wrkList.empty() ||
           getFlagBit(item->innr->value->flags,infonState::tailUnfinished)) return {};
        auto body=view(item->innr);
        for(auto child=POVDescription(body).member();child;child=POVDescription(child).adjacent())
            if(access_->subList(child)) return {};
        return body;
    }
    GraphTypeFacts facts(const POVNode& node,bool predicate=false) const {
        if(choiceBody(node)) {GraphTypeFacts result;result.valid=true;return result;}
        auto result=predicate ? access_->predicateFacts(node,false) : access_->typeFacts(node);
        if(access_->literalList(node)) {
            auto order=listOrder(node);
            if(order<0) result.valid=false;
            if(order==uUnordered && !result.explicitSize) result.hasSize=false;
        }
        return result;
    }
    std::vector<std::string> names(const POVNode& node) const {
        auto raw = access_->typeNames(node);
        std::vector<std::string> result;
        for (const auto& name : raw)
            if (std::find(result.begin(), result.end(), name) == result.end()) result.push_back(name);
        return result;
    }
    // Type words and explicit alias links are sufficient for nominal selection.
    // Reading them must not force unrelated members of a definition to resolve.
    std::vector<std::string> nominalNames(std::vector<std::string> names, bool& complete, Work& work) const {
        std::set<POVNode> seen;
        std::vector<POVNode> pending;
        std::size_t next=0;
        while (next<names.size() || !pending.empty()) {
            if (pending.empty()) {
                const auto name=names[next++];++work.visits;
                if (auto body=lookup(name)) pending.push_back(body);
                else if (declared(name)) complete=false;
                continue;
            }
            auto node=pending.back();pending.pop_back();
            if (!seen.insert(node).second) continue;
            ++work.visits;
            if (!access_->plain(node,false,false,false,false)) {complete=false;continue;}
            for (const auto& name:this->names(node))
                if (std::find(names.begin(),names.end(),name)==names.end()) names.push_back(name);
            for (const auto& rhs:POVDescription(node).work()) pending.push_back(rhs);
        }
        return names;
    }
    bool registered(const POVNode& node) const {
        catalogueKey(access_->definitionName(node));
        if (!access_->definitionReady(node)) return false;
        auto found = catalogue_.find(access_->definitionName(node));
        if (found == catalogue_.end()) return false;
        for (const auto& entry : found->second)
            if (readView(entry.declaration) == readView(node) && readView(entry.body) == readView(node)->funcArgs) return true;
        return false;
    }
    bool publish(const POVNode& node) {
        if (!access_->definitionReady(node) || registered(node)) return false;
        // Direct catalogue clients also cross the validation boundary.
        prune();
        auto& entries = catalogue_[access_->definitionName(node)];
        for (auto& entry : entries) if (readView(entry.declaration) == readView(node)) {
            entry.body = access_->definitionBody(node);declarationBodies_.insert(entry.body);return true;
        }
        entries.push_back({node, access_->definitionBody(node)});
        declarationBodies_.insert(entries.back().body);return true;
    }
    SetResult publishChanges(const POVNode& node) {
        bool changed=publish(node);
        return {changed,Changes::only({catalogueChanges_,catalogueKey(access_->definitionName(node))})};
    }
    bool changesCatalogue(const Changes& changes) const {
        return changes.keys.count(catalogueChanges_);
    }
    bool declared(const std::string& name) const { return resolveCatalogue(name).declared; }
    // Missing declarations impose no constraints. Type identity still exists;
    // an unavailable definition never equates distinct written types.
    bool optional(const POVNode& node) const {
        for (const auto& name:names(node)) if (declared(name)) return false;
        return true;
    }
    POVNode lookup(const std::string& name) const { return resolveCatalogue(name).body; }
    DefinitionSpec unit(const std::shared_ptr<WordUse>& word,Work& work) try {
        if(!word) return {};
        DefinitionSpec result;
        if(auto body=lookup(word->typeID())) {
            auto proof=describe(body,work);if(!proof) return {};
            result=std::make_shared<DefinitionRequirement>(*proof->requirement);
        } else {
            if(declared(word->typeID())) return {};
            result=std::make_shared<DefinitionRequirement>();result->facts.valid=true;
        }
        result->label=word->typeID();result->writtenType=word;
        if(std::find(result->types.begin(),result->types.end(),result->label)==result->types.end()) result->types.push_back(result->label);
        return result;
    } catch(const DefinitionConflict&) { return {}; } // An optional conversion route cannot enforce an unrelated definition.
    POVNode repeatedBody(const POVNode& node) const { return memberTemplate(node); }
    // An untyped virtual position has no stored POV. One read-only unknown
    // describes that position; occurrence data remains in the search cursor.
    POVNode matchingBody(const POVNode& node) const {
        if (auto spec = memberTemplate(node)) return spec;
        if (!access_->openTail(node)) return {};
        if (!unspecifiedMember_) {
            auto item = std::make_shared<infonView>();
            SetModeBits(item->flags, infonView::viewMode, infonView::vmOverlay);
            SetModeBits(item->flags, infonView::itmMode, infonView::mItem);
            SetModeBits(item->infSize->flags, infonState::evalMode, infonState::emUnknown);
            SetModeBits(item->value->flags, infonState::overlayType, infonState::tUnknown);
            SetModeBits(item->value->flags, infonState::evalMode, infonState::emUnknown);
            unspecifiedMember_ = access_->requirementView(item);
        }
        return unspecifiedMember_;
    }
    POVNode payload(const POVNode& node) const { return view(readView(node)); }
    POVNode selfSource(const POVNode& node) const { return view(readView(node)->thisPtr); }
    POVNode selectionBody(const POVNode& node) const { return view(access_->selectionSource(node)); }
    void retainOccurrenceOwner(const POVNode& owner) { occurrenceOwners_.insert(owner); }
    POVNode retainedOccurrenceOwner(const POVNode& node) const {
        return retainedOwner(POVDescription(node).occurrenceSource());
    }
    POVNode retainedOwner(POV* source) const {
        for (const auto& owner : occurrenceOwners_) if (owner.get() == source) return owner;
        return {};
    }
    // Order is header evidence, independent of the parent's member meaning.
    // Read aliases through existing declaration POVs; never normalize a body.
    // -1 means competing/unavailable meanings, not a proved ordered list.
    int listOrder(const POVNode& node,Work* cost=nullptr) const {
        std::vector<POVNode> pending{node};std::set<POVNode> seen;
        int result=uUnknown;
        for(std::size_t i=0;i<pending.size();++i) {
            auto at=pending[i];if(!seen.insert(at).second) continue;
            if(cost) ++cost->visits;
            auto order=readView(at)->orderMode;
            // An explicit repeated member specification supplies positional
            // structure unless a comma has declared unordered membership.
            if(order==uUnknown && access_->memberTemplate(at)) order=uOrdered;
            if(order!=uUnknown) {
                if(result!=uUnknown && result!=order) return -1;
                result=order;continue;
            }
            for(const auto& name:names(at)) {
                if(auto body=lookup(name)) pending.push_back(body);
                else if(declared(name)) return -1;
            }
        }
        return result;
    }
    bool pendingOrder(const POVNode& node) const {
        return access_->literalList(node) && readView(node)->orderMode==uUnknown && listOrder(node)!=uUnknown;
    }
    std::vector<SetCommand> orderCommands(const POVNode& node) const {
        if(!access_->literalList(node) || readView(node)->orderMode!=uUnknown) return {};
        Work cost{0,0};auto order=listOrder(node,&cost);
        if(order<=uUnknown) return {};
        if(order==uUnordered && readView(node)->value->sizeMode==fromCount) ++cost.creations;
        auto access=access_;
        return {{"Set list order from type header",[access,node,order] {
            return access->setListOrder(node,static_cast<OrderMode>(order));
        },cost}};
    }
    // A supplied description of typed parts is useful on its own. Incidental
    // traversal demands its present members, not every part of the container's type.
    // Bare type references and positional value sequences still expand through
    // their definitions; explicit RHSs remain ordinary graph constraints.
    bool describesParts(const POVNode& node) const {
        if (!access_->physicalList(node) || access_->memberTemplate(node) || access_->occurrenceOwner(node)) return false;
        SpanLayout<POVNode> layout;
        std::uint64_t visits = 0;
        if (!readListLayout(*access_, node, layout, visits,
                [&](const auto& at) { return at.phase != ListPhase::enter || access_->physicalList(at.node); },
                [](const POVNode& member) { return member; })) return false;
        bool supplied = false;
        for (const auto& segment : layout.segments) if (!segment.gap) {
            if (!access_->hasTypeRequirements(segment.node) && !WordForms::collectionType(*access_,segment.node)) return false;
            supplied = true;
        }
        return supplied;
    }
    bool required(const POVNode& node) const {
        return (access_->hasTypeRequirements(node) && !optional(node)) ||
            (access_->memberTemplate(node) && !access_->fedTemplate(node)) || access_->occurrenceOwner(node);
    }
    static bool selfExpression(const POVNode& node) {
        return getModeBits(readView(node)->flags,infonView::viewMode)==infonView::vmCursor &&
            getModeBits(readView(node)->flags,infonView::cursorMode)==infonView::cmSelf;
    }
    Certificate compiledBody(const POVNode& node,const POVNode& root) const {
        if(node==root) return {};
        auto found=compiledBodies_.find(node);
        return found!=compiledBodies_.end() && current(found->second) ? found->second : Certificate{};
    }
    // Explicit normalization demands the named value through its alias chain.
    // Incidental reads still use only a supplied partial description.
    bool definitionNames(const POVNode& node,const POVNode& root,bool fullMeaning,
                         std::vector<std::string>& result,Work* cost=nullptr) const {
        result=names(node);
        if(node!=root || !fullMeaning) return true;
        bool complete=true;Work work{0,0};
        result=nominalNames(std::move(result),complete,work);
        if(cost) cost->visits=addCost(cost->visits,work.visits);
        return complete;
    }
    bool children(const POVNode& node, const POVNode& root, std::vector<POVNode>& result, bool fullMeaning=false) const {
        if(compiledBody(node,root)) return true;
        if(node!=root && selfExpression(node)) return true;
        if (!facts(node,node==root).valid) return false;
        if (node != root && !access_->scalarTypeShape(node) && !listRequirement(node) && !choiceBody(node)) return false;
        // Registry constraints within a shared body need a read-only relation
        // protocol. Keep them visible as deferred work rather than ignoring them.
        if (node != root && !POVDescription(node, access_->registry.get()).relationships(false).empty()) return false;
        if (node != root && access_->definitionEmbedded(node) && access_->repeatedDefinitionList(node)) return false;
        auto add = [&](const POVNode& child) {
            if (std::find(result.begin(), result.end(), child) == result.end()) result.push_back(child);
        };
        std::vector<std::string> required;
        if(!definitionNames(node,root,fullMeaning,required)) return false;
        if(node==root || !describesParts(node)) for (const auto& name : required) {
            auto body = lookup(name);
            if (body && body!=node) add(body);
            else if (!body && declared(name)) return false;
        }
        if (auto spec = memberTemplate(node)) add(spec);
        if (node == root) if (auto owner = occurrenceOwner(node)) add(owner);
        if (node != root) {
            for (auto rhs : work(node)) add(rhs);
            for (auto member : members(node)) add(member);
        }
        return true;
    }
    void collect(const POVNode& node, const POVNode& root, const Certificate& proof, bool fullMeaning=false) {
        if(auto cached=compiledBody(node,root)) if(cached->evidence!=proof->evidence) {
            proof->evidence->witnesses.insert(cached->evidence->witnesses.begin(),cached->evidence->witnesses.end());
            for(const auto& binding:cached->evidence->bindings)
                if(std::none_of(proof->evidence->bindings.begin(),proof->evidence->bindings.end(),[&](const Binding& old){return old.name==binding.name;}))
                    proof->evidence->bindings.push_back(binding);
        }
        std::vector<std::string> required;
        if(!definitionNames(node,root,fullMeaning,required,&proof->composition)) proof->compilable=false;
        for (const auto& name : required) {
            auto body = lookup(name);
            auto& bindings = proof->evidence->bindings;
            if (std::none_of(bindings.begin(), bindings.end(), [&](const Binding& b) { return b.name == name; }))
                bindings.push_back({name, body});
        }
        if (node == root || proof->evidence->witnesses.count(node)) return;
        proof->evidence->witnesses.emplace(node, std::make_shared<const Witness>(Witness{node, readView(node), facts(node),
            names(node), work(node), members(node), listRequirement(node), access_->definitionEmbedded(node), access_->openTail(node), (listRequirement(node) ? listOrder(node,&proof->composition) : uUnknown), memberTemplate(node), choiceBody(node)}));
    }
    // The graph's leave edge supplies postorder: every required child has
    // already been walked. Compile requirements, never execute child normalize.
    void finish(const POVNode& node, const POVNode& root, const Certificate& proof, bool fullMeaning=false) try {
        if (!proof->compilable) return;
        if(auto cached=compiledBody(node,root)) {
            proof->evidence->compiled[node]=cached->requirement;
            proof->composition.visits=addCost(proof->composition.visits,cached->evidence->witnesses.size());
            return;
        }
        if(node!=root && selfExpression(node)) {
            auto expression=std::make_shared<DefinitionRequirement>();expression->facts.valid=true;
            expression->expressions.push_back({node,{}});
            proof->evidence->compiled[node]=expression;return;
        }
        auto& compiled = proof->evidence->compiled;
        auto requirement = std::make_shared<DefinitionRequirement>();
        requirement->facts.valid = true;
        requirement->types = names(node);
        requirement->captureLabel(node);
        if(!requirement->writtenType) if(auto type=WordForms::collectionType(*access_,node)) {
            // Infer the same nominal collection label as normalization without
            // recursively applying a declaration for that inferred spelling.
            requirement->writtenType=type;requirement->label=type->typeID();
            requirement->types.push_back(type->typeID());
        }
        std::sort(requirement->types.begin(), requirement->types.end());
        if (node != root) {
            const auto& witness = *proof->evidence->witnesses.at(node);
            requirement->facts = witness.facts;
            requirement->list = witness.list; requirement->order=witness.order==uUnknown ? uOrdered : witness.order;
            requirement->open = witness.list && (witness.open || witness.order==uUnordered);
            if(witness.choiceBody) {
                DefinitionRequirement::Choice choice;choice.source=node;
                for(const auto& member:witness.members) choice.alternatives.push_back(compiled.at(member));
                requirement->choices.push_back(std::move(choice));
            }
            for (const auto& member : witness.members) {
                if(witness.choiceBody) break;
                auto child = compiled.at(member);
                if (access_->definitionEmbedded(member)) {
                    if (!child->list) throw DefinitionConflict("non-list embedded definition span");
                    // A named alias may reveal repetition only after its walk.
                    // Never mistake its empty physical prefix for zero items.
                    if (child->open) { proof->compilable = false; return; }
                    requirement->members.insert(requirement->members.end(), child->members.begin(), child->members.end());
                } else requirement->members.push_back(child);
            }
            if (access_->positionalDefinitionList(node) && !requirement->open) {
                GraphTypeFacts extent;
                extent.valid = true; extent.hasSize = true;
                extent.size = static_cast<std::int64_t>(requirement->members.size());
                if (!requirement->facts.combine(extent)) throw DefinitionConflict("incompatible finite definition extent");
            }
        }
        if (auto source = memberTemplate(node)) {
            auto repeated = std::make_shared<DefinitionRequirement>();
            repeated->facts.valid = true; repeated->facts.overlay = infonState::LST;
            repeated->list = true; repeated->open = true; repeated->repeated = compiled.at(source); repeated->templateSource = source;
            // A local member template constrains each present part without
            // turning an explicitly unordered collection into an ordered one.
            repeated->order=node!=root ? uOrdered : uUnknown;
            // An open body denotes repeated positions, not an empty finite list.
            if (node != root && access_->repeatedDefinitionList(node)) {
                requirement->repeated = repeated->repeated; requirement->templateSource = source;
            } else requirement = combineDefinitions(requirement, repeated,proof->composition);
        }
        if (node == root) if (auto owner = occurrenceOwner(node)) {
            auto body=compiled.at(owner);
            auto index=POVDescription(node).occurrenceIndex();
            auto member=index>=0 && static_cast<std::size_t>(index)<body->members.size() ? body->members[index] : body->repeated;
            if (!member) { proof->compilable = false; return; }
            requirement = combineDefinitions(requirement, member,proof->composition);
        }
        std::vector<std::string> required;
        if(!definitionNames(node,root,fullMeaning,required,&proof->composition)) {proof->compilable=false;return;}
        if(node==root || !describesParts(node)) for (const auto& name : required) {
            if (auto body=lookup(name);body && body!=node) {
                auto meaning=scopeDefinition(compiled.at(body),body,proof->composition);
                if(meaning!=compiled.at(body)) meaning->selfScopes.push_back(body);
                requirement = combineDefinitions(requirement, definitionInUnits(meaning,requirement),proof->composition);
            }
            if(std::find(requirement->expandedTypes.begin(),requirement->expandedTypes.end(),name)==requirement->expandedTypes.end())
                requirement->expandedTypes.push_back(name);
        }
        std::sort(requirement->expandedTypes.begin(),requirement->expandedTypes.end());
        if (node != root)
            for (auto rhs : work(node)) requirement = combineDefinitions(requirement, compiled.at(rhs),proof->composition);
        compiled[node] = requirement;
        if(node!=root && declarationBodies_.count(node)) {
            auto cached=std::make_shared<Proof>();cached->requirement=requirement;
            std::vector<POVNode> sources{node};
            for(std::size_t i=0;i<sources.size();++i) {
                auto source=sources[i];
                auto witness=proof->evidence->witnesses.find(source);
                if(witness==proof->evidence->witnesses.end() || !cached->evidence->witnesses.insert(*witness).second) continue;
                const auto& value=*witness->second;
                sources.insert(sources.end(),value.work.begin(),value.work.end());
                sources.insert(sources.end(),value.members.begin(),value.members.end());
                if(value.memberTemplate) sources.push_back(value.memberTemplate);
                for(const auto& name:value.names) for(const auto& binding:proof->evidence->bindings) if(binding.name==name) {
                    if(std::none_of(cached->evidence->bindings.begin(),cached->evidence->bindings.end(),[&](const Binding& old){return old.name==name;}))
                        cached->evidence->bindings.push_back(binding);
                    if(binding.body) sources.push_back(binding.body);
                    break;
                }
            }
            proof->composition.visits=addCost(proof->composition.visits,sources.size());
            compiledBodies_[node]=cached;
        }
        if (node == root) {
            proof->requirement = requirement;
            // Compilation scratch must not retain the target through its own
            // weak-keyed certificate cache after the walk finishes.
            compiled.clear();
        }
    } catch(const DefinitionPending&) {proof->compilable=false;}
    // Compile one selected declaration subtree, including its written facts.
    // A null instance root lets the existing compiler treat every visited POV
    // as read-only source data. Unselected siblings are never compiled.
    Certificate describe(const POVNode& source, Work& work) {
        auto proof=std::make_shared<Proof>();
        std::vector<std::pair<POVNode,bool>> pending{{source,false}};
        std::set<POVNode> active;
        while(!pending.empty()) {
            auto [node,leave]=pending.back();pending.pop_back();++work.visits;
            if(leave) {finish(node,{},proof);active.erase(node);if(!proof->compilable) return {};continue;}
            if(proof->evidence->compiled.count(node)) continue;
            if(!active.insert(node).second) return {};
            std::vector<POVNode> next;
            if(!children(node,{},next)) return {};
            collect(node,{},proof);pending.push_back({node,true});
            for(auto i=next.rbegin();i!=next.rend();++i) pending.push_back({*i,false});
        }
        work.visits=addCost(work.visits,proof->composition.visits);
        proof->requirement=proof->evidence->compiled.at(source);
        proof->evidence->compiled.clear();
        // Speculative readers do not bind %self. They must not treat an
        // unevaluated contextual constraint as an unconstrained predicate.
        return proof->requirement->hasExpressions() ? Certificate{} : proof;
    }
    bool current(const Witness& witness) const {
        if(witness.checkedEpoch!=validationEpoch_) {
            std::set<DependencyKey> reads;
            bool matches;
            {
                ReadScope capture([&](const DependencyKey& key){reads.insert(key);});
                matches = readView(witness.node) == witness.infon && facts(witness.node).same(witness.facts) &&
                    witness.names == names(witness.node) && witness.work == work(witness.node) &&
                    witness.list == listRequirement(witness.node) && witness.members == members(witness.node) &&
                    witness.embedded == access_->definitionEmbedded(witness.node) && witness.open == access_->openTail(witness.node) &&
                    witness.order == (witness.list ? listOrder(witness.node) : uUnknown) &&
                    witness.memberTemplate == memberTemplate(witness.node) && witness.choiceBody==choiceBody(witness.node) &&
                    POVDescription(witness.node, access_->registry.get()).relationships(false).empty();
            }
            witness.reads=std::move(reads);witness.checkedCurrent=matches;witness.checkedEpoch=validationEpoch_;
        }
        for(const auto& key:witness.reads) ReadScope::observe(key);
        return witness.checkedCurrent;
    }
    bool current(const Certificate& proof) const {
        if (!proof->requirement) return false;
        auto& evidence=*proof->evidence;
        bool observed=evidence.reads && evidence.reads->valid();
        if(observed && evidence.checkedEpoch==validationEpoch_) return evidence.checkedCurrent;
        std::set<DependencyKey> reads;
        bool matches;
        {
            ReadScope capture([&](const DependencyKey& key){reads.insert(key);});
            // Retain names even when an earlier binding invalidates the proof.
            for(const auto& binding:evidence.bindings) catalogueKey(binding.name);
            matches=std::all_of(evidence.bindings.begin(),evidence.bindings.end(),[&](const Binding& binding) {
                return lookup(binding.name)==binding.body && (binding.body || !declared(binding.name));
            }) && std::all_of(evidence.witnesses.begin(),evidence.witnesses.end(),[&](const auto& entry) {
                return current(*entry.second);
            });
        }
        // Keep the observable group across unrelated model writes. A changed
        // name retires it; existing readers still retain that older evidence.
        if(!observed) evidence.reads=DependencyProof::make(reads);
        else evidence.reads->observe(reads);
        evidence.reads->valid();
        evidence.checkedCurrent=matches;evidence.checkedEpoch=validationEpoch_;
        return matches;
    }
    bool covers(const Certificate& proof, const POVNode& node, bool fullMeaning=false) const {
        if (!proof->requirement) return false;
        // Root-local template/occurrence relationships are not captured as a
        // witness of the target itself (which would retain its weak cache key).
        // Require their current source to have participated in this proof.
        if (auto source = memberTemplate(node)) if (!proof->evidence->witnesses.count(source)) return false;
        if (auto owner = occurrenceOwner(node)) if (!proof->evidence->witnesses.count(owner)) return false;
        const auto& types = proof->requirement->types;
        std::vector<std::string> required;
        if(!definitionNames(node,node,fullMeaning,required)) return false;
        for (const auto& name : required) {
            if (std::find(types.begin(), types.end(), name) == types.end()) return false;
            const auto& expanded=proof->requirement->expandedTypes;
            if(fullMeaning && std::find(expanded.begin(),expanded.end(),name)==expanded.end()) return false;
        }
        return true;
    }
    Certificate saved(const POVNode& node, bool fullMeaning=false) const {
        evidenceKey(node);
        auto found = evidence_.find(readView(node)); if (found == evidence_.end()) return {};
        for (const auto& proof : found->second) if (current(proof) && covers(proof, node, fullMeaning)) return proof;
        return {};
    }
    // Meaning evidence can be compiled without applying it to a candidate or
    // a shared grammar template. Nominal compatibility is checked separately.
    GraphTypeFacts scalarFacts(const POVNode& node, bool predicate = false) const {
        auto facts = predicate ? access_->predicateFacts(node,false) : access_->typeFacts(node);
        if (!facts.valid || !access_->hasTypeRequirements(node) || optional(node)) return facts;
        auto proof = saved(node);
        if (!proof || proof->requirement->hasExpressions() || proof->requirement->list || !proof->requirement->choices.empty() || !facts.combine(proof->requirement->facts)) facts.valid = false;
        return facts;
    }
    bool accepts(const Certificate& proof, const POVNode& node, bool fillParts=false) const {
        auto key=std::make_tuple(node,proof->requirement,fillParts);
        // Contextual expressions consume other goals' completion evidence.
        // Reobserve it on each use; plain data checks remain revision-cached.
        bool cacheable=!proof->requirement->hasExpressions();
        if(cacheable && accepted_.count(key)) return true;
        try {
            if(!ProteusDefinitionPlan(access_, node, proof->requirement, {}, true, fillParts).satisfied()) return false;
            if(cacheable) accepted_.insert(std::move(key));return true;
        } catch (const std::runtime_error&) { return false; }
    }
    bool satisfied(const POVNode& node, bool fillParts=false) const {
        if (!required(node)) return true;
        auto proof = saved(node,fillParts); return proof && accepts(proof, node, fillParts);
    }
    bool remember(const POVNode& node, const Certificate& proof) {
        evidenceKey(node);
        auto& certificates = evidence_[readView(node)];
        certificates.erase(std::remove_if(certificates.begin(), certificates.end(),
            [&](const Certificate& old) { return !current(old); }), certificates.end());
        for (const auto& old : certificates) {
            if (old == proof) return false;
            if (sameDefinition(old->requirement, proof->requirement) &&
                old->evidence->bindings.size() == proof->evidence->bindings.size() &&
                old->evidence->witnesses.size() == proof->evidence->witnesses.size() &&
                std::all_of(proof->evidence->witnesses.begin(), proof->evidence->witnesses.end(),
                    [&](const auto& entry) { return old->evidence->witnesses.count(entry.first); }) &&
                std::all_of(proof->evidence->bindings.begin(), proof->evidence->bindings.end(), [&](const Binding& binding) {
                    return std::any_of(old->evidence->bindings.begin(), old->evidence->bindings.end(), [&](const Binding& other) {
                        return binding.name == other.name && binding.body == other.body;
                    });
                })) return false;
        }
        certificates.push_back(proof); return true;
    }
    std::shared_ptr<ProteusDefinitionPlan> plan(const POVNode& node, const Certificate& proof, bool fillParts=false) const {
        if (!proof || !current(proof) || !covers(proof, node, fillParts)) return {};
        return std::make_shared<ProteusDefinitionPlan>(access_, node, proof->requirement, std::map<POVNode,std::int64_t>{}, true, fillParts);
    }
    SetResult rememberChanges(const POVNode& node, const Certificate& proof) {
        bool changed=remember(node,proof);
        return {changed,Changes::only({evidenceKey(node)})};
    }
    SetResult applyChanges(const POVNode& node, const Certificate& proof, const std::shared_ptr<ProteusDefinitionPlan>& plan) {
        if (!current(proof) || !covers(proof, node) || !plan->supported) return false;
        Changes changes=Changes::only({});
        if(plan->apply()) changes.include(modelChanges());
        for (const auto& binding : plan->bindings) {
            auto child = std::make_shared<Proof>();
            child->evidence = proof->evidence; child->requirement = binding.second;
            if(remember(binding.first->node,child)) changes.include(Changes::only({evidenceKey(binding.first->node)}));
        }
        return {!changes.empty(),changes};
    }
    bool apply(const POVNode& node, const Certificate& proof, const std::shared_ptr<ProteusDefinitionPlan>& plan) {
        return applyChanges(node,proof,plan);
    }
    std::shared_ptr<ProteusDefinitionPlan> reusable(const POVNode& target, const POVNode& source, const Certificate& proof) const {
        if (!current(proof)) return {};
        try {
            // This can run after another write in the same atomic Set; a
            // pre-Set instance check does not describe that intermediate state.
            ProteusDefinitionPlan sourcePlan(access_, source, proof->requirement);
            if (!sourcePlan.dataSatisfied()) return {};
            auto targetPlan = std::make_shared<ProteusDefinitionPlan>(access_, target, proof->requirement);
            return targetPlan->dataSatisfied() ? targetPlan : nullptr;
        } catch (const std::runtime_error&) { return {}; }
    }
    bool canInherit(const POVNode& target, const POVNode& source) const {
        evidenceKey(source);
        auto found = evidence_.find(readView(source)); if (found == evidence_.end()) return false;
        for (const auto& proof : found->second) if (reusable(target, source, proof)) return true;
        return false;
    }
    SetResult inheritChanges(const POVNode& target, const POVNode& source) {
        evidenceKey(source);
        auto found = evidence_.find(readView(source)); if (found == evidence_.end()) return false;
        auto certificates = found->second;
        Changes changes=Changes::only({});
        std::vector<std::pair<POVNode,Certificate>> checked;
        for (const auto& proof : certificates) if (auto plan = reusable(target, source, proof)) {
            // Only order metadata may remain: required contents already exist.
            bool transferable=!proof->requirement->hasExpressions();
            if(plan->apply()) changes.include(modelChanges());
            for (const auto& binding : plan->bindings) {
                auto child = std::make_shared<Proof>();
                child->evidence = proof->evidence; child->requirement = binding.second;
                if(remember(binding.first->node,child)) changes.include(Changes::only({evidenceKey(binding.first->node)}));
                // Contextual requirements belong to their bound instance and
                // consume goal-completion proofs; only data checks transfer.
                if(transferable) checked.push_back({binding.first->node,child});
            }
        }
        SetResult result{!changes.empty(),changes};
        if(!checked.empty()) result.retainEvidence=[this,checked=std::move(checked)] {
            for(const auto& entry:checked) {
                // A Set can also affect a declaration used by its own proof.
                // Its new data check cannot certify an obsolete meaning.
                if(!current(entry.second)) continue;
                auto found=evidence_.find(readView(entry.first));
                if(found==evidence_.end()) continue;
                for(const auto& proof:found->second)
                    if(sameDefinition(proof->requirement,entry.second->requirement) && current(proof))
                        accepted_.insert(std::make_tuple(entry.first,proof->requirement,false));
            }
        };
        return result;
    }
    bool inherit(const POVNode& target, const POVNode& source) {return inheritChanges(target,source);}

    // Publish a sparse header independently of its still-symbolic members.
    // Only top-level conjuncts are read; full member validation stays pending.
    // All allocation and mutation still belongs to the priced Set.
    std::vector<SetCommand> exposeShape(const POVNode& target) const {
        std::vector<POVNode> pending;
        auto meanings=[&](const POVNode& node) {
            for(const auto& name:names(node)) {
                if(auto body=lookup(name)) pending.push_back(body);
                else if(declared(name)) return false;
            }
            return true;
        };
        if(!meanings(target)) return {};
        if(auto owner=occurrenceOwner(target)) {
            auto member=memberTemplate(owner);
            if(!member) return {};
            pending.push_back(member);
        }
        Work cost;std::set<POVNode> seen;
        GraphTypeFacts required;required.valid=true;
        POVNode specification;
        for(std::size_t i=0;i<pending.size();++i) {
            auto source=pending[i];if(!seen.insert(source).second) continue;
            ++cost.visits;
            auto header=facts(source);
            if(!header.valid || !POVDescription(source,access_->registry.get()).relationships(false).empty()) return {};
            if(!required.combine(header)) throw DefinitionConflict("incompatible definition headers");
            if(auto member=memberTemplate(source)) {
                if(!access_->templatePrefixList(source) || (specification && readView(specification)!=readView(member))) return {};
                specification=member;
            }
            if(!meanings(source)) return {};
            for(const auto& rhs:POVDescription(source).work()) pending.push_back(rhs);
        }
        if(!specification) return {};
        auto actual=access_->typeFacts(target),combined=actual;
        if(!actual.valid) return {};
        if(!combined.combine(required)) throw DefinitionConflict("incompatible definition headers");
        bool adopt=access_->adoptableList(target);
        if(!adopt && ((!access_->physicalList(target) && !access_->templatePrefixList(target)) ||
                      !access_->definitionOrderedCandidate(target))) return {};
        auto existing=memberTemplate(target);
        if(existing && readView(existing)!=readView(specification)) return {};
        bool install=(adopt || access_->openTail(target)) && !existing;
        bool order=!access_->definitionOrderKnown(target);
        if(!adopt && !install && !order && required.accepts(actual)) return {};
        cost.creations=static_cast<unsigned>(adopt)+static_cast<unsigned>(install);
        auto access=access_;
        return {{"Expose definition list shape",[access,target,required,specification,adopt,install] {
            bool changed=adopt && access->adoptDefinitionList(target);
            changed=access->applyTypeFacts(target,required) || changed;
            changed=access->setDefinitionOrder(target) || changed;
            if(install) changed=access->installMemberTemplate(target,specification) || changed;
            return changed;
        },cost}};
    }
    std::vector<SetCommand> exposeWrittenStructure(const POVNode& target) {
        // A bare use can acquire finite written structure before the complete
        // meaning is compiled. Sparse templates and embedded spans keep their
        // own shape goals; this Set never records a type certificate.
        if(!access_->adoptableList(target)) return {};
        for(const auto& name:names(target)) if(auto body=lookup(name)) {
            // An unordered record's implicit remainder does not hide its
            // written fields. Explicit unfinished syntax still uses the span
            // goals; exposing these fields never certifies total membership.
            if(!access_->literalList(body) || access_->inputPending(body) || access_->memberTemplate(body) ||
               getFlagBit(readView(body)->value->flags,infonState::tailUnfinished)) continue;
            bool embedded=false;
            for(auto member=POVDescription(body).member();member;member=member->next) embedded |= access_->definitionEmbedded(member);
            if(embedded) continue;
            auto required=facts(body),actual=access_->typeFacts(target),combined=actual;
            if(!actual.valid || !required.valid) continue;
            if(!combined.combine(required)) throw DefinitionConflict("incompatible definition headers");
            Work cost{1,2};
            std::vector<POVNode> pending{body};std::set<POVNode> seen;
            while(!pending.empty()) {
                auto at=pending.back();pending.pop_back();if(!seen.insert(at).second) continue;
                ++cost.visits;cost.creations+=2;
                for(auto child=POVDescription(at).member();child;child=child->next) pending.push_back(child);
                for(const auto& rhs:POVDescription(at).work()) pending.push_back(rhs);
            }
            auto access=access_;
            return {{"Expose written definition structure",[access,target,body,required] {
                auto original=readView(target);
                auto instance=std::make_shared<infonView>();
                std::vector<infonView*> copied;
                readView(body)->_copyAsTypeTo(instance,true,true,original,&copied);
                original->value=instance->value;
                original->orderMode=instance->orderMode;
                if(instance->type && (!original->type || original->type->typeID()!=instance->type->typeID()))
                    original->addedTypes.push_back(std::make_shared<WordUse>(*instance->type));
                for(const auto& type:instance->addedTypes) original->addedTypes.push_back(type);
                for(const auto& rhs:instance->wrkList) original->addPOVToWrkList(rhs);
                SetModeBits(original->flags,infonView::viewMode,infonView::vmOverlay);
                SetFlagBit(original->flags,infonView::isNormed,false);
                access->applyTypeFacts(target,required);
                access->bind(target);++target->workRevision;
                return true;
            },cost}};
        }
        return {};
    }
    void prune() {
        ++validationEpoch_;accepted_.clear();resolutions_.clear();
        for(auto it=evidenceKeys_.begin();it!=evidenceKeys_.end();)
            if(it->first.expired()) it=evidenceKeys_.erase(it);else ++it;
        for (auto it = evidence_.begin(); it != evidence_.end();)
            if (it->first.expired()) it = evidence_.erase(it); else ++it;
    }
};

class ProteusTypeGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusDefinitions> definitions_;
    ProteusDefinitions::Certificate proof_;
    Visit<POVNode> entry_;
    bool entered_=false;
    std::function<void(const POVNode&,const POVNode&,const DependencyProof::Ptr&)> representedTemplate_;
public:
    ProteusTypeGoal(Query<POVNode> query, std::shared_ptr<ProteusDefinitions> definitions,
                   std::function<void(const POVNode&,const POVNode&,const DependencyProof::Ptr&)> representedTemplate)
        : GoalSpec<POVNode>(query), definitions_(std::move(definitions)), representedTemplate_(std::move(representedTemplate)) {}
    void settled(bool resolved, const DependencyProof::Ptr& evidence) override {
        // A proved template may be represented while member values remain
        // unknown. Reuse this goal's proof, without an unpriced second scan.
        if(resolved && request.kind==typeRequirements)
            if(auto source=definitions_->repeatedBody(request.node)) representedTemplate_(request.node,source,evidence);
    }
    void beginPass() override { proof_ = std::make_shared<ProteusDefinitions::Proof>(); entered_=false; }
    CommandBatch<POVNode> finishPass() override {
        if(request.kind!=typeRequirements || !entered_ || satisfied()) return {};
        auto commands=definitions_->exposeShape(request.node);
        bool recursive=false;
        if(request.argument!=fillImmediateParts) {
            const auto names=definitions_->names(request.node);
            for(const auto& ancestor:request.context) if(ancestor!=request.node)
                for(const auto& name:definitions_->names(ancestor))
                    recursive |= std::find(names.begin(),names.end(),name)!=names.end();
        }
        // This fallback exposes facts hidden by unfinished compilation. It
        // must not bypass a compiled plan that cannot establish correspondence.
        if(commands.empty() && !recursive && (!proof_->compilable || !proof_->requirement))
            commands=definitions_->exposeWrittenStructure(request.node);
        return {entry_,std::move(commands)};
    }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        auto q = visit.query;
        if(q.kind==typeRequirements) {entry_=visit;entered_=true;}
        if (q.kind == definitionEnter) definitions_->collect(q.node, request.node, proof_,request.kind==typeRequirements && request.argument==fillImmediateParts);
        if (q.kind == definitionLeave) definitions_->finish(q.node, request.node, proof_,request.kind==typeRequirements && request.argument==fillImmediateParts);
        bool entry = q.kind == typeRequirements || q.kind == compileTypeRequirements;
        auto certificate = entry ? definitions_->saved(request.node,request.kind==typeRequirements && request.argument==fillImmediateParts) : proof_;
        if (q.kind != definitionEnd && (!entry || !certificate || satisfied())) return {};
        if (request.kind == compileTypeRequirements) {
            if (!certificate || !certificate->compilable || !certificate->requirement) return {};
            auto definitions = definitions_; auto target = request.node;
            return {{"Record read-only type meaning", [definitions, target, certificate] {
                return definitions->rememberChanges(target, certificate);
            }, {entry ? 1 : addCost(1,certificate->composition.visits), 0}}};
        }
        auto plan = definitions_->plan(request.node, certificate, request.argument==fillImmediateParts);
        if (!plan || !plan->supported) return {};
        if(!entry) plan->work.visits=addCost(plan->work.visits,certificate->composition.visits);
        auto definitions = definitions_; auto target = request.node;
        return {{certificate->requirement->repeated ? "Apply repeated member requirements" :
            certificate->requirement->list ? "Apply finite list type requirements" : "Apply scalar type requirements",
            [definitions, target, certificate, plan] { return definitions->applyChanges(target, certificate, plan); }, plan->work}};
    }
    bool satisfied() const override {
        return request.kind == compileTypeRequirements ?
            (!definitions_->required(request.node) || bool(definitions_->saved(request.node))) : definitions_->satisfied(request.node, request.argument==fillImmediateParts);
    }
    bool symbolic() const override { return !satisfied(); }
};
