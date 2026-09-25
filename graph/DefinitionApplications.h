#pragma once

// Context-owned definition catalogue and reusable scalar type evidence. No
// definition body is normalized or copied into a use's work list. A certificate
// describes which immutable constraints were checked; isNormed is independent.
class ProteusDefinitions {
public:
    struct Definition { POVNode declaration, body; };
    struct Binding { std::string name; POVNode body; };
    struct Witness {
        POVNode node;
        std::shared_ptr<infonView> infon;
        GraphTypeFacts facts;
        std::vector<std::string> names;
        std::vector<POVNode> work;
    };
    struct Proof {
        GraphTypeFacts facts;
        std::vector<Binding> bindings;
        std::vector<Witness> witnesses;
        bool conflict = false;
    };
    using Certificate = std::shared_ptr<Proof>;
private:
    std::shared_ptr<ProteusGraphAccess> access_;
    std::map<std::string, std::vector<Definition>> catalogue_;
    // This slice admits only context-free scalar constraints. Their evidence
    // belongs to the infon, so another POV does not repeat the application.
    // Context-sensitive meanings will require binding context in this key.
    using Key = std::weak_ptr<infonView>;
    std::map<Key, std::vector<Certificate>, std::owner_less<Key>> evidence_;
public:
    explicit ProteusDefinitions(std::shared_ptr<ProteusGraphAccess> access) : access_(std::move(access)) {}
    std::vector<std::string> names(const POVNode& node) const {
        auto raw = access_->typeNames(node);
        std::vector<std::string> result(raw.begin(), raw.end());
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }
    bool registered(const POVNode& node) const {
        if (!access_->definitionReady(node)) return false;
        auto found = catalogue_.find(access_->definitionName(node));
        if (found == catalogue_.end()) return false;
        for (const auto& entry : found->second)
            if (entry.declaration->pItem == node->pItem && entry.body->pItem == node->pItem->funcArgs)
                return true;
        return false;
    }
    bool publish(const POVNode& node) {
        if (!access_->definitionReady(node) || registered(node)) return false;
        auto& entries = catalogue_[access_->definitionName(node)];
        for (auto& entry : entries) if (entry.declaration->pItem == node->pItem) {
            entry.body = access_->definitionBody(node); return true;
        }
        entries.push_back({node, access_->definitionBody(node)});
        return true;
    }
    POVNode lookup(const std::string& name) const {
        auto found = catalogue_.find(name);
        if (found == catalogue_.end() || found->second.size() != 1) return {};
        const auto& entry = found->second.front();
        if (!registered(entry.declaration)) return {};
        return entry.body;
    }
    // Read-only adjacency. An ambiguous/missing meaning stays unresolved; it
    // never becomes a collection of conjunctive RHSs or a cheapest-meaning pick.
    bool children(const POVNode& node, const POVNode& root, std::vector<POVNode>& result) const {
        if (!access_->typeFacts(node).valid) return false;
        for (const auto& name : names(node)) {
            auto body = lookup(name);
            if (!body) return false;
            if (std::find(result.begin(), result.end(), body) == result.end()) result.push_back(body);
        }
        if (node != root)
            for (auto rhs : access_->workSources(node))
                if (std::find(result.begin(), result.end(), rhs) == result.end()) result.push_back(rhs);
        return true;
    }
    void collect(const POVNode& node, const POVNode& root, const Certificate& proof) {
        for (const auto& name : names(node)) {
            auto body = lookup(name);
            if (!body) continue; // expansion keeps the walk from reaching its end
            auto old = std::find_if(proof->bindings.begin(), proof->bindings.end(),
                [&](const Binding& binding) { return binding.name == name; });
            if (old == proof->bindings.end()) proof->bindings.push_back({name, body});
        }
        if (node == root) return;
        for (const auto& witness : proof->witnesses) if (witness.node == node) return;
        auto facts = access_->typeFacts(node);
        if (!facts.valid) return; // unsupported bodies cannot reach definitionEnd
        if (!proof->facts.combine(facts)) proof->conflict = true;
        auto work = access_->workSources(node);
        proof->witnesses.push_back({node, node->pItem, facts, names(node), {work.begin(), work.end()}});
    }
    bool current(const Certificate& proof) const {
        for (const auto& binding : proof->bindings)
            if (lookup(binding.name) != binding.body) return false;
        for (auto& witness : proof->witnesses) {
            auto work = access_->workSources(witness.node);
            if (witness.node->pItem != witness.infon ||
                !witness.facts.same(access_->typeFacts(witness.node)) ||
                witness.names != names(witness.node) ||
                witness.work != std::vector<POVNode>(work.begin(), work.end())) return false;
        }
        return true;
    }
    bool covers(const Certificate& proof, const POVNode& node) const {
        for (const auto& name : names(node))
            if (std::none_of(proof->bindings.begin(), proof->bindings.end(),
                    [&](const Binding& binding) { return binding.name == name; })) return false;
        return true;
    }
    Certificate saved(const POVNode& node) const {
        auto found = evidence_.find(node->pItem);
        if (found == evidence_.end()) return {};
        for (const auto& proof : found->second)
            if (current(proof) && covers(proof, node)) return proof;
        return {};
    }
    bool satisfied(const POVNode& node) const {
        if (!access_->hasTypeRequirements(node)) return true;
        auto proof = saved(node);
        return proof && proof->facts.accepts(access_->typeFacts(node));
    }
    bool remember(const POVNode& node, const Certificate& proof) {
        auto& certificates = evidence_[node->pItem];
        certificates.erase(std::remove_if(certificates.begin(), certificates.end(),
            [&](const Certificate& old) { return !current(old); }), certificates.end());
        for (const auto& old : certificates) {
            if (old == proof) return false;
            // Repeated copies of independently checked equivalent evidence do
            // not grow a second list of redundant type applications.
            if (old->facts.same(proof->facts) && old->bindings.size() == proof->bindings.size() &&
                std::all_of(proof->bindings.begin(), proof->bindings.end(), [&](const Binding& binding) {
                    return std::any_of(old->bindings.begin(), old->bindings.end(), [&](const Binding& other) {
                        return binding.name == other.name && binding.body == other.body;
                    });
                })) return false;
        }
        certificates.push_back(proof); return true;
    }
    bool apply(const POVNode& node, const Certificate& proof) {
        if (!current(proof) || !covers(proof, node)) return false;
        auto actual = access_->typeFacts(node);
        if (!actual.valid) return false;
        if (proof->conflict || !actual.combine(proof->facts))
            throw std::runtime_error("incompatible scalar type requirements");
        bool changed = access_->applyTypeFacts(node, proof->facts);
        return remember(node, proof) || changed;
    }
    bool inherit(const POVNode& target, const POVNode& source) {
        auto found = evidence_.find(source->pItem);
        if (found == evidence_.end()) return false;
        auto certificates = found->second;
        bool changed = false;
        for (const auto& proof : certificates)
            if (current(proof) && proof->facts.accepts(access_->typeFacts(source)) &&
                proof->facts.accepts(access_->typeFacts(target))) changed |= remember(target, proof);
        return changed;
    }
    void prune() {
        for (auto it = evidence_.begin(); it != evidence_.end();)
            if (it->first.expired()) it = evidence_.erase(it);
            else ++it;
    }
};

class ProteusTypeGoal : public GoalSpec<POVNode> {
    std::shared_ptr<ProteusDefinitions> definitions_;
    ProteusDefinitions::Certificate proof_;
public:
    ProteusTypeGoal(Query<POVNode> query, std::shared_ptr<ProteusDefinitions> definitions)
        : GoalSpec<POVNode>(query), definitions_(std::move(definitions)) {}
    void beginPass() override { proof_ = std::make_shared<ProteusDefinitions::Proof>(); }
    std::vector<SetCommand> found(const Visit<POVNode>& visit) override {
        auto q = visit.query;
        if (q.kind == definitionEnter) definitions_->collect(q.node, request.node, proof_);
        auto certificate = q.kind == typeRequirements ? definitions_->saved(request.node) : proof_;
        if (q.kind != definitionEnd && (q.kind != typeRequirements || !certificate || satisfied())) return {};
        auto definitions = definitions_; auto target = request.node;
        return {{"Apply scalar type requirements", [definitions, target, certificate] {
            return definitions->apply(target, certificate);
        }}};
    }
    bool satisfied() const override { return definitions_->satisfied(request.node); }
    bool symbolic() const override { return !satisfied(); }
};
