// Compile after generating LocalBuild/TestProteus.cpp with CodeDog.
// g++ -std=gnu++17 -O1 -g0 -pthread -fpermissive graph/test_persistent_fields.cpp
//     -lgmp -lgmpxx $(pkg-config --cflags --libs icu-uc icu-io) -o /tmp/p4-fields
#define main proteus_test_main
#include "../LocalBuild/TestProteus.cpp"
#undef main
#include <cassert>
using namespace proteus4;
int main() {
 initialize("");
 for(int outcome:{2,3}) for(int count:{1,8,32,64}) {
  GraphNormalizer n;n.init();
  std::string text="*"+std::to_string(count+1)+"+{part:{key:_,other:7},";
  for(int i=0;i<count;++i)text+="part:{key:"+std::to_string(100+i)+",other:8},";
  auto root=graphParsed("persistent field",text+"}.part:{key:2,}");
  auto h=n.normalize(root);h.waitForCompletion();assert(!h.status().failed && h.status().withSubscribers);
  auto state=root->pItem->pathSelection;auto first=POVDescription(state->source).member();auto key=first->pItem->fetchField("key");
  auto before=h.deductionVisits();auto countBefore=h.deductionStats();assert(countBefore.records);
  auto value=graphParsed("arriving key",std::to_string(outcome));auto facts=n.access->typeFacts(value);
  auto changes=n.runtime.read([&]{return ReadScope::changedObjects({key.get(),key->pItem.get(),key->pItem->value.get()});});
  n.runtime.updateIfChanged([&]{return n.access->applyTypeFacts(key,facts);},changes);h.waitForCompletion();
  assert(!h.status().failed && !state->pending);
  assert(outcome==2 ? state->matches.size()==1 && state->matches.front()==first : state->matches.empty());
  auto after=h.deductionVisits();auto countAfter=h.deductionStats();
  for(auto other=first->next;other;other=other->next)
   for(const auto& visited:before) if(visited.first.kind==fieldPosition && visited.first.node==other)
    assert(after.at(visited.first)==visited.second);
  auto extra=countAfter.evaluations-countBefore.evaluations;
  std::cout<<"FIELD candidates="<<count+1<<" outcome="<<outcome<<" evaluations="<<countBefore.evaluations<<" additional="<<extra<<" records="<<countAfter.records<<"\n";
  assert(extra<=12);
  h.cancel();n.close();
 }
 deinitialize();std::cout<<"Persistent field checks passed\n";
}
