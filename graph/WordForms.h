#pragma once

// The existing English inflector supplies spelling rules. This component owns
// the small representation rules; the scheduler and field walk only request
// their facts or publish their Sets.
class WordForms {
public:
    static std::string pluralSingular(const std::string& word) {
        if(word.empty() || !::inflect.is_plural(word) || ::inflect.is_singular(word)) return {};
        auto singular=::inflect.convert_to_singular(word);
        return singular!=word && !singular.empty() &&
            ::inflect.convert_to_modern_plural(singular)==word ? singular : std::string{};
    }
    static std::shared_ptr<WordUse> collectionType(ProteusGraphAccess& access,const POVNode& node) {
        if(!access.literalList(node) || node->pItem->type) return {};
        auto spec=node->pItem->value->listSpec;
        if(!spec || !spec->type || spec->type->asGiven.empty() || getFlagBit(spec->type->flags,WordUse::plural)) return {};
        auto plural=::inflect.convert_to_modern_plural(spec->type->normalType());
        // Invariant spellings do not distinguish the member from its
        // collection. Assigning that same type could apply a scalar meaning.
        if(plural.empty() || plural==spec->type->normalType()) return {};
        auto type=std::make_shared<WordUse>();
        type->setAsGiven(plural);
        type->modelID=spec->type->modelID;
        SetFlagBit(type->flags,WordUse::plural,true);
        return type;
    }
    // In a field description the noun's plural form describes a collection.
    // An explicit declaration takes precedence over inferred English spelling.
    static bool collectionPredicate(const POVNode& node,ProteusDefinitions& definitions) {
        auto type=node->pItem->type;
        return type && type->modelID.empty() && !definitions.declared(type->typeID()) &&
            !pluralSingular(type->normalType()).empty();
    }
    static std::vector<SetCommand> commands(std::shared_ptr<ProteusGraphAccess> access,const POVNode& node) {
        if(auto type=collectionType(*access,node)) {
            return {{"Set collection type",[node,type] {
                if(node->pItem->type) return false;
                node->pItem->type=type;SetFlagBit(node->pItem->flags,infonView::wordDerefed,true);
                ++node->workRevision;return true;
            },{1,1}}};
        }
        auto item=node->pItem;
        if(!access->scalar(node) || !item->type || item->type->normalType()!="english-text" ||
           getModeBits(item->value->flags,infonState::overlayType)!=infonState::STR ||
           getModeBits(item->value->flags,infonState::evalMode)!=infonState::emLiteral ||
           !POVDescription(node,access->registry.get()).relationships().empty()) return {};
        std::vector<std::string> words;
        const auto& text=item->value->str;
        for(std::size_t start=0;start<text.size();) {
            auto end=text.find(' ',start);if(end==std::string::npos) end=text.size();
            if(end>start) words.push_back(text.substr(start,end-start));
            start=end+1;
        }
        return {{"Set English phrase words",[access,node,words] {
            auto item=node->pItem;
            item->type=std::make_shared<WordUse>();item->type->setAsGiven("english-phrase");
            item->value=std::make_shared<infonState>();
            SetModeBits(item->value->flags,infonState::overlayType,infonState::LST);
            SetModeBits(item->value->flags,infonState::evalMode,infonState::emLiteral);
            item->value->sizeMode=fromCount;item->orderMode=uOrdered;
            item->infSize=std::make_shared<infonState>();
            SetModeBits(item->infSize->flags,infonState::overlayType,infonState::NUM);
            SetModeBits(item->infSize->flags,infonState::evalMode,infonState::emLiteral);
            item->infSize->num=static_cast<std::int64_t>(words.size());
            for(const auto& word:words) {
                auto member=access->appendDefinitionMember(node,word);
                SetModeBits(member->pItem->flags,infonView::viewMode,infonView::vmAny);
            }
            // The template describes the words without requiring a dictionary
            // definition or inventing meanings for the individual spellings.
            auto spec=std::make_shared<infonView>();
            SetModeBits(spec->flags,infonView::viewMode,infonView::vmAny);
            SetModeBits(spec->flags,infonView::itmMode,infonView::mItem);
            SetModeBits(spec->value->flags,infonState::overlayType,infonState::tUnknown);
            SetModeBits(spec->value->flags,infonState::evalMode,infonState::emUnknown);
            SetModeBits(spec->infSize->flags,infonState::evalMode,infonState::emUnknown);
            spec->type=std::make_shared<WordUse>();spec->type->setAsGiven("word");
            item->value->listSpec=spec;
            SetFlagBit(item->flags,infonView::isNormed,false);++node->workRevision;return true;
        },{text.size()+1,words.size()*3+4}}};
    }
};
