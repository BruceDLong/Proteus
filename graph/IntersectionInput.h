#pragma once

// Negative inner extents specify a source-end origin. Lower that coordinate
// to an ordinary source window; correspondence still proves all pattern
// constraints, including those after the marked result.
class IntersectionInput {
    static std::shared_ptr<infonState> number(FlexNum value) {
        auto result=std::make_shared<infonState>();
        SetModeBits(result->flags,infonState::overlayType,infonState::NUM);
        SetModeBits(result->flags,infonState::evalMode,infonState::emLiteral);
        result->num=value;return result;
    }
public:
    static bool ready(ProteusGraphAccess& access,const POVNode& owner) {
        auto body=readView(owner);FlexNum offset;Cost visits=0;
        return getFlagBit(body->flags,infonView::isInnr) && !body->wrkList.empty() &&
            signedCoordinate(access,body->infSize,offset,visits) &&
            offset<0 && offset.getDenominator()==1 && offset>=-INT64_MAX;
    }
    static bool prepare(ProteusGraphAccess& access,const POVNode& owner) {
        if(!ready(access,owner)) return false;
        auto body=readView(owner);FlexNum offset;Cost visits=0;
        signedCoordinate(access,body->infSize,offset,visits);
        auto width=offset;width.val=-width.val;
        for(auto& input:body->wrkList) {
            // The qualifier belongs to the input relationship, not to the
            // source occurrence through which the window is searched.
            auto source=input->snapshotView(917);
            auto index=std::make_shared<POV>(917);readView(index)=std::make_shared<infonView>();
            SetModeBits(readView(index)->flags,infonView::itmMode,infonView::mItem);
            SetModeBits(readView(index)->flags,infonView::viewMode,infonView::vmOverlay);
            readView(index)->value=number(offset);
            SetModeBits(readView(index)->infSize->flags,infonState::evalMode,infonState::emUnknown);
            auto spec=std::make_shared<POV>(917);readView(spec)=std::make_shared<infonView>();
            SetModeBits(readView(spec)->flags,infonView::itmMode,infonView::mItem);
            SetModeBits(readView(spec)->flags,infonView::viewMode,infonView::vmOverlay);
            readView(spec)->orderMode=uOrdered;readView(spec)->infSize=number(width);
            SetModeBits(readView(spec)->value->flags,infonState::overlayType,infonState::LST);
            SetModeBits(readView(spec)->value->flags,infonState::evalMode,infonState::emLiteral);
            SetFlagBit(readView(spec)->value->flags,infonState::tailUnfinished,true);
            readView(spec)->value->sizeMode=fromGiven;
            // The signed origin is measured in the pattern's unit. Keep that
            // unit on the window query so positional selection uses the same
            // definition-derived coordinates as an explicit typed slice.
            auto first=body->value->items.head;
            if(first) {
                auto item=readView(first);
                if(item->value->listSpec) readView(spec)->value->listSpec=item->value->listSpec;
                else if(item->type) {
                    auto unit=std::make_shared<infonView>();unit->type=item->type;
                    SetModeBits(unit->flags,infonView::viewMode,infonView::vmOverlay);
                    SetModeBits(unit->flags,infonView::itmMode,infonView::mItem);
                    SetModeBits(unit->value->flags,infonState::overlayType,infonState::tUnknown);
                    SetModeBits(unit->value->flags,infonState::evalMode,infonState::emUnknown);
                    SetModeBits(unit->infSize->flags,infonState::evalMode,infonState::emUnknown);
                    readView(spec)->value->listSpec=unit;
                }
            }
            auto window=PathExpression::positionQuery(access,source,index,spec);
            window->flags=input->flags;
            // The window already supplies exactly the requested local extent.
            // Fixing the window's endpoints does not remove its permission
            // to interpret the source in the pattern's unit/representation.
            SetFlagBit(window->flags,POV::looseMode,input->looseType());
            SetFlagBit(window->flags,POV::looseTop,false);
            SetFlagBit(window->flags,POV::selectorLocalWork,false);input=window;
        }
        // The signed expression is consumed as a coordinate. It must no
        // longer be evaluated as the local list's cardinality.
        body->sizeExpression.reset();body->infSize=number(width);
        ++owner->workRevision;SetFlagBit(body->flags,infonView::isNormed,false);
        return true;
    }
};
