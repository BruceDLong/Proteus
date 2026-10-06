// Link against SharedBuild/libproteusGraph.so; only the public generated interface is used.
// A choice refines a field value without erasing the field declaration.
#include "proteusGraph.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
using Node=std::shared_ptr<POV>;
int main(){
 proteusGraph_initialize("");ProteusCore core;core.init("",false);auto& g=core.agent->graphNormalizer;
 auto check=[](bool pass,const char* label){if(!pass)throw std::runtime_error(label);};
 auto parse=[&](std::string t){auto p=std::make_shared<POV>(909);p->pItem=core.parseAndExtractString(t,"choice-declaration-test",false);check(bool(p->pItem),"parse");return p;};
 std::vector<Proteus4Handle> handles;
 auto norm=[&](Node p){auto h=g.normalize(p);h.waitForCompletion();check(!h.status().failed,"normalization failed");handles.push_back(h);};
 for(const auto& input:{
  "{pending:_ = *_+[pending:1 pending:2], other:3}",
  "{pending:_ = *_+[pending:1 pending:2] other:3}",
  "{pending:%? = *_+[pending:1 pending:2], other:3}",
  "{pending:%? = *_+[pending:2], other:3}",
  "{pending:*10+2 = *_+[pending:_], other:3}"}) {
  auto root=parse(input);auto original=root->pItem->value->items.head;auto other=root->pItem->value->items.tail;
  const bool independent=getFlagBit(original->pItem->flags,infonView::independentTemporalValue);
  norm(root);
  std::cout<<root->stringify()<<"\n";
  g.withRead([&]{
   check(original->pItem->type && original->pItem->type->typeID()=="pending","declaration lost");
   check(getFlagBit(original->pItem->flags,infonView::independentTemporalValue)==independent,"independence lost");
   check(g.knownField(root,"other")==other,"unrelated field hidden");
   check(g.knownField(root,"pending")==original,"pending field hidden");
   check(other->pItem->value->num==3,"other changed");
  });
  norm(original);
  check(original->pItem->type && original->pItem->type->typeID()=="pending","declaration lost on replay");
 }
 auto aliases=parse("{#slot=place}");norm(aliases);
 auto aliasField=parse("{slot:_ = *_+[place:1 place:2], other:3}");norm(aliasField);
 check(g.knownField(aliasField,"place")==aliasField->pItem->value->items.head,"declared alias lost");
 auto late=parse("{pending:*10+2 = *_+[pending:_], other:3}");
 auto field=late->pItem->value->items.head;
 auto candidate=field->pItem->wrkList.front()->pItem->innr->value->items.head;
 norm(late);auto two=parse("*10+2");
 g.withUpdate([&]{candidate->pItem->addPOVToWrkList(two);});
 for(auto& h:handles)h.waitForCompletion();
 norm(field);
 check(field->pItem->type && field->pItem->type->typeID()=="pending","late result lost declaration");
 check(field->pItem->value->num==2 && !field->pItem->isOrLike(),"late result not resolved");
 auto ordinary=parse("source:{target:7}.target");norm(ordinary);
 check(ordinary->pItem->type && ordinary->pItem->type->typeID()=="target","navigation kept source type");
 auto ambiguous=parse("{pending:1, pending:2}");
 check(!g.knownField(ambiguous,"pending"),"ambiguous field selected");
 auto untyped=parse("{?, other:3}");
 check(!g.knownField(untyped,"other"),"unknown untyped alternative ignored");
 for(auto& h:handles)h.cancel();core.deInit();proteusGraph_deinitialize();
 std::cout<<"PASS: declared choice fields, independent values, late publication, ordinary navigation, ambiguity\n";
}
