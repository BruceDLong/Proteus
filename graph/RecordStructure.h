#pragma once

// Supplied unordered parts have no positional correspondence. An existing
// identity can pair members without inventing an ordering or equating names.
// Independently supplied members still need a semantic matching plan.
struct RecordStructure {
    static bool record(ProteusGraphAccess& access,const POVNode& node) {
        return access.physicalList(node) && node->pItem->orderMode==uUnordered;
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
    static bool pairs(ProteusGraphAccess& access,const std::vector<POVNode>& left,
                      const std::vector<POVNode>& right,std::vector<std::pair<POVNode,POVNode>>& paired,
                      std::vector<POVNode>& missing,Work& work) {
        std::set<POVNode> used;
        for(const auto& source:right) {
            POVNode match;
            for(const auto& target:left) {
                ++work.visits;
                if(access.sameInfon(target,source) || access.hasDirectIdentity(target,source)) {
                    if(match || used.count(target)) return false;
                    match=target;
                }
            }
            if(match) {used.insert(match);paired.push_back({match,source});}
            else missing.push_back(source);
        }
        return used.size()==left.size();
    }
};
