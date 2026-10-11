from pathlib import Path
import collections
import difflib
import html
import json
import re
import shutil
import subprocess

BASE = Path(__file__).resolve().parent
OUT = BASE / 'report'
OUT.mkdir(exist_ok=True)
SOURCE = Path('/home/bruce/devl/Proteus4')
escape = lambda s: html.escape(str(s), quote=True)
demands = dict((int(v), k) for k, v in re.findall(r'(\w+)\s*=\s*(\d+)', (SOURCE/'graph/ProteusGraph.h').read_text().split('enum Demand {')[1].split('};')[0]))
cases = []
for i in range(1, 4):
    events = [json.loads(line) for line in (BASE/f'case-{i}.jsonl').read_text().splitlines()]
    summary = json.loads((BASE/f'case-{i}-summary.json').read_text())
    rev, root_pass = 0, 0
    for e in events:
        if e['revision'] >= 0: rev = e['revision']
        if e['event'] == 'Pass started' and e['task'] == 0: root_pass += 1
        e['displayRevision'], e['rootPass'] = rev, root_pass
    cases.append(dict(id=i, events=events, summary=summary))

def query(q):
    if not q['node']: return '—'
    text = f"{q['node']} · {demands.get(q['kind'], q['kind'])}"
    if q['argument']: text += f" · arg={q['argument']}"
    if q['related']: text += f" · related={q['related']}"
    if q['context']: text += f" · context=[{','.join(q['context'])}]"
    if q['coordinates']: text += f" · coordinates={q['coordinates']}"
    return text

def table(headers, rows, cls=''):
    return '<div class="table-wrap"><table class="'+cls+'"><thead><tr>'+''.join('<th>'+escape(h)+'</th>' for h in headers)+'</tr></thead><tbody>'+''.join('<tr>'+''.join('<td>'+str(v)+'</td>' for v in row)+'</tr>' for row in rows)+'</tbody></table></div>'

def code(s): return '<code>'+escape(s)+'</code>'
def link_event(i,n,text=None): return f'<button class="event-link" onclick="inspect({i},{n})">{escape(text or "#"+str(n))}</button>'
def dq(s): return json.dumps(str(s), ensure_ascii=False)

labels = {
 1: {'N1':'N1 / I1 · target\nsize 10 · value unknown', 'N2':'N2 / I2 · RHS\nsize unknown · value 5'},
 2: {'N1':'N1 / I1 · outer list\nsize 3 · [N2, N3, N4]', 'N2':'N2 / I2 · "A"\nsize 1 · literal string', 'N3':'N3 / I3 · $="B"\nsize unknown · string unknown', 'N4':'N4 / I4 · ("C")\nsize unknown · concatenation', 'N5':'N5 / I5 · RHS "B"\nsize 1 · literal string', 'N6':'N6 / I6 · operand "C"\nsize 1 · literal string'},
 3: {'N1':'N1 / I1 · intersection\nsize unknown · [? *10+7]', 'N2':'N2 / I3 · ?\nsize, value and overlay unknown', 'N3':'N3 / I4 · last member\nsize 10 · value 7', 'I2':'I2 · inner list storage\nsize 2 · [N2, N3]\n(no separate POV in this trace)'}
}

def graph(i, name, edges, extras=None, direction='LR'):
    if i == 2: direction = 'TB'
    lines = ['digraph G {', f'graph [rankdir={direction}, bgcolor="transparent", pad="0.25", nodesep="0.6", ranksep="0.8", fontname="DejaVu Sans"];',
      'node [shape=box, style="rounded,filled", fillcolor="#f4f7fb", color="#c8d3e3", fontname="DejaVu Sans", fontsize=12, margin="0.16,0.12"];',
      'edge [fontname="DejaVu Sans", fontsize=10, color="#247387", fontcolor="#17586a", arrowsize=0.8];']
    for n,label in labels[i].items():
        if name=='followed' and n=='I2': continue
        lines.append(f'{n} [label={dq(label)}'+(', style="dashed,rounded,filled"' if n=='I2' else '')+'];')
    for a,b,label,style in edges:
        lines.append(f'{a} -> {b} [label={dq(label)}{", "+style if style else ""}];')
    lines.extend(extras or [])
    lines.append('}')
    dot='\n'.join(lines)
    path=OUT/f'case-{i}-{name}.dot';path.write_text(dot)
    subprocess.run(['dot','-Tsvg',str(path),'-o',str(path.with_suffix('.svg'))],check=True)
    svg=path.with_suffix('.svg').read_text(); svg=svg[svg.index('<svg'):]
    svg=svg.replace('<svg ',f'<svg role="img" aria-label="Case {i}: {name} graph" ',1)
    return f'<div class="graph">{svg}</div>'

initial_edges={
1:[('N1','N2','wrkList[0]','')],
2:[('N1','N2','value.items.head',''),('N2','N3','next',''),('N3','N4','next',''),('N3','N5','wrkList[0]',''),('N4','N6','concat operand / items.head','')],
3:[('N1','I2','innr','style=dashed'),('I2','N2','value.items.head',''),('N2','N3','next','')]
}

important = {
1:[(2,'Search started: normalize N1; a single handle is created.'),(3,'T0 initialized for size, value and overlay; pass 1 begins at #4.'),(9,'T1 initialized to assess whether RHS N2 is already represented.'),(16,'Follow N1 → N2 through the work-list constraint; route cost 1.'),(20,'Check RHS representation: proof metadata only, no graph change.'),(22,'Set N1.value = 5. N1.size was already 10; N2 has no known size to copy.'),(23,'Publish model change r1. T0 and T1 are both invalidated (#24–25).'),(26,'T0 pass 2 starts; its 2 visited states are cleared.'),(31,'Reuse T1, then reset its pass (#32); no new task is created.'),(38,'Follow the same work-list edge again.'),(42,'Representation check runs again; no graph change.'),(44,'The copy Set runs again; nothing changes.'),(45,'T1 settles, then T0 settles at #46.'),(48,'waitForCompletion returns: *10+5, fully normalized.')],
2:[(2,'Search started: normalize the outer list N1.'),(8,'T1 initialized to inspect its members. The parser already supplied list size 3.'),(15,'List walk reaches N2, the known string "A"; T2 is initialized at #18.'),(24,'List walk reaches N3, $="B"; T3 initializes at #27 and RHS-assessment T4 at #34.'),(41,'List walk reaches N4, ("C"); T5 initializes at #44.'),(48,'T3 follows N3 → N5, the work-list RHS "B".'),(55,'T6 initializes to resolve the concatenation; its first visit is queued.'),(60,'Set N3.size = 1 and N3.value = "B" together.'),(61,'Publish r1; all 7 tasks created so far become dirty (#62–68).'),(69,'Root pass 2. The list walk and the already-known "A" goal start again.'),(106,'Discard T6’s old queued entry visit because its generation is stale.'),(132,'T6 follows N4 → N6, the concatenation operand "C".'),(135,'T7 initializes to demand the operand’s value.'),(140,'Set counted list size = 3; unchanged.'),(147,'Set N4.size = 1 and N4.value = "C"; replace the concat expression value.'),(148,'Publish r2 broadly; all 8 tasks become dirty (#149–156).'),(157,'Root pass 3. The same list/member/concat routes are retraced.'),(233,'Set concatenation result again; unchanged.'),(234,'Discard the older queued consolidation Set; a current one remains.'),(236,'Consolidate N3’s RHS: remove N5 from wrkList, retain its equality in the registry.'),(237,'Publish r3 broadly; all 8 tasks become dirty (#238–245).'),(246,'Root pass 4, caused by consolidation after the member values were filled.'),(283,'N3 → N5 is now a consolidated-equality edge.'),(301,'Follow the registry identity back from N5 → N3.'),(326,'A later registry return is followed, then skipped at #327 as already visited at lower cost.'),(328,'Required goals settle (#328–334). T4 is no longer required after consolidation.'),(336,'waitForCompletion returns: {\'A\' \'B\' \'C\'}, fully normalized.')],
3:[(2,'Search started: normalize intersection N1.'),(8,'T1 initialized to find the selected intersection result.'),(15,'Follow N1 → N2 (?) to inspect list extent, not to normalize its value.'),(18,'Continue N2 → N3 (*10+7). No value/size subgoal is created for N2.'),(21,'Reach the list boundary; extent validation succeeds.'),(24,'Choose the last result position, N3.'),(27,'Arrive at N3 with demand selectionFound.'),(31,'Set selected result identity N1 = N3. Retain the expression in selectionExpression.'),(32,'Publish r1 broadly; both T0 and T1 become dirty.'),(35,'Root pass 2; T1 is reused and its 6 visited states are cleared at #41.'),(47,'T0 follows the new registry identity N1 → N3.'),(51,'T1 has already restarted its extent walk through the unknown N2.'),(55,'Set N1.size = 10, N1.value = 7 and numeric overlay together.'),(56,'Publish r2; both tasks become dirty again.'),(59,'Root pass 3. The root identity route and selection route are retraced.'),(79,'Repeat the value/size Set; unchanged.'),(80,'Discard an old queued identity edge; an old extent edge is discarded at #81.'),(90,'Follow identity N1 → N3 again; skip duplicate expansion at #91.'),(98,'The last-member selection is established again.'),(102,'Set selected result identity again; unchanged.'),(103,'T1 settles, then T0 settles at #104.'),(106,'waitForCompletion returns: *10+7. N2 remains unknown and isNormed=false.')]
}

changes = {
 (1,22):'N1 / I1: value unknown → 5. Size stays 10; numeric overlay was already known.',
 (2,60):'N3 / I3: size unknown → 1; value unknown → "B". String overlay was already known.',
 (2,147):'N4 / I4: size unknown → 1; value concat([N6]) → literal string "C" (including the overlay).',
 (2,236):'N3 / I3: wrkList [N5] → []. Relationship becomes consolidated equality N3 → N5; registry also exposes N5 → N3. Values do not change.',
 (3,31):'N1 / I1: selectedResult = N3; innr I2 moves to selectionExpression; intersection value becomes an unknown scalar placeholder. Registry identity N1 ↔ N3 is added.',
 (3,55):'N1 / I1: size unknown → 10; value unknown → 7; overlay unknown → number.'
}

task_meanings={
1:['Normalize target N1','Assess N1’s RHS N2'],
2:['Normalize outer list N1','Walk N1’s members and confirm extent','Normalize "A" (N2)','Normalize $="B" (N3)','Assess N3’s RHS N5','Normalize ("C") (N4)','Resolve N4’s concatenated value','Read operand N6’s value'],
3:['Normalize intersection N1','Locate and validate its selected result N3']
}

notes={
1:'''<p>One value write produces two root passes. The second pass re-reads the same RHS and re-runs both Sets. The RHS remains in <code>wrkList</code>: its size is unknown, so the current closed-equality consolidation rule does not remove it. This does not prevent the requested target from completing.</p>''',
2:'''<p>The model needs two value-producing Sets: fill <code>$</code> from <code>"B"</code>, then evaluate <code>("C")</code>. A third changing Set consolidates the RHS relationship. Each publication restarts the root walk. The unchanged <code>"A"</code> member is entered four times.</p><p><code>("C")</code> is parsed as a concatenation expression. The outer list already has size 3 before normalization; its repeated counted-size Sets do not supply new information.</p>''',
3:'''<p>The unknown first member is only inspected for position and extent. No normalize subgoal is created for it, and it remains unknown in the final model. This is the intended demand-driven behavior.</p><p>Selection first creates an identity to the last member; a later Set obtains size, value and overlay through that identity. These two publications cause three root passes and repeated selection walks. The inner expression remains available in <code>selectionExpression</code>.</p>'''
}

css = '''
:root{color-scheme:light;--ink:#182636;--muted:#53667a;--blue:#17586a;--line:#d6e0ea;--bg:#f3f6fa;--orange:#8f4d10}*{box-sizing:border-box}body{margin:0;font:16px/1.58 system-ui,sans-serif;color:var(--ink);background:var(--bg)}header,main,footer{max-width:1220px;margin:auto;padding:32px}header{padding-top:44px;padding-bottom:20px}h1{font-size:36px;line-height:1.15;margin:8px 0 18px;letter-spacing:-.7px}h2{font-size:25px;line-height:1.3;margin:0 0 16px}h3{font-size:19px;margin:26px 0 10px}p{max-width:100ch;margin:12px 0}a,.event-link{color:var(--blue)}a:hover,.event-link:hover{text-decoration:underline}.eyebrow{font-size:12px;letter-spacing:1.3px;text-transform:uppercase;color:var(--muted);font-weight:700}nav{display:flex;gap:10px;flex-wrap:wrap;margin-top:24px}nav a{background:white;border:1px solid var(--line);border-radius:7px;padding:8px 13px;text-decoration:none}section{background:white;border:1px solid var(--line);border-radius:12px;padding:28px;margin-bottom:26px;scroll-margin-top:20px}.lede{font-size:19px}.muted,figcaption,small{color:var(--muted)}.callout{background:#edf5f7;border-left:4px solid #247387;padding:14px 18px;margin:20px 0}.callout p{margin:3px 0}code{font:14px/1.5 ui-monospace,monospace;background:#f0f3f7;padding:2px 5px;border-radius:4px;white-space:pre-wrap;overflow-wrap:anywhere}.io{background:#122e3e;color:#fff;border-radius:8px;padding:16px 20px;display:flex;gap:16px;flex-wrap:wrap;align-items:center}.io code{background:transparent;color:inherit;font-size:19px;padding:0}.io span{color:#a9c6d5}.chips{display:flex;gap:8px;flex-wrap:wrap;margin:14px 0}.chip{padding:4px 9px;border:1px solid var(--line);border-radius:20px;font-size:13px}.table-wrap{overflow-x:auto;margin:12px 0}table{border-collapse:collapse;width:100%;font-size:14px;text-align:left}th{background:#eef3f7;color:#385168;font-weight:650}th,td{border-bottom:1px solid var(--line);padding:9px 10px;vertical-align:top}td:first-child{white-space:nowrap}tbody tr:hover{background:#f8fbfd}.graph{overflow:auto;padding:10px 0}.graph svg{display:block;max-width:100%;height:auto;margin:auto}.graph svg text{font-family:system-ui,sans-serif}figure{margin:14px 0 22px}figcaption{font-size:13px;margin:8px 0}.event-link{font:inherit;border:0;padding:0;background:none;cursor:pointer;text-decoration:underline;text-underline-offset:3px}.changed{color:#805016;font-weight:650}.quiet{color:var(--muted)}.two{display:grid;grid-template-columns:1fr 1fr;gap:22px}details{border:1px solid var(--line);border-radius:8px;margin:18px 0;padding:14px 16px}summary{cursor:pointer;font-weight:650}.log-toolbar{display:flex;gap:12px;align-items:center;flex-wrap:wrap;padding:14px 0}input,select,button{font:inherit}input,select{border:1px solid #bacbd9;border-radius:5px;padding:6px 9px;background:white;color:var(--ink)}input{min-width:200px;flex:1}.log{max-height:680px;overflow:auto}.log table{font-size:13px}.log th{position:sticky;top:0;z-index:1}.log td:nth-child(5){min-width:290px}.log td:nth-child(6){min-width:270px}.log .mutation{background:#fff8ea}.log .restart{background:#edf5f7}.small{font-size:13px}.source-list li{margin:10px 0}dialog{width:min(960px,94vw);max-height:90vh;padding:25px;border:1px solid var(--line);border-radius:12px;box-shadow:0 20px 70px #15243244}dialog::backdrop{background:#112c4488}.dialog-head{display:flex;justify-content:space-between;align-items:start;gap:20px}.dialog-head button{border:1px solid var(--line);border-radius:6px;background:white;padding:6px 12px;cursor:pointer}pre{white-space:pre-wrap;overflow-wrap:anywhere;font:13px/1.45 ui-monospace,monospace;max-height:55vh;overflow:auto;background:#f3f6fa;padding:16px}.legend{font-size:13px;color:var(--muted)}footer{padding-top:0;font-size:13px;color:var(--muted)}@media(max-width:760px){header,main,footer{padding:20px 14px}section{padding:18px}h1{font-size:29px}.two{display:block}.graph svg{min-width:590px}table{font-size:13px}}@media print{body{background:white}header,main{padding:8px;max-width:none}section{break-inside:auto;box-shadow:none}nav,.log-toolbar,.rawlog,dialog{display:none!important}h2,h3{break-after:avoid}figure,tr{break-inside:avoid}.graph svg{max-height:410px}details{display:block}a{color:inherit}}
'''

parts=['<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1"><title>Proteus4 — three normalization traces</title><style>'+css+'</style></head><body>',
'''<header><div class="eyebrow">Proteus4 · observed execution · 30 September 2026</div><h1>Three normalization traces</h1><p class="lede">Each example starts one search handle. Sets then invalidate work and cause additional passes through that same search.</p><p class="muted">Captured from local commit <code>ad57661de65d6ce7bd385dde40df9a69cd21699c</code>, with normal notifications enabled. This is a diagnostic report, not an engine change or a timing benchmark.</p><nav><a href="#overview">Comparison</a><a href="#case-1">1 · Scalar RHS</a><a href="#case-2">2 · List + concat</a><a href="#case-3">3 · Intersection</a><a href="#optimization">Optimization implications</a><a href="#method">Method &amp; reproduction</a></nav></header><main><section id="overview"><h2>What the traces show</h2>''']

rows=[]
for c in cases:
    e,s,i=c['events'],c['summary'],c['id'];counts=collections.Counter(x['event'] for x in e)
    rootpasses=sum(x['event']=='Pass started' and x['task']==0 for x in e)
    changed=sum(x['event']=='Set applied' and x['extra']==1 for x in e)
    rows.append([f'<a href="#case-{i}">{code(s["input"])}</a>',code(s['result']),str(rootpasses),str(counts['Task initialized']),str(changed)+' / '+str(counts['Set applied']-changed),str(counts['Edge followed']-sum(x['event']=='Edge followed' and x['detail']=='visit' for x in e)),str(counts['Pass started']),str(counts['Stale work discarded'])])
parts += [table(['Input','Printed result','Root passes','Distinct tasks¹','Changed / unchanged Sets','Relationship follows²','All task passes','Stale items discarded'],rows),
'''<p class="small">¹ Includes the root task. ² Excludes synthetic task-entry <code>visit</code> events; includes followed edges whose destination is then skipped by deduplication. The complete log distinguishes both.</p><div class="callout"><p><strong>The clearest optimization target is invalidation scope.</strong> The domain adapter currently makes ordinary tasks observe a shared <code>modelFacts_</code> token. Thus even a targeted value publication wakes unrelated goals within these examples. Resetting a dirty task clears its visited states and goal-local findings.</p></div><p>All three results are fully normalized, with no unresolved subscription and no failure. All inputs were parsed before the search began. There are <strong>no parser-arrival restarts</strong> in this report.</p><h3>Reading the diagrams and event numbers</h3><p><code>N1</code>, <code>N2</code>, … identify actual POVs; <code>I1</code>, <code>I2</code>, … identify their infonViews. IDs are local to each example. The initial diagram shows stored forward links (reverse list links are omitted). The traversal diagram projects actual followed edges onto those POVs; an edge may change the demand at the same POV. It is a report view, not a second graph maintained by the engine.</p><p>Event numbers are sequential observations, not timestamps. P0 means the root pass has not started yet. Click an event number to inspect its exact query and model snapshot. A new task is initialized once; a later pass or task reuse does not allocate another subgoal. Set application and graph publication are separate events.</p></section>''']

for c in cases:
    i,es,s=c['id'],c['events'],c['summary']
    parts.append(f'<section id="case-{i}"><div class="eyebrow">Case {i}</div><h2>'+['Scalar RHS','List members and concatenation','Intersection result selection'][i-1]+'</h2>')
    parts.append(f'<div class="io">{code(s["input"])}<span>→</span>{code(s["result"])}</div>')
    counts=collections.Counter(e['event'] for e in es)
    parts.append('<div class="chips">'+''.join(f'<span class="chip">{escape(t)}</span>' for t in [f'{sum(e["event"]=="Pass started" and e["task"]==0 for e in es)} root passes',f'{counts["Task initialized"]} distinct tasks',f'{len(s["steps"])} native handle steps',f'{len(es)} diagnostic events','fullyNormed = true'])+'</div>')
    parts.append(notes[i])
    parts.append('<h3>Initial stored structure</h3><figure>'+graph(i,'initial',initial_edges[i])+'<figcaption>Before normalization. List <code>prev</code> links and scalar state objects are omitted for clarity; raw snapshots contain the state fields.</figcaption></figure>')
    # Project follow events onto POVs. Full query identities remain in the table.
    followed=collections.OrderedDict()
    full=collections.OrderedDict()
    for e in es:
        if e['event']!='Edge followed' or e['detail']=='visit': continue
        key=(e['from']['node'],e['to']['node'],e['detail'])
        followed.setdefault(key,[]).append(e)
        exact=(query(e['from']),query(e['to']),e['detail'])
        full.setdefault(exact,[]).append(e)
    edges=[]
    for (a,b,label),ev in followed.items():
        color='color="#a46120", fontcolor="#8f4d10"' if label in ('consolidated equality','registry identity') else ''
        edges.append((a,b,f'{label} ×{len(ev)}',color))
    parts.append('<h3>Edges actually followed</h3><figure>'+graph(i,'followed',edges)+'<figcaption>Counts span all passes. Teal = structural or work-list traversal; amber = retained or newly added identity. Node labels describe the starting model. Boundary and selection loops are demand transitions, not stored self-identities. Synthetic task-entry visits and subgoal requests are listed separately below.</figcaption></figure>')
    edge_rows=[]
    for (a,b,label),ev in full.items():
        edge_rows.append([escape(label),code(a)+'<br>→ '+code(b),str(len(ev)),', '.join(map(str,sorted(set(x['cost'] for x in ev)))),', '.join(link_event(i,x['seq']) for x in ev)])
    parts.append('<details><summary>Exact edge routes, contexts and costs</summary>'+table(['Edge','From → to query','Follows','Route cost','Events'],edge_rows)+'</details>')
    task_rows=[]
    for e in es:
        if e['event']!='Task initialized':continue
        t=e['task'];passes=[x for x in es if x['event']=='Pass started' and x['task']==t]
        parents=sorted({x['task'] for x in es if x['event']=='Subgoal requested' and x['other']==t})
        task_rows.append([f'T{t}',escape(task_meanings[i][t]),', '.join('T'+str(p) for p in parents) or '—',str(len(passes)),link_event(i,e['seq']),code(query(e['from']))])
    parts.append('<h3>Goals and subgoals</h3>'+table(['Task','Purpose','Required by','Passes','Initialized','Full goal query'],task_rows))
    parts.append('<p class="small">Every subgoal in these three traces is required, not exploratory. A task’s demand includes its context and related endpoint; the same POV may participate in several different goals.</p>')
    event_rows=[]
    for seq,description in important[i]:
        e=es[seq-1];assert e['seq']==seq
        event_rows.append([link_event(i,seq),f'P{e["rootPass"]} / r{e["displayRevision"]}',escape(description)])
    parts.append('<h3>Important events, in execution order</h3>'+table(['Event','Root pass / revision','What happened'],event_rows))
    set_rows=[]
    for e in es:
        if e['event']!='Set applied':continue
        changed=e['extra']==1
        outcome=changes.get((i,e['seq']),'No graph change. Goal-local proof metadata may still be updated.' if e['detail']=='Check RHS representation' else 'No graph change; the requested information or relationship is already present.')
        set_rows.append([link_event(i,e['seq']),f'T{e["task"]}',escape(e['detail']),str(e['cost']),'<span class="'+('changed' if changed else 'quiet')+'">'+('Changed' if changed else 'Unchanged')+'</span>',escape(outcome)])
    parts.append('<details open><summary>Every executed Set, including unchanged applications</summary>'+table(['Event','Task','Set command','Route cost','Effect','Actual target and delta'],set_rows)+'</details>')
    parts.append(f'''<details class="rawlog"><summary>Complete event log · {len(es)} events</summary><div class="log-toolbar"><label>Show <select id="mode-{i}" onchange="renderLog({i})"><option value="useful">Important + traversals</option><option value="all">All events, including queues</option><option value="sets">Sets and publications</option><option value="tasks">Task lifecycle</option></select></label><input id="filter-{i}" aria-label="Filter case {i} events" placeholder="Filter by task, event or detail…" oninput="renderLog({i})"><span class="small" id="count-{i}"></span></div><div class="log" id="log-{i}"></div></details><p class="small">Download: <a href="case-{i}-events.txt">readable full log</a> · <a href="case-{i}.jsonl">raw events and snapshots (JSONL)</a> · <a href="case-{i}-summary.json">result and native handle steps (JSON)</a> · <a href="case-{i}-followed.svg">traversal graph (SVG)</a></p></section>''')
    # Keep original capture files byte-for-byte; formatted text uses inferred current revision for invalidation callbacks.
    for suffix in ('.jsonl','-summary.json'):shutil.copy2(BASE/f'case-{i}{suffix}',OUT/f'case-{i}{suffix}')
    lines=[f'CASE {i}: {s["input"]}',f'RESULT: {s["result"]}', 'Costs are accumulated route cost, not time. P=active root pass; r=current context revision.', 'Task-entry visit is synthetic. Set from/to query is the finding, not necessarily the write target.', '']
    for e in es:
        task=f'T{e["task"]}' if e['task']>=0 else '-'
        line=f'#{e["seq"]:03} P{e["rootPass"]} r{e["displayRevision"]} {task:>3} cost={e["cost"]} {e["event"]}: {e["detail"]}'
        if e['event']=='Subgoal requested':line+=f' -> T{e["other"]} (required={e["extra"]==0})'
        if e['event']=='Set applied':line+=f' [graph changed={e["extra"]==1}]'
        if e['event']=='Pass started':line+=f' [previous visits cleared={e["extra"]}]'
        lines.extend([line,'    from: '+query(e['from']),'      to: '+query(e['to'])])
        if (i,e['seq']) in changes:lines.append('   delta: '+changes[i,e['seq']])
    (OUT/f'case-{i}-events.txt').write_text('\n'.join(lines)+'\n')

parts.append('''<section id="optimization"><h2>What to optimize next</h2><p>These are conclusions about repeated work, not a measured Rubix-cube speedup. The examples are small and the observer deliberately records a full snapshot at every event.</p>''')
parts.append(table(['Observed behavior','Candidate improvement','Correctness obligation'],[
['Every model publication invalidates every task created so far in these examples. The literal "A" is revisited four times.', 'Give tasks dependencies on the facts and relations they actually read: value, size, member extent and identity adjacency. Preserve clean tasks and their pending work.', 'A newly added useful edge must wake relevant searches. Subscribing only to currently existing edges would miss additions.'],
['A dirty task clears visited states, child demands and goal-local findings. Existing task IDs and model writes survive, but search work is repeated.', 'Retain still-valid findings and resume affected continuations within a task; revisit only evidence whose assumptions changed.', 'Queued Sets based on invalidated proofs must not execute. Keep the current generation/proof safety check until a narrower replacement is established.'],
['The list’s RHS consolidation alone causes root pass 4 and opens additional identity-return routes.', 'Treat a representation-only consolidation as a narrower relationship change, preserving unrelated list/value findings. Consider transferring the consumed-RHS proof to the retained identity.', 'The equality must remain searchable and must still detect future contradictory information; do not simply suppress its notification.'],
['Intersection selection creates an identity, then value acquisition publishes separately and restarts the selection walk.', 'Reuse a proved selection while its member extent and selection constraints remain unchanged. Publication batching could be explored after dependencies are explicit.', 'Selecting the correct member and satisfying its constraints are distinct proofs. Batching cannot assume a newly exposed RHS will always be usable.']
]))
parts.append('''<p><strong>Recommended first experiment:</strong> narrow the shared model dependency enough that copying <code>"B"</code> does not restart the unchanged <code>"A"</code> goal or discard the pending concat visit. Compare the same traces and results before touching broader scheduling policy.</p><p>Cost ordering is visible here: the first intersection identity Set costs 9 (five traversal steps plus one creation priced at 4); copying through that identity later costs 1. The later identity Set costs 5 because the identity already exists. The problem these examples expose is repeated valid work after publications, rather than a large set of expensive alternative paths.</p></section>''')

sources=[('graph/GraphSearch.h',35,'CostPolicy: visit weight 1, creation weight 4.'),('graph/GraphSearch.h',267,'wakeTask: dirty generation and reconciliation.'),('graph/GraphSearch.h',298,'graphChanged: broad notifications or dependency publications.'),('graph/GraphSearch.h',443,'resetTask: clears visited queries, child links, dependencies and pass-local goal state.'),('graph/GraphSearch.h',520,'startTask: reuse a task by its full query.'),('graph/GraphSearch.h',640,'follow: execute edge/Set, publish changes, deduplicate and refine.'),('graph/ProteusGraph.h',750,'observeTask: shared conservative modelFacts_ dependency.'),('graph/ProteusGraph.h',193,'Consolidate RHS equality: retain identity and remove pending work.'),('graph/ProteusGraph.h',392,'Set requested size/value/type: writes the goal target from the found source.'),('graph/ProteusGraph.h',596,'Set concatenation result.'),('graph/ProteusGraph.h',624,'Set selected result identity.'),('graph/RHSRepresentation.h',60,'Closed RHS consolidation requires a known source size and value.'),('graph/POVDescription.h',46,'Read-only relationships drawn from the live POV, work list and registry.')]
parts.append('''<section id="method"><h2>Method, source and reproduction</h2><p>The temporary probe includes the existing generated <code>LocalBuild/TestProteus.cpp</code>, renames its normal test entry point, and calls <code>GraphNormalizer::normalize</code> followed by <code>waitForCompletion()</code>. No renderer, world loading, streaming input or external producer is involved.</p><p>Each input was run twice in fresh normalizer contexts: once with the diagnostic observer disabled and once enabled. Printed outputs and native handle-step signatures matched for all three: action, demand kind, argument, accumulated cost and revision. This checks the observer against the unobserved run for these inputs; it is not a whole-engine regression test or a proof that instrumentation never affects timing.</p><p>The observer adds event callbacks to a temporary copy of <code>GraphSearch.h</code> and reads live model snapshots. The application’s source files and binaries were not modified. State snapshots record the objects encountered so far, including an operand that becomes detached when a concatenation is replaced. Such a retained diagnostic record is not evidence of a current graph edge.</p><p><code>Set applied</code> is captured before the ensuing graph publication, so the value changes at revision r and the notification appears at r+1. <code>Goal settled</code> can set derived <code>isNormed</code> metadata without a semantic publication. An unchanged representation check can still update task-local proof metadata.</p><p>Costs in the log are accumulated route costs, not elapsed time or a total charge for the whole search. Queue estimates, function-internal loops and unchosen candidate edges are not fully profiled. These diagrams show the stored links and edges actually followed, not every potential edge the adapter could expose.</p><h3>Source locations at the captured commit</h3><ul class="source-list">''')
for path,line,desc in sources:
    parts.append(f'<li>{code(str(SOURCE/path)+":"+str(line))} — {escape(desc)}</li>')
parts.append('''</ul><h3>Reproduce locally</h3><p>The <a href="run-probe.sh">reproduction script</a> makes a new temporary staging directory, copies the current graph headers and generated test source, instruments only the copy, builds, and runs the three cases. It requires the existing CodeDog-generated <code>LocalBuild/TestProteus.cpp</code>, SCons, a C++17 compiler, GMP and ICU. Exact counts apply to the captured commit; later engine changes may intentionally alter them.</p><pre>bash run-probe.sh /home/bruce/devl/Proteus4</pre><p>Included: <a href="trace-probe.cpp">probe source</a> · <a href="instrument.py">diagnostic instrumentation</a> · <a href="GraphSearch-diagnostic.patch">instrumentation diff</a> · <a href="capture-provenance.json">source hashes and provenance</a> · <a href="probe.log">validation output</a>.</p></section></main><footer>Local diagnostic artifact · No engine behavior changes · Event IDs and task/POV IDs are scoped to each example</footer><dialog id="event-dialog"><div class="dialog-head"><h2 id="event-title"></h2><button onclick="document.getElementById('event-dialog').close()">Close</button></div><p id="event-description"></p><pre id="event-data"></pre></dialog>''')

payload={'cases':cases,'demands':demands,'deltas':{f'{i}:{seq}':text for (i,seq),text in changes.items()}}
parts.append('<script id="trace-data" type="application/json">'+json.dumps(payload,separators=(',',':')).replace('<','\\u003c')+'</script>')
parts.append('''<script>
const data=JSON.parse(document.getElementById('trace-data').textContent);
const esc=s=>String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
function q(q){if(!q.node)return '—';return q.node+' · '+data.demands[q.kind]+(q.argument?' · arg='+q.argument:'')+(q.related?' · related='+q.related:'')+(q.context.length?' · context=['+q.context.join(',')+']':'')+(q.coordinates.length?' · coordinates=['+q.coordinates.join(',')+']':'');}
function inspect(i,n){const e=data.cases[i-1].events[n-1];document.getElementById('event-title').textContent='Case '+i+' · #'+n+' · '+e.event;document.getElementById('event-description').textContent=data.deltas[i+':'+n]||e.detail;document.getElementById('event-data').textContent=JSON.stringify(e,null,2);const d=document.getElementById('event-dialog');if(!d.open)d.showModal();}
function renderLog(i){const c=data.cases[i-1],mode=document.getElementById('mode-'+i).value,filter=document.getElementById('filter-'+i).value.toLowerCase();
 const events=c.events.filter(e=>{if(mode==='useful'&&['Edge queued','Set queued','Set started','Visit accepted','Task reused'].includes(e.event))return false;if(mode==='sets'&&!['Set queued','Set started','Set applied','Graph change published','Task invalidated','Stale work discarded'].includes(e.event))return false;if(mode==='tasks'&&!['Search requested','Task initialized','Task reused','Pass started','Subgoal requested','Task invalidated','Goal settled','Search settled','Wait returned'].includes(e.event))return false;return !filter||('#'+e.seq+' T'+e.task+' '+e.event+' '+e.detail+' '+q(e.from)+' '+q(e.to)).toLowerCase().includes(filter);});
 document.getElementById('count-'+i).textContent=events.length+' / '+c.events.length+' events';
 document.getElementById('log-'+i).innerHTML='<table><thead><tr><th>Event</th><th>P / r</th><th>Task</th><th>Cost</th><th>Action</th><th>Query / route</th></tr></thead><tbody>'+events.map(e=>'<tr class="'+(e.event==='Set applied'&&e.extra?'mutation':e.event==='Pass started'?'restart':'')+'"><td><button class="event-link" onclick="inspect('+i+','+e.seq+')">#'+e.seq+'</button></td><td>'+e.rootPass+' / '+e.displayRevision+'</td><td>'+(e.task<0?'—':'T'+e.task)+'</td><td>'+e.cost+'</td><td><strong>'+esc(e.event)+'</strong><br>'+esc(e.detail)+(e.event==='Set applied'?'<br>Graph changed: '+Boolean(e.extra):'')+(e.event==='Subgoal requested'?'<br>→ T'+e.other+' (required)':'')+(e.event==='Pass started'?'<br>Visited states cleared: '+e.extra:'')+'</td><td>'+esc(q(e.from))+(e.to.node?'<br>→ '+esc(q(e.to)):'')+'</td></tr>').join('')+'</tbody></table>';
}
for(let i=1;i<=3;i++)renderLog(i);
document.getElementById('event-dialog').addEventListener('click',e=>{if(e.target===e.currentTarget)e.currentTarget.close();});
</script></body></html>''')
(OUT/'index.html').write_text(''.join(parts))
print('Report generated:',OUT/'index.html')
