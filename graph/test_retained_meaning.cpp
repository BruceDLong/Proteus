#include "proteusGraph.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
using Node=std::shared_ptr<POV>;
int main(){
 proteusGraph_initialize("");
 for(bool aliases:{false,true})for(bool bounded:{false,true}) {
  ProteusCore core;core.init("",false);auto& g=core.agent->graphNormalizer;
  auto check=[](bool pass,const char* message){if(!pass)throw std::runtime_error(message);};
  auto parse=[&](const std::string& text){auto p=std::make_shared<POV>(909);p->pItem=core.parseAndExtractString(text,"retained-meaning",false);check(bool(p->pItem),"parse");return p;};
  std::vector<Proteus4Handle> handles;
  auto norm=[&](Node p){auto h=g.normalize(p);h.waitForCompletion();check(!h.status().failed,"normalization failed");handles.push_back(h);return h;};
  auto defs=parse(std::string("{#base=*1+{item:")+(bounded?"*1+":"")+"{x:"+(bounded?"*10+":"")+"_,}} #start=base:{item:{x:7,}} #alias=start #act={T base base:{item:{x:%?=%@[1].item.x,}}}}");norm(defs);auto written=defs->stringify();
  auto history=parse(std::string("act:{T ")+(aliases?"alias":"start")+" ?}");auto h=norm(history);
  auto first=history->pItem->value->items.head,second=first->next;
  auto item=g.knownField(first,"item"),x=g.knownField(item,"x");
  check(x&&getModeBits(x->pItem->value->flags,infonState::evalMode)==infonState::emLiteral&&x->pItem->value->num==7,"declared fact was lost after materialization");
  auto resultItem=g.knownField(second,"item"),result=g.knownField(resultItem,"x");norm(result);
  auto value=result->pItem;
  if(value->isOrLike()){check(value->innr&&value->innr->value->items.head,"missing symbolic candidate");value=value->innr->value->items.head->pItem;}
  check(getModeBits(value->value->flags,infonState::evalMode)==infonState::emLiteral&&value->value->num==7,"temporal query lost known source value");
  check(first!=second&&first->pItem!=second->pItem&&x->pItem->value->num==7,"prior state changed or identified");
  check(defs->stringify()==written,"declaration mutated");
  std::cout<<"aliases="<<aliases<<" bounded="<<bounded<<" source=7 result="<<result->stringify()<<"\n";
  for(auto& h:handles)h.cancel();core.deInit();
 }
 proteusGraph_deinitialize();
}
