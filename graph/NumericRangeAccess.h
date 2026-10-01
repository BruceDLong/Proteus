#pragma once

using NumericBounds=NumericRange<mpq_class>;
struct NumericView {
    NumericBounds bounds;
    GraphTypeFacts facts;
    bool expression=false;
    bool constraint=false;
};
// Read existing scalar/concat structure without changing an operand or a shared
// declaration. Only additive offsets of a range are contiguous numeric sets.
class NumericRangeAccess {
    ProteusGraphAccess& access_;
    std::set<POVNode> active_;
    NumericView value(const POVNode& node,const std::shared_ptr<infonState>& body) {
        NumericView out;auto item=readView(node);
        out.facts.valid=true;out.facts.overlay=infonState::NUM;
        out.facts.hasSize=getModeBits(item->infSize->flags,infonState::evalMode)==infonState::emLiteral;
        out.facts.size=item->infSize->num;out.facts.explicitSize=body->sizeMode==fromGiven;
        auto overlay=getModeBits(body->flags,infonState::overlayType),mode=getModeBits(body->flags,infonState::evalMode);
        if(getFlagBit(body->flags,infonState::undefined)) return {};
        if(getFlagBit(body->flags,infonState::inverted)) return {};
        if(overlay==infonState::NUM) {
            if(mode==infonState::emLiteral) {
                if(body->num.getDenominator()==0) return {};
                out.facts.numericExtentApplied=getFlagBit(body->flags,infonState::numericExtentApplied) || out.facts.hasSize;
                auto n=finiteNumericRepresentative(body->num,
                    out.facts.hasSize && !getFlagBit(body->flags,infonState::numericExtentApplied),out.facts.size).val;
                out.bounds=NumericBounds::literal(n);out.facts.hasValue=true;out.facts.number.val=n;
            } else if(mode==infonState::emUnknown) {
                out.constraint=true;
                if(out.facts.hasSize && out.facts.size<0) return {};
                out.bounds=out.facts.hasSize ? NumericBounds::interval(0,out.facts.size.val) : NumericBounds::any();
            }
            return out;
        }
        if((overlay==infonState::tUnknown || overlay==infonState::tUnset) && mode==infonState::emUnknown) {
            out.constraint=true;out.bounds=NumericBounds::any();return out;
        }
        if(overlay!=infonState::LST || mode!=infonState::emConcat || body->listSpec ||
           getFlagBit(body->flags,infonState::tailUnfinished) || getFlagBit(body->flags,infonState::timelike)) return {};
        out.expression=true;out.constraint=true;bool first=true;std::set<POVNode> seen;
        for(auto at=body->items.head;at;at=at->next) {
            ++work.visits;
            if(!seen.insert(at).second || getFlagBit(at->flags,POV::isSubItm) ||
               !POVDescription(at,access_.registry.get()).relationships().empty()) return {};
            auto part=read(at);if(!part.bounds.valid) return {};
            if(first) out.bounds=part.bounds;
            else {
                if(readView(at)->value->sizeMode!=fromContext || !part.bounds.point) return {};
                out.bounds=out.bounds.shifted(part.bounds.low);
            }
            first=false;
        }
        if(first) return {};
        return out;
    }
public:
    Work work{0,0};
    explicit NumericRangeAccess(ProteusGraphAccess& access):access_(access) {}
    NumericView read(const POVNode& node,bool predicate=false,bool relationship=false,bool provenance=true) {
        ++work.visits;
        if(!node || access_.unknownOverlay(node) || !access_.plainStructure(node,false,false,relationship,false,false,false) ||
           (!predicate && getFlagBit(readView(node)->flags,infonView::invertMatch)) || !active_.insert(node).second) return {};
        auto out=value(node,readView(node)->value);
        if(provenance && readView(node)->valueExpression && out.bounds.valid) {
            auto original=value(node,readView(node)->valueExpression);
            out.bounds=out.bounds.intersect(original.bounds);
            out.constraint=out.constraint || original.constraint;out.expression=out.expression || original.expression;
        }
        active_.erase(node);return out;
    }
    static bool write(ProteusGraphAccess& access,const POVNode& target,const NumericView& result) {
        NumericRangeAccess reader(access);auto current=reader.read(target,false,false,false);
        if(!result.bounds.valid || result.bounds.empty || !result.bounds.bounded ||
           (current.bounds.same(result.bounds) && (!result.facts.hasSize || current.facts.hasSize))) return false;
        auto item=readView(target);access.bind(target);
        SetModeBits(item->flags,infonView::viewMode,infonView::vmOverlay);
        if(!item->valueExpression) item->valueExpression=item->value;
        auto body=std::make_shared<infonState>();
        body->sizeMode=result.facts.explicitSize ? fromGiven : fromCount;
        if(result.bounds.point) {
            SetModeBits(body->flags,infonState::overlayType,infonState::NUM);
            SetModeBits(body->flags,infonState::evalMode,infonState::emLiteral);
            body->num.val=result.bounds.low;
            SetFlagBit(body->flags,infonState::numericExtentApplied,result.facts.numericExtentApplied || result.facts.hasSize);
        } else {
            SetModeBits(body->flags,infonState::overlayType,infonState::LST);
            SetModeBits(body->flags,infonState::evalMode,infonState::emConcat);
            for(bool offset:{false,true}) {
                auto term=std::make_shared<infonView>();
                SetModeBits(term->flags,infonView::viewMode,infonView::vmOverlay);
                SetModeBits(term->flags,infonView::itmMode,infonView::mItem);
                SetModeBits(term->infSize->flags,infonState::overlayType,infonState::NUM);
                SetModeBits(term->infSize->flags,infonState::evalMode,(offset ? infonState::emUnknown : infonState::emLiteral));
                SetModeBits(term->value->flags,infonState::overlayType,infonState::NUM);
                SetModeBits(term->value->flags,infonState::evalMode,(offset ? infonState::emLiteral : infonState::emUnknown));
                term->infSize->num.val=result.bounds.high-result.bounds.low;term->value->num.val=result.bounds.low;
                term->value->sizeMode=offset ? fromContext : fromGiven;
                auto pov=body->items.append(term,false,904);pov->pParent=target.get();access.bind(pov);
            }
        }
        if(result.facts.hasSize && getModeBits(item->infSize->flags,infonState::evalMode)==infonState::emUnknown) {
            SetModeBits(item->infSize->flags,infonState::evalMode,infonState::emLiteral);item->infSize->num=result.facts.size;
        }
        item->value=body;SetFlagBit(item->flags,infonView::isNormed,false);++target->workRevision;return true;
    }
};
