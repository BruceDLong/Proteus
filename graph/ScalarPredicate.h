#pragma once

// Both sequence predicates and field queries need proof of exclusion. Missing
// facts are not a mismatch, even when another candidate is already available.
enum class ScalarMatch { yes, no, unknown };
inline ScalarMatch compareScalar(GraphTypeFacts required, const GraphTypeFacts& actual) {
    if (!required.valid || !actual.valid) return ScalarMatch::unknown;
    if (required.accepts(actual)) return ScalarMatch::yes;
    return required.combine(actual) ? ScalarMatch::unknown : ScalarMatch::no;
}

// A predicate asks whether all available numeric possibilities satisfy it.
// A partial overlap is unknown, not proof of exclusion.
inline ScalarMatch compareNumeric(const NumericView& required,const NumericView& actual,GraphTypeFacts actualFacts={}) {
    if(!actual.bounds.valid && actualFacts.valid && actualFacts.overlay>=0 && actualFacts.overlay!=infonState::NUM) return ScalarMatch::no;
    if(!required.bounds.valid || !actual.bounds.valid) return ScalarMatch::unknown;
    if(required.bounds.empty || actual.bounds.empty) return ScalarMatch::no;
    if(required.bounds.contains(actual.bounds)) return ScalarMatch::yes;
    return required.bounds.intersect(actual.bounds).empty ? ScalarMatch::no : ScalarMatch::unknown;
}

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
inline ScalarMatch compareNominal(const ProteusDefinitions& definitions,const POVNode& required,const POVNode& actual) {
    auto left=definitions.names(required),right=definitions.names(actual);
    auto leftName=left.empty() ? std::string{} : left.front(),rightName=right.empty() ? std::string{} : right.front();
    if(auto proof=definitions.saved(required)) left.insert(left.end(),proof->requirement->types.begin(),proof->requirement->types.end());
    if(auto proof=definitions.saved(actual)) right.insert(right.end(),proof->requirement->types.begin(),proof->requirement->types.end());
    return compareNominal(leftName,left,rightName,right);
}
