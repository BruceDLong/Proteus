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

// Primary words and explicit additional types have different roles. Two
// descriptions sharing only an additional type are not nominally equivalent.
struct NominalDescription {
    std::string primary;
    std::vector<std::string> types;
};

// Immutable definition-side analysis. Candidate sets encode compareNominal's
// three positive cases; absent names do not become evidence of a mismatch.
class NominalMemberIndex {
    std::map<std::string,std::vector<std::size_t>> primary_, advertised_;
public:
    const std::vector<NominalDescription> descriptions;
    bool distinguishable=true;
    std::vector<std::size_t> candidates(const NominalDescription& member,Work& work) const {
        std::vector<std::size_t> result;
        auto append=[&](const auto& index,const std::string& name) {
            ++work.visits;
            auto found=index.find(name);if(found==index.end()) return;
            work.visits=addCost(work.visits,found->second.size());
            result.insert(result.end(),found->second.begin(),found->second.end());
        };
        append(advertised_,member.primary);
        for(const auto& name:member.types) append(primary_,name);
        std::sort(result.begin(),result.end());
        result.erase(std::unique(result.begin(),result.end()),result.end());
        return result;
    }
    NominalMemberIndex(std::vector<NominalDescription> members,Work& work)
        :descriptions(std::move(members)) {
        for(std::size_t i=0;i<descriptions.size();++i) {
            const auto& member=descriptions[i];++work.visits;
            if(member.primary.empty()) {distinguishable=false;continue;}
            primary_[member.primary].push_back(i);
            auto names=member.types;names.push_back(member.primary);
            std::sort(names.begin(),names.end());names.erase(std::unique(names.begin(),names.end()),names.end());
            for(const auto& name:names) {++work.visits;advertised_[name].push_back(i);}
        }
        if(distinguishable) for(const auto& member:descriptions)
            if(candidates(member,work).size()!=1) {distinguishable=false;break;}
    }
};

// A finite correspondence proof, not a permutation search. Existing identities
// outrank nominal candidates. Otherwise every supplied member needs a unique,
// proved match; uncertain or competing routes remain pending.
struct MemberCorrespondence {
    std::vector<std::pair<std::size_t,std::size_t>> pairs;
    std::vector<std::size_t> missing;
    bool unambiguous=true;
    MemberCorrespondence()=default;
    // Record requirements have no exact-identity override. Read each actual
    // type set once, then join only nominal candidates from the shared index.
    template<class Read>
    static MemberCorrespondence nominal(const NominalMemberIndex& required,std::size_t actualCount,Read read,Work& work) {
        MemberCorrespondence result;
        if(!required.distinguishable) {result.unambiguous=false;return result;}
        if(required.descriptions.empty()) return result;
        std::vector<std::size_t> found(required.descriptions.size(),actualCount);
        for(std::size_t actual=0;actual<actualCount;++actual) {
            ++work.visits;auto description=read(actual);
            if(description.primary.empty()) {result.unambiguous=false;return result;}
            auto candidates=required.candidates(description,work);
            // One actual matching two requirements is just as ambiguous as
            // two actuals matching one requirement. Preserve both checks.
            if(candidates.size()>1) {result.unambiguous=false;return result;}
            if(candidates.empty()) continue;
            auto source=candidates.front();
            if(found[source]!=actualCount) {result.unambiguous=false;return result;}
            found[source]=actual;
        }
        for(std::size_t source=0;source<found.size();++source) {
            ++work.visits;
            if(found[source]==actualCount) result.missing.push_back(source);
            else result.pairs.push_back({found[source],source});
        }
        return result;
    }
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
