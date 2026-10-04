#pragma once

// A read-only, finite proof that an RHS's current facts are already expressed
// by the target. This does not prove either endpoint fully normalized and never
// removes a relationship. Unknown source fields impose no new requirements.
// Layout planning uses the same compressed-span equations as correspondence;
// member comparisons inspect explicit positions without filling unknown gaps.
// Empty open ordered bodies add only header requirements.
class RHSRepresentation {
    std::shared_ptr<ProteusGraphAccess> access_;
    bool layout(const POVNode& node, SpanLayout<POVNode>& result, bool source) {
        return readListLayout(*access_, node, result, work.visits,
            [&](const auto& at) {
                return at.phase != ListPhase::enter ||
                    (access_->physicalList(at.node) && access_->orderedList(at.node) &&
                     (at.node==node || !access_->memberTemplate(at.node)) && access_->inputError(at.node).empty() &&
                     // The root's nominal requirements were checked before
                     // layout. Embedded owners still need separate evidence.
                     (!source || ((at.node==node || !access_->hasTypeRequirements(at.node)) && POVDescription(at.node).work().empty())));
            }, [](const POVNode& member) { return member; });
    }
public:
    static void retain(const std::shared_ptr<GraphRHSProof>& proof,const DependencyProof::Ptr& evidence) {
        proof->evidence=evidence;std::weak_ptr<GraphRHSProof> weak=proof;
        evidence->onInvalidated([weak] {if(auto record=weak.lock()) record->valid=false;});
    }
    static void remember(const POVNode& target,const POVNode& source,const DependencyProof::Ptr& evidence) {
        if(readView(target)->workIsRepresented(source)) return;
        auto& records=readView(target)->representedWork;
        records.erase(std::remove_if(records.begin(),records.end(),
            [](const auto& record){return !record->valid;}),records.end());
        auto proof=std::make_shared<GraphRHSProof>();proof->rhs=source;
        records.push_back(proof);retain(proof,evidence);
    }
    // Discovery can reuse a proved representation before the whole search
    // settles. Retain only positive proofs in the existing RHS records, and
    // observe their dependencies on every reuse. Mutation planners still use
    // the constructor's fresh check: unpublished writes cannot reuse a proof.
    static bool expressed(const std::shared_ptr<ProteusGraphAccess>& access,const POVNode& target,const POVNode& source,
                          const std::shared_ptr<ProteusDefinitions>& definitions,Work& cost) {
        auto context=definitions ? definitions->modelChanges() : Changes{};
        if(!ReadScope::tracksObjects() || context.all) {
            RHSRepresentation proof(access,target,source,definitions);
            cost.visits=addCost(cost.visits,proof.work.visits);return proof.represented;
        }
        for(const auto& proof:readView(target)->representedWork)
            if(proof->valid && proof->rhs==source && proof->evidence &&
               std::all_of(context.keys.begin(),context.keys.end(),[&](const auto& key){return proof->evidence->observes(key);}) &&
               proof->evidence->valid()) {++cost.visits;return true;}
        auto reads=context.keys;bool reusable=true,represented;Work work;
        {
            ReadScope capture([&](const DependencyKey& key){reads.insert(key);if(!key) reusable=false;});
            RHSRepresentation proof(access,target,source,definitions);
            represented=proof.represented;work=proof.work;
        }
        for(const auto& key:reads) ReadScope::observe(key);
        cost.visits=addCost(cost.visits,work.visits);
        if(represented && reusable) remember(target,source,DependencyProof::make(reads));
        return represented;
    }
    Work work{0, 0};
    bool represented = false, complete = false;
    RHSRepresentation(std::shared_ptr<ProteusGraphAccess> access, POVNode target, POVNode source,
                      std::shared_ptr<ProteusDefinitions> definitions = {})
        : access_(std::move(access)) {
        std::vector<std::pair<POVNode, POVNode>> pending{{std::move(target), std::move(source)}};
        std::set<std::pair<POVNode, POVNode>> visited;
        bool fullyExpressed=true;
        while (!pending.empty()) {
            auto pair = pending.back(); pending.pop_back();
            auto left = pair.first, right = pair.second;
            ++work.visits;
            // Selected views may share their actual members. Their facts are
            // already present, including pending member requirements. This is
            // representation evidence, not proof of a closed normalization.
            if(access_->sameInfon(left,right)) {fullyExpressed=false;continue;}
            // A cyclic proof cannot establish itself. Decline conservatively;
            // actual graph traversal still follows every retained constraint.
            // A checked scalar meaning is already expressed by its fields;
            // consumers with that evidence need not reject it for its label.
            if(access_->hasTypeRequirements(right)) {
                bool described=definitions && (definitions->describesParts(right) ||
                    RecordStructure::typedParts(*access_,right,work));
                if(!definitions || (!access_->scalarTypeShape(right) && !described) ||
                   (!described && !definitions->satisfied(right))) return;
                if(compareNominal(*definitions,right,left,work)!=ScalarMatch::yes) {
                    // This is an asserted equality, not a type-based search.
                    // Distinct labels may express the same value. Prove the
                    // supplied meaning in the target without adding an alias.
                    auto meaning=definitions->saved(right);
                    if(!meaning || meaning->requirement->hasExpressions()) return;
                    ProteusDefinitionPlan check(access_,left,meaning->requirement);
                    work.visits=addCost(work.visits,check.work.visits);
                    if(!check.supported || !check.satisfied()) return;
                }
            }
            if (!visited.insert(pair).second || !access_->inputError(left).empty() ||
                !access_->inputError(right).empty() ||
                access_->occurrenceOwner(right) ||
                (!access_->scalarCandidate(right) && !access_->literalList(right))) return;
            // A repeated requirement is redundant only when the target itself
            // satisfies its compiled meaning. The source having checked its own
            // members is not evidence about the target's members.
            if (access_->memberTemplate(right)) {
                auto proof=definitions ? definitions->saved(right) : ProteusDefinitions::Certificate{};
                if(!proof || !proof->requirement->repeated) return;
                ProteusDefinitionPlan meaning(access_,left,proof->requirement);
                work.visits=addCost(work.visits,meaning.work.visits);
                if(!meaning.supported || !meaning.satisfied()) return;
            }
            // A represented value alone cannot erase a transition declaration.
            if(POVDescription(right).independentTemporalValue() &&
               !POVDescription(left).independentTemporalValue()) return;
            auto actual = access_->typeFacts(left), required = access_->typeFacts(right);
            if (!required.valid || !required.accepts(actual)) return;
            // Rendering can suppress partial facts. Removing pending work
            // needs a closed, context-free equality whose remaining relation
            // can be carried by the identity registry.
            fullyExpressed &= !access_->hasTypeRequirements(left) && !access_->hasTypeRequirements(right) &&
                POVDescription(right).work().empty() && required.hasSize;
            if(access_->literalList(right))
                fullyExpressed &= access_->closedPhysicalList(left) && access_->closedPhysicalList(right);
            else fullyExpressed &= required.hasValue;
            if (access_->literalList(right) && !listHeaderOnly(*access_,right)) {
                if(RecordStructure::record(*access_,right)) {
                    std::vector<POVNode> a,b,missing;
                    std::vector<std::pair<POVNode,POVNode>> paired;
                    if(!RecordStructure::members(*access_,left,a,work) || !RecordStructure::members(*access_,right,b,work) ||
                       !RecordStructure::pairs(*access_,a,b,paired,missing,work,[&](std::size_t i,std::size_t j) {
                           // Use the same unique typed-field correspondence as
                           // record copying; no extra transitive identities are needed.
                           if(!definitions || !access_->hasTypeRequirements(b[j])) return ScalarMatch::no;
                           return compareNominal(*definitions,b[j],a[i],work);
                       }) || !missing.empty()) return;
                    pending.insert(pending.end(),paired.begin(),paired.end());
                } else {
                    SpanLayout<POVNode> a, b;
                    if (!layout(left, a, false) || !layout(right, b, true)) return;
                    auto plan = alignSpans(a, b);
                    if (!representsSpanShape(a,b,plan)) return;
                    pending.insert(pending.end(), plan.identities.begin(), plan.identities.end());
                }
            }
            for (auto rhs : POVDescription(right).work()) pending.emplace_back(left, rhs);
        }
        represented = true; complete=fullyExpressed;
    }
};

// A qualified relationship reads payload facts without inheriting the incoming
// qualifier. Its own constraints still need ordinary graph proofs. This reader
// checks those proofs; it never normalizes or discards pending work.
struct PayloadReadiness {
    bool ready;
    Work work{0,0};
    PayloadReadiness(const std::shared_ptr<ProteusGraphAccess>& access,const POVNode& payload,
                     const std::shared_ptr<ProteusDefinitions>& definitions) {
        ready=!access->hasArgument(payload) &&
            (!access->hasTypeRequirements(payload) || definitions->describesParts(payload) ||
             RecordStructure::typedParts(*access,payload,work) || definitions->satisfied(payload));
        for(const auto& relation:POVDescription(payload,access->registry.get()).relationships()) {
            RHSRepresentation proof(access,payload,relation.target,definitions);
            work.visits=addCost(work.visits,proof.work.visits);
            ready=ready && proof.represented;
        }
    }
};
