#pragma once

// Included in namespace proteus4 after the generated domain declarations.
// A live, goal-independent view of existing storage. No nodes, registry
// records or persistent adjacency are created by inspection. Relationship
// descriptors retain the original endpoints and evidence; they take no goal.
// Read under the graph context lock; registry enumeration also takes its lock.
class POVDescription {
public:
    struct Relationship {
        POVNode owner, target;
        // A null record denotes the retained work POV, including all its flags.
        // Otherwise this is the actual registry record, with its provenance.
        std::shared_ptr<IdentityRelation> identity;
        bool isWork() const { return !identity; }
    };
private:
    POVNode node_;
    IdentityRegistry* registry_;
public:
    explicit POVDescription(POVNode node, IdentityRegistry* registry = nullptr)
        : node_(std::move(node)), registry_(registry) {}
    const POVNode& adjacent(bool reverse = false) const { return reverse ? node_->prev : node_->next; }
    POV* parent() const { return node_->pParent; }
    const POVNode& alternative() const { return node_->altPOV; }
    const POVNode& outer() const { return node_->outerPOV; }
    const std::shared_ptr<infonState>& size() const { return node_->pItem->infSize; }
    const std::shared_ptr<infonState>& value() const { return node_->pItem->value; }
    const std::shared_ptr<infonView>& memberSpecification() const { return value()->listSpec; }
    static POVNode boundary(const std::shared_ptr<infonState>& body, bool last = false) {
        return last ? body->items.tail : body->items.head;
    }
    POVNode member(bool last = false) const { return boundary(value(), last); }
    // Availability and lookup are read-only; materialization stays a priced Set
    // or edge operation owned by the graph runtime.
    bool hasVirtualMembers() const { return node_->hasSparseOrderedSpanValue(); }
    std::int64_t virtualExtent() const { return node_->sparseSpanCount(); }
    POVNode existingOccurrence(std::int64_t index) const { return node_->findImplicitItemPOV(index); }
    const std::vector<POVNode>& work() const { return node_->pItem->wrkList; }
    std::vector<Relationship> relationships(bool includeWork = true) const {
        std::vector<Relationship> result;
        if (includeWork) for (const auto& rhs : work()) result.push_back({node_, rhs, {}});
        if (registry_) {
            std::lock_guard<std::mutex> lock(registry_->registryMutex);
            for (const auto& record : registry_->identities) {
                if (record->leftPOV == node_) result.push_back({node_, record->rightPOV, record});
                else if (record->rightPOV == node_) result.push_back({node_, record->leftPOV, record});
            }
        }
        return result;
    }
    bool hasIdentity(const POVNode& target) const {
        if (node_ == target) return true;
        for (const auto& relation : relationships(false)) if (relation.target == target) return true;
        return false;
    }
};
