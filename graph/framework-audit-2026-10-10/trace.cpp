#include "proteusGraph.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
using Node=std::shared_ptr<POV>;
std::string read(const char* path){std::ifstream f(path);if(!f)throw std::runtime_error(path);return {(std::istreambuf_iterator<char>(f)),{}};}
int main(int argc,char**argv){if(argc!=3)return 2;std::cout.setf(std::ios::unitbuf);std::cout.precision(12);proteusGraph_initialize("");ProteusCore core;core.init("",false);auto& g=core.agent->graphNormalizer;
auto parse=[&](std::string text){auto p=std::make_shared<POV>(909);p->pItem=core.parseAndExtractString(text,"framework-audit",false);if(!p->pItem)throw std::runtime_error(core.errorMesg);return p;};
auto defs=parse(read(argv[1]));auto dh=g.normalize(defs);dh.waitForCompletion();if(dh.status().failed)throw std::runtime_error(dh.status().error);auto defsBefore=defs->stringify();auto root=parse(read(argv[2]));auto before=root->stringify();
auto h=g.normalizeAll(root);h.waitForCompletion();auto status=h.status();if(status.failed)throw std::runtime_error(status.error);
std::map<std::string,size_t> actions;std::map<proteus4::Query<Node>,size_t> visits;auto steps=h.steps();
g.runtime.read([&]{std::cout<<"BEFORE "<<before<<"\nAFTER "<<root->stringify()<<"\nSTATUS symbolic="<<status.symbolicValues<<" declarations_unchanged="<<(defsBefore==defs->stringify())<<"\n";
for(auto& s:steps){++actions[s.action];if(s.action=="visit")++visits[s.query];auto q=s.query;std::cout<<"STEP "<<s.revision<<" | "<<s.action<<" | "<<(q.node?q.node->mySymbol():"null")<<" type="<<(q.node&&q.node->pItem?q.node->pItem->getTypeStr():"")<<" kind="<<q.kind<<" arg="<<q.argument<<" related="<<(q.related?q.related->mySymbol():"null")<<" ctx=";for(auto c:q.context)std::cout<<c->mySymbol()<<',';std::cout<<" coords=";for(auto c:q.coordinates)std::cout<<c<<',';std::cout<<'\n';}});
std::cout<<"COUNTS events="<<steps.size()<<" visits="<<actions["visit"]<<" distinct_visited="<<visits.size()<<"\n";for(auto a:actions)std::cout<<"ACTION "<<a.second<<' '<<a.first<<'\n';h.cancel();dh.cancel();core.deInit();proteusGraph_deinitialize();}
