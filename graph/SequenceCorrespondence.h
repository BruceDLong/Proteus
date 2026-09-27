#pragma once

// A correspondence cursor walks existing list members and read-only templates.
// A successful scalar comparison supplies the next edge. A mismatch supplies
// none. Repetition exposes both its end and next occurrence; no failed route is
// enumerated and no tentative occurrence is installed in the semantic graph.
// The scheduler owns the frontier, cost, cancellation and equivalent-state map.
class SequenceCorrespondence {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    static Edge<POVNode> edge(const char* label, Query<POVNode> next) {
        return {label, {1, 0}, true, [next] { return next; }};
    }
    static std::size_t stackStart(const Query<POVNode>& q) { return 2 + q.coordinates[0]; }
    POVNode pattern(const POVNode& node) const {
        return access_->canSelect(node) ? definitions_->selectionBody(node) : node;
    }
    bool sequence(const POVNode& node) const { return access_->literalList(pattern(node)); }
    // Context: source, current input node, input owners, grammar owners.
    // Coordinates: input-owner count, ListPhase, resume demand, then grammar
    // frames (logical member count, starting input offset, may-repeat flag).
    // Both halves refer to existing POVs; no duplicate adjacency is stored.
    static ListPosition<POVNode> inputPosition(const Query<POVNode>& q) {
        return {q.context[1], static_cast<ListPhase>(q.coordinates[1]),
                {q.context.begin() + 2, q.context.begin() + stackStart(q)}};
    }
    static Query<POVNode> storeInput(Query<POVNode> q, const ListPosition<POVNode>& at) {
        q.context.erase(q.context.begin() + 2, q.context.begin() + stackStart(q));
        q.context.insert(q.context.begin() + 2, at.owners.begin(), at.owners.end());
        q.context[1] = at.node;
        q.coordinates[0] = at.owners.size();
        q.coordinates[1] = static_cast<int>(at.phase);
        return q;
    }
    Expansion<POVNode> input(const Query<POVNode>& q) const {
        Expansion<POVNode> out;
        auto at = inputPosition(q);
        if (at.phase == ListPhase::cycle) return out;
        if (at.phase == ListPhase::member && !access_->subList(at.node)) {
            // Type meanings are read as constraints while a route is tentative.
            // Applying them here would mutate even a rejected typed candidate.
            auto demand=access_->hasTypeRequirements(at.node) ? compileTypeRequirements : sizeAndValue;
            out.refinements.push_back({{at.node,demand,0},{1,0},"refine input member",true});
            auto next=q; next.kind=q.coordinates[2];
            out.edges.push_back(edge("input member available", std::move(next))); return out;
        }
        if (at.phase == ListPhase::done) {
            auto next=q; next.kind=q.coordinates[2]; next.context[1].reset();
            out.edges.push_back(edge("input sequence ended", std::move(next))); return out;
        }
        if (at.phase == ListPhase::enter && !sourceReady(at.node)) return out;
        std::uint64_t visits=1;
        if (at.phase == ListPhase::leave) {
            auto boundary=listBoundary(*access_, at.node); visits+=boundary.visits;
            if (!boundary.valid || !boundary.missingExtents.empty()) return out;
        }
        auto next = storeInput(q, nextListPosition(*access_, std::move(at)));
        auto step=edge("advance input span",std::move(next)); step.work.visits=visits;
        out.edges.push_back(std::move(step)); return out;
    }
    Query<POVNode> nextInput(Query<POVNode> next) const {
        next = storeInput(next, nextListPosition(*access_, inputPosition(next)));
        next.coordinates[2]=next.kind; next.kind=correspondenceInput; return next;
    }
    static bool inverted(const POVNode& node) { return getFlagBit(node->pItem->flags, infonView::invertMatch); }
    bool readable(const POVNode& node, bool root, bool predicate = false) const {
        if (!access_->plainStructure(node, access_->canSelect(node), false, false, false, false, false) ||
            (!predicate && inverted(node)) ||
            (access_->hasTypeRequirements(node) && !definitions_->scalarFacts(node,predicate).valid)) return false;
        if (!root) for (const auto& relation : POVDescription(node, access_->registry.get()).relationships())
            if (!RHSRepresentation(access_, node, relation.target, definitions_).represented) return false;
        return true;
    }
public:
    SequenceCorrespondence(std::shared_ptr<ProteusGraphAccess> access,
                       std::shared_ptr<ProteusDefinitions> definitions)
        : access_(std::move(access)), definitions_(std::move(definitions)) {}
    // ':' loosens the current pair only. '==' propagates loose matching
    // into members; do not mutate shared source POV flags while exploring.
    static bool looseSize(const Query<POVNode>& q) {
        auto source=q.context.front();
        return q.context.size()==stackStart(q) ? source->looseSize() :
            getFlagBit(source->flags,POV::looseMode) || getFlagBit(source->flags,POV::looseInhtd);
    }
    static bool looseType(const Query<POVNode>& q) {
        return q.context.size()==stackStart(q) ? q.context.front()->looseType() : looseSize(q);
    }
    static bool relationship(ProteusGraphAccess& access, const POVNode& target, const POVNode& source) {
        bool scalar = access.wholeLooseRHS(source) && access.scalarCandidate(target) &&
            (access.unknownOverlay(target) || access.typeFacts(target).overlay == infonState::STR);
        auto overlay=getModeBits(POVDescription(source).value()->flags,infonState::overlayType);
        return source->looseType() && ((overlay==infonState::STR && (access.literalList(target) || scalar)) ||
            (overlay==infonState::LST && access.literalList(target) && (source->looseSize() || getFlagBit(source->flags, POV::isOutrFeed))));
    }
    // An identity obtained while matching names an atomic input member even
    // when that member's value is a list. Replaying its accepted proof must not
    // reinterpret the copied value as a grammar that consumes more input.
    static bool inputIdentity(ProteusGraphAccess& access, const Query<POVNode>& q) {
        return q.context.size()>stackStart(q) && q.context[1] && !access.subList(q.node) &&
            getModeBits(q.context[0]->pItem->value->flags,infonState::overlayType)==infonState::LST &&
            access.hasDirectIdentity(q.node,q.context[1]);
    }
    bool sourceReady(const POVNode& source) const {
        auto value=POVDescription(source).value();
        auto overlay=getModeBits(value->flags,infonState::overlayType);
        return access_->plain(source,false,false,true,false) &&
            getModeBits(value->flags,infonState::evalMode)==infonState::emLiteral &&
            !access_->hasTypeRequirements(source) &&
            POVDescription(source,access_->registry.get()).relationships().empty() &&
            ((overlay==infonState::STR && value->str.size()<=static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) ||
             (overlay==infonState::LST && access_->orderedList(source) && !access_->openTail(source) && !value->listSpec));
    }
    Expansion<POVNode> expand(const Query<POVNode>& q) const {
        Expansion<POVNode> out;
        if (q.kind == sequenceComplete) return out;
        if (q.kind == correspondenceInput) return input(q);
        if (q.kind == sequenceCorrespondence) {
            // A prefix relationship cannot prove the end of an unsized open
            // target. This finite Set planner must not close that stream from
            // an arbitrary nullable match; prefix publications come later.
            if (q.related->looseSize() && access_->openTail(q.node) && access_->listExtent(q.node) < 0) return out;
            if (sourceReady(q.related)) {
                bool list=getModeBits(q.related->pItem->value->flags,infonState::overlayType)==infonState::LST;
                Query<POVNode> next{q.node,list ? correspondenceInput : sequenceEnter,0,{}, {q.related,q.related},
                                    {0,static_cast<int>(ListPhase::enter),sequenceEnter}};
                out.edges.push_back(edge("enter sequence correspondence",std::move(next)));
            }
            return out;
        }
        const auto source = q.context.front();
        const auto& text = POVDescription(source).value()->str;
        bool textInput=getModeBits(source->pItem->value->flags,infonState::overlayType)==infonState::STR;
        auto body=pattern(q.node);
        bool fedRoot=getFlagBit(source->flags, POV::isOutrFeed) && q.context.size()==stackStart(q) && access_->fedTemplate(q.node);
        if (q.kind == sequenceEnter) {
            if (access_->hasTypeRequirements(q.node))
                out.refinements.push_back({{q.node,compileTypeRequirements,0},{1,0},"read pattern type meaning",true});
            if (!readable(q.node, q.context.size() == stackStart(q), true)) return out;
            // Composite exclusion needs a proof for the entire subpattern.
            // The scalar predicate rule must not negate individual children.
            NumericRangeAccess numeric(*access_);auto range=numeric.read(q.node,true);
            if(access_->hasTypeRequirements(q.node)) range.constraint=false;
            if (inverted(q.node) && !access_->scalarTypeShape(q.node) && !range.bounds.valid) return out;
            if (sequence(q.node) && !inputIdentity(*access_,q)) {
                if ((!access_->physicalList(body) && !access_->unreferencedTemplateSpan(body)) ||
                    !access_->orderedList(body) || access_->listExtent(body) == -2) return out;
                if (body!=q.node && !POVDescription(body).work().empty()) return out;
                // Context-free finite/repeated structure only for now. A cycle
                // must become a shared grammar subgoal, not recursive expansion.
                for (std::size_t i = stackStart(q); i < q.context.size(); ++i)
                    if (access_->sameInfon(q.context[i], q.node)) return out;
                auto next = q; next.kind = sequenceBoundary;
                next.related = fedRoot ? POVNode{} : POVDescription(body).member(); next.context.push_back(q.node);
                // Per owner: logical count, input start, may repeat. Counts are
                // items (including flattened embedded spans), never characters.
                next.coordinates.insert(next.coordinates.end(), {0, q.argument, 1});
                out.edges.push_back(edge("enter matching list", std::move(next)));
            } else {
                auto facts = definitions_->scalarFacts(q.node,true);
                if (!facts.valid && !range.bounds.valid) return out;
                if (!textInput) {
                    auto item=q.context[1];
                    if (!item || !readable(item,false)) return out;
                    auto supplied=definitions_->scalarFacts(item);
                    auto actualRange=numeric.read(item);
                    if (!supplied.valid && !actualRange.bounds.valid) return out;
                    auto nominal=compareNominal(*definitions_,q.node,item);
                    if(range.bounds.valid && range.constraint) {
                        auto match=actualRange.bounds.valid ? matchNumeric(range,actualRange,looseSize(q),looseType(q)).verdict :
                            compareNumeric(range,actualRange,supplied);
                        if(inverted(q.node) ? (nominal!=ScalarMatch::no && match!=ScalarMatch::no) :
                            (nominal==ScalarMatch::no || match!=ScalarMatch::yes)) return out;
                    } else if (inverted(q.node)) {
                        if (nominal!=ScalarMatch::no && compareScalar(facts, supplied)!=ScalarMatch::no) return out;
                    } else if (nominal==ScalarMatch::no || (!supplied.hasValue && !access_->literalList(item)) || !facts.combine(supplied)) return out;
                    auto next=q; next.kind=sequenceAtomMatched; ++next.argument; next.related.reset(); next.coordinates.push_back(q.argument);
                    auto matched=edge("matched input member",nextInput(std::move(next)));
                    if(range.bounds.valid && range.constraint) matched.work.visits=addCost(matched.work.visits,numeric.work.visits);
                    out.edges.push_back(std::move(matched)); return out;
                }
                if ((facts.overlay>=0 && facts.overlay!=infonState::STR) ||
                    (!facts.hasValue && !facts.hasSize && q.context.size()!=stackStart(q))) return out;
                // At the relation root, whole correspondence supplies an
                // unknown string's width. Inside a list it still needs a width
                // demand; consuming an arbitrary remainder would hide suffixes.
                std::int64_t width = facts.hasValue ? static_cast<std::int64_t>(facts.text.size()) :
                    static_cast<std::int64_t>(text.size()) - q.argument;
                if (facts.hasSize) {
                    if (facts.size < 0 || facts.size.getDenominator() != 1 ||
                        facts.size > std::numeric_limits<std::int64_t>::max()) return out;
                    auto size = facts.size.getNumerator().get_si();
                    if (facts.hasValue && width != size) return out;
                    width = size;
                }
                if (q.argument > static_cast<std::int64_t>(text.size()) ||
                    width > static_cast<std::int64_t>(text.size()) - q.argument) return out;
                GraphTypeFacts supplied; supplied.valid=true; supplied.overlay=infonState::STR;
                supplied.hasValue=true; supplied.text=text.substr(q.argument,width);
                supplied.hasSize=true; supplied.size=width;
                auto match=compareScalar(facts,supplied);
                if (inverted(q.node) ? match!=ScalarMatch::no : match==ScalarMatch::no) return out;
                auto next = q; next.kind = sequenceAtomMatched; next.argument += width;
                // Keep the beginning of this scalar occurrence in the cursor;
                // zero-width repeated matches cannot produce an infinite walk.
                next.related.reset(); next.coordinates.push_back(q.argument);
                out.edges.push_back(edge("matched text member", std::move(next)));
            }
        } else if (q.kind == sequenceBoundary) {
            auto n = q.coordinates.size();
            auto count = q.coordinates[n-3], extent = access_->listExtent(body);
            if (!q.related && (extent < 0 || count == extent)) {
                auto next = q; next.kind = sequenceMatched;
                out.edges.push_back(edge("matching list may end", std::move(next)));
            }
            if (extent >= 0 && count > extent) return out;
            auto child = q.related;
            bool feed=getFlagBit(source->flags, POV::isOutrFeed) && q.context.size()==stackStart(q)+1 && access_->fedTemplate(q.node);
            if (!child && (feed || access_->openTail(body)) && (extent<0 || count<extent) && q.coordinates[n-1])
                child=feed ? definitions_->repeatedBody(q.node) : definitions_->matchingBody(body);
            if (child) {
                auto next = q; next.node = child; next.related.reset(); next.kind = sequenceEnter;
                out.edges.push_back(edge("match next list member", std::move(next)));
            }
        } else if (q.kind == sequenceMatched || q.kind == sequenceAtomMatched) {
            auto next = q;
            std::int64_t width = 1, begin = 0;
            if (q.kind == sequenceMatched) {
                auto n = next.coordinates.size();
                if (access_->subList(q.node)) width = next.coordinates[n-3];
                begin = next.coordinates[n-2];
                next.coordinates.resize(n-3); next.context.pop_back();
            } else {
                begin = next.coordinates.back(); next.coordinates.pop_back();
            }
            if (next.context.size() == stackStart(next)) {
                if (source->looseSize() || (textInput ? q.argument == static_cast<std::int64_t>(text.size()) : !q.context[1])) {
                    next.kind = sequenceComplete; next.related = source;
                    out.edges.push_back(edge("accepted sequence correspondence", std::move(next)));
                }
            } else {
                auto n = next.coordinates.size();
                if (width > std::numeric_limits<std::int64_t>::max() - next.coordinates[n-3]) return out;
                next.node = next.context.back(); next.kind = sequenceBoundary;
                next.coordinates[n-3] += width;
                next.related = POVDescription(q.node).adjacent();
                // A nullable template can satisfy a finite extent, but never
                // forces an unbounded number of identical empty occurrences.
                next.coordinates[n-1] = q.argument != begin || access_->listExtent(pattern(next.node)) >= 0;
                out.edges.push_back(edge("propagate accepted member", std::move(next)));
            }
        }
        return out;
    }
};

class ProteusSequenceGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::shared_ptr<ProteusCompletion> completion_;
    bool verified_ = false;
    std::vector<std::pair<POVNode, POVNode>> templates_;
public:
    ProteusSequenceGoal(Query<POVNode> q, std::shared_ptr<ProteusGraphAccess> access,
                    std::shared_ptr<ProteusDefinitions> definitions,
                    std::shared_ptr<ProteusCompletion> completion)
        : GoalSpec<POVNode>(q), access_(std::move(access)), definitions_(std::move(definitions)),
          completion_(std::move(completion)) {}
    void beginPass() override { verified_ = false; templates_.clear(); }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        if (visit.query.kind != sequenceComplete) return {};
        templates_.clear();
        struct Frame { POVNode node, body, source; std::int64_t begin; DefinitionSpec spec, selected; bool instance, looseSize, looseType; };
        std::vector<Frame> stack;
        DefinitionSpec result;
        std::map<POVNode, std::int64_t> extents;
        std::map<DefinitionSpec,POVNode> sources;
        std::map<DefinitionSpec,ProteusDefinitions::Certificate> meanings;
        Cost numericVisits=0;
        // The accepted path is the proof. Compile just that path into existing
        // finite-list Sets; rejected candidates never reach this mutation plan.
        for (const auto& q : visit.path) {
            if (q.kind == sequenceEnter) {
                auto spec = std::make_shared<DefinitionRequirement>();
                auto body=access_->canSelect(q.node) ? definitions_->selectionBody(q.node) : q.node;
                spec->facts=definitions_->scalarFacts(body); spec->list=access_->literalList(body) && !SequenceCorrespondence::inputIdentity(*access_,q);
                auto types=definitions_->names(q.node);
                if (!types.empty() && !getFlagBit(q.node->pItem->flags,infonView::invertMatch)) spec->label=types.front();
                bool instance=stack.empty() || (stack.back().instance && definitions_->matchingBody(stack.back().body)!=q.node);
                // A feed's stored members are results, not its consumed pattern.
                if (!stack.empty() && access_->fedTemplate(stack.back().node)) instance=false;
                stack.push_back({q.node,body,q.context[1],q.argument,spec,{},instance,
                    SequenceCorrespondence::looseSize(q),SequenceCorrespondence::looseType(q)});
            } else if (q.kind == sequenceMatched || q.kind == sequenceAtomMatched) {
                auto frame = stack.back(); stack.pop_back();
                auto& spec = frame.spec;
                spec->facts.valid = true;
                if (spec->list) {
                    spec->facts.hasSize=true;
                    auto count = static_cast<std::int64_t>(spec->members.size());
                    spec->facts.size = count;
                    if (frame.body!=frame.node) {
                        auto mode=access_->selectionMode(frame.node);
                        if (mode==infonView::ipGetMarked) spec=frame.selected;
                        else if (!spec->members.empty()) spec=mode==infonView::ipGetFirst ? spec->members.front() : spec->members.back();
                        else return {};
                        if (!spec) return {};
                    }
                    if (frame.instance && frame.body==frame.node) extents[frame.node] = count;
                    if (frame.instance)
                        if (auto body = definitions_->repeatedBody(frame.node)) templates_.push_back({frame.node, body});
                } else if (getModeBits(request.related->pItem->value->flags,infonState::overlayType)==infonState::LST) {
                    auto supplied=definitions_->scalarFacts(frame.source);
                    NumericRangeAccess numeric(*access_);auto range=numeric.read(frame.node,true);
                    if(access_->hasTypeRequirements(frame.node)) range.constraint=false;
                    bool identity=true;
                    if (getFlagBit(frame.node->pItem->flags,infonView::invertMatch)) spec->facts=supplied;
                    else if(range.bounds.valid && range.constraint) {
                        auto actual=numeric.read(frame.source);
                        auto match=matchNumeric(range,actual,frame.looseSize,frame.looseType);
                        if(match.verdict!=ScalarMatch::yes) return {};
                        spec->facts=match.result.facts;
                        if(match.result.expression && !match.result.bounds.point) spec->numeric=match.result.bounds;
                        identity=match.identity;
                    } else spec->facts.combine(supplied);
                    numericVisits=addCost(numericVisits,numeric.work.visits);
                    if (identity && spec->label.empty()) {
                        auto types=definitions_->names(frame.source);
                        if (!types.empty()) spec->label=types.front();
                    }
                    // An atomic list-valued source stays an existing graph
                    // node. Its identity supplies members through ordinary
                    // correspondence; matching does not clone its subtree.
                    if(identity) sources[spec]=frame.source;
                } else {
                    if (!spec->facts.hasValue) spec->facts.explicitSize = false;
                    spec->facts.overlay = infonState::STR; spec->facts.hasValue = true;
                    spec->facts.text = POVDescription(request.related).value()->str.substr(frame.begin, q.argument-frame.begin);
                    spec->facts.hasSize=true; spec->facts.size = q.argument-frame.begin;
                }
                // A selection passes through its chosen child's proof. Its
                // input cursor does not describe the selected result's type.
                if (frame.body==frame.node && !spec->list) {
                    auto typed=access_->hasTypeRequirements(frame.node) && !getFlagBit(frame.node->pItem->flags,infonView::invertMatch) ? frame.node : frame.source;
                    auto meaning=typed ? definitions_->saved(typed) : nullptr;
                    if (meaning && !meaning->requirement->list && meaning->requirement->facts.accepts(spec->facts)) meanings[spec]=meaning;
                }
                if (!stack.empty() && stack.back().body->pItem->intersectMarkedRetPOV==frame.node) stack.back().selected=spec;
                if (stack.empty()) result = spec;
                else if (access_->subList(frame.node))
                    stack.back().spec->members.insert(stack.back().spec->members.end(), spec->members.begin(), spec->members.end());
                else stack.back().spec->members.push_back(spec);
            }
        }
        if (!result) return {};
        std::shared_ptr<ProteusDefinitionPlan> plan;
        try { plan = std::make_shared<ProteusDefinitionPlan>(access_, request.node, result, std::move(extents)); }
        catch (const DefinitionConflict&) { return {}; } // this candidate supplies no Set

        if (!plan->supported) return {};
        plan->work.visits=addCost(plan->work.visits,numericVisits);
        verified_ = plan->satisfied();
        std::vector<ProteusDefinitionPlan::Place> counted;
        std::vector<std::pair<ProteusDefinitionPlan::Place,POVNode>> identities;
        std::vector<std::pair<ProteusDefinitionPlan::Place,ProteusDefinitions::Certificate>> typeEvidence;
        for (const auto& binding : plan->bindings) {
            auto meaning=meanings.find(binding.second);
            if (meaning!=meanings.end()) { typeEvidence.push_back({binding.first,meaning->second}); ++plan->work.visits; }
            auto found=sources.find(binding.second);
            if (found!=sources.end() && (!binding.first->node || !access_->hasDirectIdentity(binding.first->node,found->second))) {
                verified_=false; ++plan->work.creations; identities.push_back({binding.first,found->second});
            }
            if (binding.second->facts.overlay==infonState::STR && !binding.second->list && binding.second->facts.hasValue &&
                (!binding.first->node || !access_->typeFacts(binding.first->node).hasValue))
                counted.push_back(binding.first);
        }
        auto access=access_; auto definitions=definitions_;
        return {{"Set accepted sequence correspondence", [plan, counted, identities, typeEvidence, access, definitions] {
            for (const auto& binding:plan->bindings) if(binding.first->node) access->bind(binding.first->node);
            for (const auto& identity:identities) access->bind(identity.second);
            bool changed = plan->apply();
            for (const auto& identity:identities) {
                access->bind(identity.first->node);
                access->registry->recordIdentity(identity.first->node,identity.second,iekListCorrespondence,0); changed=true;
            }
            // The accepted data already satisfies these scalar meanings.
            // Reuse their witnesses instead of expanding the same type again
            // for every output occurrence. This adds no definition RHS.
            for (const auto& typed:typeEvidence) {
                auto checked=definitions->plan(typed.first->node,typed.second);
                if (checked && checked->dataSatisfied()) changed |= definitions->apply(typed.first->node,typed.second,checked);
            }
            // Proteus3 mergeLooseStrings changes a copied fixed-width string
            // from a given size to its now-observed character count.
            for (const auto& place : counted) if (place->node->pItem->value->sizeMode != fromCount) {
                place->node->pItem->value->sizeMode = fromCount; changed = true;
            }
            return changed;
        }, plan->work}};
    }
    bool satisfied() const override { return verified_; }
    bool symbolic() const override { return !verified_; }
    void settled(bool resolved) override {
        if (!resolved) return;
        completion_->represented(request.node, request.related);
        for (const auto& entry : templates_) completion_->representedTemplate(entry.first, entry.second);
    }
};
