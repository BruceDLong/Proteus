#include "proteusGraph.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
using Node=std::shared_ptr<POV>;
int main(){
 proteusGraph_initialize("");ProteusCore core;core.init("",false);auto& g=core.agent->graphNormalizer;
 auto check=[](bool pass,const char* label){if(!pass)throw std::runtime_error(label);};
 auto parse=[&](const std::string& text){auto p=std::make_shared<POV>(909);p->pItem=core.parseAndExtractString(text,"depth-test",false);check(bool(p->pItem),"parse");return p;};
 std::vector<Proteus4Handle> handles;
 auto depth=[&](Node p,int d){auto h=g.normalizeToDepth(p,d);h.waitForCompletion();if(h.status().failed)throw std::runtime_error(h.status().error);handles.push_back(h);return h;};
 auto known=[](Node p){return getModeBits(p->pItem->value->flags,infonState::evalMode)==infonState::emLiteral;};
 auto normed=[](Node p){return getFlagBit(p->pItem->flags,infonView::isNormed);};
 auto root=parse("{a:{x:_=7} b:_=8}");auto a=root->pItem->value->items.head,b=a->next,x=a->pItem->value->items.head;
 auto zero=depth(root,0);check(!known(b)&&!known(x),"depth zero expanded members");check(!normed(root),"shallow marked tree normalized");
 auto initial=zero.steps();auto initialSteps=initial.size();
 proteus4::Query<Node> rootHeader;
 for(const auto& step:initial)if(step.action=="subgoal: resolve requested header" && step.query.node==root)rootHeader=step.query;
 check(bool(rootHeader.node),"missing root header demand");
 auto one=depth(root,1);check(known(b)&&b->pItem->value->num==8&&!known(x),"depth one boundary");check(!normed(root)&&!normed(a),"nested tree falsely normalized");
 auto two=depth(root,2);check(known(x)&&x->pItem->value->num==7,"depth two missing nested value");
 check(!zero.status().symbolicValues,"deeper request changed shallow completion");
 auto retained=two.steps();
 for(std::size_t i=initialSteps;i<retained.size();++i)
  check(!(retained[i].action=="visit" && !(retained[i].query<rootHeader) && !(rootHeader<retained[i].query)),"deeper request repeated root header");
 auto count=two.deductionStats().evaluations;auto replay=depth(root,2);
 check(replay.deductionStats().evaluations==count,"duplicate depth rewalked structure");
 std::cout<<"nested result: "<<root->stringify()<<" evaluations="<<count<<"\n";
 auto whole=parse("{*1+{*10+7} *10+8}");depth(whole,0);check(!normed(whole),"depth zero certified members");
 depth(whole,2);check(normed(whole),"complete subtree not marked normalized");
 auto span=parse("{&{x:_=7 y:_=8} z:_=9}");depth(span,1);
 auto spanFirst=span->pItem->value->items.head;
 check(known(spanFirst->pItem->value->items.head)&&known(spanFirst->pItem->value->items.tail),"embedded span consumed an extra depth level");
 auto defs=parse("{#leaf={x:_=9} #child=leaf #box={child, sibling:_=8}} ");auto hd=g.normalize(defs);hd.waitForCompletion();handles.push_back(hd);
 auto box=parse("box:{child,}");depth(box,0);check(!g.knownField(box,"sibling"),"depth zero filled omitted sibling");
 depth(box,1);auto sibling=g.knownField(box,"sibling");check(sibling&&known(sibling),"depth one missed immediate definition part");
 depth(box,2);auto child=g.knownField(box,"child");auto field=g.knownField(child,"x");check(field&&known(field)&&field->pItem->value->num==9,"definition child missing");
 auto dep=parse("{#p=*10+7 #q=p #r=q r}");auto hdep=depth(dep,1);auto value=dep->pItem->value->items.tail;check(value->pItem->value->num==7,"dependency traversal was depth capped");
 auto waiting=parse("{a:*10+_, b:*10+_}");auto ha=depth(waiting,1),hb=depth(waiting,1);ha.cancel();
 auto wa=g.knownField(waiting,"a"),wb=g.knownField(waiting,"b");auto seven=parse("*10+7");
 g.addRHS(wa,seven);g.addRHS(wb,seven);hb.waitForCompletion();check(!hb.status().failed&&!hb.status().cancelled&&wa->pItem->value->num==7&&wb->pItem->value->num==7,"peer cancellation or subscription lost");
 auto stream=parse("{a:_=7, ...}");auto hs=depth(stream,1);
 auto later=parse("b:_=8");g.withUpdate([&]{stream->pItem->value->items.appendPov(later);later->pParent=stream.get();});
 hs.waitForCompletion();check(known(later)&&later->pItem->value->num==8,"late member was not expanded");
 auto sparse=parse("*1000000+{_ | ...}");auto before=sparse->pItem->value->items.size();depth(sparse,3);check(sparse->pItem->value->items.size()==before,"depth enumerated sparse values");
 auto selected=parse("*1+{*1+{x:_=6}}@[1]");depth(selected,1);check(selected->stringify().find("x:6")!=std::string::npos,"selected value depth not forwarded");
 for(int count:{8,16,32,64}) {
  GraphNormalizer local;local.init();
  std::string text="*"+std::to_string(count)+"+{";
  for(int i=0;i<count;++i)text+="part:{x:_=7 y:_=8} ";
  auto many=parse(text+"}");
  auto initial=local.normalizeToDepth(many,0);initial.waitForCompletion();
  auto midway=local.normalizeToDepth(many,1);midway.waitForCompletion();
  auto batch=local.normalizeToDepth(many,2);batch.waitForCompletion();
  check(!batch.status().failed,"batch failure");
  for(auto part=many->pItem->value->items.head;part;part=part->next) {
   auto a=part->pItem->value->items.head,b=a->next;
   check(known(a)&&known(b)&&a->pItem->value->num==7&&b->pItem->value->num==8,"batch omitted values");
  }
  auto visits=batch.deductionStats().evaluations;
  auto repeat=local.normalizeToDepth(many,2);repeat.waitForCompletion();
  check(repeat.deductionStats().evaluations==visits,"batch replay repeated work");
  check(visits<=20*count+20,"batch expansion exceeded linear visit bound");
  std::cout<<"batch parts="<<count<<" evaluations="<<visits<<" replay added=0\n";
  initial.cancel();midway.cancel();batch.cancel();repeat.cancel();local.close();
 }
 bool invalid=false;try{g.normalizeToDepth(root,-1);}catch(const std::invalid_argument&){invalid=true;}check(invalid,"negative depth accepted");
 for(auto& h:handles)h.cancel();core.deInit();proteusGraph_deinitialize();std::cout<<"depth semantics passed\n";
}
