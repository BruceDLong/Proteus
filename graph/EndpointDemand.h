#pragma once

// Argument syntax and a symbolic last position both ask for an ordinary list
// coordinate. Wrap the existing navigation; sparse and embedded traversal stay
// in the shared graph adapter. Registry records own symbolic endpoint identity.
class EndpointDemand {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::function<Expansion<POVNode>(const Query<POVNode>&)> navigate_;
    static Query<POVNode> wrap(Query<POVNode> q,const POVNode& owner) {
        return {q.node,q.kind==itemFound ? endpointFound : endpointPosition,q.argument,owner,q.context,{q.kind}};
    }
public:
    EndpointDemand(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions,
                   std::function<Expansion<POVNode>(const Query<POVNode>&)> navigate)
        :access_(std::move(access)),definitions_(std::move(definitions)),navigate_(std::move(navigate)) {}
    Expansion<POVNode> expand(const Query<POVNode>& q) const {
        Expansion<POVNode> out;
        if(q.kind==endpointFound) return out;
        if(q.kind==endpointPosition) {
            out=navigate_({q.node,static_cast<int>(q.coordinates.front()),q.argument,{},q.context});
            for(auto& edge:out.edges) {
                auto follow=edge.follow;auto owner=q.related;
                edge.follow=[follow,owner] {auto at=follow();return Arrival<POVNode>{wrap(std::move(at.query),owner),at.graphChanged};};
            }
            return out;
        }
        auto source=q.node;std::int64_t index=0;bool last=false;
        if(q.kind==placeArgument) {
            if(!access_->hasArgument(source)) return out;
            last=getModeBits(source->pItem->flags,infonView::argPos)==infonView::apArgIsLastItem;
        } else {
            auto reference=POVDescription(q.node,access_->registry.get()).endReference();
            if(!reference) return out;
            if(reference->resolvedPOV) {
                out.refinements.push_back({{reference->resolvedPOV,sizeAndValue,0},{1,0},"normalize bound endpoint",false});
                return out;
            }
            source=definitions_->retainedOwner(reference->sourcePOV);
            if(!source) {out.deferred=true;return out;}
            last=true;index=reference->offset+1;
        }
        if(!access_->navigationList(source)) {
            out.refinements.push_back({{source,valueOnly,0},{1,0},"resolve endpoint source",true});out.deferred=true;return out;
        }
        Cost visits=1;
        if(last) {
            auto boundary=listBoundary(*access_,source);visits+=boundary.visits;
            for(const auto& missing:boundary.missingExtents)
                out.refinements.push_back({{missing,sizeOnly,0},{boundary.visits,0},"find endpoint extent",true});
            if(!boundary.valid || boundary.extent<0) {
                out.refinements.push_back({{source,sizeOnly,0},{boundary.visits,0},"find last position",true});out.deferred=true;return out;
            }
            index+=boundary.extent-1;
            if(index<0) {out.deferred=true;return out;}
        }
        auto next=wrap({source,itemAt,index},q.node);
        out.edges.push_back({"navigate endpoint",{visits,0},true,[next]{return next;}});return out;
    }
};

class ProteusEndpointGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    bool verified_=false;
public:
    ProteusEndpointGoal(Query<POVNode> query,std::shared_ptr<ProteusGraphAccess> access)
        :GoalSpec<POVNode>(query),access_(std::move(access)) {}
    void beginPass() override {verified_=false;}
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        if(visit.query.kind!=endpointFound) return {};
        auto access=access_;auto owner=request.node,selected=visit.query.node;
        if(request.kind==placeArgument) {
            if(!access->hasArgument(owner)) return {};
            auto argument=owner->pItem->funcArgs;
            return {{"Set argument constraint",[this,access,owner,selected,argument] {
                auto rhs=std::make_shared<POV>(918);rhs->pItem=argument;access->bind(rhs);access->bind(selected);
                if(!selected->pItem->hasRHS(argument)) selected->pItem->addPOVToWrkList(rhs);
                owner->pItem->funcArgs.reset();SetModeBits(owner->pItem->flags,infonView::argPos,infonView::apNoArgs);
                ++selected->workRevision;++owner->workRevision;verified_=true;return true;
            },{1,1}}};
        }
        auto reference=POVDescription(owner,access->registry.get()).endReference();
        if(!reference || reference->resolvedPOV) return {};
        return {{"Bind symbolic endpoint",[this,access,owner,selected,reference] {
            access->bind(selected);reference->resolvedPOV=selected;
            reference->identityRelation=access->registry->recordIdentity(owner,selected,iekEndRelativeResolution,0);
            SetFlagBit(owner->flags,POV::endRelativeResolved,true);verified_=true;return true;
        },{1,1}}};
    }
    bool satisfied() const override {
        if(request.kind==placeArgument) return verified_ || !access_->hasArgument(request.node);
        auto reference=POVDescription(request.node,access_->registry.get()).endReference();
        return verified_ || (reference && reference->resolvedPOV);
    }
    bool symbolic() const override {return !satisfied();}
};
