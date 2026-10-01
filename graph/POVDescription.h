#pragma once

// Included in namespace proteus4 after the generated domain declarations.
// A live, goal-independent view of existing storage. No nodes, registry
// records or persistent adjacency are created by inspection. Relationship
// descriptors retain the original endpoints and evidence; they take no goal.
// Read under the graph context lock; registry enumeration also takes its lock.
inline void observeInfon(const std::shared_ptr<infonView>& view) {
    if(!view) return;
    ReadScope::observeObject(view.get());
    ReadScope::observeObject(view->pathSelection.get());
    ReadScope::observeObject(view->innr.get());
}
inline std::shared_ptr<infonView>& readView(POV* node) {
    ReadScope::observeObject(node);
    observeInfon(node->pItem);
    return node->pItem;
}
inline std::shared_ptr<infonView>& readView(const POVNode& node) {return readView(node.get());}

class POVDescription {
public:
    struct Relationship {
        POVNode owner, target;
        // Work and replaceable result links live in POV structure. Permanent
        // identities retain the actual registry record and its provenance.
        std::shared_ptr<IdentityRelation> identity;
        bool derived = false, consolidated = false, valueOnly = false;
        bool isWork() const { return consolidated || (!identity && !derived); }
        const char* description() const {
            return valueOnly ? "temporal value reference" : consolidated ? "consolidated equality" : identity ? "registry identity" : derived ? "selected result" : "work-list constraint";
        }
    };
private:
    POVNode node_;
    IdentityRegistry* registry_;
public:
    explicit POVDescription(POVNode node, IdentityRegistry* registry = nullptr)
        : node_(std::move(node)), registry_(registry) {if(node_) readView(node_);}
    const POVNode& adjacent(bool reverse = false) const { return reverse ? node_->prev : node_->next; }
    POV* parent() const { return node_->pParent; }
    const POVNode& alternative() const { return node_->altPOV; }
    const POVNode& outer() const { return node_->outerPOV; }
    const std::shared_ptr<infonState>& size() const { return readView(node_)->infSize; }
    const std::shared_ptr<infonState>& value() const { return readView(node_)->value; }
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
    const std::vector<POVNode>& work() const { return readView(node_)->wrkList; }
    std::vector<Relationship> relationships(bool includeWork = true) const {
        std::vector<Relationship> result;
        const auto& view=readView(node_);
        if (includeWork && view->temporalValueSource)
            result.push_back({node_,view->temporalValueSource,{},true,false,true});
        if (includeWork) for (const auto& rhs : work()) {
            // A published view borrows its source's header. Those RHSs still
            // belong to the source occurrence; the derived link exposes them
            // there. Reinterpreting them here would change cursor context.
            auto field=view->pathSelection;
            if(field && field->resultView && field->result && !field->pending && field->matches.size()==1 &&
               field->resultView->workListContainsPOV(rhs)) continue;
            result.push_back({node_, rhs, {}});
        }
        // A query result may change when explicitly normalized again. Read its
        // current source link here instead of asserting a permanent identity
        // between the working query POV and every result it has ever selected.
        if (auto field=view->pathSelection)
            if (field->result && !field->pending && field->matches.size()==1 && field->result!=node_)
                result.push_back({node_,field->result,{},true});
        if (registry_) {
            std::lock_guard<std::mutex> lock(registry_->registryMutex);
            for (const auto& record : registry_->identities) {
                bool leftWork=record->leftWorkOwner==view;
                bool rightWork=record->rightWorkOwner==view;
                if (leftWork || record->leftPOV == node_)
                    result.push_back({node_, record->rightPOV, record, false, leftWork});
                else if (rightWork || record->rightPOV == node_)
                    result.push_back({node_, record->leftPOV, record, false, rightWork});
            }
        }
        return result;
    }
    std::vector<std::shared_ptr<ExternalAssociationReference>> associations(const std::string& type) const {
        std::vector<std::shared_ptr<ExternalAssociationReference>> result;
        if(registry_) registry_->collectExternalAssociationsForAnchor(node_,type,&result);
        return result;
    }
    std::shared_ptr<BoundaryReference> boundaryReference() const {
        if(!registry_) return {};
        std::lock_guard<std::mutex> lock(registry_->registryMutex);
        auto found=registry_->boundaries.find(node_);
        return found==registry_->boundaries.end() ? nullptr : found->second;
    }
    std::shared_ptr<EndRelativeReference> endReference() const {
        if(!registry_) return {};
        std::lock_guard<std::mutex> lock(registry_->registryMutex);
        for(const auto& reference:registry_->endRelativeReferences)
            if(!getFlagBit(reference->flags,EndRelativeReference::retired) && reference->referencePOV==node_) return reference;
        return {};
    }
    bool hasIdentity(const POVNode& target) const {
        if (node_ == target) return true;
        for (const auto& relation : relationships(false)) if (relation.target == target) return true;
        return false;
    }
};
