#pragma once
#include <algorithm>

namespace proteus4 {
// Exact, half-open numeric sets. A point is distinct from an empty interval;
// an unbounded set is distinct from unavailable evidence.
template<class Number> struct NumericRange {
    bool valid=false, bounded=false, point=false, empty=false;
    Number low{}, high{};
    static NumericRange any() { NumericRange r;r.valid=true;return r; }
    static NumericRange literal(Number n) {auto r=any();r.bounded=r.point=true;r.low=r.high=n;return r;}
    static NumericRange interval(Number lo,Number hi) {
        auto r=any();r.bounded=true;r.low=lo;r.high=hi;r.empty=!(lo<hi);return r;
    }
    bool same(const NumericRange& b) const {
        return valid==b.valid && bounded==b.bounded && point==b.point && empty==b.empty &&
            (!bounded || (low==b.low && high==b.high));
    }
    bool contains(const Number& n) const {return valid && !empty && (!bounded || (low<=n && (point ? n==low : n<high)));}
    bool contains(const NumericRange& b) const {
        if(!valid || !b.valid) return false;
        if(b.empty) return true;
        if(empty) return false;
        if(!bounded) return true;
        if(!b.bounded) return false;
        if(b.point) return contains(b.low);
        return !point && low<=b.low && b.high<=high;
    }
    NumericRange intersect(const NumericRange& b) const {
        if(!valid || !b.valid) return {};
        if(empty) return *this;
        if(b.empty || !bounded) return b;
        if(!b.bounded) return *this;
        if(point) return b.contains(low) ? *this : interval(low,low);
        if(b.point) return contains(b.low) ? b : interval(b.low,b.low);
        return interval(std::max(low,b.low),std::min(high,b.high));
    }
    NumericRange shifted(const Number& n) const {
        auto r=*this;if(bounded) {r.low+=n;r.high+=n;}return r;
    }
};
} // namespace proteus4
