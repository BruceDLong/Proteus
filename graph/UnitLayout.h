#pragma once

// A finite read-only projection of stored positions into a common leaf unit.
// Ordinary nesting is entered only when a homogeneous definition supplies the
// conversion. Sparse segments retain an arithmetic width and original owner.
struct UnitPosition {
    POVNode node;
    std::int64_t offset=0; // leaf-unit offset into the original virtual run
};
class UnitLayout {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::set<POVNode> active_;
    bool read(const POVNode& source,DefinitionSpec inherited={}) {
        if(!active_.insert(source).second) return false;
        if(auto mapped=definitions_->unitSpan(source)) {
            auto outer=std::move(layout);layout={};
            const auto& span=*mapped;UnitMeasure member(span.member);
            if(!member || member.base!=base || !read(span.source)) return false;
            auto window=selectSpanWindow(layout,span.start,span.count*member.width);
            layout=std::move(outer);
            if(!window.complete) return false;
            for(const auto& piece:window.pieces)
                layout.segments.push_back({{piece.node.node,piece.node.offset+piece.offset},piece.gap,piece.count});
            active_.erase(source);return true;
        }
        for(const auto& relation:POVDescription(source,access_->registry.get()).relationships()) if(relation.isWork()) {
            RHSRepresentation represented(access_,source,relation.target,definitions_);
            work.visits=addCost(work.visits,represented.work.visits);
            if(!represented.represented) return false;
        }
        SpanLayout<POVNode> native;
        std::map<POVNode,POVNode> parents;
        if(!readListLayout(*access_,source,native,work.visits,[&](const auto& at) {
            if(access_->inputPending(at.node)) return false;
            if(!at.owners.empty()) parents[at.node]=at.owners.back();
            return at.phase!=ListPhase::enter || access_->navigationList(at.node) ||
                ((access_->physicalList(at.node) || access_->templatePrefixList(at.node)) && definitions_->listOrder(at.node)==uOrdered);
        },[](const POVNode& node){return node;}) || !native.solve() || native.extent()<0) return false;
        for(const auto& segment:native.segments) {
            if(!segment.width) continue;
            auto owner=segment.gap ? segment.node : parents[segment.node];
            auto required=inherited;
            if(owner) if(auto templ=definitions_->repeatedBody(owner)) {
                auto proof=definitions_->describe(templ,work);if(!proof) return false;required=proof->requirement;
            }
            if(!segment.gap && readView(segment.node)->type) {
                auto named=definitions_->unit(readView(segment.node)->type,work);
                if(named && UnitMeasure(named)) required=named;
            }
            if(!required) required=leaf;
            UnitMeasure unit(required);
            if(!unit || unit.base!=base) return false;
            if(segment.gap) {
                if(!access_->sparse(segment.node) || segment.width>INT64_MAX/unit.width) return false;
                units[segment.node]=required;owners[segment.node]=parents[segment.node];
                layout.segments.push_back({{segment.node,0},true,segment.width*unit.width});
            } else if(unit.width>1) {
                if(!read(segment.node,required->repeated)) return false;
            } else {
                auto facts=access_->typeFacts(segment.node);
                if(!facts.valid || !facts.combine(required->facts)) return false;
                layout.segments.push_back({{segment.node,0},false,1});
            }
        }
        active_.erase(source);return true;
    }
public:
    SpanLayout<UnitPosition> layout;
    std::map<POVNode,DefinitionSpec> units;
    std::map<POVNode,POVNode> owners;
    Work work{0,0};
    std::string base;
    DefinitionSpec leaf;
    bool valid=false;
    UnitLayout(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions,
               const POVNode& source,const UnitMeasure& measure)
        :access_(std::move(access)),definitions_(std::move(definitions)),base(measure.base),leaf(measure.leaf) {
        valid=bool(measure) && read(source) && layout.extent()>=0;
    }
};
