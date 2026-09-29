#pragma once
#include "UnitProjection.h"

// Typed field and positional queries can address the same ordered content in
// different definition-derived units. Containers are views; only corresponding
// members receive identity edges. Source cardinality and nesting stay intact.
class UnitSelection {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;

public:
    struct Result {bool handled=false;Expansion<POVNode> expansion;};
    UnitSelection(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions)
        :access_(std::move(access)),definitions_(std::move(definitions)) {}
    Result expand(const Query<POVNode>& q) const {
        Result result;auto& out=result.expansion;Work work{0,0};
        bool positional=q.kind==selectPosition || q.kind==positionStart;
        auto state=positional ? q.related->pItem->pathSelection : std::shared_ptr<GraphPathSelection>{};
        if(!access_->navigationList(q.node) || definitions_->pendingOrder(q.node)) return result;
        DefinitionSpec shape;bool span=false;std::int64_t count=1;
        if(positional && state->spec) {
            auto spec=state->spec;
            if(access_->literalList(spec)) {
                auto body=definitions_->repeatedBody(spec);if(!body) return result;
                auto proof=definitions_->describe(body,work);if(!proof) return result;
                shape=proof->requirement;span=true;count=access_->listExtent(spec);if(count<0) return result;
            } else shape=definitions_->unit(spec->pItem->type,work);
        } else if(positional) {
            auto parent=definitions_->unit(q.node->pItem->type,work);
            if(parent && parent->list) shape=parent->repeated;
        } else {
            if(getFlagBit(q.related->pItem->flags,infonView::invertMatch)) return result;
            shape=definitions_->unit(q.related->pItem->type,work);
        }
        UnitMeasure unit(shape);
        // A scalar nominal field remains ordinary field selection. Conversion
        // requires an actual composite unit, not merely a shared scalar shape.
        if(!unit || !shape->list) return result;
        UnitLayout source(access_,definitions_,q.node,unit);
        work.visits=addCost(work.visits,source.work.visits);
        if(!source.valid || source.layout.extent()%unit.width) return result;
        auto extent=source.layout.extent()/unit.width;
        std::int64_t index=0;
        if(positional) {
            PayloadReadiness ready(access_,state->predicate,definitions_);
            work.visits=addCost(work.visits,ready.work.visits);
            if(!ready.ready) return result;
            FlexNum number;Cost visits=0;
            if(!signedCoordinate(*access_,state->predicate->pItem->value,number,visits) || number.getDenominator()!=1 || number>INT64_MAX || number<-INT64_MAX) return result;
            work.visits=addCost(work.visits,visits);
            auto coordinate=number.getNumerator().get_si();
            if(!state->spec && coordinate==0) return result;
            index=coordinate<0 ? extent+coordinate : coordinate-(state->spec ? 0 : 1);
        }
        result.handled=true;
        if(index<0 || index>extent || count>extent-index) {
            auto end=q;end.kind=pathEnd;
            out.edges.push_back({"unit position outside source",work,true,[end]{return end;}});return result;
        }
        auto start=index*unit.width;
        auto key=ProteusDefinitions::UnitKey{q.node,shape->label,start,count,span};
        auto window=selectSpanWindow(source.layout,start,count*unit.width);
        std::vector<std::tuple<POVNode,bool,std::int64_t,std::int64_t>> pieces;
        for(const auto& part:window.pieces) pieces.emplace_back(part.node.node,part.gap,part.node.offset+part.offset,part.count);
        auto found=definitions_->unitViews.find(key);
        auto existing=found!=definitions_->unitViews.end() && sameDefinition(found->second.shape,shape) && found->second.pieces==pieces ? found->second.result : POVNode{};
        if(existing && (!positional || state->spec)) {
            FieldPredicate predicate(access_,definitions_,out);
            auto match=predicate.match(positional ? state->spec : q.related,existing,{});
            work.visits=addCost(work.visits,predicate.work.visits);
            if(match==ScalarMatch::unknown) {out.deferred=true;return result;}
            if(match==ScalarMatch::no) {
                auto end=q;end.kind=pathEnd;
                out.edges.push_back({"unit window does not satisfy query",work,true,[end]{return end;}});return result;
            }
        }
        auto plannedShape=span ? UnitProjection::collection(shape,count) : shape;
        auto plan=std::make_shared<UnitProjection>(access_,definitions_,q.node,plannedShape,start);
        if(!plan->available) {out.deferred=true;return result;}
        work.visits=addCost(work.visits,plan->work.visits);
        work.creations=existing ? 0 : plan->work.creations;
        auto definitions=definitions_;
        out.edges.push_back({"view source in requested units",work,true,[definitions,shape,plan,key,pieces,existing,q]() mutable {
            auto node=existing;bool changed=false;
            if(!node) {
                node=plan->apply();definitions->unitViews[key]={shape,pieces,node};changed=true;
            }
            auto next=q;next.node=node;next.kind=pathMatched;next.argument=static_cast<int>(ListPhase::member);next.context={q.node};
            return Arrival<POVNode>{next,changed};
        }});
        return result;
    }
};
