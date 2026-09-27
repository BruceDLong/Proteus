#include "NumericRange.h"
#include <gmpxx.h>
#include <cassert>
#include <iostream>
using Range=proteus4::NumericRange<mpq_class>;
int main() {
    unsigned cases=0;
    // Compare exact interval operations with point membership across a grid.
    // Half-integers exercise the boundaries as well as interior values.
    for(int a=-3;a<=3;++a) for(int b=a;b<=4;++b)
        for(int c=-3;c<=3;++c) for(int d=c;d<=4;++d) {
            auto x=Range::interval(a,b),y=Range::interval(c,d),z=x.intersect(y);
            for(int n=-8;n<=10;++n) {
                mpq_class point(n,2);
                assert(z.contains(point)==(x.contains(point)&&y.contains(point)));
                assert(x.shifted(mpq_class(1,3)).contains(point+mpq_class(1,3))==x.contains(point));
            }
            assert(z.empty || (x.contains(z) && y.contains(z)));
            ++cases;
        }
    auto x=Range::interval(2,6);
    assert(x.contains(2) && !x.contains(6));
    assert(!x.intersect(Range::literal(2)).empty && x.intersect(Range::literal(6)).empty);
    assert(x.contains(Range::interval(2,6)) && !x.contains(Range::interval(1,6)));
    assert(x.intersect(Range::interval(6,9)).empty);
    auto huge=mpq_class("1000000000000000000000000000000000000");
    auto tiny=mpq_class(1,1000000000);
    auto exact=Range::interval(huge,huge+tiny);
    assert(exact.contains(huge) && !exact.contains(huge+tiny));
    assert(Range::any().intersect(x).same(x));
    assert(!Range{}.intersect(x).valid && !Range{}.contains(3));
    assert(Range::interval(3,3).empty && !Range::literal(3).empty);
    std::cout << cases << " interval pairs and exact boundary checks passed\n";
}
