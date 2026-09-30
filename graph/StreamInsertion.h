#pragma once

// Insertion is a graph mutation followed by an external effect. Claim the
// placeholder while locked; connect outside the lock without waiting for EOF.
// The parser publishes into this same infon and wakes its dependent searches.
class StreamInsertion {
    std::shared_ptr<ProteusGraphAccess> access_;
    std::vector<std::function<void()>> effects_;
public:
    explicit StreamInsertion(std::shared_ptr<ProteusGraphAccess> access) : access_(std::move(access)) {}
    std::vector<SetCommand> found(const POVNode& node) {
        if (getModeBits(node->pItem->flags,infonView::itmMode)!=infonView::mIncludeStream) return {};
        return {{"Start inserted stream",[this,node] {
            auto item=node->pItem;
            if (getModeBits(item->flags,infonView::itmMode)!=infonView::mIncludeStream) return false;
            auto core=access_->replyCore;
            if (!core) throw std::runtime_error("stream insertion requires a ProteusCore");
            auto origin=core->streamSourceOriginID(node,0);
            auto spec=item->includeSpec;
            SetModeBits(item->flags,infonView::itmMode,infonView::mItem);
            access_->prepareInput(item);
            effects_.push_back([core,item,spec,origin] {
                try {
                    if (core->requestStreamInsert(item,spec,origin)) return;
                    core->agent->graphNormalizer.failInput(item,"Could not insert stream: "+spec);
                } catch (const std::exception& error) {
                    core->agent->graphNormalizer.failInput(item,"Could not insert stream: "+spec+": "+error.what());
                }
            });
            return true;
        },{1,1}}};
    }
    std::vector<std::function<void()>> takeEffects() {
        auto result=std::move(effects_);effects_.clear();return result;
    }
};
