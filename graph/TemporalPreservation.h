#pragma once

// Adjacency supplies a guarded route, never equality of successive states.
// Composite preservation exposes fresh parts and correspondence one level at a
// time. Value inheritance and missing-part construction retain their guards.
class ProteusPreservationGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    static constexpr std::int64_t linkExistingParts=1;
    bool correspondenceOnly() const {return request.argument==linkExistingParts;}
    // Existing membership can expose correspondence before child values settle.
    // Direct relationships and root expressions still need their guarded pass.
    bool correspondenceReady(const POVNode& node) const {
        if(!access_->literalList(node) || access_->inputPending(node)) return false;
        // A retained identity whose facts are already represented cannot add
        // a competing part. Unresolved relationships still block correspondence.
        for(const auto& relation:POVDescription(node,access_->registry.get()).relationships()) {
            RHSRepresentation proof(access_,node,relation.target,definitions_);
            if(!proof.represented) return false;
        }
        if(!definitions_->required(node)) return true;
        auto proof=definitions_->saved(node);
        return proof && proof->requirement && proof->requirement->expressions.empty() &&
            proof->requirement->choices.empty() && !proof->requirement->independentTemporalValue &&
            proof->requirement->facts.accepts(access_->typeFacts(node));
    }
    bool done_=false;
    Visit<POVNode> finding_;
    bool disabled() const {return POVDescription(request.node).independentTemporalValue();}
    static bool link(const POVNode& target,const POVNode& source) {
        auto& prior=readView(target)->temporalPredecessor;
        if(prior==source) return false;
        if(prior) return false; // competing correspondence is not replacement
        prior=source;++target->workRevision;
        SetFlagBit(readView(target)->flags,infonView::isNormed,false);return true;
    }
    // An absent optional definition is not evidence that an unresolved word
    // denotes an extra part. Keep that negative correspondence provisional.
    bool unresolvedWord(const POVNode& node) const {
        return access_->hasTypeRequirements(node) && access_->unknownOverlay(node) &&
               !(access_->known(node)&valueOnly) && definitions_->optional(node);
    }
    bool members(const POVNode& owner,std::vector<POVNode>& parts,Work& work) const {
        if(readView(owner)->orderMode==uUnordered)
            return RecordStructure::members(*access_,owner,parts,work) &&
                !access_->inputPending(owner) && !getFlagBit(readView(owner)->value->flags,infonState::tailUnfinished);
        SpanLayout<POVNode> layout;
        if(!readListLayout(*access_,owner,layout,work.visits,[&](const auto& at) {
            return at.phase!=ListPhase::enter || access_->physicalList(at.node);
        },[](const POVNode& node){return node;}) || !layout.solve() || !layout.concrete()) return false;
        for(const auto& segment:layout.segments) if(!segment.gap) parts.push_back(segment.node);
        return true;
    }
public:
    // Normalization and path access advertise the same stored/virtual routes.
    // A selection can use a known member while unrelated preservation waits.
    static void contribute(ProteusGraphAccess& access,const POVNode& node,
                           const std::vector<POVNode>& context,Expansion<POVNode>& out,bool required=true) {
        POVDescription view(node);
        if(view.independentTemporalValue() || access.isDefinition(node) ||
           (access.scalarCandidate(node) && (access.known(node)&valueOnly))) return;
        if(view.temporalPredecessor())
            out.refinements.push_back({{node,preserveTemporal,0,{},context},{1,0},"preserve corresponding prior part",!required,false});
        else if(context.size()>1 && context.front()==node && BoundaryGraph::temporal(context.back()) &&
                !BoundaryGraph::temporal(node) && !access.subList(node)) {
            auto owners=context;owners.erase(owners.begin());
            out.refinements.push_back({{node,temporalPredecessor,0,{},owners},{1,0},"find previous temporal occurrence",true,false});
        }
    }
    ProteusPreservationGoal(Query<POVNode> q,std::shared_ptr<ProteusGraphAccess> access,
                           std::shared_ptr<ProteusDefinitions> definitions)
        :GoalSpec<POVNode>(std::move(q)),access_(std::move(access)),definitions_(std::move(definitions)) {}
    void beginPass() override {done_=false;finding_={request,{},0};}
    bool waitForRequirements() const override {return request.kind==preserveTemporal && !correspondenceOnly();}
    std::optional<Expansion<POVNode>> expand(const Visit<POVNode>& visit) override {
        if(visit.query.kind!=request.kind) return std::nullopt;
        Expansion<POVNode> out;
        if(disabled()) return out;
        if(request.kind==preserveTemporal && access_->scalarCandidate(request.node) &&
           (access_->known(request.node)&valueOnly)) {done_=true;return out;}
        if(request.kind==temporalPredecessor) {
            if(POVDescription(request.node).temporalPredecessor()) return out;
            auto q=request;q.kind=selectRelative;q.related=request.node;q.argument=-1;q.coordinates={1};
            out.edges.push_back({"prior temporal position",{1,0},false,[q]{return q;}});return out;
        }
        auto source=POVDescription(request.node).temporalPredecessor();
        if(!source) return out;
        if(correspondenceOnly()) return out;
        if(access_->literalList(request.node) && access_->literalList(source))
            out.refinements.push_back({{request.node,preserveTemporal,linkExistingParts,{},request.context},
                {1,0},"connect existing temporal parts",true});
        // Exhaust ordinary scalar routes, filling intermediates as usual. An
        // unknown with no further evidence is a proved absence of supplied
        // value; pending expressions/inputs remain required dependencies.
        bool scalar=access_->scalarCandidate(request.node);
        out.refinements.push_back({{request.node,scalar ? valueOnly : relationshipConstraints,
            scalar ? settleExistingRoutes : lazyParts,{},request.context},{1,0},"prove preservation guard",false});
        // A pending constraint on a containing description can still supply a
        // child's %? or change RHS. Observe those constraints before inheriting.
        for(const auto& owner:request.context) if(owner!=request.node && !BoundaryGraph::temporal(owner))
            out.refinements.push_back({{owner,relationshipConstraints,0},{1,0},"prove containing transition constraints",false});
        if(!access_->literalList(source))
            out.refinements.push_back({{source,valueOnly,0},{1,0},"resolve prior value",false});
        else out.refinements.push_back({{source,relationshipConstraints,0},{1,0},"prove prior description",false});
        if(RecordStructure::record(*access_,source)) {
            Work work{0,0};std::vector<POVNode> parts;
            if(members(source,parts,work)) for(const auto& part:parts) if(unresolvedWord(part))
                out.refinements.push_back({{part,valueOnly,0},{1,0},"resolve prior part meaning",true});
        }
        return out;
    }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        finding_=visit;
        if(request.kind!=temporalPredecessor || visit.query.kind!=itemFound || disabled()) return {};
        auto target=request.node,source=visit.query.node;
        if(target==source || POVDescription(target).temporalPredecessor()) return {};
        return {{"Set prior temporal correspondence",[target,source] {
            bool changed=link(target,source);
            return SetResult{changed,ReadScope::changedObjects({target.get(),readView(target).get()})};
        },{1,0}}};
    }
    CommandBatch<POVNode> finishPass() override {
        CommandBatch<POVNode> out{finding_,{}};
        if(request.kind!=preserveTemporal || disabled()) return out;
        auto target=request.node,source=POVDescription(target).temporalPredecessor();
        if(!source || access_->inputPending(target) || access_->inputPending(source)) return out;
        auto access=access_;
        bool linksOnly=correspondenceOnly();
        if(linksOnly && (!correspondenceReady(target) || !correspondenceReady(source))) return out;
        if(access_->scalarCandidate(source) && access_->scalarCandidate(target)) {
            // A known later value is an explicit change, not a constraint that
            // it equal the prior value. Never import a prior declaration.
            if(access_->known(target)&valueOnly) {done_=true;return out;}
            if(!(access_->known(source)&valueOnly)) return out;
            if(access_->compatible(target,source,sizeAndValue)!=1) return out;
            out.commands.push_back({"Set preserved scalar value",[access,target,source]() -> SetResult {
                if(POVDescription(target).independentTemporalValue() || (access->known(target)&valueOnly)) return false;
                bool changed=access->apply(target,source,sizeAndValue);
                return SetResult{changed,ReadScope::changedObjects({target.get(),readView(target).get(),
                    readView(target)->infSize.get(),readView(target)->value.get()})};
            },{1,0}});return out;
        }
        if(!access_->literalList(source) || BoundaryGraph::temporal(source) || BoundaryGraph::temporal(target)) {
            done_=access_->known(target)&valueOnly;return out;
        }
        bool adopt=access_->adoptableList(target);
        if(!adopt && !access_->physicalList(target)) return out;
        Work work{0,0};std::vector<POVNode> left,right,missing;bool pendingPart=false;
        std::vector<std::pair<POVNode,POVNode>> pairs;
        if(!members(source,right,work) || (!adopt && !members(target,left,work))) return out;
        auto order=readView(source)->orderMode;
        if(!adopt && readView(target)->orderMode!=order) return out;
        if(order==uUnordered) {
            MemberCorrespondence mapping(left.size(),right.size(),[&](auto a,auto b) {
                return POVDescription(left[a]).temporalPredecessor()==right[b];
            },[&](auto a,auto b) {
                if(!access_->hasTypeRequirements(right[b])) return ScalarMatch::unknown;
                return compareNominal(*definitions_,right[b],left[a],work);
            },work);
            if(!mapping.unambiguous) return out;
            for(const auto& pair:mapping.pairs) pairs.push_back({left[pair.first],right[pair.second]});
            for(auto index:mapping.missing) {
                if(unresolvedWord(right[index])) pendingPart=true;
                else missing.push_back(right[index]);
            }
            auto extent=access_->listExtent(target);
            if(extent>=0 && static_cast<std::uint64_t>(extent)<left.size()+missing.size()) return out;
        } else {
            if(!adopt && left.size()!=right.size()) return out;
            if(adopt) missing=right;
            else for(std::size_t i=0;i<left.size();++i) pairs.push_back({left[i],right[i]});
        }
        if(linksOnly) {missing.clear();pendingPart=false;}
        pairs.erase(std::remove_if(pairs.begin(),pairs.end(),[](const auto& pair) {
            return POVDescription(pair.first).independentTemporalValue() ||
                   POVDescription(pair.first).temporalPredecessor()==pair.second;
        }),pairs.end());
        if(!adopt && pairs.empty() && missing.empty()) {done_=!pendingPart;return out;}
        work.creations=addCost(work.creations,missing.size()*3+adopt);
        out.commands.push_back({linksOnly ? "Set existing temporal correspondence" : "Set corresponding temporal parts",[this,access,target,source,adopt,order,pairs,missing,linksOnly]() -> SetResult {
            if(POVDescription(target).independentTemporalValue()) return false;
            if(linksOnly && (!correspondenceReady(target) || !correspondenceReady(source))) return false;
            bool changed=false;
            auto changes=ReadScope::changedObjects({target.get(),readView(target).get(),readView(target)->value.get()});
            auto oldTail=POVDescription(target).member(true);
            if(oldTail && !missing.empty()) changes.include(ReadScope::changedObjects({POVDescription::positionKey(oldTail)}));
            if(adopt) {
                changed=access->adoptDefinitionList(target);
                readView(target)->orderMode=order;
                SetFlagBit(readView(target)->value->flags,infonState::tailUnfinished,false);
            }
            for(const auto& pair:pairs) if(!POVDescription(pair.first).independentTemporalValue() && link(pair.first,pair.second)) {
                changed=true;changes.include(ReadScope::changedObjects({pair.first.get(),readView(pair.first).get()}));
            }
            for(const auto& prior:missing) {
                auto item=std::make_shared<infonView>();
                SetModeBits(item->infSize->flags,infonState::evalMode,infonState::emUnknown);
                item->type=readView(prior)->type;item->addedTypes=readView(prior)->addedTypes;
                item->temporalPredecessor=prior;
                auto member=readView(target)->value->items.append(item,false,609);
                member->pParent=target.get();access->bind(member);changed=true;
            }
            if(changed) {++target->workRevision;SetFlagBit(readView(target)->flags,infonView::isNormed,false);}
            return SetResult{changed,changes};
        },work});return out;
    }
    bool satisfied() const override {
        return disabled() || (request.kind==temporalPredecessor ? bool(POVDescription(request.node).temporalPredecessor()) : done_);
    }
    bool symbolic() const override {return !satisfied();}
};
