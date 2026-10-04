#pragma once

// Temporal views retain source occurrences; their physical list bodies may be
// empty while the support is unresolved. No state count is inferred from time.
class TemporalGraph {
    std::shared_ptr<ProteusGraphAccess> access_;
public:
    explicit TemporalGraph(std::shared_ptr<ProteusGraphAccess> access):access_(std::move(access)) {}
    static int pathKind(const POVNode& predicate) {
        if(!predicate || !readView(predicate)) return 0;
        const auto& p=readView(predicate);
        if(!p->type || getModeBits(p->flags,infonView::viewMode)!=infonView::vmAny ||
           !p->addedTypes.empty() || !p->wrkList.empty() || p->partRef || !p->parts.empty() ||
           getFlagBit(p->flags,infonView::invertMatch)) return 0;
        auto word=p->type->typeID();return word=="astlist" ? temporalSupport : word=="cut" ? temporalCut : 0;
    }
    POVNode support(const POVNode& source) const {
        if(auto view=access_->registry->findTemporalProjectionForView(source.get())) return view->projectionPOV;
        if(auto ref=access_->registry->findTemporalProjectionForSource(source.get())) return ref->projectionPOV;
        return BoundaryGraph::temporal(source) ? source : POVNode{};
    }
    std::shared_ptr<TemporalSplitReference> split(const POVNode& request) const {
        if(!request) return {};
        std::lock_guard<std::mutex> lock(access_->registry->registryMutex);
        for(const auto& ref:access_->registry->temporalSplitReferences) if(ref->requestPOV==request) return ref;
        return {};
    }
    POVNode newSpan() const {
        auto p=std::make_shared<POV>(611);readView(p)=std::make_shared<infonView>();readView(p)->setToList();
        readView(p)->orderMode=uOrdered;
        SetFlagBit(readView(p)->value->flags,infonState::timelike,true);
        SetFlagBit(readView(p)->value->flags,infonState::tailUnfinished,true);
        readView(p)->value->sizeMode=fromGiven;
        SetModeBits(readView(p)->infSize->flags,infonState::overlayType,infonState::NUM);
        SetModeBits(readView(p)->infSize->flags,infonState::evalMode,infonState::emUnknown);
        access_->bind(p);return p;
    }
    // Manifestations run only in priced Sets under the publication lock.
    POVNode ensureSupport(const POVNode& source) const {
        if(auto p=support(source)) return p;
        access_->bind(source);auto p=newSpan();
        access_->registry->recordTemporalProjection(source,p);
        BoundaryGraph boundaries(access_);
        boundaries.connect(source,false,p,false);boundaries.connect(source,true,p,true);
        return p;
    }
    std::shared_ptr<TemporalSplitReference> ensureSplit(const POVNode& source,const POVNode& request) const {
        if(auto ref=split(request)) return ref;
        auto span=ensureSupport(source),prefix=newSpan(),suffix=newSpan();
        BoundaryGraph boundaries(access_);
        auto ref=access_->registry->recordTemporalSplit(span,prefix,suffix,
            boundaries.ensure(span,false),boundaries.ensure(prefix,false),boundaries.ensure(prefix,true),
            boundaries.ensure(suffix,false),boundaries.ensure(suffix,true),boundaries.ensure(span,true),0);
        ref->requestPOV=request;return ref;
    }
    Expansion<POVNode> expand(const Query<POVNode>& q) const {
        Expansion<POVNode> out;
        if(q.kind==temporalSupport) {
            for(const auto& relation:POVDescription(q.node,access_->registry.get()).relationships(false)) {
                auto next=q;next.node=relation.target;
                out.edges.push_back({"support of identical occurrence",{1,0},false,[next]{return next;}});
            }
            return out;
        }
        if(q.kind==temporalCut) {
            // Keep an unresolved source as an obligation: later information
            // can disprove the strict interior even though the cut is named.
            out.deferred=access_->inputPending(q.node) ||
                getModeBits(readView(q.node)->value->flags,infonState::evalMode)!=infonState::emLiteral;
            if(out.deferred) out.refinements.push_back({{q.node,valueOnly,0},{1,0},"refine cut support",true});
            out.refinements.push_back({{q.node,temporalSupport,0},{1,0},"retain temporal support",false});
            return out;
        }
        auto edge=[&](const char* label,Query<POVNode> next) {
            out.edges.push_back({label,{1,0},false,[next]{return next;}});
        };
        if(q.kind==temporalSide) {
            auto ref=q.node->temporalSplit.reference;
            if(!ref) {out.deferred=true;return out;}
            // A located strict-interior cut cannot become an outer endpoint.
            for(const auto& endpoint:{ref->sourceBegin,ref->sourceEnd})
                if(access_->registry->areIdentical(q.node,endpoint))
                    throw GoalRejected("strict interior cut coincides with support endpoint");
            edge("source span of temporal cut",{ref->sourceSpan,temporalSideSpan,q.argument,q.node,{}, {1}});
            return out;
        }
        if(q.kind==temporalSideMember) {
            edge("inspect temporal member at cut",{q.node,temporalSideSpan,q.argument,q.related});
            if(auto next=POVDescription(q.node).adjacent())
                edge("next temporal span",{next,temporalSideMember,q.argument,q.related});
            return out;
        }
        if(q.kind!=temporalSideSpan) return out;
        // Identical histories provide alternative descriptions of this same
        // occurrence. Value-only references must not supply its time boundaries.
        for(const auto& relation:POVDescription(q.node,access_->registry.get()).relationships(false)) {
            if(relation.valueOnly) continue;
            auto next=q;next.node=relation.target;
            edge("identical temporal span",next);
        }
        if(auto projection=access_->registry->findTemporalProjectionForView(q.node.get())) {
            auto next=q;next.node=projection->sourcePOV;
            edge("source of temporal support",next);
            return out;
        }
        if(access_->inputPending(q.node) ||
           getModeBits(readView(q.node)->value->flags,infonState::evalMode)!=infonState::emLiteral) {
            out.refinements.push_back({{q.node,valueOnly,0},{1,0},"resolve span at temporal cut",true});
            out.deferred=true;return out;
        }
        if(BoundaryGraph::temporal(q.node)) {
            // Compose only stored T-members. Sparse tails remain virtual;
            // neither a state count nor equal values locate a cut within them.
            out.refinements.push_back({{q.node,boundaryRelations,0},{1,0},"locate temporal member boundaries",true});
            if(auto member=POVDescription(q.node).member())
                edge("first temporal span",{member,temporalSideMember,q.argument,q.related});
            for(const auto& member:access_->demandedOccurrences(q.node))
                edge("already demanded temporal occurrence",{member,temporalSideSpan,q.argument,q.related});
            return out;
        }
        bool interior=!q.coordinates.empty() && q.coordinates.front()!=0;
        if(readView(q.node)->isNull()) {
            if(interior) throw GoalRejected("strict interior cut has empty support");
            return out; // A null joins spans; it supplies no state on either side.
        }
        auto boundary=BoundaryGraph(access_).lookup(q.node,q.argument<0);
        if(interior || (boundary && access_->registry->areIdentical(q.related,boundary)))
            edge(q.argument<0 ? "S-value before cut" : "S-value after cut",{q.node,temporalValueFound,0});
        return out;
    }
};

class ProteusTemporalGoal:public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    TemporalGraph graph_;
    POVNode result_;
    std::shared_ptr<TemporalSplitReference> split_;
public:
    ProteusTemporalGoal(Query<POVNode> q,std::shared_ptr<ProteusGraphAccess> access)
        :GoalSpec<POVNode>(q),access_(std::move(access)),graph_(access_) {}
    void beginPass() override {result_.reset();}
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        if(request.kind==temporalSide) {
            if(visit.query.kind==temporalValueFound) {
                if(result_ && !access_->registry->areIdentical(result_,visit.query.node))
                    throw GoalRejected("cut identifies different temporal occurrences on one side");
                result_=visit.query.node;
            }
            return {};
        }
        auto source=request.node;
        if(request.kind==temporalSupport) {
            if(visit.query.node!=source) {
                auto a=graph_.support(source),b=graph_.support(visit.query.node);
                if(a && b && !access_->registry->areIdentical(a,b)) {
                    auto access=access_;
                    return {{"Identify supports of identical occurrences",[access,a,b]{
                        access->registry->recordIdentity(a,b,iekTemporalProjection,0);return true;
                    },{1,1}}};
                }
                return {};
            }
            if(auto p=graph_.support(source)) {
                result_=p;
                // A later refinement can reveal that this is already a T-span.
                if(p!=source && BoundaryGraph::temporal(source) && !access_->registry->areIdentical(source,p)) {
                    auto access=access_;
                    return {{"Identify refined temporal support",[access,source,p] {
                        access->registry->recordIdentity(source,p,iekTemporalProjection,0);return true;
                    },{1,1}}};
                }
                return {};
            }
            auto graph=graph_;
            return {{"Create symbolic temporal support",[this,graph,source]{result_=graph.ensureSupport(source);return true;},{1,6}}};
        }
        if(readView(source)->isNull() || (BoundaryGraph::temporal(source) && access_->listExtent(source)==0))
            throw GoalRejected("strict interior cut requires a nonempty support");
        if(!split_) split_=graph_.split(request.related);
        if(split_) {result_=split_->symbolicCut;return {};}
        auto graph=graph_;auto owner=request.related;
        return {{"Create symbolic temporal cut",[this,graph,source,owner] {
            split_=graph.ensureSplit(source,owner);result_=split_->symbolicCut;return true;
        },{1,18}}};
    }
    bool satisfied() const override {return bool(result_);}
    bool symbolic() const override {return !result_;}
    POVNode result() const override {return result_;}
};

// An explicit %? declaration follows same-occurrence constraints independently
// of value acquisition. Copying a prior temporal value does not copy its policy.
class ProteusTemporalDeclarationGoal : public GoalSpec<POVNode> {
public:
    explicit ProteusTemporalDeclarationGoal(Query<POVNode> q):GoalSpec<POVNode>(std::move(q)) {}
    std::vector<SetCommand> found(const Visit<POVNode>&) override {
        if(satisfied() || !request.related || !POVDescription(request.related).independentTemporalValue()) return {};
        auto node=request.node,source=request.related;
        return {{"Set temporal value independence",[node,source]() -> SetResult {
            if(POVDescription(node).independentTemporalValue() || !POVDescription(source).independentTemporalValue()) return false;
            SetFlagBit(readView(node)->flags,infonView::independentTemporalValue,true);
            SetFlagBit(readView(node)->flags,infonView::isNormed,false);
            ++node->workRevision;
            return {true,ReadScope::changedObjects({node.get(),readView(node).get()})};
        }}};
    }
    bool satisfied() const override {return POVDescription(request.node).independentTemporalValue();}
    bool symbolic() const override {return !satisfied();}
};
