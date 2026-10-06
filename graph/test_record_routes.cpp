// Public-interface regression: correspondence connects a materialized record
// to partial supplied parts without recursively compiling their runtime values.
#include "proteusGraph.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
using Node=std::shared_ptr<POV>;

int main() {
    proteusGraph_initialize("");ProteusCore core;core.init("",false);
    auto& graph=core.agent->graphNormalizer;
    auto check=[](bool ok,const char* why) {if(!ok) throw std::runtime_error(why);};
    auto parse=[&](const std::string& text) {
        auto n=std::make_shared<POV>(909);
        n->pItem=core.parseAndExtractString(text,"record-routes",false);
        check(bool(n->pItem),"parse failed");return n;
    };
    std::vector<Proteus4Handle> handles;
    auto norm=[&](Node n,int depth=-1) {
        check(bool(n),"missing member");
        auto h=depth<0 ? graph.normalize(n) : graph.normalizeToDepth(n,depth);
        h.waitForCompletion();check(!h.status().failed,"normalization failed");handles.push_back(h);
    };
    auto declarations=parse(
        "{#table={3 4} #part=*2+{key:$,field:_} "
        "#left=part:{key:'left',} #right=part:{key:'right',} "
        "#shape=*2+{left,right} "
        "#source=shape:{left:{field=table@[2],},right:{field:3}}}");
    norm(declarations);auto before=declarations->stringify();
    auto target=parse("shape");norm(target,1);
    // Both occurrences already have their distinguishing key. A supplied
    // field is an additional part, not permission to copy its formula here.
    check(!graph.knownField(graph.knownField(target,"left"),"field"),"target was not partial");
    graph.addRHS(target,parse("source"));norm(target);
    auto left=graph.knownField(target,"left");norm(left);
    auto number=[&](Node owner,int expected) {
        auto field=graph.knownField(owner,"field");norm(field);
        check(getModeBits(field->pItem->value->flags,infonState::evalMode)==infonState::emLiteral &&
              field->pItem->value->num==expected,"supplied fact did not reach existing member");
    };
    number(left,4);number(graph.knownField(target,"right"),3);
    auto settled=target->stringify();norm(target);
    check(target->stringify()==settled,"unstable replay");
    check(declarations->stringify()==before,"shared declaration changed");
    graph.addRHS(left,parse("{field:5,}"));
    auto rejected=graph.normalize(left);rejected.waitForCompletion();
    check(rejected.status().failed,"known contradiction was accepted");rejected.cancel();
    for(auto& h:handles) h.cancel();core.deInit();proteusGraph_deinitialize();
    std::cout<<"existing typed parts receive formula 4 and literal 3; replay and contradiction passed\n";
}
