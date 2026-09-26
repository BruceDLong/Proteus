#pragma once

// A path needs the complete candidate set, even when that set is a oneOf.
// A caller asking for the resulting value still sees that value as symbolic.
enum PathResultUse { pathValue = 0, pathCandidates = 1 };

// Field predicates and positional coordinates supply different routes to
// existing members. Both use the same path continuation and result policy.
// An ordered field walk cannot skip an unresolved earlier candidate.
class PathSelection {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::function<Expansion<POVNode>(const Query<POVNode>&)> navigate_;
    static Edge<POVNode> edge(const char* label, Query<POVNode> query, Cost visits=1) {
        return {label, {visits,0}, true, [query] { return query; }};
    }
    static bool unordered(const POVNode& node) {
        return node->pItem->orderMode==uUnordered;
    }
    // Ordinary path walks store [source, embedded owners...]. A walk through
    // a choice stores [source, choice, alternative, embedded owners...] and
    // marks that continuation in coordinates. These are traversal cursors,
    // not additional graph nodes or adjacency storage.
    static std::size_t ownerOffset(const Query<POVNode>& q) {return q.coordinates.empty() ? 1 : 3;}
    void nextAlternative(Expansion<POVNode>& out,const POVNode& choice,const POVNode& current,const POVNode& predicate,int kind) const {
        auto next=current ? current->next : choice->pItem->innr->value->items.head;
        out.edges.push_back(edge("next path alternative",
            {next ? next : choice,next ? pathAlternative : pathAlternativesEnd,kind,predicate,{choice}}));
    }
    void advance(Expansion<POVNode>& out, const Query<POVNode>& q, ListPosition<POVNode> at, Cost visits=1) const {
        auto next=nextListPosition(*access_,std::move(at));
        if (next.phase==ListPhase::cycle) {unresolved(out,q,"unresolved cyclic alternative",visits);return;}
        Query<POVNode> follow{next.phase==ListPhase::done ? q.context.front() : next.node,
            next.phase==ListPhase::done ? pathEnd : fieldPosition,
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
            auto unknown=q;unknown.kind=pathAlternativeUnknown;
            out.edges.push_back(edge(label,std::move(unknown),visits));
        }
    }
    // Index syntax translates to the existing zero-based navigation demand.
    // An inverted literal is a signed coordinate here, rather than a scalar
    // waiting for a size against which to take its complement.
    Expansion<POVNode> position(const Query<POVNode>& q) const {
        Expansion<POVNode> out;
        auto selector=q.related;auto item=selector->pItem;auto value=item->value;
        bool constrained=!POVDescription(selector,access_->registry.get()).relationships().empty() || definitions_->required(selector);
        bool literal=access_->plainStructure(selector,false,false,false,false,false,true) &&
            !getFlagBit(item->flags,infonView::invertMatch) &&
            getModeBits(value->flags,infonState::overlayType)==infonState::NUM &&
            getModeBits(value->flags,infonState::evalMode)==infonState::emLiteral &&
            (!getFlagBit(value->flags,infonState::inverted) || !constrained);
        if(!literal || constrained)
            out.refinements.push_back({{selector,valueOnly,0},{1,0},"resolve path index",false});
        if(!literal) {
            unresolved(out,q,"unresolved path index");return out;
        }
        auto number=value->num;
        if(getFlagBit(value->flags,infonState::inverted)) number.val=-number.val;
        if(number.getDenominator()==0) {access_->numericLiteralError();return out;}
        number.val.canonicalize();
        if(number.getDenominator()!=1 || number==0 || number>INT64_MAX || number < -INT64_MAX) {
            unresolved(out,q,"unsupported path index");return out;
        }
        auto magnitude=number;
        if(number<0) magnitude.val=-magnitude.val;
        std::int64_t index=magnitude.getNumerator().get_ui();
        Cost visits=1;
        if(number<0) {
            if(!access_->navigationList(q.node)) {
                out.refinements.push_back({{q.node,valueOnly,0},{1,0},"resolve indexed list",true});
                unresolved(out,q,"unresolved indexed list");return out;
            }
            auto boundary=listBoundary(*access_,q.node);visits=addCost(visits,boundary.visits);
            for(const auto& missing:boundary.missingExtents)
                out.refinements.push_back({{missing,sizeOnly,0},{boundary.visits,0},"find embedded extent",true});
            if(boundary.extent<0)
                out.refinements.push_back({{q.node,sizeOnly,0},{boundary.visits,0},"find end-relative extent",true});
            if(!boundary.valid || boundary.extent<0) {
                unresolved(out,q,"unknown end-relative position",boundary.visits);return out;
            }
            if(index>boundary.extent) {
                auto end=q;end.kind=pathEnd;out.edges.push_back(edge("position outside list",end,boundary.visits));return out;
            }
            index=boundary.extent-index;
        } else {
            --index;
            if(access_->navigationList(q.node) && access_->listExtent(q.node)>=0 && index>=access_->listExtent(q.node)) {
                auto boundary=listBoundary(*access_,q.node);
                if(!boundary.valid) {unresolved(out,q,"invalid indexed extent",boundary.visits);return out;}
                auto end=q;end.kind=pathEnd;out.edges.push_back(edge("position outside list",end,boundary.visits));return out;
            }
        }
        auto next=wrapIndex({q.node,itemAt,index},q);
        out.edges.push_back(edge("navigate path index",std::move(next),visits));
        return out;
    }
    // Carry a path continuation around navigation, without changing the
    // navigation cursor or teaching the generic graph search about paths.
    static Query<POVNode> wrapIndex(Query<POVNode> navigation,const Query<POVNode>& frame) {
        auto prefix=frame.context;
        if(navigation.kind==itemFound)
            return {navigation.node,pathMatched,static_cast<int>(ListPhase::member),frame.related,prefix,frame.coordinates};
        auto kind=navigation.kind;
        prefix.insert(prefix.end(),navigation.context.begin(),navigation.context.end());
        return {navigation.node,pathIndexStep,navigation.argument,frame.related,prefix,{kind,static_cast<std::int64_t>(frame.context.size())}};
    }
    Expansion<POVNode> indexStep(const Query<POVNode>& q) const {
        auto count=q.coordinates[1];
        Query<POVNode> frame{q.context.front(),positionStart,0,q.related,{q.context.begin(),q.context.begin()+count},count==3 ? std::vector<std::int64_t>{selectPosition} : std::vector<std::int64_t>{}};
        Query<POVNode> navigation{q.node,static_cast<int>(q.coordinates[0]),q.argument,{}, {q.context.begin()+count,q.context.end()}};
        auto out=navigate_(navigation);
        for(auto& route:out.edges) {
            auto follow=route.follow;
            route.follow=[follow,frame] {
                auto arrived=follow();
                return Arrival<POVNode>{wrapIndex(std::move(arrived.query),frame),arrived.graphChanged};
            };
        }
        if(out.edges.empty()) unresolved(out,frame,"unresolved indexed alternative");
        // A dead route is not a missing language alternative. The path goal
        // decides whether another route supplied this same requested position.
        out.deferred=false;
        return out;
    }
public:
    PathSelection(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions,
                  std::function<Expansion<POVNode>(const Query<POVNode>&)> navigate)
        :access_(std::move(access)),definitions_(std::move(definitions)),navigate_(std::move(navigate)) {}
    Expansion<POVNode> expand(const Query<POVNode>& q) const {
        Expansion<POVNode> out;
        if(q.kind==positionStart) return position(q);
        if(q.kind==pathIndexStep) return indexStep(q);
        if(q.kind==pathAlternativesEnd) {
            out.deferred=getFlagBit(q.node->pItem->innr->value->flags,infonState::tailUnfinished);
            return out;
        }
        if(q.kind==pathEnd || q.kind==pathAlternativeUnknown) {
            if(!q.coordinates.empty()) nextAlternative(out,q.context[1],q.context[2],q.related,q.coordinates.front());
            return out;
        }
        if(q.kind==pathAlternative) {
            Query<POVNode> branch{q.node,q.argument==selectPosition ? positionStart : fieldPosition,static_cast<int>(ListPhase::enter),q.related,{q.node,q.context.front(),q.node},{q.argument}};
            out.edges.push_back(edge("select from path alternative",std::move(branch)));
            return out;
        }
        if(q.kind==selectField || q.kind==selectPosition) {
            // A path continuation demands its predecessor's selection, not
            // complete normalization of that intermediate object.
            if (auto field=q.node->pItem->pathSelection) {
                out.refinements.push_back({{field->source,field->positional ? selectPosition : selectField,pathCandidates,field->predicate,{q.node}},
                    {0,0},"select preceding path segment",false});
                if (field->result) {
                    auto next=q;next.node=field->result;
                    out.edges.push_back(edge("continue from selected path",std::move(next)));
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
                nextAlternative(out,q.node,{},q.related,q.kind);return out;
            }
            if(q.kind==selectPosition) {
                auto start=q;start.kind=positionStart;start.context={q.node};
                return position(start);
            }
            if(!sourceReady(q.node)) {out.deferred=true;return out;}
            out.edges.push_back(edge("scan field candidates",{q.node,fieldPosition,static_cast<int>(ListPhase::enter),q.related,{q.node}}));
            return out;
        }
        ListPosition<POVNode> at{q.node,static_cast<ListPhase>(q.argument),{q.context.begin()+ownerOffset(q),q.context.end()}};
        if(q.kind==pathMatched || q.kind==pathUnknown) {
            if(q.kind==pathMatched && !unordered(q.context.front())) {
                if(!q.coordinates.empty()) nextAlternative(out,q.context[1],q.context[2],q.related,q.coordinates.front());
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
                    auto waiting=q;waiting.kind=pathUnknown;
                    out.edges.push_back(edge("unresolved field boundary",std::move(waiting),visits));
                } else unresolved(out,q,"unresolved alternative boundary",visits);
                return out;
            }
        } else if(at.phase==ListPhase::member && !access_->subList(at.node)) {
            FieldPredicate predicate(access_,definitions_,out);
            auto match=predicate.match(q.related,at.node,at.owners);
            visits=addCost(visits,predicate.work.visits);
            if(match==ScalarMatch::yes) {
                auto found=q;found.kind=pathMatched;
                out.edges.push_back(edge("compatible field member",std::move(found),visits));return out;
            }
            if(match==ScalarMatch::unknown) {
                out.deferred=true;
                if(unordered(q.context.front())) {
                    auto waiting=q;waiting.kind=pathUnknown;
                    out.edges.push_back(edge("unresolved field candidate",std::move(waiting),visits));
                } else unresolved(out,q,"unresolved alternative candidate",visits);
                return out;
            }
        }
        advance(out,q,std::move(at),visits);return out;
    }
};

// Field and index segments share continuation and publication. Intermediate
// query POVs retain provenance, while candidate/source POVs remain shared.
// Writes and retained-parent paths need their own continuation semantics.
class PathExpression {
    static std::vector<std::shared_ptr<PartPath>> segments(ProteusGraphAccess& access,const POVNode& node) {
        auto item=node->pItem;
        if (!item->partRef || !item->parts.empty() || !item->wrkList.empty() || item->pathSelection ||
            !access.plainStructure(node,item->isOrLike(),false,false,false,true,false) ||
            !POVDescription(node,access.registry.get()).relationships(false).empty()) return {};
        std::vector<std::shared_ptr<PartPath>> result;
        std::set<std::shared_ptr<PartPath>> seen;
        for (auto part=item->partRef;part;) {
            auto kind=getModeBits(part->flags,PartPath::pathMode);
            if (!seen.insert(part).second || (kind!=PartPath::dotField && kind!=PartPath::idxField) ||
                getFlagBit(part->flags,PartPath::hasSemiColon) || part->spec || !part->path ||
                !part->path->parts.empty() || (kind==PartPath::dotField && !part->path->wrkList.empty())) return {};
            if(kind==PartPath::dotField && part->continuation) return {};
            // The expression inside @[ ] owns its paths. Only the separate
            // continuation after ] belongs to this outer path.
            auto tail=kind==PartPath::dotField ? part->path : part->continuation;
            if(kind==PartPath::idxField && tail && (!tail->parts.empty() || !tail->wrkList.empty() || tail->funcArgs)) return {};
            result.push_back(part);part=tail ? tail->partRef : nullptr;
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
            bool positional=getModeBits(path[i]->flags,PartPath::pathMode)==PartPath::idxField;
            if(!positional) predicate->pItem->partRef.reset();
            auto state=std::make_shared<GraphPathSelection>();
            state->positional=positional;state->expression=expression;state->source=source;state->predicate=predicate;
            auto result=std::make_shared<infonView>();
            SetModeBits(result->flags,infonView::itmMode,infonView::mItem);
            SetModeBits(result->flags,infonView::viewMode,infonView::vmOverlay);
            SetFlagBit(result->flags,infonView::doReply,owner==node && getFlagBit(original->flags,infonView::doReply));
            SetModeBits(result->infSize->flags,infonState::overlayType,infonState::NUM);
            SetModeBits(result->infSize->flags,infonState::evalMode,infonState::emUnknown);
            SetModeBits(result->value->flags,infonState::overlayType,infonState::tUnknown);
            SetModeBits(result->value->flags,infonState::evalMode,infonState::emUnknown);
            result->originID=original->originID;result->infCharPos=original->infCharPos;result->replyCharPos=original->replyCharPos;
            result->pathSelection=state;owner->pItem=result;
            access.bind(predicate);access.bind(owner);++owner->workRevision;
            source=owner;
        }
        return true;
    }
};

class ProteusPathGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::vector<POVNode> matches_,published_;
    POVNode result_;
    bool verified_=false,waiting_=false,publishedPending_=false,aggregating_=false;
    bool branchMatched_=false;
    std::vector<POVNode> positionBranches_;
    std::map<POVNode,POVNode> positionWinners_;
    bool positionsEnded_=false,positionTailPending_=false,positionReady_=false;
    Visit<POVNode> positionFinding_;
    bool collectPosition(const Query<POVNode>& q) {
        if(q.kind==pathAlternative) {aggregating_=true;positionBranches_.push_back(q.node);}
        if(q.kind==pathMatched) {
            // Search visits routes in cost order. Keep one winner for each
            // requested position, independently of the other oneOf branches.
            positionWinners_.emplace(q.context.front(),q.node);
            if(!aggregating_ && matches_.empty()) matches_.push_back(q.node);
        }
        if(q.kind==pathAlternativesEnd) {
            positionsEnded_=true;
            positionTailPending_=getFlagBit(q.node->pItem->innr->value->flags,infonState::tailUnfinished);
        }
        if(aggregating_ || positionsEnded_) {
            matches_.clear();waiting_=positionTailPending_;
            for(const auto& branch:positionBranches_) {
                auto winner=positionWinners_.find(branch);
                if(winner==positionWinners_.end()) waiting_=true;
                else if(std::find(matches_.begin(),matches_.end(),winner->second)==matches_.end()) matches_.push_back(winner->second);
            }
            return positionsEnded_;
        }
        return q.kind==pathMatched || q.kind==pathEnd;
    }
    bool collectedAlternatives() const {return result_ && (publishedPending_ || published_.size()!=1);}
public:
    ProteusPathGoal(Query<POVNode> query,std::shared_ptr<ProteusGraphAccess> access,
                     std::shared_ptr<ProteusDefinitions> definitions)
        :GoalSpec<POVNode>(query),access_(std::move(access)),definitions_(std::move(definitions)) {
        if(!request.context.empty()) {
            auto state=request.context.front()->pItem->pathSelection;
            if(state) {result_=state->result;published_=state->matches;publishedPending_=state->pending;}
        }
    }
    void beginPass() override {
        matches_.clear();verified_=waiting_=aggregating_=false;branchMatched_=false;
        positionBranches_.clear();positionWinners_.clear();positionsEnded_=positionTailPending_=positionReady_=false;
        positionFinding_={request,{},0};
    }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        auto q=visit.query;
        bool finished=false;
        if(request.kind==selectPosition) {
            auto winners=positionWinners_.size();
            positionReady_=collectPosition(q) || positionReady_;
            if((positionWinners_.size()!=winners || q.kind==pathAlternativesEnd || q.kind==pathEnd) && visit.cost>=positionFinding_.cost)
                positionFinding_=visit;
            return {};
        }
        if(q.kind==pathAlternative) {aggregating_=true;branchMatched_=false;}
        if(q.kind==pathUnknown || q.kind==pathAlternativeUnknown) waiting_=true;
        if(q.kind==pathMatched) {
            branchMatched_=true;
            if(std::find(matches_.begin(),matches_.end(),q.node)==matches_.end()) matches_.push_back(q.node);
        }
        if(q.kind==pathEnd && aggregating_ && !branchMatched_) waiting_=true;
        if(q.kind==pathAlternativesEnd && getFlagBit(q.node->pItem->innr->value->flags,infonState::tailUnfinished)) waiting_=true;
        finished=q.kind==pathAlternativesEnd || (!aggregating_ && (q.kind==pathEnd ||
            (q.kind==pathMatched && q.context.front()->pItem->orderMode!=uUnordered)));
        return finished ? publish() : std::vector<SetCommand>{};
    }
    CommandBatch<POVNode> finishPass() override {
        if(request.kind!=selectPosition || !positionReady_) return {};
        return {positionFinding_,publish()};
    }
private:
    std::vector<SetCommand> publish() {
        const bool alternatives=waiting_ || matches_.size()!=1;
        if(result_ && published_==matches_ && publishedPending_==waiting_) {
            bool current=true;
            if(!alternatives && !request.context.empty()) {
                auto state=request.context.front()->pItem->pathSelection;
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
        return {{"Set path selection result",[this,matches,access,pending,alternatives,reusable] {
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
                auto state=previous->pathSelection;
                auto result=std::make_shared<infonView>(*result_->pItem);
                result->pathSelection=state;result->selectedResult=result_;
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
public:
    bool satisfied() const override {return verified_ || (request.kind==selectPosition && positionReady_ && !waiting_);}
    bool symbolic() const override {return !verified_ || (request.argument==pathValue && result_ && result_->pItem->isOrLike());}
    POVNode result() const override {return result_;}
};
