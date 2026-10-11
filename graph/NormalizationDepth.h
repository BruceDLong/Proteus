#pragma once

// Depth bounds expansion of the requested result. Its header goal can still
// follow any dependency needed to establish that result. Loaded member links
// form a retained frontier; a larger request reuses the shared header goals.
constexpr std::int64_t allContainedParts = -1;

class ProteusDepthGoal : public GoalSpec<POVNode> {
    using Q=Query<POVNode>;
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::shared_ptr<ProteusCompletion> completion_;
    PersistentTraversal<POVNode> traversal_;
    Expansion<POVNode> expandAt(const Q& q) {
        Expansion<POVNode> out;
        auto edge=[&](Q next) {
            Edge<POVNode> step{"next requested part",{0,0},false,[next]{return next;}};
            step.destination=next;out.edges.push_back(std::move(step));
        };
        if(q.kind==normalizationDepth) {
            // A whole-subtree request also settles values behind equal/selected
            // sources. Header-only access can expose a local copy's parts while
            // leaving its source's temporal preservation unevaluated.
            auto extent=q.argument==allContainedParts ? fillImmediateParts : exposeStructure;
            out.refinements.push_back({{q.node,sizeAndValue,extent,{},q.context},{0,0},"resolve requested header",false});
            if(q.argument==0) return out;
            if(definitions_->required(q.node))
                out.refinements.push_back({{q.node,typeRequirements,fillImmediateParts,{},q.context},{0,0},"expose requested parts",false});
            // Alternatives and selected aliases do not add a containment level.
            if(auto path=readView(q.node)->pathSelection;path && !path->matches.empty()) {
                for(auto match:path->matches) if(match!=q.node)
                    out.refinements.push_back({{match,normalizationDepth,q.argument,{},PathExpression::location(path,match)},
                        {0,0},"expand selected result",false});
                return out;
            }
            if(!access_->physicalList(q.node) && !access_->growingList(q.node)) return out;
            auto owners=(access_->subList(q.node) || BoundaryGraph::temporal(q.node)) ? q.context : std::vector<POVNode>{};
            if(!owners.empty()) owners.erase(owners.begin());
            for(auto parent:owners) if(access_->sameInfon(parent,q.node)) {out.deferred=true;return out;}
            out.refinements.push_back({{q.node,listContents,sizeOnly,{},owners},{0,0},"check requested member extent",false});
            owners.push_back(q.node);
            if(auto first=POVDescription(q.node).member()) edge({first,depthMember,q.argument,{},std::move(owners)});
        } else if(q.kind==depthMember) {
            POVDescription member(q.node);
            auto context=q.context;context.insert(context.begin(),q.node);
            auto remaining=q.argument==allContainedParts ? allContainedParts :
                q.argument-(member.position().embedded ? 0 : 1);
            out.refinements.push_back({{q.node,normalizationDepth,remaining,{},std::move(context)},
                {0,0},"expand requested part",false});
            if(auto next=member.adjacent()) {auto onward=q;onward.node=next;edge(std::move(onward));}
        }
        return out;
    }
public:
    ProteusDepthGoal(Q query,std::shared_ptr<ProteusGraphAccess> access,
                     std::shared_ptr<ProteusDefinitions> definitions,std::shared_ptr<ProteusCompletion> completion)
        :GoalSpec<POVNode>(query),access_(std::move(access)),definitions_(std::move(definitions)),
         completion_(std::move(completion)),traversal_(query,[](const Q&,bool){}) {}
    bool persistent() const override {return true;}
    bool retainPath() const override {return false;}
    void connect(std::function<void()> wake,DependencyKey broad,DependencyKey untracked) override {
        traversal_.connect(std::move(wake),std::move(broad),std::move(untracked));
    }
    DeductionStats deductionStats() const override {return traversal_.counters();}
    std::vector<std::pair<Q,std::size_t>> deductionVisits() const override {return traversal_.visits();}
    std::vector<SetCommand> found(const Visit<POVNode>&) override {return {};}
    std::optional<Expansion<POVNode>> expand(const Visit<POVNode>&) override {
        traversal_.advance([this](const Q& q){return expandAt(q);});
        Expansion<POVNode> out;out.refinements=traversal_.takeRequirements();out.deferred=traversal_.deferred();
        if(traversal_.ready()) {auto q=request;out.edges.push_back({"continue requested depth",{0,0},false,[q]{return q;}});}
        return out;
    }
    bool satisfied() const override {traversal_.observe();return !traversal_.ready();}
    bool symbolic() const override {return !satisfied();}
    void settled(bool resolved,const DependencyProof::Ptr& evidence) override {
        if(!resolved) return;
        // A satisfied depth request is not proof of the entire subtree. Only
        // independently completed children allow the ordinary isNormed flag.
        if(access_->physicalList(request.node)) {
            if(request.argument==0 || !access_->closedPhysicalList(request.node)) return;
            std::set<POVNode> seen;
            for(auto member=POVDescription(request.node).member();member;member=POVDescription(member).adjacent())
                if(!seen.insert(member).second || !getFlagBit(readView(member)->flags,infonView::isNormed)) return;
        } else if(!access_->scalar(request.node) && !access_->definitionReady(request.node)) return;
        completion_->mark(request,evidence);
    }
};
