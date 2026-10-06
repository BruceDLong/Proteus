#include "GraphSearch.h"
#include <cassert>
#include <iostream>
using namespace proteus4;
struct Item;
using Node=std::shared_ptr<Item>;
struct Item {int value=7;bool rejected=false;std::vector<Node> children;DependencyKey facts=std::make_shared<Dependency>();};
class Goal:public GoalSpec<Node> {
public:
 using GoalSpec::GoalSpec;
 std::vector<SetCommand> found(const Visit<Node>&) override {
  ReadScope::observe({request.node->facts});
  if(request.node->rejected) throw GoalRejected("rejected item");
  return {};
 }
 bool satisfied()const override {ReadScope::observe({request.node->facts});return request.kind==0 || request.node->value>=0;}
 bool symbolic()const override {return !satisfied();}
};
class Access:public GraphAccess<Node> {
public:
 std::map<Query<Node>,int> visits;
 bool tracksDependencies()const override{return true;}
 void observeTask(const Query<Node>&) override{}
 std::shared_ptr<GoalSpec<Node>> goal(const Query<Node>& q) override{return std::make_shared<Goal>(q);}
 Expansion<Node> expand(const Query<Node>& q)override {
  ++visits[q];ReadScope::observe({q.node->facts});Expansion<Node> out;
  if(q.kind==0) {
   out.refinements.push_back({{q.node,1},{0,0},"header",false});
   if(q.argument>0)for(const auto& child:q.node->children)
    out.refinements.push_back({{child,0,q.argument-1},{0,0},"part",false});
  }
  return out;
 }
};
int main(){
 auto a=std::make_shared<Access>();GraphManager<Node> g(a);
 auto root=std::make_shared<Item>(),child=std::make_shared<Item>(),leaf=std::make_shared<Item>();
 root->children={child};child->children={leaf};leaf->value=-1;
 Query<Node> group{root,0};
 auto shallow=g.normalizeShared({root,0,0},group);shallow.waitForCompletion();assert(!shallow.status().symbolicValues);
 auto count=[&](Node n){return g.read([&]{return a->visits[{n,1}];});};
 assert(count(root)==1 && count(child)==0);
 auto deep=g.normalizeShared({root,0,2},group);deep.waitForCompletion();
 assert(deep.status().symbolicValues && deep.status().withSubscribers);
 assert(count(root)==1 && count(child)==1 && count(leaf)==1);
 assert(shallow.status().fullyNormed && !shallow.status().symbolicValues);
 auto other=g.normalizeShared({root,0,2},group);other.waitForCompletion();
 assert(count(root)==1 && count(child)==1 && count(leaf)==1);
 deep.cancel();assert(deep.status().cancelled && !other.status().cancelled);
 g.updateIfChanged([&]{leaf->value=9;return true;},Changes::only({leaf->facts}));other.waitForCompletion();
 assert(!other.status().symbolicValues && count(root)==1 && count(child)==1);
 auto bad=std::make_shared<Item>();bad->rejected=true;
 auto failure=g.normalizeShared({bad,0},group);failure.waitForCompletion();assert(failure.status().failed);
 auto again=g.normalizeShared({root,0,2},group);again.waitForCompletion();assert(!again.status().failed && !again.status().symbolicValues);
 auto waiting=std::make_shared<Item>();waiting->value=-1;
 auto cancelled=g.normalizeShared({waiting,0},group);cancelled.waitForCompletion();cancelled.cancel();
 auto before=count(waiting);
 g.updateIfChanged([&]{waiting->value=4;return true;},Changes::only({waiting->facts}));
 auto last=g.normalizeShared({root,0,2},group);last.waitForCompletion();assert(count(waiting)==before);
 auto dropped=std::make_shared<Item>();dropped->value=-1;
 {
  auto released=g.normalizeShared({dropped,0},group);released.waitForCompletion();
 }
 auto droppedVisits=count(dropped);
 g.updateIfChanged([&]{dropped->value=3;return true;},Changes::only({dropped->facts}));
 auto keeper=g.normalizeShared({root,0,2},group);keeper.waitForCompletion();assert(count(dropped)==droppedVisits);
 auto pending=std::make_shared<Item>();pending->value=-1;
 auto closed=g.normalizeShared({pending,0},group);closed.waitForCompletion();g.close();assert(closed.status().cancelled);
 std::cout<<"shared requests: reuse, depth extension, late data, independent cancellation/rejection, shutdown passed\n";
}
