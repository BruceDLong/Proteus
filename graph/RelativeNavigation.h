#pragma once

// Structural and temporal neighbors share one bounded list walk. The temporal
// policy also enters nested T-spans, and stops at an ordinary S-list context.
class RelativeNavigation {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    static bool temporal(const Query<POVNode>& q) {return !q.coordinates.empty() && q.coordinates.front()!=0;}
    struct WalkAccess {
        ProteusGraphAccess* access;
        bool temporal;
        POVNode first(const POVNode& p,bool reverse) const {return access->first(p,reverse);}
        POVNode next(const POVNode& p,bool reverse) const {return access->next(p,reverse);}
        bool sameInfon(const POVNode& a,const POVNode& b) const {return access->sameInfon(a,b);}
        bool subList(const POVNode& p) const {return access->subList(p) || (temporal && BoundaryGraph::temporal(p));}
        bool inputPending(const POVNode& p) const {return access->inputPending(p);}
    };
    static Edge<POVNode> direct(const char* label,Query<POVNode> q,bool alternative=false) {
        return {label,{1,0},alternative,[q]{return q;}};
    }
    void boundaryNeeds(Expansion<POVNode>& result,const POVNode& node,const ListBoundary<POVNode>& boundary) {
        for(const auto& missing:boundary.missingExtents)
            result.refinements.push_back({{missing,sizeOnly,0},{boundary.visits,0},"find embedded extent",true});
        if(boundary.extent<0) result.refinements.push_back({{node,sizeOnly,0},{boundary.visits,0},"find span extent",true});
    }
    void relativeStep(Expansion<POVNode>& result, const Query<POVNode>& query, ListPhase phase,
                      Cost inspection = 0) {
        if (phase == ListPhase::leave && query.node == query.related) { result.deferred = true; return; }
        WalkAccess walk{access_.get(),temporal(query)};
        auto at = ListPosition<POVNode>{query.node, phase, query.context};
        auto next = phase == ListPhase::enter ? nextListPosition(walk, std::move(at), true, query.argument < 0) :
                                               adjacentListPosition(walk, std::move(at), true, query.argument < 0);
        if (next.phase == ListPhase::done || next.phase == ListPhase::cycle) { result.deferred = true; return; }
        auto kind = next.phase == ListPhase::member ? relativeMember : relativeBoundary;
        auto edge = direct("logical neighbor step", {next.node, kind, query.argument, query.related, next.owners,query.coordinates});
        edge.work.visits += inspection; result.edges.push_back(std::move(edge));
    }
public:
    RelativeNavigation(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions)
        :access_(std::move(access)),definitions_(std::move(definitions)) {}
    Expansion<POVNode> expand(const Query<POVNode>& query) {
        Expansion<POVNode> result;auto node=query.node;
        POVDescription description(node,access_->registry.get());
        if (query.kind == selectRelative) {
            if (!query.context.empty() && (!access_->definitionOrderedCandidate(query.context.back()) ||
                access_->hasConcat(query.context.back(),1) || access_->hasConcat(query.context.back(),2))) {
                result.deferred = true; return result;
            }
            auto owners=query.context;
            if(temporal(query)) {
                // A spatial parent is not a temporal continuation. Retain just
                // the contiguous temporal/embedded owner chain of this occurrence.
                auto first=owners.size();
                while(first && (BoundaryGraph::temporal(owners[first-1]) || access_->subList(owners[first-1]))) --first;
                owners.erase(owners.begin(),owners.begin()+first);
                while(!owners.empty() && !BoundaryGraph::temporal(owners.front())) owners.erase(owners.begin());
                if(owners.empty()) {result.deferred=true;return result;}
            } else if(!owners.empty()) {
                // Normal %prev/%next stay in the immediate structural sequence,
                // even when a containing T-context is available to other goals.
                auto first=owners.size();
                while(first && access_->subList(owners[first-1])) --first;
                if(first) --first;
                owners.erase(owners.begin(),owners.begin()+first);
            }
            result.edges.push_back(direct("relative occurrence", {query.related, relativeItem, query.argument, owners.empty() ? POVNode{} : owners.front(), owners,query.coordinates}));
            return result;
        }
        if (query.kind == relativeItem) {
            if (auto owner = definitions_->retainedOccurrenceOwner(node)) {
                auto index = description.occurrenceIndex();
                auto extent = access_->listExtent(owner);
                if (index >= 0 && index < extent &&
                    (query.argument < 0 ? index > 0 : index < extent - 1))
                    result.edges.push_back(direct("relative sparse coordinate", {owner, itemAt, index + query.argument}));
                else if(temporal(query) && !query.context.empty()) {
                    auto outer=query;outer.node=owner;outer.kind=relativeBoundary;
                    if(outer.context.back()==owner) outer.context.pop_back();
                    relativeStep(result,outer,ListPhase::leave);
                }
            } else relativeStep(result, query, ListPhase::leave);
            return result;
        }
        if (query.kind == relativeMember || query.kind == relativeBoundary) {
            if (query.kind == relativeMember && !access_->subList(node) && !(temporal(query) && BoundaryGraph::temporal(node))) {
                result.edges.push_back(direct("adjacent occurrence", {node, itemFound, 0})); return result;
            }
            if (!access_->navigationList(node)) {
                result.refinements.push_back({{node, valueOnly, 0}, {1, 0}, "resolve navigation span", true});
                result.deferred = true; return result;
            }
            // The known beginning of a span and a closed end need no extent
            // scan. Open ends require evidence about the remaining positions.
            if (!access_->openTail(node) || (query.kind == relativeMember && query.argument > 0 && description.member()) ||
                (query.kind == relativeBoundary && query.argument < 0)) {
                relativeStep(result, query, query.kind == relativeMember ? ListPhase::enter : ListPhase::leave);
                return result;
            }
            auto boundary = listBoundary(*access_, node);
            if (!boundary.valid) { result.deferred = true; return result; }
            boundaryNeeds(result, node, boundary);
            if (boundary.tail == 0) {
                relativeStep(result, query, query.kind == relativeMember ? ListPhase::enter : ListPhase::leave, boundary.visits);
            } else {
                auto index = query.kind == relativeBoundary ? boundary.prefix :
                             query.argument < 0 ? boundary.extent - 1 : 0;
                if (index >= 0 && (boundary.extent < 0 || index < boundary.extent)) {
                    auto edge = direct("open span position", {node, itemAt, index}, true);
                    edge.work.visits += boundary.visits; result.edges.push_back(std::move(edge));
                }
                result.refinements.push_back({{node, valueOnly, 0}, {boundary.visits, 0}, "resolve open span boundary", true});
            }
            return result;
        }
        return result;
    }
};
