// Public-interface integration test: no generated engine .cpp or private goals.
#include "proteusGraph.hpp"
#include <cassert>
#include <iostream>

struct HostCore : ProteusCore {
    bool callback=false;
    bool normalizeFromStreamSpecAtOrigin(std::shared_ptr<POV>, std::string, uint64_t) override {
        callback=true;return true;
    }
};

int main() {
    proteusGraph_initialize("");
    HostCore core;
    assert(core.init("", false));
    auto& graph=core.agent->graphNormalizer;
    auto parse=[&](const std::string& text) {
        auto node=std::make_shared<POV>(901);
        node->pItem=core.parseAndExtractString(text,"shared-library-test",false);
        if(!node->pItem) throw std::runtime_error("parse failed: "+text);
        return node;
    };
    auto normalize=[&](const std::shared_ptr<POV>& node) {
        auto handle=graph.normalize(node);handle.waitForCompletion();
        if(handle.status().failed) throw std::runtime_error(handle.status().error);
        return handle;
    };
    auto scalar=parse("*10+_ = 5");
    auto original=scalar->pItem;
    auto handle=normalize(scalar);
    graph.withRead([&] {
        assert(scalar->pItem==original);
        assert(scalar->pItem->value->num==5);
        assert(scalar->pItem->infSize->num==10);
    });
    assert(handle.status().fullyNormed);

    auto pending=parse("*10+_");
    auto waiting=normalize(pending);
    auto seven=parse("7");
    auto revision=graph.revision();
    graph.withUpdate([&] { pending->pItem->addPOVToWrkList(seven); });
    waiting.waitForCompletion();
    assert(!waiting.status().failed);
    assert(pending->pItem->value->num==7);
    assert(graph.revision()>revision);

    auto history=parse("{T {a:1, b:7} {a:2,}}");
    auto temporal=normalize(history);
    auto second=graph.findItem(history,1);second.waitForCompletion();
    assert(second.result());
    auto state=normalize(second.result());
    auto b=graph.knownField(second.result(),"b");assert(b);
    auto field=normalize(b);assert(b->pItem->value->num==7);
    auto a=graph.knownField(second.result(),"a");assert(a);
    auto changed=normalize(a);assert(a->pItem->value->num==2);
    waiting.cancel();handle.cancel();temporal.cancel();second.cancel();
    state.cancel();field.cancel();changed.cancel();
    core.ProteusCore::normalizeFromStreamSpec(scalar,"host-callback-test");
    assert(core.callback);
    core.deInit();
    proteusGraph_deinitialize();
    std::cout << "shared library: scalar, live POV, subscription, temporal preservation, shutdown passed\n";
}
