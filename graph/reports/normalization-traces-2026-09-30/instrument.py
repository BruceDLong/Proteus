from pathlib import Path
import sys
p=Path(sys.argv[1])/'graph/GraphSearch.h';s=p.read_text()
def replace(a,b):
 global s
 assert a in s,a[:100]
 s=s.replace(a,b,1)
replace('template<class Node> struct Visit {', '''// Diagnostic-only observer. No scheduling or semantic behavior is changed.
template<class Node> struct DiagnosticEvent {
    std::string event, detail;
    std::int64_t task=-1, other=-1, revision=-1;
    Query<Node> from{}, to{};
    Cost cost=0;
    std::int64_t extra=0;
};
template<class Node> inline std::function<void(const DiagnosticEvent<Node>&)> diagnosticSink;
template<class Node> void diagnostic(DiagnosticEvent<Node> event) {
    if(diagnosticSink<Node>) diagnosticSink<Node>(event);
}

template<class Node> struct Visit {''')
replace('if(newlyDirty) {task.dirty=true;++task.generation;}', '''if(newlyDirty) {task.dirty=true;++task.generation;
            diagnostic<Node>({"Task invalidated","",(std::int64_t)index,-1,-1,task.goal->request,{},0,(std::int64_t)task.generation});
        }''')
replace('        ++revision;\n        if(changes.all)', '''        ++revision;
        diagnostic<Node>({"Graph change published",changes.all ? "broad" : "dependency keys",-1,-1,(std::int64_t)revision,{},{},0,(std::int64_t)changes.keys.size()});
        if(changes.all)''')
replace('        s.frontier.push({task, cost, estimate, s.sequence++, std::move(edge),', '''        diagnostic<Node>({edge.command ? "Set queued" : "Edge queued",edge.description,(std::int64_t)task,-1,(std::int64_t)s.status.revision,path.empty() ? s.tasks[task].goal->request : path.back(),{},cost,(std::int64_t)s.tasks[task].generation});
        s.frontier.push({task, cost, estimate, s.sequence++, std::move(edge),''')
replace('        s.rejected.erase(index);\n        task.dirty=false;', '''        diagnostic<Node>({"Pass started","visited entries cleared",(std::int64_t)index,-1,(std::int64_t)c.revision,task.goal->request,{},task.baseCost,(std::int64_t)task.visited.size()});
        s.rejected.erase(index);
        task.dirty=false;''')
replace('            s.tasks.push_back(std::move(task));\n        } else index=existing->second;', '''            s.tasks.push_back(std::move(task));
            diagnostic<Node>({"Task initialized","",(std::int64_t)index,-1,(std::int64_t)c.revision,query,{},cost});
        } else {
            index=existing->second;
            diagnostic<Node>({"Task reused","",(std::int64_t)index,-1,(std::int64_t)c.revision,query,{},cost});
        }''')
replace('             task.goal->settled(true,proof);', '''             task.goal->settled(true,proof);
             diagnostic<Node>({"Goal settled","resolved",(std::int64_t)i,-1,(std::int64_t)c.revision,task.goal->request,{}});''')
replace('                s.tasks[i].goal->settled(false,{});', '''                s.tasks[i].goal->settled(false,{});
                diagnostic<Node>({"Goal settled","symbolic",(std::int64_t)i,-1,(std::int64_t)c.revision,s.tasks[i].goal->request,{}});''')
replace('            c.changed.notify_all();\n            return;\n        }\n        auto pending = s.frontier.top();', '''            diagnostic<Node>({"Search settled",s.status.withSubscribers ? "subscribed" : "terminal",0,-1,(std::int64_t)c.revision,s.root->request,{}});
            c.changed.notify_all();
            return;
        }
        auto pending = s.frontier.top();''')
replace('        if (pending.generation != task.generation) return;', '''        if (pending.generation != task.generation) {
            diagnostic<Node>({"Stale work discarded",pending.edge.description,(std::int64_t)pending.task,-1,(std::int64_t)c.revision,pending.path.empty() ? task.goal->request : pending.path.back(),{},pending.cost,(std::int64_t)pending.generation});
            return;
        }''')
replace('        auto arrival = pending.edge.follow();\n        auto query = arrival.query;', '''        auto origin=pending.path.empty() ? goal->request : pending.path.back();
        if(pending.edge.command) diagnostic<Node>({"Set started",pending.edge.description,(std::int64_t)pending.task,-1,(std::int64_t)c.revision,origin,{},pending.cost});
        auto arrival = pending.edge.follow();
        auto query = arrival.query;
        diagnostic<Node>({pending.edge.command ? "Set applied" : "Edge followed",pending.edge.description,(std::int64_t)pending.task,-1,(std::int64_t)c.revision,origin,query,pending.cost,arrival.graphChanged ? 1 : 0});''')
replace('        if (previous != visited.end() && previous->second <= pending.cost) return;', '''        if (previous != visited.end() && previous->second <= pending.cost) {
            diagnostic<Node>({"Visit skipped","already visited at equal or lower cost",(std::int64_t)pending.task,-1,(std::int64_t)c.revision,origin,query,pending.cost});
            return;
        }''')
replace('        Visit<Node> visit{query, pending.path, pending.cost};', '''        diagnostic<Node>({"Visit accepted",pending.edge.description,(std::int64_t)pending.task,-1,(std::int64_t)c.revision,origin,query,pending.cost});
        Visit<Node> visit{query, pending.path, pending.cost};''')
replace('            auto child = startTask(c,s,c.access->goal(refinement.query),cost);', '''            auto child = startTask(c,s,c.access->goal(refinement.query),cost);
            diagnostic<Node>({"Subgoal requested",refinement.description,(std::int64_t)pending.task,(std::int64_t)child,(std::int64_t)c.revision,query,refinement.query,cost,refinement.alternative ? 1 : 0});''')
replace('        sessions.push_back(session);\n        context_->changed.notify_all();', '''        sessions.push_back(session);
        diagnostic<Node>({"Search requested","normalize",0,-1,(std::int64_t)context_->revision,query,{}});
        context_->changed.notify_all();''')
p.write_text(s)
