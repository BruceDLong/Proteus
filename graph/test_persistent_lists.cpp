// Build after generating LocalBuild/TestProteus.cpp with CodeDog.
#define main proteus_test_main
#include "../LocalBuild/TestProteus.cpp"
#undef main
#include <cassert>
using namespace proteus4;
static POVNode parse(const std::string& text) {return graphParsed("persistent list",text);}
static Changes writes(GraphNormalizer& n,const POVNode& p,bool position=false) {
    return n.runtime.read([&]{auto c=ReadScope::changedObjects({p.get(),p->pItem.get(),p->pItem->value.get(),p->pItem->infSize.get()});
        if(position)c.include(ReadScope::changedObjects({POVDescription::positionKey(p)}));return c;});
}
int main(int argc,char** argv) {
 if(argc>1)return proteus_test_main(argc,argv);
 initialize("");
 for(int count:{8,16,32,64}) {
    GraphNormalizer n;n.init();std::string text="{";
    for(int i=0;i<count;++i)text+="*10+_ ";
    auto root=parse(text+"}");auto h=n.runtime.normalize({root,listContents,sizeAndValue});h.waitForCompletion();
    assert(!h.status().failed && h.status().withSubscribers);
    auto initial=h.deductionStats();auto visits=h.deductionVisits();assert(initial.records==std::size_t(count+2));
    for(auto child=POVDescription(root).member();child;child=child->next) {
        auto facts=n.access->typeFacts(parse("*10+3"));
        n.runtime.updateIfChanged([&]{return n.access->applyTypeFacts(child,facts);},writes(n,child));h.waitForCompletion();
        assert(!h.status().failed);
        // A value changes no physical position or nonembedded count contribution.
        auto after=h.deductionVisits();for(const auto& [q,times]:visits)assert(after.at(q)==times);
    }
    assert(!h.status().symbolicValues && !h.status().withSubscribers);
    auto final=h.deductionStats();assert(final.evaluations==initial.evaluations && final.records==initial.records);
    std::cout<<"VALUES "<<count<<" positions="<<final.records<<" evaluations="<<final.evaluations<<" additional=0\n";
    n.close();
 }
 for(int count:{8,16,32,64}) {
    GraphNormalizer n;n.init();auto root=parse("{...}");
    auto h=n.runtime.normalize({root,listContents,sizeAndValue});h.waitForCompletion();
    assert(!h.status().failed && h.status().withSubscribers);
    for(int i=0;i<count;++i) {
        auto added=parse("*10+3");auto last=POVDescription(root).member(true);auto before=h.deductionVisits();
        auto changes=writes(n,root);if(last)changes.include(writes(n,last,true));
        n.runtime.updateIfChanged([&]{root->pItem->value->items.appendPov(added);added->pParent=root.get();return true;},changes);h.waitForCompletion();
        assert(!h.status().failed && h.status().withSubscribers);
        auto after=h.deductionVisits();
        for(const auto& [q,times]:before)if(q.kind==listMember && q.node!=last)assert(after.at(q)==times);
    }
    n.runtime.updateIfChanged([&]{SetFlagBit(root->pItem->value->flags,infonState::tailUnfinished,false);return true;},writes(n,root));h.waitForCompletion();
    assert(!h.status().failed && !h.status().symbolicValues);
    assert(root->pItem->infSize->num==count);
    auto stats=h.deductionStats();assert(stats.records<=std::size_t(2*count+2));assert(stats.evaluations<=std::size_t(8*count+12));
    std::cout<<"APPENDS "<<count<<" positions="<<stats.records<<" evaluations="<<stats.evaluations<<"\n";n.close();
 }
 {
    GraphNormalizer n;n.init();auto root=parse("*5+{ &{...} 9 }");auto span=POVDescription(root).member();
    auto h=n.runtime.normalize({root,listContents,sizeOnly});h.waitForCompletion();
    assert(!h.status().failed && h.status().withSubscribers);auto before=h.deductionVisits();
    auto facts=n.access->typeFacts(parse("*4+{...}"));
    n.runtime.updateIfChanged([&]{return n.access->applyTypeFacts(span,facts);},writes(n,span));h.waitForCompletion();
    assert(!h.status().failed && !h.status().symbolicValues && root->pItem->infSize->num==5);
    auto after=h.deductionVisits();
    for(const auto& [q,times]:before)if(q.kind==listMember && q.node==span->next)assert(after.at(q)==times);
    std::cout<<"EMBEDDED extent=5\n";n.close();
 }
 {
    GraphNormalizer n;n.init();auto root=parse("{*10+_ *10+7}");
    auto h=n.runtime.normalize({root,listContents,sizeAndValue});h.waitForCompletion();auto before=h.deductionStats();h.cancel();
    auto child=POVDescription(root).member();auto facts=n.access->typeFacts(parse("*10+3"));
    n.runtime.updateIfChanged([&]{return n.access->applyTypeFacts(child,facts);},writes(n,child));h.waitForCompletion();
    assert(h.deductionStats().evaluations==before.evaluations);n.close();
 }
 deinitialize();std::cout<<"Persistent list checks passed\n";
}
