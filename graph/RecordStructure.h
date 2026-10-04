#pragma once

// Supplied unordered parts have no positional correspondence. An existing
// identity can pair members without inventing an ordering or equating names.
// Nominal correspondence is planned separately before publishing identities.
struct RecordStructure {
    static bool record(ProteusGraphAccess& access,const POVNode& node) {
        return access.physicalList(node) && readView(node)->orderMode==uUnordered;
    }
    static bool members(ProteusGraphAccess& access,const POVNode& node,
                        std::vector<POVNode>& result,Work& work) {
        if(!record(access,node) || access.listExtent(node)==-2) return false;
        std::set<POVNode> visited;
        for(auto member=POVDescription(node).member();member;member=POVDescription(member).adjacent()) {
            ++work.visits;
            if(!visited.insert(member).second || access.subList(member)) return false;
            result.push_back(member);
        }
        auto extent=access.listExtent(node);
        if(extent>=0 && static_cast<std::uint64_t>(extent)<result.size())
            throw DefinitionConflict("unordered members exceed explicit extent");
        return true;
    }
    // Inherited views also carry useful typed payloads. Reading their facts
    // must not change the separate policy for expanding written definitions.
    static bool typedParts(ProteusGraphAccess& access,const POVNode& node,Work& work) {
        if(!record(access,node) || access.memberTemplate(node) || access.occurrenceOwner(node)) return false;
        std::vector<POVNode> parts;
        if(!members(access,node,parts,work) || parts.empty()) return false;
        return std::all_of(parts.begin(),parts.end(),[&](const auto& part) {
            return access.hasTypeRequirements(part) || WordForms::collectionType(access,part);
        });
    }
    template<class Match>
    static bool pairs(ProteusGraphAccess& access,const std::vector<POVNode>& left,
                      const std::vector<POVNode>& right,std::vector<std::pair<POVNode,POVNode>>& paired,
                      std::vector<POVNode>& missing,Work& work,Match match) {
        MemberCorrespondence mapping(left.size(),right.size(),[&](std::size_t a,std::size_t b) {
            return access.sameInfon(left[a],right[b]) || access.hasDirectIdentity(left[a],right[b]);
        },match,work);
        if(!mapping.unambiguous) return false;
        for(const auto& pair:mapping.pairs) paired.push_back({left[pair.first],right[pair.second]});
        for(auto index:mapping.missing) missing.push_back(right[index]);
        return true;
    }
};
