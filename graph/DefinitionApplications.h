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
        bool list = false, embedded = false, open = false;
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
    mutable POVNode unspecifiedMember_;
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
        return access_->positionalDefinitionList(node) || access_->repeatedDefinitionList(node);
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
    bool declared(const std::string& name) const { return catalogue_.count(name)!=0; }
    // Missing declarations impose no constraints. Type identity still exists;
    // an unavailable definition never equates distinct written types.
    bool optional(const POVNode& node) const {
        for (const auto& name:names(node)) if (declared(name)) return false;
        return true;
    }
    POVNode lookup(const std::string& name) const {
        auto found = catalogue_.find(name);
        if (found == catalogue_.end() || found->second.size() != 1) return {};
        const auto& entry = found->second.front();
        if (access_->definitionName(entry.declaration) != name || !registered(entry.declaration)) return {};
        return entry.body;
    }
    POVNode repeatedBody(const POVNode& node) const { return memberTemplate(node); }
    // An untyped virtual position has no stored POV. One read-only unknown
    // describes that position; occurrence data remains in the search cursor.
    POVNode matchingBody(const POVNode& node) const {
        if (auto spec = memberTemplate(node)) return spec;
        if (!access_->openTail(node)) return {};
        if (!unspecifiedMember_) {
            auto item = std::make_shared<infonView>();
            SetModeBits(item->flags, infonView::viewMode, infonView::vmOverlay);
            SetModeBits(item->flags, infonView::itmMode, infonView::mItem);
            SetModeBits(item->infSize->flags, infonState::evalMode, infonState::emUnknown);
            SetModeBits(item->value->flags, infonState::overlayType, infonState::tUnknown);
            SetModeBits(item->value->flags, infonState::evalMode, infonState::emUnknown);
            unspecifiedMember_ = access_->requirementView(item);
        }
        return unspecifiedMember_;
    }
    POVNode payload(const POVNode& node) const { return view(node->pItem); }
    POVNode selectionBody(const POVNode& node) const { return view(access_->selectionSource(node)); }
    void retainOccurrenceOwner(const POVNode& owner) { occurrenceOwners_.insert(owner); }
    POVNode retainedOccurrenceOwner(const POVNode& node) const {
        return retainedOwner(POVDescription(node).occurrenceSource());
    }
    POVNode retainedOwner(POV* source) const {
        for (const auto& owner : occurrenceOwners_) if (owner.get() == source) return owner;
        return {};
    }
    // A supplied description of typed parts is useful on its own. Normalizing
    // it demands its present members, not every part of the container's type.
    // Bare type references and positional value sequences still expand through
    // their definitions; explicit RHSs remain ordinary graph constraints.
    bool describesParts(const POVNode& node) const {
        if (!access_->physicalList(node) || access_->memberTemplate(node) || access_->occurrenceOwner(node)) return false;
        SpanLayout<POVNode> layout;
        std::uint64_t visits = 0;
        if (!readListLayout(*access_, node, layout, visits,
                [&](const auto& at) { return at.phase != ListPhase::enter || access_->physicalList(at.node); },
                [](const POVNode& member) { return member; })) return false;
        bool supplied = false;
        for (const auto& segment : layout.segments) if (!segment.gap) {
            if (!access_->hasTypeRequirements(segment.node)) return false;
            supplied = true;
        }
        return supplied;
    }
    bool required(const POVNode& node) const {
        return (access_->hasTypeRequirements(node) && !optional(node)) ||
            (access_->memberTemplate(node) && !access_->fedTemplate(node)) || access_->occurrenceOwner(node);
    }
    bool children(const POVNode& node, const POVNode& root, std::vector<POVNode>& result) const {
        if (!(node == root ? access_->predicateFacts(node,false) : access_->typeFacts(node)).valid) return false;
        if (node != root && !access_->scalarTypeShape(node) && !listRequirement(node)) return false;
        // Registry constraints within a shared body need a read-only relation
        // protocol. Keep them visible as deferred work rather than ignoring them.
        if (node != root && !POVDescription(node, access_->registry.get()).relationships(false).empty()) return false;
        if (node != root && access_->definitionEmbedded(node) && access_->repeatedDefinitionList(node)) return false;
        auto add = [&](const POVNode& child) {
            if (std::find(result.begin(), result.end(), child) == result.end()) result.push_back(child);
        };
        for (const auto& name : names(node)) {
            auto body = lookup(name);
            if (body) add(body);
            else if (declared(name)) return false;
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
            auto body = lookup(name);
            auto& bindings = proof->evidence->bindings;
            if (std::none_of(bindings.begin(), bindings.end(), [&](const Binding& b) { return b.name == name; }))
                bindings.push_back({name, body});
        }
        if (node == root || proof->evidence->witnesses.count(node)) return;
        proof->evidence->witnesses.emplace(node, Witness{node, node->pItem, access_->typeFacts(node),
            names(node), work(node), members(node), listRequirement(node), access_->definitionEmbedded(node), access_->openTail(node), memberTemplate(node)});
    }
    // The graph's leave edge supplies postorder: every required child has
    // already been walked. Compile requirements, never execute child normalize.
    void finish(const POVNode& node, const POVNode& root, const Certificate& proof) {
        if (!proof->compilable) return;
        auto& compiled = proof->evidence->compiled;
        auto requirement = std::make_shared<DefinitionRequirement>();
        requirement->facts.valid = true;
        requirement->types = names(node);
        requirement->captureLabel(node);
        std::sort(requirement->types.begin(), requirement->types.end());
        if (node != root) {
            auto& witness = proof->evidence->witnesses.at(node);
            requirement->facts = witness.facts;
            requirement->list = witness.list; requirement->open = witness.list && witness.open;
            for (const auto& member : witness.members) {
                auto child = compiled.at(member);
                if (access_->definitionEmbedded(member)) {
                    if (!child->list) throw DefinitionConflict("non-list embedded definition span");
                    // A named alias may reveal repetition only after its walk.
                    // Never mistake its empty physical prefix for zero items.
                    if (child->open) { proof->compilable = false; return; }
                    requirement->members.insert(requirement->members.end(), child->members.begin(), child->members.end());
                } else requirement->members.push_back(child);
            }
            if (access_->positionalDefinitionList(node) && !requirement->open) {
                GraphTypeFacts extent;
                extent.valid = true; extent.hasSize = true;
                extent.size = static_cast<std::int64_t>(requirement->members.size());
                if (!requirement->facts.combine(extent)) throw DefinitionConflict("incompatible finite definition extent");
            }
        }
        if (auto source = memberTemplate(node)) {
            auto repeated = std::make_shared<DefinitionRequirement>();
            repeated->facts.valid = true; repeated->facts.overlay = infonState::LST;
            repeated->list = true; repeated->open = true; repeated->repeated = compiled.at(source); repeated->templateSource = source;
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
        for (const auto& name : names(node))
            if (auto body=lookup(name)) requirement = combineDefinitions(requirement, compiled.at(body));
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
    // Compile one selected declaration subtree, including its written facts.
    // A null instance root lets the existing compiler treat every visited POV
    // as read-only source data. Unselected siblings are never compiled.
    Certificate describe(const POVNode& source, Work& work) {
        auto proof=std::make_shared<Proof>();
        std::vector<std::pair<POVNode,bool>> pending{{source,false}};
        std::set<POVNode> active;
        while(!pending.empty()) {
            auto [node,leave]=pending.back();pending.pop_back();++work.visits;
            if(leave) {finish(node,{},proof);active.erase(node);if(!proof->compilable) return {};continue;}
            if(proof->evidence->compiled.count(node)) continue;
            if(!active.insert(node).second) return {};
            std::vector<POVNode> next;
            if(!children(node,{},next)) return {};
            collect(node,{},proof);pending.push_back({node,true});
            for(auto i=next.rbegin();i!=next.rend();++i) pending.push_back({*i,false});
        }
        proof->requirement=proof->evidence->compiled.at(source);
        proof->evidence->compiled.clear();return proof;
    }
    bool current(const Certificate& proof) const {
        if (!proof->requirement) return false;
        for (const auto& binding : proof->evidence->bindings)
            if (lookup(binding.name) != binding.body || (!binding.body && declared(binding.name))) return false;
        for (auto& entry : proof->evidence->witnesses) {
            auto& witness = entry.second;
            if (witness.node->pItem != witness.infon || !witness.facts.same(access_->typeFacts(witness.node)) ||
                witness.names != names(witness.node) || witness.work != work(witness.node) ||
                witness.list != listRequirement(witness.node) || witness.members != members(witness.node) ||
                witness.embedded != access_->definitionEmbedded(witness.node) || witness.open != access_->openTail(witness.node) ||
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
    // Meaning evidence can be compiled without applying it to a candidate or
    // a shared grammar template. Nominal compatibility is checked separately.
    GraphTypeFacts scalarFacts(const POVNode& node, bool predicate = false) const {
        auto facts = predicate ? access_->predicateFacts(node,false) : access_->typeFacts(node);
        if (!facts.valid || !access_->hasTypeRequirements(node) || optional(node)) return facts;
        auto proof = saved(node);
        if (!proof || proof->requirement->list || !facts.combine(proof->requirement->facts)) facts.valid = false;
        return facts;
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
        bool entry = q.kind == typeRequirements || q.kind == compileTypeRequirements;
        auto certificate = entry ? definitions_->saved(request.node) : proof_;
        if (q.kind != definitionEnd && (!entry || !certificate || satisfied())) return {};
        if (request.kind == compileTypeRequirements) {
            if (!certificate || !certificate->compilable || !certificate->requirement) return {};
            auto definitions = definitions_; auto target = request.node;
            return {{"Record read-only type meaning", [definitions, target, certificate] {
                return definitions->remember(target, certificate);
            }, {1, 0}}};
        }
        auto plan = definitions_->plan(request.node, certificate);
        if (!plan || !plan->supported) return {};
        auto definitions = definitions_; auto target = request.node;
        return {{certificate->requirement->repeated ? "Apply repeated member requirements" :
            certificate->requirement->list ? "Apply finite list type requirements" : "Apply scalar type requirements",
            [definitions, target, certificate, plan] { return definitions->apply(target, certificate, plan); }, plan->work}};
    }
    bool satisfied() const override {
        return request.kind == compileTypeRequirements ?
            (!definitions_->required(request.node) || bool(definitions_->saved(request.node))) : definitions_->satisfied(request.node);
    }
    bool symbolic() const override { return !satisfied(); }
};
