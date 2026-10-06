#include "GraphSearch.h"
#include <cassert>
#include <iostream>
using namespace proteus4;
struct Item;
using N=std::shared_ptr<Item>;
using Q=Query<N>;
struct Item {
    int value=-1;
    DependencyKey valueReady=std::make_shared<Dependency>(),members=std::make_shared<Dependency>();
    std::vector<N> children,required;
    bool legacy=false;
};
static N item(int value=-1) {auto n=std::make_shared<Item>();n->value=value;return n;}
static Edge<N> link(N n) {
    Q q{n,1};Edge<N> e{"member",{},false,[q]{return q;}};e.destination=q;return e;
}
struct SumGoal : GoalSpec<N> {
    std::map<N,int> contributions;
    std::map<N,int> evaluated;
    int sum=0,starts=0,published=-1;
    PersistentTraversal<N> traversal;
    SumGoal(Q root):GoalSpec<N>(root),traversal(root,[this](const Q& q,bool active) {
        auto old=contributions.find(q.node);
        if(old!=contributions.end()) {sum-=old->second;contributions.erase(old);}
        if(active) {
            ReadScope::observe(q.node->valueReady);
            if(q.node->value>=0) {contributions[q.node]=q.node->value;sum+=q.node->value;}
        }
    }) {}
    bool persistent() const override {return true;}
    void connect(std::function<void()> wake,DependencyKey broad,DependencyKey untracked) override {traversal.connect(wake,broad,untracked);}
    void beginPass() override {++starts;}
    std::vector<SetCommand> found(const Visit<N>&) override {return {};}
    bool satisfied() const override {return false;}
    bool symbolic() const override {traversal.observe();return traversal.deferred();}
    std::optional<Expansion<N>> expand(const Visit<N>&) override {
        traversal.advance([&](const Q& q) {
            ++evaluated[q.node];Expansion<N> out;
            ReadScope::observe(q.node->members);ReadScope::observe(q.node->valueReady);
            for(auto n:q.node->children) out.edges.push_back(link(n));
            for(auto n:q.node->required) out.refinements.push_back({{n,1},{},"required",false});
            out.deferred=q.node->value<0;return out;
        });
        Expansion<N> out;out.refinements=traversal.takeRequirements();out.deferred=traversal.deferred();
        if(traversal.ready()) out.edges.push_back(link(request.node));
        return out;
    }
    CommandBatch<N> finishPass() override {
        if(published==sum) return {};
        auto expected=sum;
        return {{request,{},0},{{"publish sum",[this,expected] {
            published=expected;return SetResult{true,Changes::only({})};
        }}}};
    }
};
struct LegacyGoal : GoalSpec<N> {
    using GoalSpec<N>::GoalSpec;
    bool satisfied() const override {return false;}
    bool symbolic() const override {return false;}
    std::vector<SetCommand> found(const Visit<N>&) override {return {};}
};
struct Access : GraphAccess<N> {
    std::shared_ptr<SumGoal> sum;
    bool tracksDependencies() const override {return true;}
    std::shared_ptr<GoalSpec<N>> goal(const Q& q) override {
        if(q.node->legacy) return std::make_shared<LegacyGoal>(q);
        auto created=std::make_shared<SumGoal>(q);if(!sum) sum=created;return created;
    }
    Expansion<N> expand(const Q& q) override {
        assert(q.node->legacy);ReadScope::observe(q.node->members);Expansion<N> out;
        for(auto n:q.node->required) out.refinements.push_back({{n,1},{},"required",false});
        return out;
    }
};
int main() {
    for(int count:{8,16,32,64}) {
        auto root=item(0);for(int i=0;i<count;++i)root->children.push_back(item());
        auto a=std::make_shared<Access>();GraphManager<N> manager(a);
        auto h=manager.normalize({root,1});h.waitForCompletion();
        for(int i=0;i<count;++i) {
            auto n=root->children[i];manager.updateIfChanged([&]{n->value=i+1;return true;},Changes::only({n->valueReady}));
            h.waitForCompletion();assert(a->sum->published==(i+1)*(i+2)/2);
        }
        assert(a->sum->starts==1 && a->sum->evaluated[root]==1);
        assert(a->sum->traversal.counters().evaluations==std::size_t(2*count+1));
        assert(!h.status().symbolicValues && !h.status().failed);
        std::cout<<count<<" arrivals: "<<a->sum->traversal.counters().evaluations<<" evaluations, one root evaluation\n";
    }
    {
        auto root=item(0),unknown=item(),branch=item(3),leaf=item(4);root->children={unknown,branch};branch->children={leaf};
        auto a=std::make_shared<Access>();GraphManager<N> manager(a);
        auto h=manager.normalize({root,1});h.waitForCompletion();assert(a->sum->published==7);
        auto before=a->sum->traversal.counters().evaluations;
        // Duplicate publication rechecks its local premise, never the prefix.
        manager.updateIfChanged([]{return true;},Changes::only({unknown->valueReady}));h.waitForCompletion();
        assert(a->sum->published==7 && a->sum->traversal.counters().evaluations==before+1 && a->sum->evaluated[root]==1);
        auto added=item(9);
        manager.updateIfChanged([&]{root->children.push_back(added);return true;},Changes::only({root->members}));h.waitForCompletion();
        assert(a->sum->published==16 && a->sum->evaluated[branch]==1 && a->sum->evaluated[leaf]==1);
        // Removing real support withdraws only the disconnected branch.
        manager.updateIfChanged([&]{root->children={unknown,added};return true;},Changes::only({root->members}));h.waitForCompletion();
        assert(a->sum->published==9 && a->sum->evaluated[added]==1);
        h.cancel();before=a->sum->traversal.counters().evaluations;
        manager.updateIfChanged([&]{unknown->value=100;return true;},Changes::only({unknown->valueReady}));h.waitForCompletion();
        assert(a->sum->published==9 && a->sum->traversal.counters().evaluations==before);
    }
    {
        auto root=item(0),unknown=item(),aNode=item(1),bNode=item(2),shared=item(4);
        root->children={unknown,aNode,bNode};aNode->children={shared};bNode->children={shared};shared->children={aNode};
        auto a=std::make_shared<Access>();GraphManager<N> manager(a);
        auto h=manager.normalize({root,1});h.waitForCompletion();assert(a->sum->published==7);
        assert(a->sum->evaluated[shared]==1);
        manager.updateIfChanged([&]{root->children={unknown,bNode};return true;},Changes::only({root->members}));h.waitForCompletion();
        assert(a->sum->published==7 && a->sum->evaluated[shared]==1);
        manager.updateIfChanged([&]{root->children={unknown};return true;},Changes::only({root->members}));h.waitForCompletion();
        assert(a->sum->published==0);
        auto notifications=a->sum->traversal.counters().notifications;
        manager.updateIfChanged([]{return true;},Changes::only({shared->valueReady}));h.waitForCompletion();
        assert(a->sum->traversal.counters().notifications==notifications);
        manager.updateIfChanged([&]{root->children={unknown,bNode};return true;},Changes::only({root->members}));h.waitForCompletion();
        assert(a->sum->published==7);
        h.cancel();shared->children.clear();
    }
    {
        // An ancestor reset temporarily detaches a persistent producer. Its
        // retained prerequisite must reactivate too, even if only it is dirty.
        auto root=item(0),parent=item(0),leaf=item();root->legacy=true;
        root->required={parent};parent->required={leaf};
        auto a=std::make_shared<Access>();GraphManager<N> manager(a);
        auto h=manager.normalize({root,1});h.waitForCompletion();assert(h.status().withSubscribers);
        manager.updateIfChanged([&]{leaf->value=7;return true;},Changes::only({root->members,leaf->valueReady}));
        h.waitForCompletion();assert(!h.status().symbolicValues && !h.status().failed && a->sum->starts==1);
    }
    {
        // Completion evidence must outlive the producer's caller, without a
        // flattened copy of its entire dependency graph.
        auto root=item(3);DependencyProof::Ptr proof;std::weak_ptr<void> lifetime;
        {
            PersistentTraversal<N> traversal({root,1},[](const Q&,bool){});
            auto broad=std::make_shared<Dependency>();traversal.connect([]{},broad,broad);
            traversal.advance([](const Q& q){ReadScope::observe(q.node->valueReady);return Expansion<N>{};});
            std::set<DependencyKey> reads;{ReadScope capture([&](const auto& k){reads.insert(k);});traversal.observe();}
            proof=DependencyProof::make(reads);lifetime=(*reads.begin())->evidence;
        }
        assert(proof->valid() && !lifetime.expired());root->valueReady->publish();assert(!proof->valid());
        proof.reset();assert(lifetime.expired());
    }
    std::cout<<"Persistent traversal scenarios passed\n";
}
