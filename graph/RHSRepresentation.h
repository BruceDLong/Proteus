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
            if (!visited.insert(pair).second || !access_->inputError(left).empty() ||
                !access_->inputError(right).empty() ||
                (access_->hasTypeRequirements(right) &&
                 !(definitions && (access_->scalarTypeShape(right) || definitions->describesParts(right)) &&
                   (definitions->describesParts(right) || definitions->satisfied(right)) &&
                   compareNominal(*definitions,right,left,work)==ScalarMatch::yes)) ||
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
                       !RecordStructure::pairs(*access_,a,b,paired,missing,work) || !missing.empty()) return;
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
            (!access->hasTypeRequirements(payload) || definitions->describesParts(payload) || definitions->satisfied(payload));
        for(const auto& relation:POVDescription(payload,access->registry.get()).relationships()) {
            RHSRepresentation proof(access,payload,relation.target,definitions);
            work.visits=addCost(work.visits,proof.work.visits);
            ready=ready && proof.represented;
        }
    }
};
