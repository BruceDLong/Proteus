// Public-interface regression: a transition selected as an RHS retains both
// explicitly changed facts and facts supplied by the previous occurrence.
#include "proteusGraph.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
using Node=std::shared_ptr<POV>;

int main() {
    proteusGraph_initialize("");
    for(int wrapping=0;wrapping<3;++wrapping) {
        ProteusCore core;core.init("",false);
        auto& graph=core.agent->graphNormalizer;
        auto check=[](bool pass,const char* why) {if(!pass) throw std::runtime_error(why);};
        auto parse=[&](const std::string& text) {
            auto node=std::make_shared<POV>(909);
            node->pItem=core.parseAndExtractString(text,"rhs-transition",false);
            check(bool(node->pItem),"parse failed");return node;
        };
        std::vector<Proteus4Handle> handles;
        auto norm=[&](Node node) {
            check(bool(node),"missing node");
            auto h=graph.normalize(node);h.waitForCompletion();
            check(!h.status().failed,"normalization failed");handles.push_back(h);
        };
        auto definitions=parse(
            "{#part=*3+{label:$,cargo:*100+_,pose:*24+_} "
            "#left=part:{label:'left',} #right=part:{label:'right',} "
            "#state=*2+{left,right} "
            "#known=state:{left:{cargo:7,pose:1},right:{cargo:9,pose:2}} "
            "#table={20 10} #change={T state state:{left:{"
            "cargo:%?=%@[1]~part:{label:'right',}.cargo,"
            "pose:%?=table@[%@[1]~part:{label:'right',}.pose]},}}}");
        norm(definitions);auto declaration=definitions->stringify();
        const std::string expression="change:{T known ?}@[2]";
        auto root=parse(wrapping==0 ? expression : wrapping==1 ?
            "{result:%?="+expression+",}" :
            "{T {marker:100,} {result:%?="+expression+",}}");
        // Deliberately do not normalize the source state first.
        norm(root);
        auto result=wrapping==0 ? root : graph.knownField(wrapping==1 ? root :
            root->pItem->value->items.tail,"result");
        norm(result);
        for(const auto* name:{"left","right"}) {
            auto part=graph.knownField(result,name);norm(part);
            for(const auto* field:{"cargo","pose"}) norm(graph.knownField(part,field));
        }
        auto number=[&](const char* part,const char* field,int expected) {
            auto node=graph.knownField(graph.knownField(result,part),field);
            return node && getModeBits(node->pItem->value->flags,infonState::evalMode)==infonState::emLiteral &&
                node->pItem->value->num==expected;
        };
        check(number("left","cargo",9) && number("left","pose",10),"changed facts missing");
        check(number("right","cargo",9) && number("right","pose",2),"preserved facts missing");
        auto settled=result->stringify();norm(result);
        check(result->stringify()==settled,"unstable replay");
        check(definitions->stringify()==declaration,"shared declaration changed");
        std::cout<<"wrapping="<<wrapping<<" changed=9,10 preserved=9,2\n";
        for(auto& h:handles) h.cancel();core.deInit();
    }
    proteusGraph_deinitialize();
}
