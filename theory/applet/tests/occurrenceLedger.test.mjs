import assert from 'node:assert/strict';
import {buildOccurrenceTrace, canonicalTraversal} from '../src/occurrenceLedger.ts';
let cases=0, events=0, atomicAdvances=0, rebase=0;
for(let N=4;N<=120;N++)for(let b=2;b<N;b++)if(N%b===0){for(let v=0;v<N;v++){
 const t=buildOccurrenceTrace(N,v,b);
 cases++;events+=t.events.length;atomicAdvances+=t.frames.filter(f=>f.kind==='advance').length;
 if(t.finalBaseline.some((x,i)=>x!==t.inheritedBaseline[i]))rebase++;
 assert.equal(t.benchmarkMatch,true,`${N} ${v} ${b}`);
 assert.deepEqual(t.selectedTraversal,canonicalTraversal(N,v).slice(0,N/b));
 const last=t.frames.at(-1);
 const final=last.counters.map((count,i)=>t.selectedTraversal[i]+count*N);
 assert.deepEqual([...final].sort((a,b)=>a-b),t.finalPositions);
 assert.deepEqual(t.finalPositions.map(x=>x%N),t.finalBaseline);
 for(const e of t.events)for(const m of e.moves)assert.equal((m.newPosition-m.oldPosition)/N,m.newCounter-m.oldCounter);
}}
console.log({cases,events,atomicAdvances,rebase});