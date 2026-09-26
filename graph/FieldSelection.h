#pragma once

// Field selection classifies existing logical members. The only difference
// between ordered and unordered lookup is the result policy; both consume the
// same predicate evidence. A cheap later route cannot skip an unknown
// earlier candidate and claim to have found the first match.
class FieldSelection {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    static Edge<POVNode> edge(const char* label, Query<POVNode> query, Cost visits=1) {
        return {label, {visits,0}, true, [query] { return query; }};
    }
    static bool unordered(const POVNode& node) {
        return node->pItem->orderMode==uUnordered;
    }
    void advance(Expansion<POVNode>& out, const Query<POVNode>& q, ListPosition<POVNode> at, Cost visits=1) const {
        auto next=nextListPosition(*access_,std::move(at));
        if (next.phase==ListPhase::cycle) { out.deferred=true; return; }
        Query<POVNode> follow{next.phase==ListPhase::done ? q.context.front() : next.node,
            next.phase==ListPhase::done ? fieldEnd : fieldPosition,
            static_cast<int>(next.phase),q.related,{q.context.front()}};
        follow.context.insert(follow.context.end(),next.owners.begin(),next.owners.end());
        out.edges.push_back(edge("next field position",std::move(follow),visits));
    }
    bool sourceReady(const POVNode& node) const {
        return (access_->physicalList(node) || access_->unreferencedTemplateSpan(node)) && !access_->hasTypeRequirements(node) &&
            POVDescription(node,access_->registry.get()).relationships().empty();
    }
public:
    FieldSelection(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions)
        :access_(std::move(access)),definitions_(std::move(definitions)) {}
    Expansion<POVNode> expand(const Query<POVNode>& q) const {
        Expansion<POVNode> out;
        if(q.kind==fieldEnd) return out;
        if(q.kind==selectField) {
            if(!sourceReady(q.node)) {out.deferred=true;return out;}
            out.edges.push_back(edge("scan field candidates",{q.node,fieldPosition,static_cast<int>(ListPhase::enter),q.related,{q.node}}));
            return out;
        }
        ListPosition<POVNode> at{q.node,static_cast<ListPhase>(q.argument),{q.context.begin()+1,q.context.end()}};
        if(q.kind==fieldMatched || q.kind==fieldUnknown) {
            if(q.kind==fieldMatched && !unordered(q.context.front())) return out;
            advance(out,q,std::move(at));return out;
        }
        Cost visits=1;
        if(at.phase==ListPhase::enter) {
            if(!sourceReady(at.node) || (!unordered(q.context.front()) && unordered(at.node))) {out.deferred=true;return out;}
        } else if(at.phase==ListPhase::leave) {
            auto boundary=listBoundary(*access_,at.node);
            visits=addCost(visits,boundary.visits);
            if(!boundary.valid || boundary.tail!=0 || !boundary.missingExtents.empty()) {
                out.deferred=true;return out;
            }
        } else if(at.phase==ListPhase::member && !access_->subList(at.node)) {
            FieldPredicate predicate(access_,definitions_,out);
            auto match=predicate.match(q.related,at.node,at.owners);
            visits=addCost(visits,predicate.work.visits);
            if(match==ScalarMatch::yes) {
                auto found=q;found.kind=fieldMatched;
                out.edges.push_back(edge("compatible field member",std::move(found),visits));return out;
            }
            if(match==ScalarMatch::unknown) {
                out.deferred=true;
                if(unordered(q.context.front())) {
                    auto waiting=q;waiting.kind=fieldUnknown;
                    out.edges.push_back(edge("unresolved field candidate",std::move(waiting),visits));
                }
                return out;
            }
        }
        advance(out,q,std::move(at),visits);return out;
    }
};

// Dot syntax supplies an existing source and a predicate to the same goal.
// Keep the source's state/work and the parsed predicate intact while the working
// POV becomes the result. Chained paths, writes and retained-parent paths need
// their own continuation handling; do not silently consume them here.
class FieldPath {
public:
    static bool ready(ProteusGraphAccess& access,const POVNode& node) {
        auto item=node->pItem;auto part=item->partRef;
        return part && getModeBits(part->flags,PartPath::pathMode)==PartPath::dotField &&
            !getFlagBit(part->flags,PartPath::hasSemiColon) && !part->spec && !part->continuation && part->path &&
            item->parts.empty() && item->wrkList.empty() && !item->fieldSelection &&
            part->path->parts.empty() && !part->path->partRef && part->path->wrkList.empty() &&
            access.plainStructure(node,false,false,false,false,true) &&
            POVDescription(node,access.registry.get()).relationships(false).empty();
    }
    static bool prepare(ProteusGraphAccess& access,const POVNode& node) {
        if(!ready(access,node)) return false;
        auto original=node->pItem;
        auto source=node->snapshotView(915);
        source->pItem=std::make_shared<infonView>(*original);
        source->pItem->partRef.reset();SetFlagBit(source->pItem->flags,infonView::doReply,false);
        auto predicate=std::make_shared<POV>(915);predicate->pItem=original->partRef->path;
        auto state=std::make_shared<GraphFieldSelection>();
        state->expression=std::make_shared<infonView>(*original);
        SetFlagBit(state->expression->flags,infonView::doReply,false);
        state->source=source;state->predicate=predicate;
        auto result=std::make_shared<infonView>();
        SetModeBits(result->flags,infonView::itmMode,infonView::mItem);
        SetModeBits(result->flags,infonView::viewMode,infonView::vmOverlay);
        SetFlagBit(result->flags,infonView::doReply,getFlagBit(original->flags,infonView::doReply));
        SetModeBits(result->infSize->flags,infonState::overlayType,infonState::NUM);
        SetModeBits(result->infSize->flags,infonState::evalMode,infonState::emUnknown);
        SetModeBits(result->value->flags,infonState::overlayType,infonState::tUnknown);
        SetModeBits(result->value->flags,infonState::evalMode,infonState::emUnknown);
        result->originID=original->originID;result->infCharPos=original->infCharPos;result->replyCharPos=original->replyCharPos;
        result->fieldSelection=state;node->pItem=result;
        access.bind(source);access.bind(predicate);access.bind(node);++node->workRevision;
        return true;
    }
};

class ProteusFieldGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::vector<POVNode> matches_,published_;
    POVNode result_;
    bool verified_=false,waiting_=false;
public:
    ProteusFieldGoal(Query<POVNode> query,std::shared_ptr<ProteusGraphAccess> access,
                     std::shared_ptr<ProteusDefinitions> definitions)
        :GoalSpec<POVNode>(query),access_(std::move(access)),definitions_(std::move(definitions)) {
        if(!request.context.empty()) {
            auto state=request.context.front()->pItem->fieldSelection;
            if(state) {result_=state->result;published_=state->matches;}
        }
    }
    void beginPass() override {matches_.clear();verified_=waiting_=false;}
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        auto q=visit.query;
        bool unordered=request.node->pItem->orderMode==uUnordered;
        if(q.kind==fieldUnknown) waiting_=true;
        if(q.kind==fieldMatched) matches_.push_back(q.node);
        if((q.kind!=fieldEnd && (q.kind!=fieldMatched || unordered)) || waiting_ || matches_.empty()) return {};
        if(result_ && published_==matches_) {verified_=true;return {};}
        auto matches=matches_;auto access=access_;
        Work work{matches.size(),matches.size()==1 ? 0 : 2*matches.size()+3};
        if (!request.context.empty()) ++work.creations;
        return {{"Set field selection result",[this,matches,access] {
            if(matches.size()==1) result_=matches.front();
            else {
                auto item=std::make_shared<infonView>();
                auto alternatives=makeInnrInfonView();
                for(const auto& source:matches) {
                    auto member=alternatives->value->items.append(source->pItem,false,914);
                    access->bind(member);access->bind(source);
                    access->registry->recordIdentity(member,source,iekListCorrespondence,0);
                }
                item->convertThisToIntersection(alternatives,fromGiven);
                SetModeBits(item->value->flags,infonState::overlayType,infonState::tUnknown);
                auto result=std::make_shared<POV>(914);result->pItem=item;access->bind(result);
                for(auto member=alternatives->value->items.head;member;member=member->next) member->pParent=result.get();
                result_=result;
            }
            published_=matches;verified_=true;
            if(!request.context.empty()) {
                auto owner=request.context.front();auto previous=owner->pItem;
                auto state=previous->fieldSelection;
                auto result=std::make_shared<infonView>(*result_->pItem);
                result->fieldSelection=state;result->selectedResult=result_;
                result->originID=previous->originID;result->infCharPos=previous->infCharPos;result->replyCharPos=previous->replyCharPos;
                SetFlagBit(result->flags,infonView::doReply,getFlagBit(previous->flags,infonView::doReply));
                SetFlagBit(result->flags,infonView::isNormed,false);
                state->result=result_;state->matches=matches;
                owner->pItem=result;access->bind(owner);++owner->workRevision;
                if(matches.size()==1) {
                    definitions_->inherit(owner,result_);
                }
            }
            return true;
        },work}};
    }
    bool satisfied() const override {return verified_;}
    bool symbolic() const override {return !verified_ || (result_ && result_->pItem->isOrLike());}
    POVNode result() const override {return result_;}
};
