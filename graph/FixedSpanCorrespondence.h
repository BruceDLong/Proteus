#pragma once
#include "UnitProjection.h"
#include "SparseOccurrence.h"

// A compressed route through the same fixed sequence: published members and
// measured virtual runs. Only singleton demands allocate occurrences.
class FixedSpanCorrespondence {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    using Layout=SpanLayout<POVNode>;
    struct Pair {POVNode target,source;std::int64_t index=-1;};
    std::vector<Pair> pairs_;
    struct Span {POVNode target;SpanWindow<POVNode> window;};
    std::vector<Span> spans_;
    std::map<POVNode,POVNode> owners_;
    struct UnitSpan {POVNode target,source;DefinitionSpec member;std::int64_t start,count;};
    std::vector<UnitSpan> unitSpans_;
    struct UnitPair {POVNode target,existing;DefinitionSpec shape;ProteusDefinitions::UnitKey key;std::shared_ptr<UnitProjection> plan;};
    std::vector<UnitPair> unitPairs_;
    std::vector<std::pair<POVNode,std::shared_ptr<UnitProjection>>> unitFills_;
    bool units(const POVNode& source,const Layout& left) {
        DefinitionSpec exemplar;
        for(const auto& part:left.segments) {
            auto shape=description(part.gap ? definitions_->repeatedBody(part.node) : part.node);
            if(UnitMeasure(shape) && shape->list) {exemplar=shape;break;}
        }
        if(!exemplar) return false;
        UnitMeasure measure(exemplar);auto payload=definitions_->payload(source);
        if(auto field=source->pItem->pathSelection;field && !field->pending && field->matches.size()==1) payload=field->result;
        UnitLayout right(access_,definitions_,payload,measure);
        work.visits=addCost(work.visits,right.work.visits);if(!right.valid) return false;
        std::int64_t start=0;
        for(const auto& part:left.segments) {
            if(!part.width) continue;
            auto shape=description(part.gap ? definitions_->repeatedBody(part.node) : part.node);
            if(!shape) return false;
            UnitMeasure unit(shape);
            if(!unit && !shape->list && compatible(shape,measure.leaf)) unit=UnitMeasure(measure.leaf);
            if(!unit || unit.base!=measure.base || part.width>INT64_MAX/unit.width) return false;
            auto width=part.width*unit.width;
            if(start>right.layout.extent() || width>right.layout.extent()-start) return false;
            auto window=selectSpanWindow(right.layout,start,width);
            if(!window.complete) return false;
            // Every virtual source position must supply the requested unit's
            // leaf constraints. No unexamined input is discarded by a skip.
            for(const auto& piece:window.pieces) {
                if(!piece.gap && !candidateReady(piece.node.node)) return false;
                auto actual=piece.gap ? right.units.at(piece.node.node) : description(piece.node.node);
                auto predicate=shape->list ? unit.leaf : shape;
                if(!actual || (piece.gap && UnitMeasure(actual).width!=1) || !compatible(predicate,actual)) return false;
            }
            if(part.gap) {
                if(!access_->demandedOccurrences(part.node).empty()) return false;
                if(std::any_of(window.pieces.begin(),window.pieces.end(),[](const auto& piece){return !piece.gap;})) {
                    auto projected=std::make_shared<UnitProjection>(access_,definitions_,payload,UnitProjection::collection(shape,part.width),start);
                    if(!projected->available) return false;
                    work.visits=addCost(work.visits,projected->work.visits);work.creations=addCost(work.creations,projected->work.creations);
                    unitFills_.push_back({part.node,projected});
                } else unitSpans_.push_back({part.node,payload,shape,start,part.width});
            } else {
                auto projected=std::make_shared<UnitProjection>(access_,definitions_,payload,unit.spec,start);
                if(!projected->available) return false;
                work.visits=addCost(work.visits,projected->work.visits);
                work.creations=addCost(work.creations,projected->work.creations);
                auto key=ProteusDefinitions::UnitKey{payload,unit.spec->label,start,1,false};
                auto cached=definitions_->unitViews.find(key);
                auto existing=cached!=definitions_->unitViews.end() && sameDefinition(cached->second.shape,unit.spec) && cached->second.pieces==projected->pieces ? cached->second.result : POVNode{};
                unitPairs_.push_back({part.node,existing,unit.spec,key,projected});
            }
            start+=width;
        }
        work.creations=addCost(work.creations,unitSpans_.size()*2+unitPairs_.size()*2);
        return source->looseSize() || start==right.layout.extent();
    }
    bool candidateReady(const POVNode& node) {
        PayloadReadiness ready(access_,node,definitions_);
        work.visits=addCost(work.visits,ready.work.visits);return ready.ready;
    }
    DefinitionSpec description(const POVNode& node) {
        if(!node) return {};
        auto proof=definitions_->describe(definitions_->payload(node),work);
        return proof ? proof->requirement : DefinitionSpec{};
    }
    bool compatible(const DefinitionSpec& a,const DefinitionSpec& b) {
        if(a && b && a->list && b->list && UnitMeasure(a) && sameDefinition(a,b)) return true;
        if(!a || !b || a->list || (b->list && !UnitMeasure(b)) || !a->choices.empty() || !b->choices.empty() ||
           a->numeric.valid || b->numeric.valid) return false;
        if(compareNominal(a->label,a->types,b->label,b->types)==ScalarMatch::no) return false;
        auto facts=a->facts;return facts.combine(b->facts);
    }
    bool layout(const POVNode& node,Layout& result) {
        if(!readListLayout(*access_,node,result,work.visits,[&](const auto& at) {
            if(access_->inputPending(at.node)) return false;
            if(!at.owners.empty()) owners_[at.node]=at.owners.back();
            return at.phase!=ListPhase::enter || access_->navigationList(at.node);
        },[](const POVNode& item){return item;})) return false;
        if(!result.solve() || result.extent()<0) return false;
        for(const auto& segment:result.segments) if(segment.gap && segment.width) {
            if(!access_->sparse(segment.node)) return false;
        }
        return true;
    }
public:
    Work work{0,0};
    bool supported=false;
    FixedSpanCorrespondence(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions,
                         const POVNode& target,const POVNode& source)
        :access_(std::move(access)),definitions_(std::move(definitions)) {
        if(!source->looseType() || getFlagBit(source->flags,POV::isOutrFeed)) return;
        auto payload=definitions_->payload(source);
        if(auto field=source->pItem->pathSelection;field && !field->pending && field->matches.size()==1) payload=field->result;
        Layout left,right;
        if(!layout(target,left)) return;
        if(!layout(payload,right)) return;

        auto prepare=[&] {
            auto count=left.extent();
            if(count>right.extent() || (!source->looseSize() && count!=right.extent())) return;
            std::int64_t offset=0;
            for(const auto& a:left.segments) {
                if(!a.width) continue;
                auto required=description(a.gap ? definitions_->repeatedBody(a.node) : a.node);
                if(!required) return;
                auto window=selectSpanWindow(right,offset,a.width);
                if(!window.complete) return;
                for(const auto& b:window.pieces) {
                    auto actualNode=b.gap ? definitions_->repeatedBody(b.node) : b.node;
                    auto existing=b.gap && !a.gap ? access_->existingSparseItem(b.node,b.offset) : POVNode{};
                    if(existing) actualNode=existing;
                    if((!b.gap || existing) && !candidateReady(actualNode)) return;
                    auto actual=description(actualNode);
                    bool atomicUnit=UnitMeasure(required) && required->list && !b.gap && b.node->pItem->type &&
                        compareNominal(required->label,required->types,b.node->pItem->type->typeID(),{})==ScalarMatch::yes;
                    if(atomicUnit) {
                        auto facts=required->facts;atomicUnit=facts.combine(access_->typeFacts(b.node));
                    }
                    if(!atomicUnit && !compatible(required,actual)) return;
                    if(!a.gap) pairs_.push_back({a.node,b.node,b.gap ? b.offset : -1});
                }
                if(a.gap) {
                    if(!access_->demandedOccurrences(a.node).empty()) {
                        if(window.pieces.size()!=1 || !window.pieces.front().gap) return;
                        const auto& piece=window.pieces.front();
                        if(!a.node->repetitionSpan.family || a.node->repetitionSpan.family!=piece.node->repetitionSpan.family ||
                           a.node->repetitionSpan.startOrdinal!=piece.node->repetitionSpan.startOrdinal+piece.offset) return;
                    }
                    spans_.push_back({a.node,std::move(window)});
                }
                offset+=a.width;
            }
            work.creations=pairs_.size()*5+spans_.size()*3;
            supported=true;
        };prepare();
        if(!supported) {
            pairs_.clear();spans_.clear();
            supported=units(source,left);
        }
    }
    bool apply() {
        bool changed=false;
        for(const auto& span:spans_) {
            if(span.window.pieces.size()==1 && span.window.pieces.front().gap) {
                auto piece=span.window.pieces.front();
                auto family=access_->registry->getOrCreateRepetitionFamily(piece.node.get());
                auto ordinal=piece.node->repetitionSpan.startOrdinal+piece.offset;
                if(span.target->repetitionSpan.family!=family || span.target->repetitionSpan.startOrdinal!=ordinal) {
                    span.target->repetitionSpan.family=family;span.target->repetitionSpan.startOrdinal=ordinal;
                    access_->bind(span.target);definitions_->retainOccurrenceOwner(span.target);
                    definitions_->retainOccurrenceOwner(piece.node);++span.target->workRevision;changed=true;
                }
            } else {
                auto view=std::make_shared<GraphSliceView>();for(const auto& piece:span.window.pieces) view->count+=piece.count;
                auto result=SliceSelection::manifest(access_,definitions_,view,span.window,changed);
                span.target->pItem->value->items=std::move(result->pItem->value->items);
                for(auto child=POVDescription(span.target).member();child;child=POVDescription(child).adjacent()) child->pParent=span.target.get();
                SetFlagBit(span.target->pItem->value->flags,infonState::tailUnfinished,false);
                ++span.target->workRevision;changed=true;
            }
        }
        for(const auto& fill:unitFills_) {
            auto source=fill.second->apply();
            fill.first->pItem->value->items=std::move(source->pItem->value->items);
            for(auto child=POVDescription(fill.first).member();child;child=POVDescription(child).adjacent()) child->pParent=fill.first.get();
            SetFlagBit(fill.first->pItem->value->flags,infonState::tailUnfinished,false);
            ++fill.first->workRevision;changed=true;
        }
        for(const auto& span:unitSpans_) {
            auto found=definitions_->unitSpans.find(span.target);
            if(found==definitions_->unitSpans.end() || found->second.source!=span.source || found->second.start!=span.start ||
               found->second.count!=span.count || !sameDefinition(found->second.member,span.member)) {
                definitions_->unitSpans[span.target]={span.source,span.member,span.start,span.count};
                ++span.target->workRevision;changed=true;
            }
        }
        for(const auto& pair:unitPairs_) {
            auto source=pair.existing ? pair.existing : pair.plan->apply();
            if(!pair.existing) definitions_->unitViews[pair.key]={pair.shape,pair.plan->pieces,source};
            if(!access_->hasDirectIdentity(pair.target,source)) {
                access_->registry->recordIdentity(pair.target,source,iekListCorrespondence,0);changed=true;
            }
            changed=access_->apply(source,pair.target,sizeAndValue) || changed;
            for(auto& command:SparseOccurrence::commands(access_,definitions_,source)) changed=command.apply() || changed;
        }
        for(const auto& pair:pairs_) {
            auto source=pair.source;
            if(pair.index>=0) {
                definitions_->retainOccurrenceOwner(source);
                if(auto owner=owners_[source]) {
                    source->pParent=owner.get();definitions_->retainOccurrenceOwner(owner);
                }
                auto old=access_->existingSparseItem(source,pair.index);
                source=access_->sparseItem(source,pair.index);changed|=!old;
            }
            if(!source) continue;
            access_->bind(pair.target);access_->bind(source);
            if(!access_->hasDirectIdentity(pair.target,source)) {
                access_->registry->recordIdentity(pair.target,source,iekListCorrespondence,0);changed=true;
            }
            changed=access_->apply(source,pair.target,sizeAndValue) || changed;
            for(auto& command:SparseOccurrence::commands(access_,definitions_,source)) changed=command.apply() || changed;
            if(!pair.target->pItem->type && source->pItem->type) {
                pair.target->pItem->type=std::make_shared<WordUse>(*source->pItem->type);++pair.target->workRevision;changed=true;
            }
        }
        return changed;
    }
};
