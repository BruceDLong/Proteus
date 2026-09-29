#pragma once

// A homogeneous ordered definition measures a unit in terms of its leaf type.
// Neither the arithmetic nor the representation depends on temporal meaning.
struct UnitMeasure {
    DefinitionSpec spec,leaf;
    std::string base;
    std::int64_t width=0;
    explicit UnitMeasure(DefinitionSpec requirement):spec(std::move(requirement)) {
        auto at=spec;std::set<DefinitionSpec> seen;std::int64_t scale=1;
        while(at && seen.insert(at).second) {
            if(!at->choices.empty() || at->numeric.valid || at->facts.hasValue) return;
            if(!at->list) {
                if(at->label.empty()) return;
                leaf=at;base=at->label;width=scale;return;
            }
            if(at->order==uUnordered || !at->members.empty() || !at->repeated) return;
            auto count=at->memberLayout().extent();
            if(count<=0 || count>INT64_MAX/scale) return;
            scale*=count;at=at->repeated;
        }
    }
    explicit operator bool() const {return width>0;}
};

// Written members are measured in their own listSpec. A declaration using
// another compatible unit contributes the converted extent, not a contradictory
// scalar/list conjunction at every written member position.
inline DefinitionSpec definitionInUnits(const DefinitionSpec& declared,const DefinitionSpec& supplied) {
    if(!declared->list || !declared->repeated || !supplied->list || !supplied->repeated ||
       declared->order==uUnordered || supplied->order==uUnordered || !declared->members.empty()) return declared;
    UnitMeasure from(declared->repeated),to(supplied->repeated);
    if(!from || !to || from.base!=to.base || from.width==to.width) return declared;
    auto result=std::make_shared<DefinitionRequirement>(*declared);
    if(result->facts.hasSize) {
        result->facts.size.val*=from.width;result->facts.size.val/=to.width;
        result->facts.size.val.canonicalize();
        if(result->facts.size.getDenominator()!=1) throw DefinitionConflict("nonintegral definition unit extent");
    }
    result->repeated=supplied->repeated;result->templateSource=supplied->templateSource;
    return result;
}
