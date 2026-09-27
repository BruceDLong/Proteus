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
        auto body=owner->pItem;FlexNum offset;Cost visits=0;
        return getFlagBit(body->flags,infonView::isInnr) && !body->wrkList.empty() &&
            signedCoordinate(access,body->infSize,offset,visits) &&
            offset<0 && offset.getDenominator()==1 && offset>=-INT64_MAX;
    }
    static bool prepare(ProteusGraphAccess& access,const POVNode& owner) {
        if(!ready(access,owner)) return false;
        auto body=owner->pItem;FlexNum offset;Cost visits=0;
        signedCoordinate(access,body->infSize,offset,visits);
        auto width=offset;width.val=-width.val;
        for(auto& input:body->wrkList) {
            // The qualifier belongs to the input relationship, not to the
            // source occurrence through which the window is searched.
            auto source=input->snapshotView(917);
            auto index=std::make_shared<POV>(917);index->pItem=std::make_shared<infonView>();
            SetModeBits(index->pItem->flags,infonView::itmMode,infonView::mItem);
            SetModeBits(index->pItem->flags,infonView::viewMode,infonView::vmOverlay);
            index->pItem->value=number(offset);
            SetModeBits(index->pItem->infSize->flags,infonState::evalMode,infonState::emUnknown);
            auto spec=std::make_shared<POV>(917);spec->pItem=std::make_shared<infonView>();
            SetModeBits(spec->pItem->flags,infonView::itmMode,infonView::mItem);
            SetModeBits(spec->pItem->flags,infonView::viewMode,infonView::vmOverlay);
            spec->pItem->orderMode=uOrdered;spec->pItem->infSize=number(width);
            SetModeBits(spec->pItem->value->flags,infonState::overlayType,infonState::LST);
            SetModeBits(spec->pItem->value->flags,infonState::evalMode,infonState::emLiteral);
            SetFlagBit(spec->pItem->value->flags,infonState::tailUnfinished,true);
            spec->pItem->value->sizeMode=fromGiven;
            auto window=PathExpression::positionQuery(access,source,index,spec);
            window->flags=input->flags;
            // The window already supplies exactly the requested local extent.
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
