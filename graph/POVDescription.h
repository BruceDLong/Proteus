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
        // Work and replaceable result links live in POV structure. Permanent
        // identities retain the actual registry record and its provenance.
        std::shared_ptr<IdentityRelation> identity;
        bool derived = false, consolidated = false;
        bool isWork() const { return consolidated || (!identity && !derived); }
        const char* description() const {
            return consolidated ? "consolidated equality" : identity ? "registry identity" : derived ? "selected result" : "work-list constraint";
        }
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
    POV* occurrenceSource() const { return node_->implicitItem.sourcePov; }
    std::int64_t occurrenceIndex() const { return node_->implicitItem.sourceIndex; }
    const std::vector<POVNode>& work() const { return node_->pItem->wrkList; }
    std::vector<Relationship> relationships(bool includeWork = true) const {
        std::vector<Relationship> result;
        if (includeWork) for (const auto& rhs : work()) {
            // A published view borrows its source's header. Those RHSs still
            // belong to the source occurrence; the derived link exposes them
            // there. Reinterpreting them here would change cursor context.
            auto field=node_->pItem->pathSelection;
            if(field && field->resultView && field->result && !field->pending && field->matches.size()==1 &&
               field->resultView->workListContainsPOV(rhs)) continue;
            result.push_back({node_, rhs, {}});
        }
        // A query result may change when explicitly normalized again. Read its
        // current source link here instead of asserting a permanent identity
        // between the working query POV and every result it has ever selected.
        if (auto field=node_->pItem->pathSelection)
            if (field->result && !field->pending && field->matches.size()==1 && field->result!=node_)
                result.push_back({node_,field->result,{},true});
        if (registry_) {
            std::lock_guard<std::mutex> lock(registry_->registryMutex);
            for (const auto& record : registry_->identities) {
                bool leftWork=record->leftWorkOwner==node_->pItem;
                bool rightWork=record->rightWorkOwner==node_->pItem;
                if (leftWork || record->leftPOV == node_)
                    result.push_back({node_, record->rightPOV, record, false, leftWork});
                else if (rightWork || record->rightPOV == node_)
                    result.push_back({node_, record->leftPOV, record, false, rightWork});
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
