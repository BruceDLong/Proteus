#define main original_test_main
#include "TestProteus.cpp"
#undef main
#include <iomanip>

using TraceNode=proteus4::POVNode;
static std::string jq(const std::string& value) {
    std::ostringstream out;out<<'"';
    for(unsigned char c:value) switch(c) {
        case '"':out<<"\\\"";break;case '\\':out<<"\\\\";break;
        case '\n':out<<"\\n";break;case '\r':out<<"\\r";break;case '\t':out<<"\\t";break;
        default:if(c<32) out<<"\\u"<<std::hex<<std::setw(4)<<std::setfill('0')<<(int)c<<std::dec;else out<<c;
    }
    out<<'"';return out.str();
}
struct Capture {
    std::ofstream out;
    std::vector<TraceNode> nodes;
    std::vector<std::shared_ptr<infonView>> views;
    std::map<const POV*,int> nodeIDs;
    std::map<const infonView*,int> viewIDs;
    std::map<const infonState*,int> stateIDs;
    int serial=0;
    explicit Capture(const std::string& path):out(path) {}
    std::string node(const TraceNode& n) {
        if(!n)return "";
        auto [it,added]=nodeIDs.emplace(n.get(),nodeIDs.size()+1);
        if(added)nodes.push_back(n);
        return "N"+std::to_string(it->second);
    }
    std::string view(const std::shared_ptr<infonView>& v) {
        if(!v)return "";
        auto [it,added]=viewIDs.emplace(v.get(),viewIDs.size()+1);
        if(added)views.push_back(v);
        return "I"+std::to_string(it->second);
    }
    void discover() {
        std::size_t a=0,b=0;
        while(a<nodes.size() || b<views.size()) {
            while(a<nodes.size()) {
                auto n=nodes[a++];view(n->pItem);node(n->next);node(n->prev);node(n->outerPOV);node(n->altPOV);
                if(n->identityRegistry) for(const auto& r:proteus4::POVDescription(n,n->identityRegistry).relationships()) node(r.target);
            }
            while(b<views.size()) {
                auto v=views[b++];view(v->innr);view(v->selectionExpression);node(v->selectedResult);
                if(v->value) {node(v->value->items.head);view(v->value->listSpec);}
                for(const auto& w:v->wrkList)node(w);
            }
        }
    }
    std::string state(const std::shared_ptr<infonState>& v) {
        if(!v)return "null";
        auto [it,unused]=stateIDs.emplace(v.get(),stateIDs.size()+1);
        static const char* overlays[]={"unset","unknown","number","string","list"};
        static const char* modes[]={"unknown","concat","literal","intersection"};
        auto overlay=getModeBits(v->flags,infonState::overlayType);
        auto mode=getModeBits(v->flags,infonState::evalMode);
        std::ostringstream o;o<<"{\"id\":"<<jq("S"+std::to_string(it->second))<<",\"overlay\":"<<jq(overlays[overlay])<<",\"mode\":"<<jq(modes[mode]);
        if(overlay==infonState::NUM && mode==infonState::emLiteral) o<<",\"number\":"<<jq(v->num.val.get_str());
        if(overlay==infonState::STR && mode==infonState::emLiteral) o<<",\"text\":"<<jq(v->str);
        o<<",\"open\":"<<(getFlagBit(v->flags,infonState::tailUnfinished)?"true":"false")<<",\"members\":[";
        bool first=true;for(auto n=v->items.head;n;n=n->next) {if(!first)o<<',';first=false;o<<jq(node(n));}
        o<<"]}";return o.str();
    }
    std::string snapshot() {
        discover();std::ostringstream o;o<<"{\"nodes\":[";bool first=true;
        for(const auto& n:nodes) {
            if(!first)o<<',';first=false;
            o<<"{\"id\":"<<jq(node(n))<<",\"infon\":"<<jq(view(n->pItem))<<",\"next\":"<<jq(node(n->next))<<",\"prev\":"<<jq(node(n->prev))<<",\"parent\":";
            auto p=nodeIDs.find(n->pParent);o<<jq(p==nodeIDs.end()?"":"N"+std::to_string(p->second));
            o<<",\"relations\":[";bool f=true;
            if(n->identityRegistry) for(const auto& r:proteus4::POVDescription(n,n->identityRegistry).relationships()) {
                if(!f)o<<',';f=false;o<<"{\"label\":"<<jq(r.description())<<",\"to\":"<<jq(node(r.target))<<"}";
            }
            o<<"]}";
        }
        o<<"],\"infons\":[";first=true;
        for(const auto& v:views) {
            if(!first)o<<',';first=false;
            o<<"{\"id\":"<<jq(view(v))<<",\"type\":"<<jq(v->type?v->type->normedWord:"")<<",\"size\":"<<state(v->infSize)<<",\"value\":"<<state(v->value)<<",\"normed\":"<<(getFlagBit(v->flags,infonView::isNormed)?"true":"false")<<",\"parse\":"<<getModeBits(v->flags,infonView::streamState)<<",\"inner\":"<<jq(view(v->innr))<<",\"selectionExpression\":"<<jq(view(v->selectionExpression))<<",\"selected\":"<<jq(node(v->selectedResult))<<",\"work\":[";
            bool f=true;for(const auto& w:v->wrkList) {if(!f)o<<',';f=false;o<<jq(node(w));}o<<"]}";
        }
        o<<"]}";return o.str();
    }
    std::string query(const proteus4::Query<TraceNode>& q) {
        std::ostringstream o;o<<"{\"node\":"<<jq(node(q.node))<<",\"kind\":"<<q.kind<<",\"argument\":"<<q.argument<<",\"related\":"<<jq(node(q.related))<<",\"context\":[";
        bool first=true;for(const auto& n:q.context){if(!first)o<<',';first=false;o<<jq(node(n));}o<<"],\"coordinates\":[";first=true;
        for(auto c:q.coordinates){if(!first)o<<',';first=false;o<<c;}o<<"]}";return o.str();
    }
    void event(const proteus4::DiagnosticEvent<TraceNode>& e) {
        auto from=query(e.from),to=query(e.to);auto model=snapshot();
        out<<"{\"seq\":"<<++serial<<",\"event\":"<<jq(e.event)<<",\"detail\":"<<jq(e.detail)<<",\"task\":"<<e.task<<",\"other\":"<<e.other<<",\"revision\":"<<e.revision<<",\"cost\":"<<e.cost<<",\"extra\":"<<e.extra<<",\"from\":"<<from<<",\"to\":"<<to<<",\"snapshot\":"<<model<<"}\n";
    }
};
static std::vector<std::string> signature(const Proteus4Handle& h) {
    std::vector<std::string> r;
    for(const auto& s:h.steps())r.push_back(s.action+"|"+std::to_string(s.query.kind)+"|"+std::to_string(s.query.argument)+"|"+std::to_string(s.cost)+"|"+std::to_string(s.revision));
    return r;
}
int main() {
    initialize("");
    std::vector<std::string> inputs={"*10+_=5",R"({"A" $="B"  ("C") })","[? *10+7]"};
    for(std::size_t k=0;k<inputs.size();++k) {
        const auto label="case-"+std::to_string(k+1);
        std::vector<std::string> baseline;std::string baselineResult;
        for(int trial=0;trial<2;++trial) {
            GraphNormalizer n;n.init();auto root=graphParsed(label,inputs[k]);
            if(!root || !root->pItem) {std::cerr<<"Parse failed "<<inputs[k]<<"\n";return 2;}
            std::unique_ptr<Capture> capture;
            if(trial) {
                capture=std::make_unique<Capture>("../"+label+".jsonl");capture->node(root);
                capture->event({"Parsed input",inputs[k],-1,-1,(std::int64_t)n.runtime.revision(),{root,proteus4::sizeAndValue,proteus4::fillImmediateParts},{}});
                proteus4::diagnosticSink<TraceNode> = [&](const auto& e){capture->event(e);};
            }
            auto h=n.normalize(root);h.waitForCompletion();auto status=h.status();
            if(trial) {
                proteus4::diagnosticSink<TraceNode> = {};
                capture->event({"Wait returned",status.failed ? status.error : status.symbolicValues ? "symbolic" : "resolved",0,-1,(std::int64_t)n.runtime.revision(),{root,proteus4::sizeAndValue,1},{}});
            }
            auto sig=signature(h);auto result=toString(root->pItem.get());
            if(!trial) {baseline=sig;baselineResult=result;}
            else {
                bool same=sig==baseline && result==baselineResult;
                std::ofstream summary("../"+label+"-summary.json");
                summary<<"{\"input\":"<<jq(inputs[k])<<",\"result\":"<<jq(result)<<",\"failed\":"<<(status.failed?"true":"false")<<",\"symbolic\":"<<(status.symbolicValues?"true":"false")<<",\"subscribers\":"<<(status.withSubscribers?"true":"false")<<",\"fullyNormed\":"<<(status.fullyNormed?"true":"false")<<",\"isNormed\":"<<(getFlagBit(root->pItem->flags,infonView::isNormed)?"true":"false")<<",\"instrumentationMatchesControl\":"<<(same?"true":"false")<<",\"steps\":[";
                bool first=true;for(const auto& s:h.steps()) {
                    if(!first)summary<<',';first=false;
                    summary<<"{\"action\":"<<jq(s.action)<<",\"query\":"<<capture->query(s.query)<<",\"cost\":"<<s.cost<<",\"revision\":"<<s.revision<<"}";
                }
                summary<<"]}\n";
                std::cout<<label<<" result="<<result<<" native_steps="<<sig.size()<<" captured_events="<<capture->serial<<" control_match="<<same<<" failed="<<status.failed<<" symbolic="<<status.symbolicValues<<"\n";
                if(!same || status.failed)return 3;
            }
            h.cancel();n.close();
        }
    }
}
