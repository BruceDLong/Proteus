#pragma once
#include "SliceSelection.h"
#include "SignedCoordinate.h"
#include "UnitSelection.h"

// A path needs the complete candidate set, even when that set is a oneOf.
// A caller asking for the resulting value still sees that value as symbolic.
enum PathResultUse { pathValue = 0, pathCandidates = 1 };
inline Query<POVNode> pathRequest(const POVNode& owner,int use=pathValue) {
    auto state=readView(owner)->pathSelection;
    return {state->source,state->environmentCursor ? selectEnvironment : state->alternatives ? filterChoice : state->identity ? selectSelf : state->external ? externalField : state->resolution ? resolveWord : state->positional ? selectPosition : selectField,use,
            state->identity || state->positional ? owner : state->predicate,{owner}};
}

// Field predicates and positional coordinates supply different routes to
// existing members. Both use the same path continuation and result policy.
// An ordered field walk cannot skip an unresolved earlier candidate.
class PathSelection {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::function<Expansion<POVNode>(const Query<POVNode>&)> navigate_;
    std::shared_ptr<FieldCandidates> candidates_;
    static Edge<POVNode> edge(const char* label, Query<POVNode> query, Cost visits=1) {
        Edge<POVNode> result{label, {visits,0}, true, [query] { return query; }};
        result.destination=query;return result;
    }
    static bool unordered(const POVNode& node) {
        return readView(node)->orderMode==uUnordered;
    }
    // Ordinary path walks store [source, embedded owners...]. A walk through
    // a choice stores [source, choice, alternative, embedded owners...] and
    // marks that continuation in coordinates. These are traversal cursors,
    // not additional graph nodes or adjacency storage.
    static std::size_t ownerOffset(const Query<POVNode>& q) {return q.coordinates.empty() ? 1 : 3;}
    void nextAlternative(Expansion<POVNode>& out,const POVNode& choice,const POVNode& current,const POVNode& predicate,int kind) const {
        auto next=current ? current->next : readView(choice)->innr->value->items.head;
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
    bool sourceReady(const POVNode& node,Expansion<POVNode>& out,Cost& visits) const {
        ProteusPreservationGoal::contribute(*access_,node,{},out,false);
        if(auto state=readView(node)->pathSelection) {
            if(!state->updates.empty())
                out.refinements.push_back({{node,relationshipConstraints,0},{1,0},"apply source path constraints",false});
            for(const auto& effect:state->effects)
                out.refinements.push_back({{effect,sizeAndValue,0},{1,0},"complete source path effect",false});
        }
        bool ready=(access_->physicalList(node) || access_->unreferencedTemplateSpan(node)) && !definitions_->pendingOrder(node);
        const bool hasStructure=ready;
        if(ready) for (const auto& relation:POVDescription(node,access_->registry.get()).relationships()) {
            if(access_->sameInfon(node,relation.target)) continue;
            Work work{0,0};bool represented=RHSRepresentation::expressed(access_,node,relation.target,definitions_,work);
            visits=addCost(visits,work.visits);
            ready=ready && represented;
        }
        // A source may acquire its list structure through a definition or RHS.
        // Keep that opportunity visible without requiring unrelated fields to
        // finish before a usable member can be selected. Existing relationships
        // remain valid evidence once their facts are represented in the source.
        if(!ready) out.refinements.push_back({{node,hasStructure ? sizeAndValue : valueOnly,hasStructure ? lazyParts : exposeStructure},{visits,0},"expose field source structure",true});
        return ready;
    }
    void unresolved(Expansion<POVNode>& out,const Query<POVNode>& q,const char* label,Cost visits=1) const {
        out.deferred=true;
        if(!q.coordinates.empty()) {
            auto unknown=q;unknown.kind=pathAlternativeUnknown;
            out.edges.push_back(edge(label,std::move(unknown),visits));
        }
    }
    // Index syntax translates to the existing zero-based navigation demand.
    // Numeric coordinates carry their sign in the rational payload; their
    // source-list bounds are resolved by navigation, not scalar wraparound.
    int orderedEndpoint(const POVNode& source,const POVNode& predicate) const {
        if(!access_->navigationList(source) || !access_->plain(predicate,false,false,false,false) || !readView(predicate)->type ||
           !readView(predicate)->addedTypes.empty() || !POVDescription(predicate,access_->registry.get()).relationships().empty()) return 0;
        auto facts=access_->typeFacts(predicate);
        if(!facts.valid || facts.hasSize || facts.hasValue || facts.overlay>=0) return 0;
        auto word=readView(predicate)->type->typeID();
        return word=="first" ? 1 : word=="last" ? -1 : 0;
    }
    Expansion<POVNode> position(const Query<POVNode>& q,int endpoint=0) const {
        if(!endpoint) {
            auto units=UnitSelection(access_,definitions_).expand(q);
            if(units.handled) return units.expansion;
        }
        Expansion<POVNode> out;
        auto state=readView(q.related)->pathSelection;
        auto selector=endpoint ? q.related : state->predicate;
        auto spec=endpoint ? POVNode{} : state->spec;
        auto item=readView(selector);auto value=item->value;
        bool constrained=!endpoint && (!POVDescription(selector,access_->registry.get()).relationships().empty() || definitions_->required(selector));
        FlexNum number;Cost coordinateVisits=0;
        bool literal=access_->plainStructure(selector,false,false,false,false,false,true) &&
            !getFlagBit(item->flags,infonView::invertMatch) &&
            signedCoordinate(*access_,value,number,coordinateVisits) &&
            (!getFlagBit(value->flags,infonState::inverted) || !constrained);
        if(endpoint) {literal=true;number.val=endpoint;coordinateVisits=1;}
        if(!literal || constrained)
            out.refinements.push_back({{selector,valueOnly,0},{1,0},"resolve path index",false});
        if(!literal) {
            unresolved(out,q,"unresolved path index");return out;
        }
        if(number.getDenominator()==0) throw std::runtime_error("numeric literal has zero denominator");
        number.val.canonicalize();
        if(number.getDenominator()!=1 || (!spec && number==0) || number>INT64_MAX || number < -INT64_MAX) {
            unresolved(out,q,"unsupported path index");return out;
        }
        auto magnitude=number;
        if(number<0) magnitude.val=-magnitude.val;
        std::int64_t index=magnitude.getNumerator().get_ui();
        Cost visits=coordinateVisits;
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
            if(!spec) --index;
            if(access_->navigationList(q.node) && access_->listExtent(q.node)>=0 && (index>access_->listExtent(q.node) || ((!spec || access_->scalarCandidate(spec)) && index==access_->listExtent(q.node)))) {
                auto boundary=listBoundary(*access_,q.node);
                if(!boundary.valid) {unresolved(out,q,"invalid indexed extent",boundary.visits);return out;}
                auto end=q;end.kind=pathEnd;out.edges.push_back(edge("position outside list",end,boundary.visits));return out;
            }
        }
        if(spec && access_->literalList(spec)) {
            auto slice=q;slice.kind=pathSliceStart;slice.argument=index;
            out.edges.push_back(edge("select span window",slice,visits));return out;
        }
        if(spec && !access_->scalarCandidate(spec)) {
            out.refinements.push_back({{spec,valueOnly,0},{1,0},"resolve slice specification",true});
            unresolved(out,q,"unknown slice specification");return out;
        }
        auto next=wrapIndex({q.node,itemAt,index},q);
        out.edges.push_back(edge("navigate path index",std::move(next),visits));
        return out;
    }
    // Carry a path continuation around navigation, without changing the
    // navigation cursor or teaching the generic graph search about paths.
    static Query<POVNode> wrapIndex(Query<POVNode> navigation,const Query<POVNode>& frame) {
        auto prefix=frame.context;
        prefix.insert(prefix.end(),navigation.context.begin(),navigation.context.end());
        if(navigation.kind==itemFound)
            return {navigation.node,readView(frame.related)->pathSelection && readView(frame.related)->pathSelection->spec ? pathSliceCandidate : pathMatched,static_cast<int>(ListPhase::member),frame.related,prefix,frame.coordinates};
        auto kind=navigation.kind;
        return {navigation.node,pathIndexStep,navigation.argument,frame.related,prefix,{kind,static_cast<std::int64_t>(frame.context.size()),frame.coordinates.empty() ? 0 : frame.coordinates.front()}};
    }
    Expansion<POVNode> indexStep(const Query<POVNode>& q) const {
        auto count=q.coordinates[1];
        Query<POVNode> frame{q.context.front(),positionStart,0,q.related,{q.context.begin(),q.context.begin()+count},count==3 ? std::vector<std::int64_t>{q.coordinates[2]} : std::vector<std::int64_t>{}};
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
    Expansion<POVNode> choice(const Query<POVNode>& q) const {
        Expansion<POVNode> out;
        if(q.kind>=choiceMatched) return out;
        if(q.kind==filterChoice) {
            // A provisional choice is still a live path query. Refresh its
            // producer before filtering the current alternatives, just as a
            // chained field/index access refreshes its preceding segment.
            if(auto field=readView(q.node)->pathSelection) {
                out.refinements.push_back({pathRequest(q.node,pathCandidates),{0,0},"refresh choice source",false});
                if(!field->updates.empty())
                    out.refinements.push_back({{q.node,relationshipConstraints,0},{1,0},"apply choice source constraints",false});
                if(field->result) {
                    auto next=q;next.node=field->result;
                    for(const auto& location:field->locations) if(location.source==field->result && !location.context.empty()) {
                        next.context.resize(1);
                        next.context.insert(next.context.end(),location.context.begin()+1,location.context.end());
                        break;
                    }
                    out.edges.push_back(edge("filter selected path alternatives",std::move(next)));
                }
                return out;
            }
            // A formerly provisional choice can collapse to one concrete
            // value. Filter that value itself, not its record/list members.
            if(!readView(q.node)->isOrLike()) {
                FieldPredicate predicate(access_,definitions_,out);
                auto match=predicate.match(q.related,q.node,{q.context.begin()+1,q.context.end()});
                if(match!=ScalarMatch::no) {
                    auto found=q;found.kind=match==ScalarMatch::yes ? choiceMatched : choiceUnknown;
                    out.edges.push_back(edge("compatible resolved choice",std::move(found),predicate.work.visits));
                }
                auto end=q;end.kind=choiceEnd;
                out.edges.push_back(edge("resolved choice boundary",std::move(end)));
                return out;
            }
            auto body=readView(q.node)->innr;
            if(!access_->plain(q.node,true,false,false,false) || !access_->literalSelectionBody(body) ||
               body->partRef || !body->wrkList.empty() || !POVDescription(q.node,access_->registry.get()).relationships().empty()) {
                out.deferred=true;return out;
            }
            out.edges.push_back(edge("inspect choice members",{definitions_->selectionBody(q.node),choicePosition,
                static_cast<int>(ListPhase::enter),q.related,{q.node}}));return out;
        }
        ListPosition<POVNode> at{q.node,static_cast<ListPhase>(q.argument),{q.context.begin()+1,q.context.end()}};
        Cost visits=1;
        auto pending=[&] {
            auto missing=q;missing.kind=choicePending;
            out.edges.push_back(edge("unresolved choice information",std::move(missing)));
        };
        if(at.phase==ListPhase::enter && !sourceReady(at.node,out,visits)) {pending();out.deferred=true;return out;}
        if(at.phase==ListPhase::leave) {
            auto boundary=listBoundary(*access_,at.node);visits=addCost(visits,boundary.visits);
            if(!boundary.valid || boundary.tail!=0 || !boundary.missingExtents.empty()) pending();
        } else if(at.phase==ListPhase::member && !access_->subList(at.node)) {
            FieldPredicate predicate(access_,definitions_,out);
            auto match=predicate.match(q.related,at.node,at.owners);
            visits=addCost(visits,predicate.work.visits);
            if(match!=ScalarMatch::no) {
                auto found=q;found.kind=match==ScalarMatch::yes ? choiceMatched : choiceUnknown;
                out.edges.push_back(edge("compatible choice member",std::move(found),visits));
            }
        }
        auto next=nextListPosition(*access_,std::move(at));
        if(next.phase==ListPhase::cycle) {pending();out.deferred=true;return out;}
        Query<POVNode> follow{next.phase==ListPhase::done ? q.context.front() : next.node,
            next.phase==ListPhase::done ? choiceEnd : choicePosition,static_cast<int>(next.phase),q.related,{q.context.front()}};
        follow.context.insert(follow.context.end(),next.owners.begin(),next.owners.end());
        out.edges.push_back(edge("next choice position",std::move(follow)));return out;
    }
public:
    PathSelection(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions,
                  std::function<Expansion<POVNode>(const Query<POVNode>&)> navigate,std::shared_ptr<FieldCandidates> candidates)
        :access_(std::move(access)),definitions_(std::move(definitions)),navigate_(std::move(navigate)),candidates_(std::move(candidates)) {}
    Expansion<POVNode> expand(const Query<POVNode>& q) const {
        Expansion<POVNode> out;
        if(q.kind==pathMatched && (BoundaryGraph::direction(q.related) || TemporalGraph::pathKind(q.related))) return out;
        if(q.kind>=filterChoice) return choice(q);
        if(q.kind==selectSelf) {
            out.edges.push_back(edge("retain path source",{q.node,pathSelfFound,0,q.related,{q.node}}));return out;
        }
        if(q.kind==pathSelfFound) return out;
        if(q.kind==positionStart) return position(q);
        if(q.kind==pathSliceStart || q.kind==pathSliceCandidate) {
            auto result=SliceSelection(access_,definitions_).expand(q);
            if(result.deferred) unresolved(result,q,"unresolved slice alternative");
            result.deferred=false;
            return result;
        }
        if(q.kind==pathIndexStep) return indexStep(q);
        if(q.kind==pathAlternativesEnd) {
            out.deferred=getFlagBit(readView(q.node)->innr->value->flags,infonState::tailUnfinished);
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
            if (auto field=readView(q.node)->pathSelection) {
                out.refinements.push_back({pathRequest(q.node,pathCandidates),{0,0},"select preceding path segment",false});
                if(!field->updates.empty())
                    out.refinements.push_back({{q.node,relationshipConstraints,0},{1,0},"apply preceding path constraints",false});
                if (field->result) {
                    for(const auto& location:field->locations)
                        ProteusPreservationGoal::contribute(*access_,location.source,location.context,out,false);
                    if(q.kind==selectField && BoundaryGraph::direction(q.related))
                        for(const auto& location:field->locations) if(location.source==field->result)
                            for(const auto& owner:location.context) if(owner!=field->result && BoundaryGraph::temporal(owner))
                                out.refinements.push_back({{owner,boundaryRelations,0},{1,0},"boundary in selected occurrence context",false});
                    auto next=q;next.node=field->result;
                    out.edges.push_back(edge("continue from selected path",std::move(next)));
                }
                return out;
            }
            if(q.kind==selectField) if(auto kind=TemporalGraph::pathKind(q.related)) {
                TemporalGraph temporal(access_);auto owner=q.context.front();
                out.refinements.push_back({{q.node,kind,0,owner},{1,0},"find temporal reference",false});
                auto ref=kind==temporalCut ? temporal.split(owner) : nullptr;
                auto selected=kind==temporalSupport ? temporal.support(q.node) : ref ? ref->symbolicCut : POVNode{};
                if(selected) out.edges.push_back(edge("selected temporal reference",{selected,pathMatched,0,q.related,{q.node}}));
                else out.deferred=true;
                return out;
            }
            if(q.kind==selectField) if(auto direction=BoundaryGraph::direction(q.related)) {
                BoundaryGraph boundaries(access_);
                out.refinements.push_back({{q.node,boundaryAccess,direction},{1,0},"find infon boundary",false});
                if(auto boundary=boundaries.lookup(q.node,direction>0))
                    out.edges.push_back(edge("selected infon boundary",{boundary,pathMatched,0,q.related,{q.node}}));
                return out;
            }
            if(readView(q.node)->isOrLike()) {
                auto body=readView(q.node)->innr;
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
            if(auto endpoint=orderedEndpoint(q.node,q.related)) {
                auto start=q;start.context={q.node};return position(start,endpoint);
            }
            auto units=UnitSelection(access_,definitions_).expand(q);
            if(units.handled) return units.expansion;
            out.edges.push_back(edge("scan field candidates",{q.node,fieldPosition,static_cast<int>(ListPhase::enter),q.related,{q.node}}));
            return out;
        }
        ListPosition<POVNode> at{q.node,static_cast<ListPhase>(q.argument),{q.context.begin()+ownerOffset(q),q.context.end()}};
        if(q.kind==pathMatched || q.kind==pathUnknown) {
            if(q.kind==pathMatched && !unordered(q.context.front())) {
                if(!q.coordinates.empty()) nextAlternative(out,q.context[1],q.context[2],q.related,q.coordinates.front());
                return out;
            }
            if(!unordered(q.context.front())) advance(out,q,std::move(at));
            return out;
        }
        Cost visits=1;
        if(at.phase==ListPhase::enter) {
            if(q.node==q.context.front())
                if(auto endpoint=orderedEndpoint(q.node,q.related)) return position(q,endpoint);
            if(!sourceReady(at.node,out,visits) || (!unordered(q.context.front()) && unordered(at.node))) {
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
                    advance(out,q,std::move(at),visits);
                } else unresolved(out,q,"unresolved alternative boundary",visits);
                return out;
            }
        } else if(at.phase==ListPhase::member && !access_->subList(at.node)) {
            if(access_->isDefinition(at.node)) {
                out.refinements.push_back({{at.node,sizeAndValue,0},{1,0},"publish encountered definition",false});
                advance(out,q,std::move(at),visits);return out;
            }
            auto match=candidates_->match(q.related,at.node,at.owners,out,visits);
            if(match==ScalarMatch::yes) {
                auto found=q;found.kind=pathMatched;
                out.edges.push_back(edge("compatible field member",std::move(found),visits));
                if(unordered(q.context.front())) advance(out,q,std::move(at),visits);
                return out;
            }
            if(match==ScalarMatch::unknown) {
                out.deferred=true;
                if(unordered(q.context.front())) {
                    auto waiting=q;waiting.kind=pathUnknown;
                    out.edges.push_back(edge("unresolved field candidate",std::move(waiting),visits));
                } else unresolved(out,q,"unresolved alternative candidate",visits);
                if(unordered(q.context.front())) advance(out,q,std::move(at),visits);
                return out;
            }
        }
        advance(out,q,std::move(at),visits);return out;
    }
};

// Parse structure owns continuation: retained parts are obligations on the
// parent, while partRef returns a descendant. All selections use one walk.
class PathExpression {
    using Infon=std::shared_ptr<infonView>;
    using Part=std::shared_ptr<PartPath>;
    static Infon tail(const Part& part) {
        return getModeBits(part->flags,PartPath::pathMode)==PartPath::idxField ? part->continuation : part->path;
    }
    static bool inspect(const Infon& syntax,std::set<Part>& seen,Cost& count) {
        if(!syntax) return true;
        auto parts=syntax->parts;if(syntax->partRef) parts.push_back(syntax->partRef);
        for(const auto& part:parts) {
            auto kind=getModeBits(part->flags,PartPath::pathMode);
            if(!seen.insert(part).second || !part->path || (kind!=PartPath::dotField && kind!=PartPath::idxField && kind!=PartPath::resolutionField && kind!=PartPath::upField && kind!=PartPath::asTListField) ||
               (kind!=PartPath::idxField && (part->spec || part->continuation))) return false;
            auto continuation=tail(part);
            if(continuation && continuation->funcArgs) return false;
            ++count;if(!inspect(continuation,seen,count)) return false;
        }
        return true;
    }
    static bool choiceReady(ProteusGraphAccess& access,const POVNode& node) {
        const auto& item=readView(node);
        if(item->pathSelection || item->wrkList.size()!=1 || !access.plain(node,item->isOrLike(),false,false,false) ||
           !POVDescription(node,access.registry.get()).relationships(false).empty()) return false;
        auto rhs=item->wrkList.front();
        if(!access.plain(rhs,readView(rhs)->isOrLike(),false,false,false) || !POVDescription(rhs,access.registry.get()).relationships().empty()) return false;
        return item->isOrLike()!=readView(rhs)->isOrLike();
    }
    static bool environmentCursor(const Infon& item) {
        return getModeBits(item->flags,infonView::viewMode)==infonView::vmCursor &&
               (getModeBits(item->flags,infonView::cursorMode)==infonView::cmSelf ||
                getModeBits(item->flags,infonView::cursorMode)==infonView::cmContext ||
                getModeBits(item->flags,infonView::cursorMode)==infonView::cmWorld);
    }
    static Cost count(ProteusGraphAccess& access,const POVNode& node) {
        auto item=readView(node);
        bool contextual=environmentCursor(item);
        if((!contextual && !item->partRef && item->parts.empty()) || !item->wrkList.empty() || item->pathSelection ||
           !access.plainStructure(node,item->isOrLike(),false,true,contextual,true,false) ||
           !POVDescription(node,access.registry.get()).relationships(false).empty()) return 0;
        Cost result=contextual ? 1 : 0;std::set<Part> seen;
        return inspect(item,seen,result) ? result : 0;
    }
    static void initialize(ProteusGraphAccess& access,const POVNode& owner,const POVNode& source,
                           const POVNode& predicate,const POVNode& spec,bool positional,bool identity,const Infon& expression={},Changes* changes=nullptr) {
        auto result=readView(owner)=std::make_shared<infonView>();
        SetModeBits(result->flags,infonView::itmMode,infonView::mItem);
        SetModeBits(result->flags,infonView::viewMode,infonView::vmOverlay);
        SetModeBits(result->infSize->flags,infonState::overlayType,infonState::NUM);
        SetModeBits(result->infSize->flags,infonState::evalMode,infonState::emUnknown);
        SetModeBits(result->value->flags,infonState::overlayType,infonState::tUnknown);
        SetModeBits(result->value->flags,infonState::evalMode,infonState::emUnknown);
        auto state=result->pathSelection=std::make_shared<GraphPathSelection>();
        state->positional=positional;state->identity=identity;state->source=source;state->predicate=predicate;state->spec=spec;state->expression=expression;
        if(expression) {result->originID=expression->originID;result->infCharPos=expression->infCharPos;result->replyCharPos=expression->replyCharPos;}
        for(const auto& node:{source,predicate,spec,owner})
            if(node && access.bind(node) && changes) *changes=Changes{};
        ++owner->workRevision;
    }
    static POVNode lower(ProteusGraphAccess& access,const POVNode& source,const Infon& syntax,
                         const POVNode& destination,const Infon& expression,std::vector<POVNode>& effects,Changes& changes) {
        if(!syntax) return source;
        auto step=[&](const Part& part,const POVNode& output) {
            auto continuation=tail(part);
            auto owner=continuation && continuation->partRef ? std::make_shared<POV>(915) : output;
            bool positional=getModeBits(part->flags,PartPath::pathMode)==PartPath::idxField;
            auto predicate=std::make_shared<POV>(915);readView(predicate)=std::make_shared<infonView>(*part->path);
            if(!positional) {readView(predicate)->partRef.reset();readView(predicate)->parts.clear();readView(predicate)->wrkList.clear();}
            POVNode spec;
            if(part->spec) {spec=std::make_shared<POV>(916);readView(spec)=part->spec;}
            initialize(access,owner,source,predicate,spec,positional,false,expression,&changes);
            readView(owner)->pathSelection->external=getModeBits(part->flags,PartPath::pathMode)==PartPath::upField;
            readView(owner)->pathSelection->resolution=getModeBits(part->flags,PartPath::pathMode)==PartPath::resolutionField;
            if(continuation) readView(owner)->pathSelection->updates=continuation->wrkList;
            if(continuation && !continuation->wrkList.empty()) effects.push_back(owner);
            return lower(access,owner,continuation,output,expression,effects,changes);
        };
        for(const auto& part:syntax->parts) effects.push_back(step(part,std::make_shared<POV>(915)));
        return syntax->partRef ? step(syntax->partRef,destination) : source;
    }
public:
    // Filtering an asserted value changes the value, not the declaration of
    // its containing field. Ordinary navigation still takes its source type.
    static void retainChoiceDeclaration(const Infon& result,const Infon& declaration) {
        if(!declaration) return;
        auto add=[&](const std::shared_ptr<WordUse>& type) {
            if(!type || (result->type && result->type->typeID()==type->typeID())) return;
            for(const auto& old:result->addedTypes) if(old->typeID()==type->typeID()) return;
            result->addedTypes.push_back(type);
        };
        if(declaration->type) {
            auto selected=result->type;result->type=declaration->type;add(selected);
        }
        for(const auto& type:declaration->addedTypes) add(type);
        if(getFlagBit(declaration->flags,infonView::independentTemporalValue))
            SetFlagBit(result->flags,infonView::independentTemporalValue,true);
    }
    static POVNode positionQuery(ProteusGraphAccess& access,const POVNode& source,const POVNode& predicate,const POVNode& spec) {
        auto owner=std::make_shared<POV>(915);initialize(access,owner,source,predicate,spec,true,false);return owner;
    }
    static bool ready(ProteusGraphAccess& access,const POVNode& node) {return choiceReady(access,node) || count(access,node)!=0;}
    static Work work(ProteusGraphAccess& access,const POVNode& node) {auto n=count(access,node);return {n+1,8*n+6};}
    static SetResult prepare(ProteusGraphAccess& access,const POVNode& node) {
        auto changes=ReadScope::changedObjects({node.get()});
        auto before=POVDescription::positionSnapshot(node);
        auto published=[&] {
            POVDescription::includePositionChange(changes,node,before);
            return SetResult{true,changes};
        };
        if(!ready(access,node)) return false;
        auto original=readView(node);
        if(choiceReady(access,node)) {
            auto left=node->snapshotView(915);readView(left)=std::make_shared<infonView>(*original);readView(left)->wrkList.clear();
            auto right=original->wrkList.front();bool leftChoice=readView(left)->isOrLike();
            initialize(access,node,leftChoice ? left : right,leftChoice ? right : left,{},false,false,original,&changes);
            readView(node)->pathSelection->alternatives=true;
            retainChoiceDeclaration(readView(node),original);
            SetFlagBit(readView(node)->flags,infonView::doReply,getFlagBit(original->flags,infonView::doReply));
            return published();
        }
        auto expression=std::make_shared<infonView>(*original);SetFlagBit(expression->flags,infonView::doReply,false);
        auto source=node->snapshotView(915);readView(source)=std::make_shared<infonView>(*original);
        readView(source)->partRef.reset();readView(source)->parts.clear();SetFlagBit(readView(source)->flags,infonView::doReply,false);if(access.bind(source)) changes=Changes{};
        if(environmentCursor(original)) {
            auto cursor=original->partRef || !original->parts.empty() ? std::make_shared<POV>(915) : node;
            initialize(access,cursor,source,{},{},false,true,expression,&changes);
            readView(cursor)->pathSelection->environmentCursor=getModeBits(original->flags,infonView::cursorMode);
            if(cursor==node) {
                SetFlagBit(readView(node)->flags,infonView::doReply,getFlagBit(original->flags,infonView::doReply));
                return published();
            }
            source=cursor;
        }
        std::vector<POVNode> effects;
        auto returnedSyntax=expression;
        while(returnedSyntax && returnedSyntax->partRef) returnedSyntax=tail(returnedSyntax->partRef);
        // A returned parent owns its retained children, whose source is that
        // parent's selection query. Keep the outer view separate so these
        // references form a DAG instead of parent -> child -> parent.
        bool retainedParent=expression->partRef && returnedSyntax && !returnedSyntax->parts.empty();
        auto destination=retainedParent ? std::make_shared<POV>(915) : node;
        auto returned=lower(access,source,expression,destination,expression,effects,changes);
        if(returned!=node) initialize(access,node,returned,{},{},false,true,expression,&changes);
        auto state=readView(node)->pathSelection;
        std::set<POVNode> seen;
        for(const auto& effect:effects) if(effect!=node && seen.insert(effect).second) state->effects.push_back(effect);
        SetFlagBit(readView(node)->flags,infonView::doReply,getFlagBit(original->flags,infonView::doReply));
        return published();
    }
    static std::vector<POVNode> choiceConstraints(const POVNode& node) {
        std::vector<POVNode> result;auto state=readView(node)->pathSelection;
        if(!state || !state->alternatives) return result;
        for(const auto& rhs:readView(node)->wrkList)
            if(!state->resultView || !state->resultView->workListContainsPOV(rhs)) result.push_back(rhs);
        return result;
    }
    static bool constrainChoice(ProteusGraphAccess& access,const POVNode& node,const std::vector<POVNode>& constraints) {
        if(constraints.empty()) return false;
        auto predicate=readView(node)->pathSelection->predicate;
        for(const auto& rhs:constraints) {
            access.bind(rhs);readView(predicate)->addPOVToWrkList(rhs);
            auto& work=readView(node)->wrkList;work.erase(std::remove(work.begin(),work.end(),rhs),work.end());
        }
        ++predicate->workRevision;++node->workRevision;return true;
    }
    static std::vector<POVNode> location(const std::shared_ptr<GraphPathSelection>& state,const POVNode& source) {
        for(const auto& at:state->locations) if(at.source==source) return at.context;
        return {};
    }
    static POVNode updateTarget(const std::shared_ptr<GraphPathSelection>& state) {
        if(!state->result || state->matches.size()!=1 ||
           (state->pending && !readView(state->result)->implicitMembershipChoice)) return {};
        return state->matches.front();
    }
    static bool updatesReady(const std::shared_ptr<GraphPathSelection>& state) {
        return state->updates.empty() || bool(updateTarget(state));
    }
    static bool needsUpdates(const std::shared_ptr<GraphPathSelection>& state) {
        if(!updatesReady(state) || state->updates.empty()) return false;
        for(const auto& rhs:state->updates) if(!readView(updateTarget(state))->hasRHS(readView(rhs))) return true;
        return false;
    }
    static bool applyUpdates(ProteusGraphAccess& access,const std::shared_ptr<GraphPathSelection>& state) {
        if(!needsUpdates(state)) return false;
        auto target=updateTarget(state);access.bind(target);
        for(const auto& rhs:state->updates) {access.bind(rhs);readView(target)->addPOVToWrkList(rhs);}
        ++target->workRevision;return true;
    }
};

class ProteusPathGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::function<Expansion<POVNode>(const Query<POVNode>&)> expand_;
    FieldCandidates::Answers answers_;
    std::int64_t candidateEpoch_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::vector<POVNode> matches_,published_;
    POVNode result_;
    std::map<POVNode,std::vector<POVNode>> locations_;
    bool verified_=false,waiting_=false,publishedPending_=false,aggregating_=false;
    bool branchMatched_=false,uncertainCandidates_=false;
    std::vector<POVNode> positionBranches_;
    std::map<POVNode,POVNode> positionWinners_;
    std::set<POVNode> positionEnds_;
    bool positionsEnded_=false,positionTailPending_=false,positionTailUncertain_=false,positionReady_=false;
    Visit<POVNode> positionFinding_;
    using Q=Query<POVNode>;
    struct Contribution {
        POVNode member,branch;
        std::vector<POVNode> location;
        std::shared_ptr<infonView> view;
        std::uint64_t revision=0;
        bool pending=false,uncertain=false,terminal=false,branchEnd=false;
        bool operator==(const Contribution& b) const {
            return std::tie(member,branch,location,view,revision,pending,uncertain,terminal,branchEnd)==
                   std::tie(b.member,b.branch,b.location,b.view,b.revision,b.pending,b.uncertain,b.terminal,b.branchEnd);
        }
    };
    std::unique_ptr<PersistentTraversal<POVNode>> traversal_;
    std::function<void()> resume_;
    std::map<Q,Contribution> contributions_;
    std::map<POVNode,std::map<Q,std::vector<POVNode>>> memberProofs_;
    struct Branch {std::size_t matches=0,ends=0;};
    std::map<POVNode,Branch> branches_;
    std::size_t pendingCount_=0,uncertainCount_=0,terminalCount_=0,emptyBranches_=0;
    bool resultChanged_=true;
    std::map<POVNode,std::size_t> sourceOrder_;
    std::size_t nextSourceOrder_=0;
    Contribution describe(const Q& q) {
        Contribution c;
        if(q.kind==pathMatched) {
            c.member=q.node;c.view=readView(q.node);c.revision=q.node->workRevision;
            c.location={q.node};auto offset=q.coordinates.empty() ? 1 : 3;
            c.location.insert(c.location.end(),q.context.begin()+offset,q.context.end());
            c.terminal=readView(q.context.front())->orderMode!=uUnordered;
        }
        if(q.kind==pathUnknown || q.kind==pathAlternativeUnknown) {
            c.pending=true;
            bool implicit=q.kind==pathUnknown && q.argument==static_cast<int>(ListPhase::leave) &&
                readView(q.node)->orderMode==uUnordered && access_->physicalList(q.node) &&
                !getFlagBit(readView(q.node)->value->flags,infonState::tailUnfinished);
            if(implicit) implicit=listBoundary(*access_,q.node).missingExtents.empty();
            c.uncertain=!implicit;
        }
        if(q.kind==pathEnd || q.kind==pathAlternativesEnd) c.terminal=true;
        if(q.kind==pathAlternativesEnd && getFlagBit(readView(q.node)->innr->value->flags,infonState::tailUnfinished)) {
            c.pending=true;c.uncertain=!readView(q.node)->implicitMembershipChoice;
        }
        if(!q.coordinates.empty() && (q.kind==pathMatched || q.kind==pathEnd)) {
            c.branch=q.context[2];c.branchEnd=q.kind==pathEnd;
        }
        return c;
    }
    void contribute(const Q& q,bool active) {
        if(active && q.kind==fieldPosition && q.argument==static_cast<int>(ListPhase::member))
            if(!sourceOrder_.count(q.node)) sourceOrder_[q.node]=nextSourceOrder_++;
        // Navigation has no output contribution. Its reads belong to the
        // traversal entry, independently of candidate/result evidence.
        if(q.kind!=pathMatched && q.kind!=pathUnknown && q.kind!=pathAlternativeUnknown &&
           q.kind!=pathEnd && q.kind!=pathAlternativesEnd) return;
        auto old=contributions_.find(q);Contribution next;
        if(active) next=describe(q);
        if(active && old!=contributions_.end() && old->second==next) return;
        if(active && old!=contributions_.end() && next.member && old->second.member==next.member) {
            // Refining a retained source updates its alias evidence, not its
            // membership or place among the other known alternatives.
            old->second=next;memberProofs_[next.member][q]=next.location;
            locations_[next.member]=memberProofs_[next.member].begin()->second;
            verified_=false;resultChanged_=true;return;
        }
        auto adjust=[&](const Contribution& c,bool add) {
            auto amount=add ? 1 : -1;
            pendingCount_+=amount*c.pending;uncertainCount_+=amount*c.uncertain;terminalCount_+=amount*c.terminal;
            if(c.branch) {
                auto& b=branches_[c.branch];emptyBranches_-=b.ends && !b.matches;
                b.matches+=amount*bool(c.member);b.ends+=amount*c.branchEnd;
                emptyBranches_+=b.ends && !b.matches;
            }
            if(c.member) {
                auto& proofs=memberProofs_[c.member];
                if(add) {
                    if(proofs.empty()) {
                        if(!sourceOrder_.count(c.member)) sourceOrder_[c.member]=nextSourceOrder_++;
                        auto before=std::find_if(matches_.begin(),matches_.end(),[&](const POVNode& m) {
                            return sourceOrder_.at(m)>sourceOrder_.at(c.member);
                        });
                        matches_.insert(before,c.member);
                    }
                    proofs[q]=c.location;locations_[c.member]=proofs.begin()->second;
                } else {
                    proofs.erase(q);
                    if(proofs.empty()) {
                        matches_.erase(std::remove(matches_.begin(),matches_.end(),c.member),matches_.end());
                        locations_.erase(c.member);memberProofs_.erase(c.member);
                    } else locations_[c.member]=proofs.begin()->second;
                }
            }
        };
        if(old!=contributions_.end()) {adjust(old->second,false);contributions_.erase(old);}
        if(active) {contributions_[q]=next;adjust(next,true);}
        waiting_=pendingCount_ || emptyBranches_;
        uncertainCandidates_=uncertainCount_ || emptyBranches_;
        verified_=false;resultChanged_=true;
    }
    bool collectPosition(const Query<POVNode>& q) {
        if(q.kind==pathAlternative) {aggregating_=true;positionBranches_.push_back(q.node);}
        if(q.kind==pathMatched) {
            // Keep the first discovered winner for each
            // requested position, independently of the other oneOf branches.
            positionWinners_.emplace(q.context.front(),q.node);
            if(!aggregating_ && matches_.empty()) matches_.push_back(q.node);
        }
        if(q.kind==pathEnd) positionEnds_.insert(q.context.front());
        if(q.kind==pathAlternativesEnd) {
            positionsEnded_=true;
            positionTailPending_=getFlagBit(readView(q.node)->innr->value->flags,infonState::tailUnfinished);
            positionTailUncertain_=positionTailPending_ && !readView(q.node)->implicitMembershipChoice;
        }
        if(aggregating_ || positionsEnded_) {
            matches_.clear();waiting_=positionTailPending_;uncertainCandidates_=positionTailUncertain_;
            for(const auto& branch:positionBranches_) {
                auto winner=positionWinners_.find(branch);
                if(winner==positionWinners_.end()) {if(!positionEnds_.count(branch)) waiting_=uncertainCandidates_=true;}
                else if(std::find(matches_.begin(),matches_.end(),winner->second)==matches_.end()) matches_.push_back(winner->second);
            }
            return positionsEnded_;
        }
        return q.kind==pathMatched || q.kind==pathEnd;
    }
    bool collectedAlternatives() const {return result_ && (publishedPending_ || published_.size()!=1);}
public:
    ProteusPathGoal(Query<POVNode> query,std::shared_ptr<ProteusGraphAccess> access,
                     std::shared_ptr<ProteusDefinitions> definitions,std::shared_ptr<FieldCandidates> candidates={},
                     std::function<Expansion<POVNode>(const Query<POVNode>&)> expand={})
        :GoalSpec<POVNode>(query),access_(std::move(access)),expand_(std::move(expand)),candidateEpoch_(candidates ? candidates->epoch() : 0),definitions_(std::move(definitions)) {
        if(!request.context.empty()) {
            auto state=readView(request.context.front())->pathSelection;
            if(state) {result_=state->result;published_=state->matches;publishedPending_=state->pending;}
        }
        if(expand_ && request.kind==selectField && !BoundaryGraph::direction(request.related) && !TemporalGraph::pathKind(request.related))
            traversal_=std::make_unique<PersistentTraversal<POVNode>>(request,[this](const Q& q,bool active){contribute(q,active);});
    }
    bool persistent() const override {return bool(traversal_);}
    DeductionStats deductionStats() const override {return traversal_ ? traversal_->counters() : DeductionStats{};}
    std::vector<std::pair<Q,std::size_t>> deductionVisits() const override {
        return traversal_ ? traversal_->visits() : std::vector<std::pair<Q,std::size_t>>{};
    }
    bool retainPath() const override {return !traversal_;}
    void connect(std::function<void()> wake,DependencyKey broad,DependencyKey untracked) override {
        resume_=wake;if(traversal_) traversal_->connect(std::move(wake),std::move(broad),std::move(untracked));
    }
    std::optional<Expansion<POVNode>> expand(const Visit<POVNode>& visit) override {
        if(!expand_) return std::nullopt;
        FieldCandidates::Scope scope(answers_,candidateEpoch_);
        if(!traversal_) return expand_(visit.query);
        try {traversal_->advance([this](const Q& q){return expand_(q);});}
        catch(const NonPersistentNavigation&) {
            // Indexed/unit navigation may materialize an occurrence. Keep that
            // operation in the scheduler until it has an incremental producer.
            traversal_.reset();contributions_.clear();memberProofs_.clear();branches_.clear();
            pendingCount_=uncertainCount_=terminalCount_=emptyBranches_=0;
            if(resume_) resume_();return Expansion<POVNode>{};
        }
        Expansion<POVNode> out;out.refinements=traversal_->takeRequirements();out.deferred=traversal_->deferred();
        if(traversal_->ready()) {
            auto q=request;out.edges.push_back({"continue field deduction",{0,0},false,[q]{return q;}});
        }
        return out;
    }
    void beginPass() override {
        matches_.clear();locations_.clear();verified_=waiting_=aggregating_=false;branchMatched_=uncertainCandidates_=false;
        positionBranches_.clear();positionWinners_.clear();positionEnds_.clear();positionsEnded_=positionTailPending_=positionTailUncertain_=positionReady_=false;
        positionFinding_={request,{},0};
    }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        if(traversal_) return {};
        auto q=visit.query;
        if(q.kind==externalField && request.kind==externalField && !readView(q.node)->pathSelection && !readView(q.node)->isOrLike()) {
            auto state=readView(request.context.front())->pathSelection;
            auto type=readView(q.related)->type;
            if(type && !state->updates.empty() && POVDescription(q.node,access_->registry.get()).associations(type->typeID()).empty()) {
                auto access=access_;auto anchor=q.node,predicate=q.related;
                return {{"Set external property",[access,anchor,predicate] {
                    auto value=std::make_shared<POV>(919);readView(value)=std::make_shared<infonView>();
                    readView(value)->type=readView(predicate)->type;readView(value)->addedTypes=readView(predicate)->addedTypes;
                    SetModeBits(readView(value)->flags,infonView::itmMode,infonView::mItem);
                    SetModeBits(readView(value)->flags,infonView::viewMode,infonView::vmOverlay);
                    SetModeBits(readView(value)->infSize->flags,infonState::evalMode,infonState::emUnknown);
                    SetModeBits(readView(value)->value->flags,infonState::evalMode,infonState::emUnknown);
                    SetModeBits(readView(value)->value->flags,infonState::overlayType,infonState::tUnknown);
                    access->bind(value);access->bind(anchor);
                    // Predicate storage stays read-only. Its explicit facts
                    // constrain independent property storage through ordinary work.
                    readView(value)->addPOVToWrkList(predicate);
                    access->registry->recordExternalAssociation(anchor,predicate,value,{},readView(predicate)->type->typeID(),0);
                    return true;
                },{1,2}}};
            }
        }
        if(q.kind==resolutionStep) {
            auto kind=static_cast<int>(q.coordinates[0]);bool matched=q.coordinates[1];
            q.coordinates.erase(q.coordinates.begin(),q.coordinates.begin()+2);q.kind=kind;
            // A proved direct absence continues to the external tier. Do not
            // publish an empty direct result while that continuation is pending.
            if(q.kind==pathEnd && !matched) return {};
        }
        if(q.kind==externalMatched || q.kind==externalUnknown || q.kind==externalEnd) {
            if(q.kind==externalMatched || q.kind==externalUnknown) {
                if(std::find(matches_.begin(),matches_.end(),q.node)==matches_.end()) matches_.push_back(q.node);
                locations_.emplace(q.node,std::vector<POVNode>{q.node});
            }
            if(q.kind==externalUnknown) waiting_=uncertainCandidates_=true;
            if(q.kind==externalEnd && q.coordinates.empty()) positionReady_=true;
            if(visit.cost>=positionFinding_.cost) positionFinding_=visit;
            return {};
        }
        if(request.kind==filterChoice) {
            if(q.kind==choiceMatched || q.kind==choiceUnknown) {
                if(std::find(matches_.begin(),matches_.end(),q.node)==matches_.end()) matches_.push_back(q.node);
                std::vector<POVNode> location{q.node};location.insert(location.end(),q.context.begin()+1,q.context.end());
                locations_.emplace(q.node,std::move(location));
            }
            if(q.kind==choiceUnknown || q.kind==choicePending) {
                waiting_=true;
                // Filtering an implicit membership choice must retain why its
                // tail is open. Known matches stay actionable; an unresolved
                // predicate, explicit tail, or nested gap remains uncertain.
                bool implicit=q.kind==choicePending && q.argument==static_cast<int>(ListPhase::leave) &&
                    readView(request.node)->implicitMembershipChoice &&
                    q.node==definitions_->selectionBody(request.node);
                if(implicit) {
                    auto boundary=listBoundary(*access_,q.node);
                    implicit=boundary.valid && boundary.missingExtents.empty();
                }
                uncertainCandidates_ |= !implicit;
            }
            if(q.kind==choiceEnd) positionReady_=true;
            if(visit.cost>=positionFinding_.cost) positionFinding_=visit;
            return {};
        }
        if(q.kind==pathSelfFound) q.kind=pathMatched;
        if(request.kind==selectField && (BoundaryGraph::direction(request.related) || TemporalGraph::pathKind(request.related)) && q.kind==pathMatched) {
            matches_={q.node};locations_[q.node]={q.node};return publish();
        }
        if(q.kind==pathMatched) {
            std::vector<POVNode> location{q.node};
            auto offset=q.coordinates.empty() ? 1 : 3;
            location.insert(location.end(),q.context.begin()+offset,q.context.end());
            locations_.emplace(q.node,std::move(location));
        }
        bool finished=false;
        if(request.kind==selectPosition || request.kind==selectSelf || request.kind==selectEnvironment) {
            auto winners=positionWinners_.size();
            positionReady_=collectPosition(q) || positionReady_;
            if((positionWinners_.size()!=winners || q.kind==pathAlternativesEnd || q.kind==pathEnd) && visit.cost>=positionFinding_.cost)
                positionFinding_=visit;
            return {};
        }
        if(q.kind==pathAlternative) {aggregating_=true;branchMatched_=false;}
        if(q.kind==pathUnknown || q.kind==pathAlternativeUnknown) {
            waiting_=true;
            // Only the implicit open membership of a fully inspected record
            // leaves its known candidates usable. Explicit tails, unknown
            // spans, and unresolved predicates still withhold a write.
            bool implicit=q.kind==pathUnknown && q.argument==static_cast<int>(ListPhase::leave) &&
                readView(q.node)->orderMode==uUnordered && access_->physicalList(q.node) &&
                !getFlagBit(readView(q.node)->value->flags,infonState::tailUnfinished);
            if(implicit) implicit=listBoundary(*access_,q.node).missingExtents.empty();
            uncertainCandidates_ |= !implicit;
        }
        if(q.kind==pathMatched) {
            branchMatched_=true;
            if(std::find(matches_.begin(),matches_.end(),q.node)==matches_.end()) matches_.push_back(q.node);
        }
        if(q.kind==pathEnd && aggregating_ && !branchMatched_) waiting_=uncertainCandidates_=true;
        if(q.kind==pathAlternativesEnd && getFlagBit(readView(q.node)->innr->value->flags,infonState::tailUnfinished)) {
            waiting_=true;uncertainCandidates_ |= !readView(q.node)->implicitMembershipChoice;
        }
        finished=q.kind==pathAlternativesEnd || (!aggregating_ && (q.kind==pathEnd ||
            (q.kind==pathMatched && readView(q.context.front())->orderMode!=uUnordered)));
        if(request.kind==resolveWord) {
            // External candidates can arrive after their branch boundary.
            // Collect all explored routes before publishing a choice result.
            positionReady_=positionReady_ || finished;
            if(visit.cost>=positionFinding_.cost) positionFinding_=visit;
            return {};
        }
        return finished ? publish() : std::vector<SetCommand>{};
    }
    CommandBatch<POVNode> finishPass() override {
        if(traversal_) {
            if(traversal_->ready() || !terminalCount_) return {};
            return {{request,{},0},publish()};
        }
        if(!positionReady_) return {};
        return {positionFinding_,publish()};
    }
private:
    std::vector<SetCommand> publish() {
        if(traversal_ && !resultChanged_) {verified_=!waiting_;return {};}
        const bool alternatives=waiting_ || matches_.size()!=1;
        const bool implicitMembership=waiting_ && !uncertainCandidates_;
        if(result_ && published_==matches_ && publishedPending_==waiting_ &&
           (!alternatives || readView(result_)->implicitMembershipChoice==implicitMembership)) {
            bool current=true;
            if(!alternatives && !request.context.empty()) {
                auto state=readView(request.context.front())->pathSelection;
                current=state->resultView==readView(result_) && state->resultRevision==result_->workRevision && PathExpression::location(state,result_)==locations_.at(result_);
            }
            if(alternatives) {
                if(!request.context.empty()) {
                    auto state=readView(request.context.front())->pathSelection;
                    for(const auto& source:matches_)
                        if(PathExpression::location(state,source)!=locations_.at(source)) current=false;
                }
                auto member=readView(result_)->innr->value->items.head;
                for(const auto& source:matches_) {
                    if(!member || readView(member)!=readView(source)) {current=false;break;}
                    member=member->next;
                }
            }
            if(current) {verified_=!waiting_;resultChanged_=false;return {};}
        }
        // Existing alternatives keep their POV/identity when more information
        // arrives. Only newly matching source occurrences need a new alias.
        std::map<POVNode,POVNode> reusable;
        if(collectedAlternatives()) {
            auto member=readView(result_)->innr->value->items.head;
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
        return {{"Set path selection result",[this,matches,access,pending,alternatives,reusable,implicitMembership]() -> SetResult {
            auto changes=Changes::only({});
            // A distinct view can share this mutable inner state without
            // sharing the innr view itself. Until state-slot readers have
            // their own keys, keep publication conservative for that alias.
            if(collectedAlternatives() && readView(result_)->innr->value.use_count()>1) changes=Changes{};
            std::map<POVNode,POVDescription::PositionFacts> positions;
            auto changedNode=[&](const POVNode& node) {
                if(!node) return;
                positions.emplace(node,POVDescription::positionSnapshot(node));
                auto view=readView(node);
                changes.include(ReadScope::changedObjects({node.get(),view.get(),view->pathSelection.get(),view->innr.get()}));
            };
            // Replacing a selection changes its binding and shared path state.
            // Rebuilding a choice also changes its inner list and alias links.
            if(!request.context.empty()) changedNode(request.context.front());
            if(collectedAlternatives()) changedNode(result_);
            if(!alternatives) {
                // Registry-held aliases can outlive their retired choice.
                for(const auto& entry:reusable) {changedNode(entry.second);entry.second->pParent=nullptr;}
                result_=matches.front();
            } else {
                if(!collectedAlternatives()) {
                    auto item=std::make_shared<infonView>();
                    item->convertThisToIntersection(makeInnrInfonView(),fromGiven);
                    SetModeBits(item->value->flags,infonState::overlayType,infonState::tUnknown);
                    result_=std::make_shared<POV>(914);readView(result_)=item;access->bind(result_);
                }
                auto body=readView(result_)->innr->value;
                std::set<POVNode> retained(matches.begin(),matches.end());
                for(const auto& entry:reusable) if(!retained.count(entry.first)) {
                    auto member=entry.second;
                    changedNode(member);changedNode(member->prev);changedNode(member->next);
                    body->items.erase(member);member->prev.reset();member->next.reset();member->pParent=nullptr;
                }
                auto cursor=body->items.head;
                for(const auto& source:matches) {
                    auto found=reusable.find(source);POVNode member;
                    if(found!=reusable.end()) {
                        member=found->second;
                        if(readView(member)!=readView(source)) {changedNode(member);readView(member)=readView(source);}
                    } else {
                        member=std::make_shared<POV>(914);readView(member)=readView(source);
                        bool named=access->bind(member);named=access->bind(source) || named;
                        access->registry->recordIdentity(member,source,iekListCorrespondence,0);
                        changedNode(source);changedNode(member);
                        if(named) changes=Changes{};
                    }
                    if(member!=cursor) {
                        // Splice just the changed member. Existing aliases and
                        // their unchanged links remain intact across arrivals.
                        changedNode(member);changedNode(member->prev);changedNode(member->next);
                        if(found!=reusable.end()) body->items.erase(member);
                        auto previous=cursor ? cursor->prev : body->items.tail;
                        changedNode(previous);changedNode(cursor);
                        member->prev=previous;member->next=cursor;
                        if(previous) previous->next=member;else body->items.head=member;
                        if(cursor) cursor->prev=member;else body->items.tail=member;
                    }
                    member->pParent=result_.get();cursor=member->next;
                }
                SetFlagBit(body->flags,infonState::tailUnfinished,pending);
                readView(result_)->implicitMembershipChoice=implicitMembership;
                ++result_->workRevision;
            }
            SetResult publication{true,changes};
            published_=matches;publishedPending_=pending;verified_=!pending;resultChanged_=false;
            if(!request.context.empty()) {
                auto owner=request.context.front();auto previous=readView(owner);
                auto state=previous->pathSelection;
                auto result=std::make_shared<infonView>(*readView(result_));
                result->pathSelection=state;result->selectedResult=result_;
                if(state->alternatives) PathExpression::retainChoiceDeclaration(result,state->expression);
                result->originID=previous->originID;result->infCharPos=previous->infCharPos;result->replyCharPos=previous->replyCharPos;
                SetFlagBit(result->flags,infonView::doReply,getFlagBit(previous->flags,infonView::doReply));
                SetFlagBit(result->flags,infonView::isNormed,false);
                state->result=result_;state->matches=matches;state->pending=pending;
                state->resultView=readView(result_);state->resultRevision=result_->workRevision;
                state->locations.clear();
                for(const auto& source:matches) {
                    GraphPathLocation at;at.source=source;at.context=locations_.at(source);
                    state->locations.push_back(std::move(at));
                }
                readView(owner)=result;
                if(access->bind(owner)) publication.changes=Changes{};
                ++owner->workRevision;
                if(!alternatives) {
                    auto inherited=definitions_->inheritChanges(owner,result_);
                    if(inherited.changed) publication.changes->include(inherited.changes.value_or(Changes{}));
                    publication.retainEvidence=std::move(inherited.retainEvidence);
                }
            }
            for(const auto& previous:positions)
                POVDescription::includePositionChange(*publication.changes,previous.first,previous.second);
            return publication;
        },work}};
    }
public:
    bool satisfied() const override {return verified_ || ((request.kind==selectPosition || request.kind==selectSelf) && positionReady_ && !waiting_);}
    bool symbolic() const override {if(traversal_) traversal_->observe();return !verified_ || (request.argument==pathValue && result_ && readView(result_)->isOrLike());}
    POVNode result() const override {return result_;}
};

// A relationship can demand a path result without demanding literal size/value
// on the incoming qualified POV. The path owns its retained effects and updates;
// correspondence separately proves what its discovered payload supplies.
class ProteusPathExpressionGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
public:
    ProteusPathExpressionGoal(Query<POVNode> query,std::shared_ptr<ProteusGraphAccess> access)
        :GoalSpec<POVNode>(query),access_(std::move(access)) {}
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        if(visit.query.node!=request.node) return {};
        auto access=access_;auto node=request.node;
        if(PathExpression::ready(*access,node))
            return {{"Prepare RHS path selection",[access,node]{return PathExpression::prepare(*access,node);},PathExpression::work(*access,node)}};
        auto state=readView(node)->pathSelection;
        if(state && PathExpression::needsUpdates(state))
            return {{"Set RHS path updates",[access,state]{return PathExpression::applyUpdates(*access,state);},{state->updates.size(),0}}};
        return {};
    }
    bool satisfied() const override {
        auto state=readView(request.node)->pathSelection;
        return state && state->result && !state->pending && PathExpression::updatesReady(state);
    }
    bool symbolic() const override {return !satisfied();}
};
