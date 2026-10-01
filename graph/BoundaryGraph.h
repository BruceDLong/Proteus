#pragma once

// Boundary nodes are ordinary null POVs. The registry owns their incidence
// with a source occurrence; inspection never allocates or materializes members.
class BoundaryGraph {
    std::shared_ptr<ProteusGraphAccess> access_;
public:
    explicit BoundaryGraph(std::shared_ptr<ProteusGraphAccess> access):access_(std::move(access)) {}
    static bool temporal(const POVNode& source) {
        return source && readView(source) &&
            getModeBits(readView(source)->value->flags,infonState::overlayType)==infonState::LST &&
            getModeBits(readView(source)->value->flags,infonState::evalMode)==infonState::emLiteral &&
            getFlagBit(readView(source)->value->flags,infonState::timelike) && readView(source)->orderMode!=uUnordered;
    }
    static int direction(const POVNode& predicate) {
        if(!predicate || !readView(predicate)) return 0;
        const auto& item=readView(predicate);
        if(!item->type || getModeBits(item->flags,infonView::viewMode)!=infonView::vmAny ||
           !item->addedTypes.empty() || !item->wrkList.empty() || item->partRef || !item->parts.empty() ||
           getFlagBit(item->flags,infonView::invertMatch)) return 0;
        auto word=item->type->typeID();return word=="begin" ? -1 : word=="end" ? 1 : 0;
    }
    POVNode lookup(const POVNode& source,bool end) const {
        if(readView(source)->isNull()) return source;
        auto reference=POVDescription(source,access_->registry.get()).boundaryReference();
        return reference ? (end ? reference->end : reference->begin) : POVNode{};
    }
    // Called only from priced Sets, under the graph publication lock.
    POVNode ensure(const POVNode& source,bool end) const {
        if(auto existing=lookup(source,end)) return existing;
        access_->bind(source);
        auto boundary=std::make_shared<POV>(609); // povSrcTemporalBoundary
        readView(boundary)=std::make_shared<infonView>();readView(boundary)->setToNull();
        readView(boundary)->identityName=access_->registry->occurrenceScope.allocate();
        access_->bind(boundary);
        std::lock_guard<std::mutex> lock(access_->registry->registryMutex);
        auto& reference=access_->registry->boundaries[source];
        if(!reference) {reference=std::make_shared<BoundaryReference>();reference->source=source;}
        auto& slot=end ? reference->end : reference->begin;
        if(!slot) slot=boundary;
        return slot;
    }
    bool connect(const POVNode& left,bool leftEnd,const POVNode& right,bool rightEnd) const {
        bool created=!lookup(left,leftEnd) || !lookup(right,rightEnd);
        auto a=ensure(left,leftEnd),b=ensure(right,rightEnd);
        if(access_->registry->areIdentical(a,b)) return created;
        access_->registry->recordIdentity(a,b,iekTemporalBoundary,0);return true;
    }
    Expansion<POVNode> expand(const Query<POVNode>& q) const {
        Expansion<POVNode> out;
        auto edge=[&](const char* label,Query<POVNode> target) {
            out.edges.push_back({label,{1,0},false,[target] {return target;}});
        };
        if(q.kind==boundaryAccess) {
            out.deferred=access_->inputPending(q.node);
            for(const auto& relation:POVDescription(q.node,access_->registry.get()).relationships(false))
                edge("boundary of identical occurrence",{relation.target,boundaryAccess,q.argument});
            if(temporal(q.node)) out.refinements.push_back({{q.node,boundaryRelations,0},{1,0},"inspect T-list boundary identities",false});
            return out;
        }
        if(q.kind==boundaryRelations) {
            if(!temporal(q.node)) return out;
            // A parser may still change its body. Boundary existence is already
            // known; composition waits for a stable body and subscribes normally.
            if(access_->inputPending(q.node)) {out.deferred=true;return out;}
            auto member=POVDescription(q.node).member();
            edge("first T-list span",member ? Query<POVNode>{member,boundaryMember,0,q.node} :
                 Query<POVNode>{q.node,boundaryEnd,0});
        } else if(q.kind==boundaryMember) {
            auto next=POVDescription(q.node).adjacent();
            edge("next T-list span",next ? Query<POVNode>{next,boundaryMember,0,q.related,{q.node}} :
                 Query<POVNode>{q.related,boundaryEnd,0,q.node});
            // Ordinary nested lists occupy one span; only a T-list contributes
            // its internal composition identities. Sparse tails stay virtual.
            if(temporal(q.node)) out.refinements.push_back({{q.node,boundaryRelations,0},{1,0},"compose nested T-list boundaries",false});
        } else if(q.kind==boundaryEnd) {
            auto extent=listBoundary(*access_,q.node);
            out.deferred=!extent.valid || extent.tail!=0 || access_->inputPending(q.node);
        }
        return out;
    }
};

class ProteusBoundaryGoal:public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    BoundaryGraph graph_;
    bool reachedEnd_=false;
public:
    ProteusBoundaryGoal(Query<POVNode> q,std::shared_ptr<ProteusGraphAccess> access)
        :GoalSpec<POVNode>(std::move(q)),access_(std::move(access)),graph_(access_) {}
    void beginPass() override {reachedEnd_=false;}
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        const auto& q=visit.query;
        if(request.kind==boundaryAccess) {
            auto graph=graph_;auto source=request.node,alias=q.node;bool end=request.argument>0;
            // Boundaries requested before the source became null remain valid
            // references. Identify them with the now-known self boundary.
            if(readView(alias)->isNull())
                if(auto prior=POVDescription(alias,access_->registry.get()).boundaryReference()) {
                    std::vector<SetCommand> commands;
                    for(const auto& boundary:{prior->begin,prior->end})
                        if(boundary && !access_->registry->areIdentical(boundary,alias)) {
                            auto access=access_;
                            commands.push_back({"Identify prior boundary with null occurrence",[access,boundary,alias] {
                                access->registry->recordIdentity(boundary,alias,iekTemporalBoundary,0);return true;
                            },{1,1}});
                        }
                    if(!commands.empty()) return commands;
                }
            if(auto existing=graph_.lookup(source,end)) {
                if(alias==source) return {};
                auto other=graph_.lookup(alias,end);
                if(other && access_->registry->areIdentical(existing,other)) return {};
                return {{"Identify boundaries of identical occurrences",[graph,source,alias,end] {
                    return graph.connect(source,end,alias,end);
                },{2,5}}};
            }
            return {{"Create boundary occurrence",[graph,source,end] {graph.ensure(source,end);return true;},{1,4}}};
        }
        if(q.kind==boundaryRelations) {
            if(!BoundaryGraph::temporal(q.node)) {reachedEnd_=true;return {};}
            if(graph_.lookup(q.node,false) && graph_.lookup(q.node,true)) return {};
            auto graph=graph_;auto source=q.node;
            return {{"Create T-list boundary occurrences",[graph,source] {graph.ensure(source,false);graph.ensure(source,true);return true;},{1,8}}};
        }
        POVNode left,right;bool leftEnd=false,rightEnd=false;
        if(q.kind==boundaryMember) {
            left=q.context.empty() ? q.related : q.context.front();leftEnd=!q.context.empty();right=q.node;
        } else if(q.kind==boundaryEnd) {
            reachedEnd_=true;
            auto extent=listBoundary(*access_,q.node);
            if(!extent.valid || extent.tail!=0 || access_->inputPending(q.node)) return {};
            left=q.related ? q.related : q.node;leftEnd=bool(q.related);right=q.node;rightEnd=true;
        } else return {};
        auto a=graph_.lookup(left,leftEnd),b=graph_.lookup(right,rightEnd);
        if(a && b && access_->registry->areIdentical(a,b)) return {};
        auto graph=graph_;
        return {{"Establish T-list boundary identity",[graph,left,leftEnd,right,rightEnd] {
            return graph.connect(left,leftEnd,right,rightEnd);
        },{2,9}}};
    }
    bool satisfied() const override {
        return request.kind==boundaryAccess ? bool(graph_.lookup(request.node,request.argument>0)) : reachedEnd_;
    }
    bool symbolic() const override {return !satisfied();}
    POVNode result() const override {return request.kind==boundaryAccess ? graph_.lookup(request.node,request.argument>0) : request.node;}
};
