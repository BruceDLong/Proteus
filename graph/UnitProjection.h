#pragma once
#include "UnitLayout.h"

// A finite plan over published positions and measured gaps. Whole unknown runs
// become coordinate edges; only a demanded unit or stored value gets a POV.
class UnitProjection {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    POVNode source_;
    struct Part {
        DefinitionSpec shape;
        std::int64_t start=0,count=1;
        bool gap=false;
        POVNode leaf,owner;
        std::int64_t index=-1;
        std::vector<Part> children;
    };
    Part plan_;
    bool plan(Part& part,const UnitLayout& source) {
        UnitMeasure unit(part.shape);if(!unit) return false;
        if(!part.shape->list) {
            auto window=selectSpanWindow(source.layout,part.start,1);
            if(!window.complete || window.pieces.size()!=1) return false;
            auto piece=window.pieces.front();part.leaf=piece.node.node;
            if(piece.gap) {
                auto native=source.units.find(part.leaf);
                if(native==source.units.end() || UnitMeasure(native->second).width!=1) return false;
                part.index=piece.node.offset+piece.offset;
                if(auto owner=source.owners.find(part.leaf);owner!=source.owners.end()) part.owner=owner->second;
            }
            work.creations=addCost(work.creations,3);return true;
        }
        auto count=part.shape->memberLayout().extent();UnitMeasure member(part.shape->repeated);
        if(count<0 || !member) return false;
        work.creations=addCost(work.creations,3);
        for(std::int64_t i=0;i<count;) {
            auto start=part.start+i*member.width;
            auto remaining=selectSpanWindow(source.layout,start,(count-i)*member.width);
            if(!remaining.complete || remaining.pieces.empty()) return false;
            const auto& first=remaining.pieces.front();
            auto run=first.gap ? first.count/member.width : 0;
            if(run) {
                Part child;child.shape=part.shape->repeated;child.start=start;child.count=run;child.gap=true;
                part.children.push_back(std::move(child));work.creations=addCost(work.creations,3);i+=run;
            } else {
                Part child;child.shape=part.shape->repeated;child.start=start;
                if(!plan(child,source)) return false;
                part.children.push_back(std::move(child));++i;
            }
        }
        return true;
    }
    POVNode materialize(const Part& part) const {
        if(part.leaf) {
            if(part.index<0) return part.leaf;
            definitions_->retainOccurrenceOwner(part.leaf);
            if(part.owner) {part.leaf->pParent=part.owner.get();definitions_->retainOccurrenceOwner(part.owner);}
            return access_->sparseItem(part.leaf,part.index);
        }
        auto node=std::make_shared<POV>(922);node->pItem=std::make_shared<infonView>();
        auto item=node->pItem;auto shape=part.shape;
        SetModeBits(item->flags,infonView::viewMode,infonView::vmOverlay);
        SetModeBits(item->flags,infonView::itmMode,infonView::mItem);item->orderMode=uOrdered;
        SetModeBits(item->value->flags,infonState::overlayType,infonState::LST);
        SetModeBits(item->value->flags,infonState::evalMode,infonState::emLiteral);
        SetModeBits(item->infSize->flags,infonState::overlayType,infonState::NUM);
        SetModeBits(item->infSize->flags,infonState::evalMode,infonState::emLiteral);
        if(part.gap) {
            SetFlagBit(node->flags,POV::isSubItm,true);
            SetFlagBit(item->value->flags,infonState::tailUnfinished,true);
            item->value->sizeMode=fromGiven;item->infSize->num=part.count;
            auto spec=item->value->listSpec=std::make_shared<infonView>();
            SetModeBits(spec->flags,infonView::viewMode,infonView::vmAny);
            SetModeBits(spec->flags,infonView::itmMode,infonView::mItem);
            SetModeBits(spec->value->flags,infonState::overlayType,infonState::tUnknown);
            SetModeBits(spec->value->flags,infonState::evalMode,infonState::emUnknown);
            SetModeBits(spec->infSize->flags,infonState::evalMode,infonState::emUnknown);
            spec->type=shape->writtenType ? std::make_shared<WordUse>(*shape->writtenType) : nullptr;
            definitions_->unitSpans[node]={source_,shape,part.start,part.count};
        } else {
            item->type=shape->writtenType ? std::make_shared<WordUse>(*shape->writtenType) : nullptr;
            SetFlagBit(item->value->flags,infonState::timelike,shape->facts.timelike);
            item->value->sizeMode=fromCount;item->infSize->num=shape->memberLayout().extent();
            item->value->listSpec=shape->templateSource ? shape->templateSource->pItem : nullptr;
            for(const auto& child:part.children) {
                auto member=materialize(child);
                if(child.gap || child.shape->list) {item->value->items.appendPov(member);member->pParent=node.get();}
                else access_->appendTailMember(node,member);
            }
        }
        access_->bind(node);return node;
    }
public:
    Work work{0,0};bool available=false;
    std::vector<std::tuple<POVNode,bool,std::int64_t,std::int64_t>> pieces;
    UnitProjection(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions,
                   const POVNode& source,const DefinitionSpec& shape,std::int64_t start)
        :access_(std::move(access)),definitions_(std::move(definitions)),source_(source) {
        if(shape && shape->list && shape->memberLayout().extent()==0 && UnitMeasure(shape->repeated)) {
            plan_.shape=shape;plan_.start=start;work.creations=3;available=true;return;
        }
        UnitMeasure unit(shape);if(!unit) return;
        UnitLayout layout(access_,definitions_,source,unit);work=layout.work;
        if(!layout.valid) return;
        auto window=selectSpanWindow(layout.layout,start,unit.width);if(!window.complete) return;
        for(const auto& piece:window.pieces) {
            if(piece.gap && UnitMeasure(layout.units.at(piece.node.node)).width!=1) return;
            pieces.emplace_back(piece.node.node,piece.gap,piece.node.offset+piece.offset,piece.count);
        }
        plan_.shape=shape;plan_.start=start;available=plan(plan_,layout);
    }
    POVNode apply() const {return materialize(plan_);}
    static DefinitionSpec collection(const DefinitionSpec& member,std::int64_t count) {
        auto shape=std::make_shared<DefinitionRequirement>();shape->list=true;shape->open=true;
        shape->facts.valid=true;shape->facts.overlay=infonState::LST;shape->facts.hasSize=true;shape->facts.size=count;
        shape->repeated=member;return shape;
    }
    static std::shared_ptr<Expansion<POVNode>> indexed(const std::shared_ptr<ProteusGraphAccess>& access,
            const std::shared_ptr<ProteusDefinitions>& definitions,const Query<POVNode>& query) {
        auto found=definitions->unitSpan(query.node);
        if(!found) return {};
        auto result=std::make_shared<Expansion<POVNode>>();auto& out=*result;const auto& span=*found;
        if(query.argument<0 || query.argument>=span.count) return result;
        auto start=span.start+query.argument*UnitMeasure(span.member).width;
        auto plan=std::make_shared<UnitProjection>(access,definitions,span.source,span.member,start);
        if(!plan->available) {out.deferred=true;return result;}
        auto key=ProteusDefinitions::UnitKey{span.source,span.member->label,start,1,false};
        auto cached=definitions->unitViews.find(key);
        auto existing=cached!=definitions->unitViews.end() && sameDefinition(cached->second.shape,span.member) && cached->second.pieces==plan->pieces ? cached->second.result : POVNode{};
        if(existing) plan->work.creations=0;
        auto shape=span.member;
        out.edges.push_back({"indexed unit span",plan->work,true,[definitions,plan,key,shape,existing,query] {
            auto result=existing ? existing : plan->apply();
            if(!existing) definitions->unitViews[key]={shape,plan->pieces,result};
            return Arrival<POVNode>{{result,itemFound,0,{},query.context},!existing};
        }});return result;
    }
};
