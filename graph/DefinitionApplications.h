#pragma once
#include "DefinitionPlan.h"

// Context-owned catalogue and reusable type evidence. Bodies are read through
// existing POVs. Application never normalizes or specializes the shared body.
class ProteusDefinitions {
public:
    struct Definition { POVNode declaration, body; };
    struct Binding { std::string name; POVNode body; };
    struct Witness {
        POVNode node;
        std::shared_ptr<infonView> infon;
        GraphTypeFacts facts;
        std::vector<std::string> names;
        std::vector<POVNode> work, members;
        bool list = false, embedded = false;
        POVNode memberTemplate;
    };
    struct Evidence {
        std::vector<Binding> bindings;
        std::map<POVNode, Witness> witnesses;
        std::map<POVNode, DefinitionSpec> compiled;
    };
    struct Proof {
        std::shared_ptr<Evidence> evidence = std::make_shared<Evidence>();
        DefinitionSpec requirement;
        bool compilable = true;
    };
    using Certificate = std::shared_ptr<Proof>;
private:
    std::shared_ptr<ProteusGraphAccess> access_;
    std::map<std::string, std::vector<Definition>> catalogue_;
    // Stable read-only POVs for existing template/owner infons. This exposes
    // stored relations to the walk; it does not clone or normalize their data.
    mutable std::map<std::shared_ptr<infonView>, POVNode> views_;
    // Registry occurrence backlinks are non-owning. Retain each demanded
    // source for this context so a selected item can outlive its lookup handle.
    std::set<POVNode> occurrenceOwners_;
    POVNode view(const std::shared_ptr<infonView>& infon) const {
        if (!infon) return {};
        auto& result = views_[infon];
        if (!result) result = access_->requirementView(infon);
        return result;
    }
    POVNode memberTemplate(const POVNode& node) const { return view(access_->memberTemplate(node)); }
    POVNode occurrenceOwner(const POVNode& node) const { return view(access_->occurrenceOwner(node)); }
    bool listRequirement(const POVNode& node) const {
        return access_->finiteDefinitionList(node) || access_->repeatedDefinitionList(node);
    }
    // Admitted requirements are context-free. Contextual meanings will need a
    // binding-context key; an additional POV alone must not repeat application.
    using Key = std::weak_ptr<infonView>;
    std::map<Key, std::vector<Certificate>, std::owner_less<Key>> evidence_;
    std::vector<POVNode> members(const POVNode& node) const {
        auto raw = access_->definitionMembers(node); return {raw.begin(), raw.end()};
    }
    std::vector<POVNode> work(const POVNode& node) const {
        return POVDescription(node).work();
    }
public:
    explicit ProteusDefinitions(std::shared_ptr<ProteusGraphAccess> access) : access_(std::move(access)) {}
    std::vector<std::string> names(const POVNode& node) const {
        auto raw = access_->typeNames(node);
        std::vector<std::string> result;
        for (const auto& name : raw)
            if (std::find(result.begin(), result.end(), name) == result.end()) result.push_back(name);
        return result;
    }
    bool registered(const POVNode& node) const {
        if (!access_->definitionReady(node)) return false;
        auto found = catalogue_.find(access_->definitionName(node));
        if (found == catalogue_.end()) return false;
        for (const auto& entry : found->second)
            if (entry.declaration->pItem == node->pItem && entry.body->pItem == node->pItem->funcArgs) return true;
        return false;
    }
    bool publish(const POVNode& node) {
        if (!access_->definitionReady(node) || registered(node)) return false;
        auto& entries = catalogue_[access_->definitionName(node)];
        for (auto& entry : entries) if (entry.declaration->pItem == node->pItem) {
            entry.body = access_->definitionBody(node); return true;
        }
        entries.push_back({node, access_->definitionBody(node)}); return true;
    }
    POVNode lookup(const std::string& name) const {
        auto found = catalogue_.find(name);
        if (found == catalogue_.end() || found->second.size() != 1) return {};
        const auto& entry = found->second.front();
        if (access_->definitionName(entry.declaration) != name || !registered(entry.declaration)) return {};
        return entry.body;
    }
    void retainOccurrenceOwner(const POVNode& owner) { occurrenceOwners_.insert(owner); }
    bool required(const POVNode& node) const {
        return access_->hasTypeRequirements(node) || access_->memberTemplate(node) || access_->occurrenceOwner(node);
    }
    bool children(const POVNode& node, const POVNode& root, std::vector<POVNode>& result) const {
        if (!access_->typeFacts(node).valid) return false;
        if (node != root && !access_->scalarTypeShape(node) && !listRequirement(node)) return false;
        // Registry constraints within a shared body need a read-only relation
        // protocol. Keep them visible as deferred work rather than ignoring them.
        if (node != root && !POVDescription(node, access_->registry.get()).relationships(false).empty()) return false;
        if (node != root && access_->definitionEmbedded(node) && access_->repeatedDefinitionList(node)) return false;
        auto add = [&](const POVNode& child) {
            if (std::find(result.begin(), result.end(), child) == result.end()) result.push_back(child);
        };
        for (const auto& name : names(node)) {
            auto body = lookup(name); if (!body) return false; add(body);
        }
        if (auto spec = memberTemplate(node)) add(spec);
        if (node == root) if (auto owner = occurrenceOwner(node)) add(owner);
        if (node != root) {
            for (auto rhs : work(node)) add(rhs);
            for (auto member : members(node)) add(member);
        }
        return true;
    }
    void collect(const POVNode& node, const POVNode& root, const Certificate& proof) {
        for (const auto& name : names(node)) {
            auto body = lookup(name); if (!body) continue;
            auto& bindings = proof->evidence->bindings;
            if (std::none_of(bindings.begin(), bindings.end(), [&](const Binding& b) { return b.name == name; }))
                bindings.push_back({name, body});
        }
        if (node == root || proof->evidence->witnesses.count(node)) return;
        proof->evidence->witnesses.emplace(node, Witness{node, node->pItem, access_->typeFacts(node),
            names(node), work(node), members(node), listRequirement(node), access_->definitionEmbedded(node), memberTemplate(node)});
    }
    // The graph's leave edge supplies postorder: every required child has
    // already been walked. Compile requirements, never execute child normalize.
    void finish(const POVNode& node, const POVNode& root, const Certificate& proof) {
        if (!proof->compilable) return;
        auto& compiled = proof->evidence->compiled;
        auto requirement = std::make_shared<DefinitionRequirement>();
        requirement->types = names(node);
        if (!requirement->types.empty()) requirement->label = requirement->types.front();
        std::sort(requirement->types.begin(), requirement->types.end());
        if (node != root) {
            auto& witness = proof->evidence->witnesses.at(node);
            requirement->facts = witness.facts;
            requirement->list = witness.list;
            for (const auto& member : witness.members) {
                auto child = compiled.at(member);
                if (access_->definitionEmbedded(member)) {
                    if (!child->list) throw std::runtime_error("non-list embedded definition span");
                    // A named alias may reveal repetition only after its walk.
                    // Never mistake its empty physical prefix for zero items.
                    if (child->repeated) { proof->compilable = false; return; }
                    requirement->members.insert(requirement->members.end(), child->members.begin(), child->members.end());
                } else requirement->members.push_back(child);
            }
            if (access_->finiteDefinitionList(node)) {
                GraphTypeFacts extent;
                extent.valid = true; extent.hasSize = true;
                extent.size = static_cast<std::int64_t>(requirement->members.size());
                if (!requirement->facts.combine(extent)) throw std::runtime_error("incompatible finite definition extent");
            }
        }
        if (auto source = memberTemplate(node)) {
            auto repeated = std::make_shared<DefinitionRequirement>();
            repeated->facts.valid = true; repeated->facts.overlay = infonState::LST;
            repeated->list = true; repeated->repeated = compiled.at(source); repeated->templateSource = source;
            // An open body denotes repeated positions, not an empty finite list.
            if (node != root && access_->repeatedDefinitionList(node)) {
                requirement->repeated = repeated->repeated; requirement->templateSource = source;
            } else requirement = combineDefinitions(requirement, repeated);
        }
        if (node == root) if (auto owner = occurrenceOwner(node)) {
            auto repeated = compiled.at(owner)->repeated;
            if (!repeated) { proof->compilable = false; return; }
            requirement = combineDefinitions(requirement, repeated);
        }
        for (const auto& name : names(node)) requirement = combineDefinitions(requirement, compiled.at(lookup(name)));
        if (node != root)
            for (auto rhs : work(node)) requirement = combineDefinitions(requirement, compiled.at(rhs));
        compiled[node] = requirement;
        if (node == root) {
            proof->requirement = requirement;
            // Compilation scratch must not retain the target through its own
            // weak-keyed certificate cache after the walk finishes.
            compiled.clear();
        }
    }
    bool current(const Certificate& proof) const {
        if (!proof->requirement) return false;
        for (const auto& binding : proof->evidence->bindings)
            if (lookup(binding.name) != binding.body) return false;
        for (auto& entry : proof->evidence->witnesses) {
            auto& witness = entry.second;
            if (witness.node->pItem != witness.infon || !witness.facts.same(access_->typeFacts(witness.node)) ||
                witness.names != names(witness.node) || witness.work != work(witness.node) ||
                witness.list != listRequirement(witness.node) || witness.members != members(witness.node) ||
                witness.embedded != access_->definitionEmbedded(witness.node) ||
                witness.memberTemplate != memberTemplate(witness.node) || !POVDescription(witness.node, access_->registry.get()).relationships(false).empty()) return false;
        }
        return true;
    }
    bool covers(const Certificate& proof, const POVNode& node) const {
        if (!proof->requirement) return false;
        // Root-local template/occurrence relationships are not captured as a
        // witness of the target itself (which would retain its weak cache key).
        // Require their current source to have participated in this proof.
        if (auto source = memberTemplate(node)) if (!proof->evidence->witnesses.count(source)) return false;
        if (auto owner = occurrenceOwner(node)) if (!proof->evidence->witnesses.count(owner)) return false;
        const auto& types = proof->requirement->types;
        for (const auto& name : names(node))
            if (std::find(types.begin(), types.end(), name) == types.end()) return false;
        return true;
    }
    Certificate saved(const POVNode& node) const {
        auto found = evidence_.find(node->pItem); if (found == evidence_.end()) return {};
        for (const auto& proof : found->second) if (current(proof) && covers(proof, node)) return proof;
        return {};
    }
    bool accepts(const Certificate& proof, const POVNode& node) const {
        try { return ProteusDefinitionPlan(access_, node, proof->requirement).satisfied(); }
        catch (const std::runtime_error&) { return false; }
    }
    bool satisfied(const POVNode& node) const {
        if (!required(node)) return true;
        auto proof = saved(node); return proof && accepts(proof, node);
    }
    bool remember(const POVNode& node, const Certificate& proof) {
        auto& certificates = evidence_[node->pItem];
        certificates.erase(std::remove_if(certificates.begin(), certificates.end(),
            [&](const Certificate& old) { return !current(old); }), certificates.end());
        for (const auto& old : certificates) {
            if (old == proof) return false;
            if (sameDefinition(old->requirement, proof->requirement) &&
                old->evidence->bindings.size() == proof->evidence->bindings.size() &&
                old->evidence->witnesses.size() == proof->evidence->witnesses.size() &&
                std::all_of(proof->evidence->witnesses.begin(), proof->evidence->witnesses.end(),
                    [&](const auto& entry) { return old->evidence->witnesses.count(entry.first); }) &&
                std::all_of(proof->evidence->bindings.begin(), proof->evidence->bindings.end(), [&](const Binding& binding) {
                    return std::any_of(old->evidence->bindings.begin(), old->evidence->bindings.end(), [&](const Binding& other) {
                        return binding.name == other.name && binding.body == other.body;
                    });
                })) return false;
        }
        certificates.push_back(proof); return true;
    }
    std::shared_ptr<ProteusDefinitionPlan> plan(const POVNode& node, const Certificate& proof) const {
        if (!current(proof) || !covers(proof, node)) return {};
        return std::make_shared<ProteusDefinitionPlan>(access_, node, proof->requirement);
    }
    bool apply(const POVNode& node, const Certificate& proof, const std::shared_ptr<ProteusDefinitionPlan>& plan) {
        if (!current(proof) || !covers(proof, node) || !plan->supported) return false;
        bool changed = plan->apply();
        for (const auto& binding : plan->bindings) {
            auto child = std::make_shared<Proof>();
            child->evidence = proof->evidence; child->requirement = binding.second;
            changed |= remember(binding.first->node, child);
        }
        return changed;
    }
    std::shared_ptr<ProteusDefinitionPlan> reusable(const POVNode& target, const POVNode& source, const Certificate& proof) const {
        if (!current(proof)) return {};
        try {
            ProteusDefinitionPlan sourcePlan(access_, source, proof->requirement);
            if (!sourcePlan.dataSatisfied()) return {};
            auto targetPlan = std::make_shared<ProteusDefinitionPlan>(access_, target, proof->requirement);
            return targetPlan->dataSatisfied() ? targetPlan : nullptr;
        } catch (const std::runtime_error&) { return {}; }
    }
    bool canInherit(const POVNode& target, const POVNode& source) const {
        auto found = evidence_.find(source->pItem); if (found == evidence_.end()) return false;
        for (const auto& proof : found->second) if (reusable(target, source, proof)) return true;
        return false;
    }
    bool inherit(const POVNode& target, const POVNode& source) {
        auto found = evidence_.find(source->pItem); if (found == evidence_.end()) return false;
        auto certificates = found->second;
        bool changed = false;
        for (const auto& proof : certificates) if (auto plan = reusable(target, source, proof)) {
            // Only order metadata may remain: required contents already exist.
            changed |= plan->apply();
            for (const auto& binding : plan->bindings) {
                auto child = std::make_shared<Proof>();
                child->evidence = proof->evidence; child->requirement = binding.second;
                changed |= remember(binding.first->node, child);
            }
        }
        return changed;
    }
    void prune() {
        for (auto it = evidence_.begin(); it != evidence_.end();)
            if (it->first.expired()) it = evidence_.erase(it); else ++it;
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
        if (q.kind == definitionLeave) definitions_->finish(q.node, request.node, proof_);
        auto certificate = q.kind == typeRequirements ? definitions_->saved(request.node) : proof_;
        if (q.kind != definitionEnd && (q.kind != typeRequirements || !certificate || satisfied())) return {};
        auto plan = definitions_->plan(request.node, certificate);
        if (!plan || !plan->supported) return {};
        auto definitions = definitions_; auto target = request.node;
        return {{certificate->requirement->repeated ? "Apply repeated member requirements" :
            certificate->requirement->list ? "Apply finite list type requirements" : "Apply scalar type requirements",
            [definitions, target, certificate, plan] { return definitions->apply(target, certificate, plan); }, plan->work}};
    }
    bool satisfied() const override { return definitions_->satisfied(request.node); }
    bool symbolic() const override { return !satisfied(); }
};
