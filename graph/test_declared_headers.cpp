#include "proteusGraph.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
using Node=std::shared_ptr<POV>;

int main() {
    std::cout.setf(std::ios::unitbuf);proteusGraph_initialize("");
    for(bool bounded:{false,true}) for(bool alias:{false,true}) {
        ProteusCore core;core.init("",false);auto& graph=core.agent->graphNormalizer;
        auto check=[](bool ok,const char* message){if(!ok) throw std::runtime_error(message);};
        auto parse=[&](const std::string& text) {
            auto n=std::make_shared<POV>(909);n->pItem=core.parseAndExtractString(text,"declared-headers",false);
            check(bool(n->pItem),"parse failed");return n;
        };
        std::vector<Proteus4Handle> handles;
        auto norm=[&](Node n) {auto h=graph.normalize(n);h.waitForCompletion();check(!h.status().failed,"normalization failed");handles.push_back(h);};
        const std::string shape=std::string(bounded?"*2+":"")+"{left,right}";
        auto definitions=parse(
            "{#part=*3+{label:$,cargo:*100+_,pose:*24+_} "
            "#left=part:{label:'left',} #right=part:{label:'right',} "+
            (alias?"#shape="+shape+" #state=shape ":"#state="+shape+" ")+
            "#known=state:{left:{cargo:7,pose:1},right:{cargo:9,pose:2}} "
            "#table={20 10} #catalog={0 1 2 3 4 5 6 7 8 99} "
            "#change={T state state:{left:{cargo:%?=%@[1]~part:{label:'right',}.cargo,"
            "pose:%?=table@[%@[1]~part:{label:'right',}.pose]},}}}");
        norm(definitions);auto before=definitions->stringify();
        const std::string expression="change:{T known ?}@[2]~part:{label:'left',}.pose";
        auto query=parse(expression);norm(query);
        auto known=query->pItem;
        if(!bounded) {
            check(known->isOrLike() && known->innr &&
                  getFlagBit(known->innr->value->flags,infonState::tailUnfinished),"unbounded membership was closed");
            check(bool(known->innr->value->items.head),"missing known candidate");
            known=known->innr->value->items.head->pItem;
        } else check(!known->isOrLike(),"finite selection remained open");
        if(bounded) check(getModeBits(known->value->flags,infonState::evalMode)==infonState::emLiteral &&
              known->value->num==10,"changed pose missing");
        auto indexed=parse("catalog@["+expression+"]");norm(indexed);
        const bool numeric=getModeBits(indexed->pItem->value->flags,infonState::overlayType)==infonState::NUM &&
            getModeBits(indexed->pItem->value->flags,infonState::evalMode)==infonState::emLiteral;
        check(bounded ? numeric && indexed->pItem->value->num==99 : !numeric,"index consumed an unresolved choice or missed a finite result");
        auto settled=query->stringify();norm(query);check(query->stringify()==settled,"unstable replay");
        check(definitions->stringify()==before,"declaration changed");
        std::cout<<"bounded="<<bounded<<" alias="<<alias<<" indexed="<<(numeric?"99":"symbolic")<<"\n";
        for(auto& h:handles) h.cancel();core.deInit();
    }
    {
        ProteusCore core;core.init("",false);auto& graph=core.agent->graphNormalizer;
        auto check=[](bool ok,const char* message){if(!ok)throw std::runtime_error(message);};
        auto parse=[&](const std::string& text){auto n=std::make_shared<POV>(909);n->pItem=core.parseAndExtractString(text,"partial-header",false);check(bool(n->pItem),"parse failed");return n;};
        std::vector<Proteus4Handle> handles;
        auto norm=[&](Node n){auto h=graph.normalize(n);h.waitForCompletion();handles.push_back(h);return h;};
        auto defs=parse("{#item=*2+{label:$,other:*10+_} #alias=item #description={selected=alias:{label:'present',},} #bad={selected=alias:*3+{label:'present',},}}");
        check(!norm(defs).status().failed,"definitions failed");auto before=defs->stringify();
        auto instance=parse("description");check(!norm(instance).status().failed,"partial description failed");
        auto selected=graph.knownField(instance,"selected");check(bool(selected),"missing selected description");
        check(getModeBits(selected->pItem->infSize->flags,infonState::evalMode)==infonState::emLiteral && selected->pItem->infSize->num==2,"lost declared extent");
        check(!graph.knownField(selected,"other"),"header read expanded omitted part");
        auto bad=parse("bad");check(norm(bad).status().failed,"conflicting explicit extents were accepted");
        check(defs->stringify()==before,"shared definition changed");
        std::cout<<"partial extent=2 omitted field remains absent; conflicting extent rejected\n";
        // A new instance must not reuse a header read from before the publication.
        auto item=defs->pItem->value->items.head->pItem->funcArgs;
        graph.withUpdate([&]{SetModeBits(item->infSize->flags,infonState::evalMode,infonState::emUnknown);item->value->sizeMode=fromCount;});
        auto later=parse("description");auto lh=norm(later);if(lh.status().failed)std::cerr<<"LATE ERROR "<<lh.status().error<<"\n";check(!lh.status().failed,"changed header failed");
        auto field=graph.knownField(later,"selected");
        check(field && getModeBits(field->pItem->infSize->flags,infonState::evalMode)==infonState::emUnknown,"stale header proof survived publication");
        check(!graph.knownField(field,"other"),"late header read expanded omitted part");
        std::cout<<"late header publication keeps the new extent unknown\n";
        for(auto& h:handles)h.cancel();core.deInit();
    }
    proteusGraph_deinitialize();
}
