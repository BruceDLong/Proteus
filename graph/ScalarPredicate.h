#pragma once

// Both sequence predicates and field queries need proof of exclusion. Missing
// facts are not a mismatch, even when another candidate is already available.
enum class ScalarMatch { yes, no, unknown };
inline ScalarMatch compareScalar(GraphTypeFacts required, const GraphTypeFacts& actual) {
    if (!required.valid || !actual.valid) return ScalarMatch::unknown;
    if (required.accepts(actual)) return ScalarMatch::yes;
    return required.combine(actual) ? ScalarMatch::unknown : ScalarMatch::no;
}

// Qualified scalar text matching shares the endpoint rule with numeric
// matching. A colon does not supply a missing target size, even when the
// available text is concrete. Existing literal prefixes are compared without
// overwriting either operand.
struct TextMatch {
    ScalarMatch verdict=ScalarMatch::unknown;
    GraphTypeFacts result;
};
inline TextMatch matchText(GraphTypeFacts left,const GraphTypeFacts& right,bool looseSize) {
    TextMatch match;match.result=left;
    if(!left.valid || !right.valid || right.overlay!=infonState::STR || !right.hasValue) return match;
    if(!looseSize && left.hasSize && right.hasSize && left.size!=right.size) {
        match.verdict=ScalarMatch::no;return match;
    }
    auto& result=match.result;result.overlay=infonState::STR;
    if(left.hasValue) {
        auto width=std::min(left.text.size(),right.text.size());
        if(left.text.compare(0,width,right.text,0,width)!=0 || (!looseSize && left.text.size()!=right.text.size())) {
            match.verdict=ScalarMatch::no;return match;
        }
    } else {
        std::size_t width=right.text.size();
        if(left.hasSize) {
            if(left.size<0 || left.size.getDenominator()!=1 || left.size>std::numeric_limits<std::int64_t>::max()) return match;
            auto requested=left.size.getNumerator().get_si();
            if(static_cast<std::uint64_t>(requested)>width) return match;
            // Whole correspondence cannot publish a truncated value and then
            // discover the remaining text on the next pass. With an unknown
            // source endpoint, wait for enough evidence to establish its end.
            if(!looseSize && static_cast<std::uint64_t>(requested)!=width) return match;
            width=static_cast<std::size_t>(requested);
        }
        result.hasValue=true;result.text=right.text.substr(0,width);
    }
    if(!looseSize && !result.hasSize && right.hasSize) {result.hasSize=true;result.size=right.size;}
    match.verdict=ScalarMatch::yes;return match;
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

// The relation's endpoint mode is independent of numeric value matching.
// A bare unknown RHS supplies no value constraint. Its size is copied only
// when the relationship requires matching ends, exactly as for list prefixes.
struct NumericMatch {
    ScalarMatch verdict=ScalarMatch::unknown;
    NumericView result;
    bool identity=false;
};
inline NumericMatch matchNumeric(const NumericView& left,const NumericView& right,
                                 bool looseSize,bool looseType) {
    NumericMatch match;match.result=left;
    if(!left.bounds.valid || !right.bounds.valid) return match;
    if(left.bounds.empty || right.bounds.empty) {match.verdict=ScalarMatch::no;return match;}
    auto& result=match.result;
    if(!looseSize && left.facts.hasSize && right.facts.hasSize && left.facts.size!=right.facts.size) {
        match.verdict=ScalarMatch::no;return match;
    }
    if(!looseType) {
        auto a=left.facts,b=right.facts;a.hasValue=b.hasValue=false;
        if(!a.combine(b)) {match.verdict=ScalarMatch::no;return match;}
        result.facts=a;result.bounds=left.bounds.intersect(right.bounds);
        result.expression=left.expression || right.expression;
        match.verdict=result.bounds.empty ? ScalarMatch::no : ScalarMatch::yes;
    } else if(!right.bounds.point && !right.expression) {
        // A completely unknown input cannot yet prove a sequence match.
        // A known size supplies range evidence without supplying a value.
        if(!right.facts.hasSize) return match;
        match.verdict=ScalarMatch::yes; // an unknown value adds no information
    } else {
        match.verdict=compareNumeric(left,right);
        if(match.verdict!=ScalarMatch::yes) return match;
        result.bounds=right.bounds;result.expression=right.expression;
        result.facts.explicitSize=right.facts.explicitSize;
    }
    if(!looseSize && !result.facts.hasSize && right.facts.hasSize) {
        result.facts.hasSize=true;result.facts.size=right.facts.size;
        result.facts.explicitSize=result.facts.explicitSize || right.facts.explicitSize;
        if(!result.bounds.point && !result.expression) result.bounds=NumericBounds::interval(0,result.facts.size.val);
    }
    result.facts.hasValue=result.bounds.point;
    if(result.bounds.point) result.facts.number.val=result.bounds.low;
    match.identity=!looseSize && (right.bounds.point || right.expression) && result.bounds.same(right.bounds) &&
        result.facts.hasSize==right.facts.hasSize && (!result.facts.hasSize || result.facts.size==right.facts.size);
    return match;
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
