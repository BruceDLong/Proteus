#pragma once

// Word resolution gives direct members semantic precedence. This continuation
// wraps the existing field walk; a proved direct absence exposes the registry
// tier. Unknown direct candidates keep that tier unavailable, regardless of cost.
class Resolution {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::function<Expansion<POVNode>(const Query<POVNode>&)> navigate_;
    static Edge<POVNode> edge(const char* label,Query<POVNode> query,Cost visits=1) {
        return {label,{visits,0},true,[query]{return query;}};
    }
    static Query<POVNode> wrap(Query<POVNode> query,bool matched) {
        query.coordinates.insert(query.coordinates.begin(),{query.kind,matched});
        query.kind=resolutionStep;return query;
    }
public:
    Resolution(std::shared_ptr<ProteusGraphAccess> access,std::shared_ptr<ProteusDefinitions> definitions,
               std::function<Expansion<POVNode>(const Query<POVNode>&)> navigate)
        :access_(std::move(access)),definitions_(std::move(definitions)),navigate_(std::move(navigate)) {}
    Expansion<POVNode> expand(Query<POVNode> q) const {
        Expansion<POVNode> out;
        if(q.kind==resolveWord || q.kind==resolutionStep) {
            bool matched=false;
            if(q.kind==resolutionStep) {
                q.kind=q.coordinates[0];matched=q.coordinates[1];
                q.coordinates.erase(q.coordinates.begin(),q.coordinates.begin()+2);
            } else q.kind=selectField;
            // Choice branches require separate precedence proofs per branch.
            if(readView(q.node)->isOrLike()) {out.deferred=true;return out;}
            if(q.kind==pathEnd && !matched) {
                out.edges.push_back(edge("look up external property",{q.context.front(),externalField,0,q.related,q.context}));return out;
            }
            matched=matched || q.kind==pathMatched || q.kind==pathUnknown;
            out=PathSelection(access_,definitions_,navigate_).expand(q);
            for(auto& route:out.edges) {
                auto follow=route.follow;
                route.follow=[follow,matched] {auto at=follow();return Arrival<POVNode>{wrap(std::move(at.query),matched),at.graphChanged};};
            }
            return out;
        }
        if(q.kind==externalMatched || q.kind==externalUnknown || q.kind==externalEnd) return out;
        if(q.kind==externalField && readView(q.node)->pathSelection) {
            auto state=readView(q.node)->pathSelection;
            out.refinements.push_back({pathRequest(q.node,pathCandidates),{0,0},"select external property anchor",false});
            if(state->result && !state->pending && state->matches.size()==1) {
                q.node=state->result;out.edges.push_back(edge("use selected property anchor",q));
            } else out.deferred=true;
            return out;
        }
        if(q.kind==externalField) {
            if(readView(q.node)->isOrLike() || !q.related || !readView(q.related)->type) {out.deferred=true;return out;}
            auto references=POVDescription(q.node,access_->registry.get()).associations(readView(q.related)->type->typeID());
            Cost visits=references.size()+1;
            for(const auto& reference:references)
                out.edges.push_back(edge("external property candidate",{reference->valuePOV,externalPosition,0,q.related,{q.node}},visits));
            out.edges.push_back(edge("external property boundary",{q.node,externalEnd,0,q.related,{q.node}},visits));return out;
        }
        auto candidate=q.node;Cost visits=1;
        FieldPredicate predicate(access_,definitions_,out);
        auto match=predicate.match(q.related,candidate,{});visits=addCost(visits,predicate.work.visits);
        if(match!=ScalarMatch::no)
            out.edges.push_back(edge("matching external property",{candidate,match==ScalarMatch::yes ? externalMatched : externalUnknown,0,q.related,q.context},visits));
        return out;
    }
};
