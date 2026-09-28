#pragma once

// Missing evidence must remain distinct from a proved mismatch.
enum class ScalarMatch { yes, no, unknown };

// Nominal compatibility follows Proteus3's primary/added-type rule. Explicit
// definition aliases extend those type sets; a shared structural shape or a
// shared ancestor alone does not make two sibling types interchangeable.
inline ScalarMatch compareNominal(const std::string& required,const std::vector<std::string>& requiredTypes,
                                  const std::string& actual,const std::vector<std::string>& actualTypes) {
    if (required.empty()) return ScalarMatch::yes;
    if (actual.empty()) return ScalarMatch::unknown;
    if (required==actual || std::find(actualTypes.begin(),actualTypes.end(),required)!=actualTypes.end() ||
        std::find(requiredTypes.begin(),requiredTypes.end(),actual)!=requiredTypes.end()) return ScalarMatch::yes;
    return ScalarMatch::no;
}

// A finite correspondence proof, not a permutation search. Existing identities
// outrank nominal candidates. Otherwise every supplied member needs a unique,
// proved match; uncertain or competing routes remain pending.
struct MemberCorrespondence {
    std::vector<std::pair<std::size_t,std::size_t>> pairs;
    std::vector<std::size_t> missing;
    bool unambiguous=true;
    template<class Exact,class Match>
    MemberCorrespondence(std::size_t left,std::size_t right,Exact exact,Match match,Work& work) {
        std::set<std::size_t> used;
        for(std::size_t source=0;source<right;++source) {
            std::size_t found=left;
            for(std::size_t target=0;target<left;++target) {
                ++work.visits;
                if(!exact(target,source)) continue;
                if(found!=left) {unambiguous=false;return;}
                found=target;
            }
            if(found==left) for(std::size_t target=0;target<left;++target) {
                ++work.visits;
                auto verdict=match(target,source);
                if(verdict==ScalarMatch::unknown) {unambiguous=false;return;}
                if(verdict!=ScalarMatch::yes) continue;
                if(found!=left) {unambiguous=false;return;}
                found=target;
            }
            if(found==left) missing.push_back(source);
            else if(!used.insert(found).second) {unambiguous=false;return;}
            else pairs.push_back({found,source});
        }
    }
};
