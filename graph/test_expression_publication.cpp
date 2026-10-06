// Public-interface regression: local contextual definitions retain unrelated searches.
#include "proteusGraph.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
using Node=std::shared_ptr<POV>;
int main(){
 proteusGraph_initialize("");
 for(int count:{1,8,32}) {
  ProteusCore core;core.init("",false);auto& g=core.agent->graphNormalizer;
  auto check=[](bool pass,const char* message){if(!pass)throw std::runtime_error(message);};
  auto parse=[&](const std::string& text){auto p=std::make_shared<POV>(909);p->pItem=core.parseAndExtractString(text,"expression-publication",false);check(bool(p->pItem),"parse failed");return p;};
  std::vector<Proteus4Handle> handles;
  auto norm=[&](Node p){auto h=g.normalize(p);h.waitForCompletion();check(!h.status().failed,"normalization failed");handles.push_back(h);return h;};
  auto definitions=parse("{#sample={x:*10+7 y=%.x} #variable={x:*10+_ y=%.x}}");norm(definitions);
  auto original=definitions->stringify();
  std::vector<Node> waiting;std::vector<Proteus4Handle> subscribers;std::vector<std::size_t> before;
  for(int i=0;i<count;++i){waiting.push_back(parse("*10+_"));subscribers.push_back(norm(waiting.back()));before.push_back(subscribers.back().steps().size());}
  for(int n=0;n<4;++n) {
   auto instance=parse("sample");norm(instance);
   auto y=instance->pItem->fetchField("y");
   check(y && y->pItem->value->num==7 && getModeBits(y->pItem->value->flags,infonState::evalMode)==infonState::emLiteral,"contextual result missing");
  }
  for(int i=0;i<count;++i){subscribers[i].waitForCompletion();check(subscribers[i].status().withSubscribers,"unrelated subscription lost");auto extra=subscribers[i].steps().size()-before[i];if(extra)std::cerr<<"extra="<<extra<<"\n";check(extra==0,"unrelated task restarted");}
  // Independent instances retain their own context when values arrive later.
  auto a=parse("variable"),b=parse("variable");auto ha=norm(a),hb=norm(b);
  auto ax=a->pItem->fetchField("x"),ay=a->pItem->fetchField("y");
  auto bx=b->pItem->fetchField("x"),by=b->pItem->fetchField("y");
  check(ax && ay && bx && by,"missing variable parts");
  auto dependent=norm(ay);
  g.addRHS(ax,parse("*10+4"));g.addRHS(bx,parse("*10+9"));
  ha.waitForCompletion();hb.waitForCompletion();norm(a);norm(b);dependent.waitForCompletion();
  check(!dependent.status().failed && ay->pItem->value->num==4 && by->pItem->value->num==9 && ay->pItem!=by->pItem,"late context or dependent wake lost");
  check(definitions->stringify()==original,"shared definition changed");
  g.addRHS(waiting.front(),parse("*10+6"));subscribers.front().waitForCompletion();
  check(waiting.front()->pItem->value->num==6,"unrelated subscriber no longer responds");
  for(auto& h:handles)h.cancel();core.deInit();
  std::cout<<count<<" unrelated subscribers: no restarts; contextual and late values preserved\n";
 }
 proteusGraph_deinitialize();
}
