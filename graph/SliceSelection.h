#pragma once

// Fixed windows reuse the shared logical layout. A window is a derived list
// view, not a copy of its members and not an instruction to expand a sparse run.
class SliceSelection {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    using Window = SpanWindow<POVNode>;
    static bool samePieces(const std::shared_ptr<GraphSliceView>& view,const Window& window) {
        if(view->pieces.size()!=window.pieces.size()) return false;
        for(std::size_t i=0;i<window.pieces.size();++i) {
            auto saved=view->pieces[i];const auto& piece=window.pieces[i];
            if(saved->source!=piece.node || saved->gap!=piece.gap || saved->offset!=piece.offset || saved->count!=piece.count) return false;
        }
        return true;
    }
    static POVNode manifest(const std::shared_ptr<ProteusGraphAccess>& access,
                            const std::shared_ptr<ProteusDefinitions>& definitions,
                            const std::shared_ptr<GraphSliceView>& view,const Window& window,bool& changed) {
        if(!view->result) {
            view->result=std::make_shared<POV>(916);
            auto item=view->result->pItem=std::make_shared<infonView>();
            SetModeBits(item->flags,infonView::itmMode,infonView::mItem);
            SetModeBits(item->flags,infonView::viewMode,infonView::vmOverlay);
            item->orderMode=uOrdered;
            SetModeBits(item->infSize->flags,infonState::overlayType,infonState::NUM);
            SetModeBits(item->infSize->flags,infonState::evalMode,infonState::emLiteral);
            item->infSize->num=view->count;
            SetModeBits(item->value->flags,infonState::overlayType,infonState::LST);
            SetModeBits(item->value->flags,infonState::evalMode,infonState::emLiteral);
            item->value->sizeMode=fromGiven;
            access->bind(view->result);changed=true;
        }
        if(!samePieces(view,window)) {
            changed=true;
            using Key=std::tuple<POVNode,bool,std::int64_t,std::int64_t>;
            std::map<Key,std::shared_ptr<GraphSlicePiece>> reusable;
            for(auto& piece:view->pieces) {
                reusable[{piece->source,piece->gap,piece->offset,piece->count}]=piece;
                piece->alias->pParent=nullptr;piece->alias->next.reset();piece->alias->prev.reset();
            }
            view->result->pItem->value->items.clear();view->pieces.clear();
            for(const auto& part:window.pieces) {
                auto found=reusable.find({part.node,part.gap,part.offset,part.count});
                if(found!=reusable.end()) {
                    auto piece=found->second;piece->alias->pParent=view->result.get();
                    view->result->pItem->value->items.appendPov(piece->alias);view->pieces.push_back(piece);continue;
                }
                auto piece=std::make_shared<GraphSlicePiece>();
                piece->source=part.node;piece->gap=part.gap;piece->offset=part.offset;piece->count=part.count;
                if(part.gap) {
                    piece->alias=std::make_shared<POV>(916);
                    SetFlagBit(piece->alias->flags,POV::isSubItm,true);piece->alias->pParent=view->result.get();
                    view->result->pItem->value->items.appendPov(piece->alias);
                } else {
                    // The expression belongs to the source occurrence. A
                    // neutral identity view acquires its facts through normal
                    // Sets, without interpreting a cursor in a second context.
                    piece->alias=access->appendTailMember(view->result,part.node);
                }
                view->pieces.push_back(piece);
                changed=true;
            }
        }
        for(auto& piece:view->pieces) {
            if(piece->sourceView==piece->source->pItem && piece->sourceRevision==piece->source->workRevision) continue;
            piece->sourceView=piece->source->pItem;piece->sourceRevision=piece->source->workRevision;
            if(piece->gap) {
                piece->alias->pItem=piece->source->makeSparseSpanCopy(piece->count);
                piece->alias->pItem->orderMode=uOrdered;
                if(access->sparse(piece->source)) {
                    auto family=access->registry->getOrCreateRepetitionFamily(piece->source.get());
                    piece->alias->repetitionSpan.family=family;
                    piece->alias->repetitionSpan.startOrdinal=piece->source->repetitionSpan.startOrdinal+piece->offset;
                    definitions->retainOccurrenceOwner(piece->source);
                    definitions->retainOccurrenceOwner(piece->alias);
                }
            } else {
                piece->alias->pItem->type=piece->source->pItem->type;
                piece->alias->pItem->addedTypes=piece->source->pItem->addedTypes;
                definitions->inherit(piece->alias,piece->source);
            }
            access->bind(piece->alias);++piece->alias->workRevision;changed=true;
        }
        if(changed) {
            ++view->result->workRevision;
            SetFlagBit(view->result->pItem->flags,infonView::isNormed,false);
        }
        return view->result;
    }
public:
    SliceSelection(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions)
        :access_(std::move(access)),definitions_(std::move(definitions)) {}
    Expansion<POVNode> expand(const Query<POVNode>& q) const {
        Expansion<POVNode> out;
        auto state=q.related->pItem->pathSelection;auto spec=state->spec;
        if(q.kind==pathSliceCandidate) {
            FieldPredicate predicate(access_,definitions_,out);
            auto match=predicate.match(spec,q.node,{});
            auto visits=addCost(1,predicate.work.visits);
            if(match==ScalarMatch::unknown) {out.deferred=true;return out;}
            auto next=q;next.kind=match==ScalarMatch::yes ? pathMatched : pathEnd;
            out.edges.push_back({"check slice specification",{visits,0},true,[next]{return next;}});
            return out;
        }
        // Expose alternative identities before asking whether this stored
        // shape can supply the window. A cheaper unresolved route cannot hide them.
        for(const auto& relation:POVDescription(q.node,access_->registry.get()).relationships(false)) {
            auto next=q;next.node=relation.target;
            out.edges.push_back({"identity span access",{1,0},true,[next]{return next;}});
        }
        auto refine=[&](const POVNode& node,int demand,const char* description) {
            out.refinements.push_back({{node,demand,0},{1,0},description,true});
        };
        if(definitions_->required(q.node)) refine(q.node,typeRequirements,"expose slice source shape");
        auto boundary=listBoundary(*access_,spec);
        for(const auto& missing:boundary.missingExtents) refine(missing,sizeOnly,"find slice member extent");
        if(!boundary.valid || boundary.extent<0) {
            refine(spec,sizeOnly,"find slice extent");out.deferred=true;return out;
        }
        SpanLayout<POVNode> layout;Cost visits=boundary.visits;
        std::map<POVNode,std::vector<POVNode>> locations;
        if(!readListLayout(*access_,q.node,layout,visits,[&](const auto& at) {
            if(at.phase==ListPhase::enter || at.phase==ListPhase::member) locations[at.node]=at.owners;
            if(at.phase!=ListPhase::enter) return !access_->inputPending(at.node);
            if(!access_->navigationList(at.node)) {
                refine(at.node,valueOnly,"expose slice list");return false;
            }
            if(access_->listExtent(at.node)<0) refine(at.node,sizeOnly,"find slice source extent");
            return !access_->inputPending(at.node);
        },[](const POVNode& node){return node;})) {out.deferred=true;return out;}
        auto window=selectSpanWindow(layout,q.argument,boundary.extent);
        if(window.outside) {
            auto end=q;end.kind=pathEnd;
            out.edges.push_back({"span outside list",{visits,0},true,[end]{return end;}});return out;
        }
        if(!window.complete) {out.deferred=true;return out;}
        for(const auto& piece:window.pieces)
            if(piece.gap && piece.node->pItem->value->listSpec && !access_->sparse(piece.node)) {
                // A prefixed repetition needs its tail's occurrence mapping.
                // Do not reuse the root's ordinals as if that prefix were absent.
                out.deferred=true;return out;
            }
        // Source expressions and virtual gaps remain searchable even when
        // the window's shape is already known. Preserve the original owner
        // stack for contextual references; these are optional routes, not a
        // requirement to normalize unrelated parts before continuing a path.
        for(const auto& piece:window.pieces) if(piece.gap || access_->known(piece.node)!=sizeAndValue) {
            auto context=locations.at(piece.node);context.insert(context.begin(),piece.node);
            out.refinements.push_back({{piece.node,sizeAndValue,0,{},context},{1,0},"refine slice source piece",true});
        }
        std::shared_ptr<GraphSliceView> saved;
        for(const auto& view:state->slices)
            if(view->source==q.node && view->start==q.argument && view->count==boundary.extent) {saved=view;break;}
        Work work{visits,0};
        if(!saved || !samePieces(saved,window)) work.creations=3+3*window.pieces.size();
        else for(const auto& piece:saved->pieces)
            if(piece->sourceView!=piece->source->pItem || piece->sourceRevision!=piece->source->workRevision) ++work.creations;
        auto access=access_;auto definitions=definitions_;
        out.edges.push_back({"materialize span view",work,true,[access,definitions,state,saved,window,q,count=boundary.extent]() mutable {
            if(!saved) {
                saved=std::make_shared<GraphSliceView>();saved->source=q.node;saved->start=q.argument;saved->count=count;
                state->slices.push_back(saved);
            }
            bool changed=false;auto result=manifest(access,definitions,saved,window,changed);
            auto next=q;next.node=result;next.kind=pathSliceCandidate;
            return Arrival<POVNode>{next,changed};
        }});
        return out;
    }
};
