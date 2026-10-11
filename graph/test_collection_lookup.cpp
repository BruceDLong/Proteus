// Public-interface regression: inferred collection lookup reads a known list
// header without waiting for unrelated values in its pending RHS.
#include "proteusGraph.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
using Node=std::shared_ptr<POV>;
int main() {
    std::cout.setf(std::ios::unitbuf);proteusGraph_initialize("");ProteusCore core;core.init("",false);
    auto& graph=core.agent->graphNormalizer;
    auto check=[](bool pass,const char* why){if(!pass)throw std::runtime_error(why);};
    auto parse=[&](const std::string& text){auto node=std::make_shared<POV>(909);node->pItem=core.parseAndExtractString(text,"collection-lookup",false);check(bool(node->pItem),"parse failed");return node;};
    std::vector<Proteus4Handle> handles;
    auto norm=[&](Node node,int depth=-1){check(bool(node),"missing member");auto h=depth<0?graph.normalize(node):graph.normalizeToDepth(node,depth);h.waitForCompletion();if(h.status().failed)throw std::runtime_error(h.status().error);handles.push_back(h);return h;};
    auto number=[&](Node node,int expected){check(node && getModeBits(node->pItem->value->flags,infonState::evalMode)==infonState::emLiteral && node->pItem->value->num==expected,"wrong or unresolved number");};
    auto definitions=parse(
        "{#part=*3+{label:$,cargo:*100+_,pose:*24+_} "
        "#left=part:{label:'left',} #right=part:{label:'right',} "
        "#state=*2+{left,right} #known=state:{left:{cargo:7,pose:1},right:{cargo:9,pose:2}} "
        "#table={20 10} #catalog={0 1 2 3 4 5 6 7 8 99} "
        "#change={T state state:{left:{cargo:%?=%@[1]~part:{label:'right',}.cargo,pose:%?=table@[%@[1]~part:{label:'right',}.pose]},}} "
        "#view=*2+{placements,derived=catalog@[%.placements~part:{label:'left',}.pose]}}");
    norm(definitions);auto before=definitions->stringify();
    auto root=parse("view:{placements:%?=change:{T known ?}@[2],}");norm(root,1);
    auto derived=graph.knownField(root,"derived");norm(derived,1);number(derived,99);
    check(definitions->stringify()==before,"shared declaration changed");
    auto settled=root->stringify();norm(derived,1);check(root->stringify()==settled,"unstable replay");
    std::cout<<"derived transition lookup = 99; shared declarations and replay preserved\n";
    auto scalar=parse("*1+{placements:7,}.placements");norm(scalar);
    check(scalar->pItem->isOrLike() && !scalar->pItem->innr->value->items.head && !getFlagBit(scalar->pItem->innr->value->flags,infonState::tailUnfinished),"scalar accepted as inferred collection");
    auto distinct=parse("*1+{parts:*1+{item:7},}.placements");norm(distinct);
    check(distinct->pItem->isOrLike() && !distinct->pItem->innr->value->items.head,"unrelated nominal collection matched");
    auto multiple=parse("*2+{placements:*1+{item:7},placements:*1+{item:8}}.placements");norm(multiple);
    check(multiple->pItem->isOrLike() && multiple->pItem->innr->value->items.size()==2,"multiple matches collapsed");
    auto open=parse("{placements:*1+{item:7},}.placements");norm(open);
    check(open->pItem->isOrLike() && getFlagBit(open->pItem->innr->value->flags,infonState::tailUnfinished),"open membership collapsed");
    auto waiting=parse("*1+{placements,}.placements");
    auto h=norm(waiting);
    auto source=waiting->pItem->pathSelection->source->pItem->value->items.head;
    check(waiting->pItem->isOrLike() && getFlagBit(waiting->pItem->innr->value->flags,infonState::tailUnfinished),"unknown header accepted");
    graph.addRHS(source,parse("*1+{item:7}"));h.waitForCompletion();norm(waiting);
    check(!waiting->pItem->isOrLike() && waiting->pItem->value->items.head,"late list header not selected");
    std::cout<<"scalar exclusion, nominal mismatch, multiple matches, open membership and late header passed\n";
    norm(parse("{#placements=rat}"));
    auto declared=parse("*1+{placements:7,}.placements");norm(declared);number(declared,7);
    std::cout<<"explicit declaration overrides inferred collection\n";
    for(auto& handle:handles)handle.cancel();core.deInit();proteusGraph_deinitialize();
}
