#pragma once

// A path needs the complete candidate set, even when that set is a oneOf.
// A caller asking for the resulting value still sees that value as symbolic.
enum FieldResultUse { fieldValue = 0, fieldCandidates = 1 };

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
    // Ordinary field walks store [source, embedded owners...]. A walk through
    // a choice stores [source, choice, alternative, embedded owners...] and
    // marks that continuation in coordinates. These are traversal cursors,
    // not additional graph nodes or adjacency storage.
    static std::size_t ownerOffset(const Query<POVNode>& q) {return q.coordinates.empty() ? 1 : 3;}
    void nextAlternative(Expansion<POVNode>& out,const POVNode& choice,const POVNode& current,const POVNode& predicate) const {
        auto next=current ? current->next : choice->pItem->innr->value->items.head;
        out.edges.push_back(edge("next field alternative",
            {next ? next : choice,next ? fieldAlternative : fieldAlternativesEnd,0,predicate,{choice}}));
    }
    void advance(Expansion<POVNode>& out, const Query<POVNode>& q, ListPosition<POVNode> at, Cost visits=1) const {
        auto next=nextListPosition(*access_,std::move(at));
        if (next.phase==ListPhase::cycle) {unresolved(out,q,"unresolved cyclic alternative",visits);return;}
        Query<POVNode> follow{next.phase==ListPhase::done ? q.context.front() : next.node,
            next.phase==ListPhase::done ? fieldEnd : fieldPosition,
            static_cast<int>(next.phase),q.related,{q.context.begin(),q.context.begin()+ownerOffset(q)},q.coordinates};
        follow.context.insert(follow.context.end(),next.owners.begin(),next.owners.end());
        out.edges.push_back(edge("next field position",std::move(follow),visits));
    }
    bool sourceReady(const POVNode& node) const {
        if (!access_->physicalList(node) && !access_->unreferencedTemplateSpan(node)) return false;
        for (const auto& relation:POVDescription(node,access_->registry.get()).relationships())
            if (!access_->sameInfon(node,relation.target)) return false;
        return true;
    }
    void unresolved(Expansion<POVNode>& out,const Query<POVNode>& q,const char* label,Cost visits=1) const {
        out.deferred=true;
        if(!q.coordinates.empty()) {
            auto unknown=q;unknown.kind=fieldAlternativeUnknown;
            out.edges.push_back(edge(label,std::move(unknown),visits));
        }
    }
public:
    FieldSelection(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions)
        :access_(std::move(access)),definitions_(std::move(definitions)) {}
    Expansion<POVNode> expand(const Query<POVNode>& q) const {
        Expansion<POVNode> out;
        if(q.kind==fieldAlternativesEnd) {
            out.deferred=getFlagBit(q.node->pItem->innr->value->flags,infonState::tailUnfinished);
            return out;
        }
        if(q.kind==fieldEnd || q.kind==fieldAlternativeUnknown) {
            if(!q.coordinates.empty()) nextAlternative(out,q.context[1],q.context[2],q.related);
            return out;
        }
        if(q.kind==fieldAlternative) {
            Query<POVNode> branch{q.node,fieldPosition,static_cast<int>(ListPhase::enter),q.related,{q.node,q.context.front(),q.node},{1}};
            out.edges.push_back(edge("scan alternative fields",std::move(branch)));
            return out;
        }
        if(q.kind==selectField) {
            // A path continuation demands its predecessor's selection, not
            // complete normalization of that intermediate object.
            if (auto field=q.node->pItem->fieldSelection) {
                out.refinements.push_back({{field->source,selectField,fieldCandidates,field->predicate,{q.node}},
                    {0,0},"select preceding field",false});
                if (field->result) {
                    auto next=q;next.node=field->result;
                    out.edges.push_back(edge("continue from selected field",std::move(next)));
                }
                return out;
            }
            if(q.node->pItem->isOrLike()) {
                auto body=q.node->pItem->innr;
                if(!access_->plain(q.node,true,false,false,false) || !access_->literalSelectionBody(body) ||
                    body->partRef || !body->wrkList.empty() ||
                    !POVDescription(q.node,access_->registry.get()).relationships().empty()) {
                    out.deferred=true;return out;
                }
                nextAlternative(out,q.node,{},q.related);return out;
            }
            if(!sourceReady(q.node)) {out.deferred=true;return out;}
            out.edges.push_back(edge("scan field candidates",{q.node,fieldPosition,static_cast<int>(ListPhase::enter),q.related,{q.node}}));
            return out;
        }
        ListPosition<POVNode> at{q.node,static_cast<ListPhase>(q.argument),{q.context.begin()+ownerOffset(q),q.context.end()}};
        if(q.kind==fieldMatched || q.kind==fieldUnknown) {
            if(q.kind==fieldMatched && !unordered(q.context.front())) {
                if(!q.coordinates.empty()) nextAlternative(out,q.context[1],q.context[2],q.related);
                return out;
            }
            advance(out,q,std::move(at));return out;
        }
        Cost visits=1;
        if(at.phase==ListPhase::enter) {
            if(!sourceReady(at.node) || (!unordered(q.context.front()) && unordered(at.node))) {
                unresolved(out,q,"unresolved alternative structure");return out;
            }
        } else if(at.phase==ListPhase::leave) {
            auto boundary=listBoundary(*access_,at.node);
            visits=addCost(visits,boundary.visits);
            if(!boundary.valid) {unresolved(out,q,"unresolved alternative boundary",visits);return out;}
            if(boundary.tail!=0 || !boundary.missingExtents.empty()) {
                out.deferred=true;
                if(unordered(q.context.front())) {
                    auto waiting=q;waiting.kind=fieldUnknown;
                    out.edges.push_back(edge("unresolved field boundary",std::move(waiting),visits));
                } else unresolved(out,q,"unresolved alternative boundary",visits);
                return out;
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
                } else unresolved(out,q,"unresolved alternative candidate",visits);
                return out;
            }
        }
        advance(out,q,std::move(at),visits);return out;
    }
};

// Each dot segment is the same field-selection goal. Intermediate query POVs
// retain the parsed path's provenance, while candidate/source POVs remain shared.
// Writes and retained-parent paths need their own continuation semantics.
class FieldPath {
    static std::vector<std::shared_ptr<PartPath>> segments(ProteusGraphAccess& access,const POVNode& node) {
        auto item=node->pItem;
        if (!item->partRef || !item->parts.empty() || !item->wrkList.empty() || item->fieldSelection ||
            !access.plainStructure(node,false,false,false,false,true) ||
            !POVDescription(node,access.registry.get()).relationships(false).empty()) return {};
        std::vector<std::shared_ptr<PartPath>> result;
        std::set<std::shared_ptr<PartPath>> seen;
        for (auto part=item->partRef;part;part=part->path->partRef) {
            if (!seen.insert(part).second || getModeBits(part->flags,PartPath::pathMode)!=PartPath::dotField ||
                getFlagBit(part->flags,PartPath::hasSemiColon) || part->spec || part->continuation || !part->path ||
                !part->path->parts.empty() || !part->path->wrkList.empty()) return {};
            result.push_back(part);
        }
        return result;
    }
public:
    static bool ready(ProteusGraphAccess& access,const POVNode& node) {return !segments(access,node).empty();}
    static Work work(ProteusGraphAccess& access,const POVNode& node) {
        auto count=segments(access,node).size();return {count,6*count+1};
    }
    static bool prepare(ProteusGraphAccess& access,const POVNode& node) {
        auto path=segments(access,node);if(path.empty()) return false;
        auto original=node->pItem;
        auto source=node->snapshotView(915);
        source->pItem=std::make_shared<infonView>(*original);
        source->pItem->partRef.reset();SetFlagBit(source->pItem->flags,infonView::doReply,false);
        auto expression=std::make_shared<infonView>(*original);
        SetFlagBit(expression->flags,infonView::doReply,false);
        access.bind(source);
        for (std::size_t i=0;i<path.size();++i) {
            auto owner=i+1==path.size() ? node : std::make_shared<POV>(915);
            auto predicate=std::make_shared<POV>(915);
            // A following path is a continuation, not a condition on this
            // segment's candidate. Preserve the original parsed predicates.
            predicate->pItem=std::make_shared<infonView>(*path[i]->path);
            predicate->pItem->partRef.reset();
            auto state=std::make_shared<GraphFieldSelection>();
            state->expression=expression;state->source=source;state->predicate=predicate;
            auto result=std::make_shared<infonView>();
            SetModeBits(result->flags,infonView::itmMode,infonView::mItem);
            SetModeBits(result->flags,infonView::viewMode,infonView::vmOverlay);
            SetFlagBit(result->flags,infonView::doReply,owner==node && getFlagBit(original->flags,infonView::doReply));
            SetModeBits(result->infSize->flags,infonState::overlayType,infonState::NUM);
            SetModeBits(result->infSize->flags,infonState::evalMode,infonState::emUnknown);
            SetModeBits(result->value->flags,infonState::overlayType,infonState::tUnknown);
            SetModeBits(result->value->flags,infonState::evalMode,infonState::emUnknown);
            result->originID=original->originID;result->infCharPos=original->infCharPos;result->replyCharPos=original->replyCharPos;
            result->fieldSelection=state;owner->pItem=result;
            access.bind(predicate);access.bind(owner);++owner->workRevision;
            source=owner;
        }
        return true;
    }
};

class ProteusFieldGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::vector<POVNode> matches_,published_;
    POVNode result_;
    bool verified_=false,waiting_=false,publishedPending_=false,aggregating_=false;
    bool branchMatched_=false;
    bool collectedAlternatives() const {return result_ && (publishedPending_ || published_.size()!=1);}
public:
    ProteusFieldGoal(Query<POVNode> query,std::shared_ptr<ProteusGraphAccess> access,
                     std::shared_ptr<ProteusDefinitions> definitions)
        :GoalSpec<POVNode>(query),access_(std::move(access)),definitions_(std::move(definitions)) {
        if(!request.context.empty()) {
            auto state=request.context.front()->pItem->fieldSelection;
            if(state) {result_=state->result;published_=state->matches;publishedPending_=state->pending;}
        }
    }
    void beginPass() override {matches_.clear();verified_=waiting_=aggregating_=false;branchMatched_=false;}
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        auto q=visit.query;
        if(q.kind==fieldAlternative) {aggregating_=true;branchMatched_=false;}
        if(q.kind==fieldUnknown || q.kind==fieldAlternativeUnknown) waiting_=true;
        if(q.kind==fieldMatched) {
            branchMatched_=true;
            if(std::find(matches_.begin(),matches_.end(),q.node)==matches_.end()) matches_.push_back(q.node);
        }
        if(q.kind==fieldEnd && aggregating_ && !branchMatched_) waiting_=true;
        if(q.kind==fieldAlternativesEnd && getFlagBit(q.node->pItem->innr->value->flags,infonState::tailUnfinished)) waiting_=true;
        bool finished=q.kind==fieldAlternativesEnd || (!aggregating_ && (q.kind==fieldEnd ||
            (q.kind==fieldMatched && q.context.front()->pItem->orderMode!=uUnordered)));
        if(!finished) return {};
        const bool alternatives=waiting_ || matches_.size()!=1;
        if(result_ && published_==matches_ && publishedPending_==waiting_) {
            bool current=true;
            if(!alternatives && !request.context.empty()) {
                auto state=request.context.front()->pItem->fieldSelection;
                current=state->resultView==result_->pItem && state->resultRevision==result_->workRevision;
            }
            if(alternatives) {
                auto member=result_->pItem->innr->value->items.head;
                for(const auto& source:matches_) {
                    if(!member || member->pItem!=source->pItem) {current=false;break;}
                    member=member->next;
                }
            }
            if(current) {verified_=!waiting_;return {};}
        }
        // Existing alternatives keep their POV/identity when more information
        // arrives. Only newly matching source occurrences need a new alias.
        std::map<POVNode,POVNode> reusable;
        if(collectedAlternatives()) {
            auto member=result_->pItem->innr->value->items.head;
            for(const auto& source:published_) {
                if(!member) break;
                reusable[source]=member;member=member->next;
            }
        }
        auto matches=matches_;auto access=access_;bool pending=waiting_;
        Cost created=0;
        if(alternatives) {
            created=collectedAlternatives() ? 0 : 3;
            for(const auto& source:matches) if(!reusable.count(source)) created+=2;
        }
        Work work{matches.size(),created};
        if (!request.context.empty()) ++work.creations;
        return {{"Set field selection result",[this,matches,access,pending,alternatives,reusable] {
            if(!alternatives) {
                // Registry-held aliases can outlive their retired choice.
                for(const auto& entry:reusable) entry.second->pParent=nullptr;
                result_=matches.front();
            } else {
                if(!collectedAlternatives()) {
                    auto item=std::make_shared<infonView>();
                    item->convertThisToIntersection(makeInnrInfonView(),fromGiven);
                    SetModeBits(item->value->flags,infonState::overlayType,infonState::tUnknown);
                    result_=std::make_shared<POV>(914);result_->pItem=item;access->bind(result_);
                }
                auto body=result_->pItem->innr->value;
                body->items.clear();
                for(const auto& entry:reusable) {
                    entry.second->next.reset();entry.second->prev.reset();entry.second->pParent=nullptr;
                }
                for(const auto& source:matches) {
                    auto found=reusable.find(source);
                    POVNode member;
                    if(found!=reusable.end()) {
                        member=found->second;member->pItem=source->pItem;body->items.appendPov(member);
                    } else {
                        member=body->items.append(source->pItem,false,914);
                        access->bind(member);access->bind(source);
                        access->registry->recordIdentity(member,source,iekListCorrespondence,0);
                    }
                    member->pParent=result_.get();
                }
                SetFlagBit(body->flags,infonState::tailUnfinished,pending);
                ++result_->workRevision;
            }
            published_=matches;publishedPending_=pending;verified_=!pending;
            if(!request.context.empty()) {
                auto owner=request.context.front();auto previous=owner->pItem;
                auto state=previous->fieldSelection;
                auto result=std::make_shared<infonView>(*result_->pItem);
                result->fieldSelection=state;result->selectedResult=result_;
                result->originID=previous->originID;result->infCharPos=previous->infCharPos;result->replyCharPos=previous->replyCharPos;
                SetFlagBit(result->flags,infonView::doReply,getFlagBit(previous->flags,infonView::doReply));
                SetFlagBit(result->flags,infonView::isNormed,false);
                state->result=result_;state->matches=matches;state->pending=pending;
                state->resultView=result_->pItem;state->resultRevision=result_->workRevision;
                owner->pItem=result;access->bind(owner);++owner->workRevision;
                if(!alternatives) definitions_->inherit(owner,result_);
            }
            return true;
        },work}};
    }
    bool satisfied() const override {return verified_;}
    bool symbolic() const override {return !verified_ || (request.argument==fieldValue && result_ && result_->pItem->isOrLike());}
    POVNode result() const override {return result_;}
};
