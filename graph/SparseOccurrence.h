#pragma once

// A virtual position remains virtual while it carries only its template's
// information. Once its value is supplied, publish that one occurrence in the
// stored list and retain the compressed remainder and its ordinal identities.
struct SparseOccurrence {
    static std::vector<SetCommand> commands(const std::shared_ptr<ProteusGraphAccess>& access,
                                           const std::shared_ptr<ProteusDefinitions>& definitions,
                                           const POVNode& item) {
        auto source=definitions->retainedOccurrenceOwner(item);
        if(!source || !access->sparse(source) || !access->subList(source) || !source->pParent ||
           !item->repetitionOccurrence || item->repetitionOccurrence->materializedPOV ||
           !access->typeFacts(item).hasValue) return {};
        Work proofWork{0,0};
        if(auto body=definitions->repeatedBody(source)) {
            auto proof=definitions->describe(body,proofWork);
            if(!proof) return {};
            auto combined=access->typeFacts(item);
            if(!combined.combine(proof->requirement->facts))
                throw DefinitionConflict("incompatible sparse occurrence value");
            // A template's existing constant is not a newly supplied value.
            // Reading it must leave both the run and canonical POV untouched.
            if(proof->requirement->facts.hasValue) return {};
        }
        auto index=POVDescription(item).occurrenceIndex();
        if(index<0 || index>=access->listExtent(source)) return {};
        return {{"Publish valued sparse occurrence",[access,definitions,source,item,index] {
            auto span=index ? source->splitSparseSpanPrefix(index) : source;
            if(!span) return false;
            definitions->retainOccurrenceOwner(span);
            auto exposed=span->exposeSparseHeadWithItem(readView(item));
            if(!exposed) return false;
            access->bind(exposed);
            ++source->workRevision;++exposed->workRevision;
            if(exposed->pParent) {++exposed->pParent->workRevision;readView(exposed->pParent)->noteNormalizationWork();}
            return true;
        },{addCost(proofWork.visits,1),3}}};
    }
};
