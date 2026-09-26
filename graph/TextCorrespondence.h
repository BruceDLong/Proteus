#pragma once

// A correspondence cursor walks existing list members and read-only templates.
// A successful scalar comparison supplies the next edge. A mismatch supplies
// none. Repetition exposes both its end and next occurrence; no failed route is
// enumerated and no tentative occurrence is installed in the semantic graph.
// The scheduler owns the frontier, cost, cancellation and equivalent-state map.
class TextCorrespondence {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    static Edge<POVNode> edge(const char* label, Query<POVNode> next) {
        return {label, {1, 0}, true, [next] { return next; }};
    }
    bool readable(const POVNode& node, bool root) const {
        if (!access_->plain(node, false, false, false, false) || access_->hasTypeRequirements(node)) return false;
        if (!root) for (const auto& relation : POVDescription(node, access_->registry.get()).relationships())
            if (!RHSRepresentation(access_, node, relation.target).represented) return false;
        return true;
    }
public:
    TextCorrespondence(std::shared_ptr<ProteusGraphAccess> access,
                       std::shared_ptr<ProteusDefinitions> definitions)
        : access_(std::move(access)), definitions_(std::move(definitions)) {}
    static bool relationship(ProteusGraphAccess& access, const POVNode& target, const POVNode& source) {
        bool scalar = access.wholeLooseRHS(source) && access.scalarCandidate(target) &&
            (access.unknownOverlay(target) || access.typeFacts(target).overlay == infonState::STR);
        return (access.literalList(target) || scalar) && source->looseType() &&
            getModeBits(POVDescription(source).value()->flags, infonState::overlayType) == infonState::STR;
    }
    bool sourceReady(const POVNode& source) const {
        auto value = POVDescription(source).value();
        return access_->plain(source, false, false, true, false) &&
            getModeBits(value->flags, infonState::overlayType) == infonState::STR && getModeBits(value->flags, infonState::evalMode) == infonState::emLiteral &&
            !access_->hasTypeRequirements(source) &&
            POVDescription(source, access_->registry.get()).relationships().empty() &&
            value->str.size() <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
    }
    Expansion<POVNode> expand(const Query<POVNode>& q) const {
        Expansion<POVNode> out;
        if (q.kind == textComplete) return out;
        if (q.kind == textCorrespondence) {
            // A prefix relationship cannot prove the end of an unsized open
            // target. This finite Set planner must not close that stream from
            // an arbitrary nullable match; prefix publications come later.
            if (q.related->looseSize() && access_->openTail(q.node) && access_->listExtent(q.node) < 0) return out;
            if (sourceReady(q.related))
                out.edges.push_back(edge("enter text correspondence", {q.node, textEnter, 0, {}, {q.related}}));
            return out;
        }
        const auto source = q.context.front();
        const auto& text = POVDescription(source).value()->str;
        if (q.kind == textEnter) {
            if (!readable(q.node, q.context.size() == 1)) return out;
            if (access_->literalList(q.node)) {
                if ((!access_->physicalList(q.node) && !access_->unreferencedTemplateSpan(q.node)) ||
                    !access_->orderedList(q.node) || access_->listExtent(q.node) == -2) return out;
                // Context-free finite/repeated structure only for now. A cycle
                // must become a shared grammar subgoal, not recursive expansion.
                for (std::size_t i = 1; i < q.context.size(); ++i)
                    if (access_->sameInfon(q.context[i], q.node)) return out;
                auto next = q; next.kind = textBoundary;
                next.related = POVDescription(q.node).member(); next.context.push_back(q.node);
                // Per owner: logical count, input start, may repeat. Counts are
                // items (including flattened embedded spans), never characters.
                next.coordinates.insert(next.coordinates.end(), {0, q.argument, 1});
                out.edges.push_back(edge("enter matching list", std::move(next)));
            } else {
                auto facts = access_->typeFacts(q.node);
                if (!facts.valid || (facts.overlay >= 0 && facts.overlay != infonState::STR) ||
                    (!facts.hasValue && !facts.hasSize && q.context.size() != 1)) return out;
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
                if (facts.hasValue && text.compare(q.argument, width, facts.text) != 0) return out;
                auto next = q; next.kind = textMatched; next.argument += width;
                // Keep the beginning of this scalar occurrence in the cursor;
                // zero-width repeated matches cannot produce an infinite walk.
                next.related.reset(); next.coordinates.push_back(q.argument);
                out.edges.push_back(edge("matched text member", std::move(next)));
            }
        } else if (q.kind == textBoundary) {
            auto n = q.coordinates.size();
            auto count = q.coordinates[n-3], extent = access_->listExtent(q.node);
            if (!q.related && (extent < 0 || count == extent)) {
                auto next = q; next.kind = textMatched;
                out.edges.push_back(edge("matching list may end", std::move(next)));
            }
            if (extent >= 0 && count > extent) return out;
            auto child = q.related;
            if (!child && access_->openTail(q.node) && (extent < 0 || count < extent) && q.coordinates[n-1])
                child = definitions_->repeatedBody(q.node);
            if (child) {
                auto next = q; next.node = child; next.related.reset(); next.kind = textEnter;
                out.edges.push_back(edge("match next list member", std::move(next)));
            }
        } else if (q.kind == textMatched) {
            auto next = q;
            std::int64_t width = 1, begin = 0;
            if (access_->literalList(q.node)) {
                auto n = next.coordinates.size();
                if (access_->subList(q.node)) width = next.coordinates[n-3];
                begin = next.coordinates[n-2];
                next.coordinates.resize(n-3); next.context.pop_back();
            } else {
                begin = next.coordinates.back(); next.coordinates.pop_back();
            }
            if (next.context.size() == 1) {
                if (source->looseSize() || q.argument == static_cast<std::int64_t>(text.size())) {
                    next.kind = textComplete; next.related = source;
                    out.edges.push_back(edge("accepted text correspondence", std::move(next)));
                }
            } else {
                auto n = next.coordinates.size();
                if (width > std::numeric_limits<std::int64_t>::max() - next.coordinates[n-3]) return out;
                next.node = next.context.back(); next.kind = textBoundary;
                next.coordinates[n-3] += width;
                next.related = POVDescription(q.node).adjacent();
                // A nullable template can satisfy a finite extent, but never
                // forces an unbounded number of identical empty occurrences.
                next.coordinates[n-1] = q.argument != begin || access_->listExtent(next.node) >= 0;
                out.edges.push_back(edge("propagate accepted member", std::move(next)));
            }
        }
        return out;
    }
};

class ProteusTextGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::shared_ptr<ProteusDefinitions> definitions_;
    std::shared_ptr<ProteusCompletion> completion_;
    bool verified_ = false;
    std::vector<std::pair<POVNode, POVNode>> templates_;
public:
    ProteusTextGoal(Query<POVNode> q, std::shared_ptr<ProteusGraphAccess> access,
                    std::shared_ptr<ProteusDefinitions> definitions,
                    std::shared_ptr<ProteusCompletion> completion)
        : GoalSpec<POVNode>(q), access_(std::move(access)), definitions_(std::move(definitions)),
          completion_(std::move(completion)) {}
    void beginPass() override { verified_ = false; templates_.clear(); }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        if (visit.query.kind != textComplete) return {};
        struct Frame { POVNode node; std::int64_t begin; DefinitionSpec spec; bool instance; };
        std::vector<Frame> stack;
        DefinitionSpec result;
        std::map<POVNode, std::int64_t> extents;
        // The accepted path is the proof. Compile just that path into existing
        // finite-list Sets; rejected candidates never reach this mutation plan.
        for (const auto& q : visit.path) {
            if (q.kind == textEnter) {
                auto spec = std::make_shared<DefinitionRequirement>();
                spec->facts = access_->typeFacts(q.node);
                spec->list = access_->literalList(q.node);
                bool instance = stack.empty() || (stack.back().instance &&
                    definitions_->repeatedBody(stack.back().node) != q.node);
                stack.push_back({q.node, q.argument, spec, instance});
            } else if (q.kind == textMatched) {
                auto frame = stack.back(); stack.pop_back();
                auto& spec = frame.spec;
                spec->facts.valid = true; spec->facts.hasSize = true;
                if (spec->list) {
                    auto count = static_cast<std::int64_t>(spec->members.size());
                    spec->facts.size = count;
                    if (frame.instance) extents[frame.node] = count;
                    if (frame.instance)
                        if (auto body = definitions_->repeatedBody(frame.node)) templates_.push_back({frame.node, body});
                } else {
                    if (!spec->facts.hasValue) spec->facts.explicitSize = false;
                    spec->facts.overlay = infonState::STR; spec->facts.hasValue = true;
                    spec->facts.text = POVDescription(request.related).value()->str.substr(frame.begin, q.argument-frame.begin);
                    spec->facts.size = q.argument-frame.begin;
                }
                if (stack.empty()) result = spec;
                else if (access_->subList(frame.node))
                    stack.back().spec->members.insert(stack.back().spec->members.end(), spec->members.begin(), spec->members.end());
                else stack.back().spec->members.push_back(spec);
            }
        }
        if (!result) return {};
        auto plan = std::make_shared<ProteusDefinitionPlan>(access_, request.node, result, std::move(extents));
        if (!plan->supported) return {};
        verified_ = plan->satisfied();
        std::vector<ProteusDefinitionPlan::Place> counted;
        for (const auto& binding : plan->bindings)
            if (!binding.second->list && binding.second->facts.hasValue &&
                (!binding.first->node || !access_->typeFacts(binding.first->node).hasValue))
                counted.push_back(binding.first);
        return {{"Set accepted text correspondence", [plan, counted] {
            bool changed = plan->apply();
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
