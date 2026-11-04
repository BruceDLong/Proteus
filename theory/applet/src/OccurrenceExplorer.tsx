import type { OccurrenceTrace, OccurrenceFrame } from "./occurrenceLedger";
import "./occurrenceExplorer.css";

type Props = {
  trace: OccurrenceTrace;
  step: number;
  onStepChange: (next: number) => void;
  showFinal: boolean;
};

function fraction(n: number, d: number): string {
  return `${n}/${d}`;
}

function eventNames(trace: OccurrenceTrace, eventIndex: number): string {
  const event = trace.events[eventIndex];
  if (!event) return "No boundary event";
  return `Boundary ${fraction(event.fraction.numerator, event.fraction.denominator)}`;
}

export function OccurrenceExplorer({ trace, step, onStepChange, showFinal }: Props) {
  const actualStep = showFinal ? trace.frames.length - 1 : Math.max(0, Math.min(step, trace.frames.length - 1));
  const frame: OccurrenceFrame = trace.frames[actualStep];
  const event = frame.eventIndex >= 0 ? trace.events[frame.eventIndex] : undefined;
  const selected = trace.selectedTraversal.map((root, t) => ({
    root,
    t,
    count: frame.counters[t],
    position: root + frame.counters[t] * trace.parentSize,
  })).sort((a, b) => a.position - b.position);
  const rootSet = new Set(trace.selectedTraversal);
  const boundaries = trace.frames.map((f, i) => f.kind === "boundary" ? i : -1).filter(i => i >= 0);
  const previousBoundary = [...boundaries].reverse().find(i => i < actualStep);
  const nextBoundary = boundaries.find(i => i > actualStep);
  const movesAtEvent = event?.moves.reduce((total, move) => total + move.newCounter - move.oldCounter, 0) ?? 0;

  return (
    <section className="occurrenceExplorer" aria-label="Occurrence counter investigation">
      <div className="occurrenceExplorerHeader">
        <div>
          <h2>Occurrence counter ledger <span className="occurrenceExperimental">experimental · phase 0</span></h2>
          <p>Each singlet keeps its root identity. A counter advance selects the next appearance of that same singlet; it does not reorder the parent baseline.</p>
        </div>
        <div className="occurrenceContext">
          <span>Parent *{trace.parentSize}+{trace.parentValue}</span>
          <span>Selected {trace.factorSize} of {trace.parentSize} singlets</span>
          <span>Boundaries: {trace.events.length}</span>
        </div>
      </div>
      <div className="occurrenceExplainer">
        <span>Index in parent</span>
        <strong>root singlet + counter × parent size</strong>
        <span>For root 2: counter 0 → 2, counter 1 → {trace.parentSize + 2}, counter 2 → {2 * trace.parentSize + 2}</span>
      </div>
      <div className="occurrenceRootBand" aria-label="Unchanged parent baseline">
        <div className="occurrenceSubhead">Parent baseline (never changes)</div>
        <div className="occurrenceBaselineRow">
          {Array.from({length: trace.parentSize}, (_, i) => (
            <span key={i} className={`occurrenceBaselineCell ${rootSet.has(i) ? "occurrenceBaselineSelected" : ""}`}>{i}</span>
          ))}
        </div>
      </div>

      <div className="occurrenceStepHeader">
        <div>
          <div className="occurrenceSubhead">{showFinal ? "Completed occurrence history" : frame.title}</div>
          <p>{showFinal ? "All event updates are applied. The resulting order is read from occurrence coordinates." : frame.detail}</p>
        </div>
        <span className="occurrenceStepBadge">{actualStep + 1} / {trace.frames.length} frames</span>
      </div>
      {!showFinal && <div className="occurrenceTransport" aria-label="Occurrence investigation controls">
        <button type="button" onClick={() => onStepChange(Math.max(0, actualStep - 1))} disabled={actualStep === 0}>Previous +N</button>
        <button type="button" onClick={() => onStepChange(Math.min(trace.frames.length - 1, actualStep + 1))} disabled={actualStep >= trace.frames.length - 1}>Next +N</button>
        <button type="button" onClick={() => onStepChange(previousBoundary ?? 0)} disabled={actualStep === 0}>Previous boundary</button>
        <button type="button" onClick={() => onStepChange(nextBoundary ?? trace.frames.length - 1)} disabled={actualStep >= trace.frames.length - 1}>Next boundary</button>
      </div>}
      <div className="occurrenceEvents">
        <div className="occurrenceEventHeader">
          <strong>{event ? eventNames(trace, frame.eventIndex) : "Before the first boundary"}</strong>
          {event && <span>{movesAtEvent} total +{trace.parentSize} advances in this event</span>}
        </div>
        {event ? <div className="occurrenceGroups">
          <div><b>Crossing zero:</b> {event.zeroPhase.length
            ? event.zeroPhase.map(t => `t${t} (root ${trace.selectedTraversal[t]})`).join(", ")
            : "none"}</div>
          <div><b>Other coincident groups:</b> {event.nonzeroTies.length
            ? event.nonzeroTies.map(group => `[${group.map(t => `t${t}: ${trace.selectedTraversal[t]}`).join(", ")}]`).join("  ")
            : "none"}</div>
          <div><b>Event ordering constraint:</b> [{event.afterOrder.join(", ")}]</div>
        </div> : <p className="occurrenceSmallNote">{trace.events.length === 0
          ? "No rational threshold is crossed. All occurrence counters remain zero."
          : `Rotation begins at ${fraction(trace.alpha.numerator, trace.alpha.denominator)} and is examined toward ${fraction(trace.beta.numerator, trace.beta.denominator)}.`}</p>}
      </div>

      <div className="occurrenceSubhead">Selected occurrences, in increasing position</div>
      <div className="occurrenceCards" aria-label="Current singlet occurrence counters">
        {selected.map(({root, t, count, position}) => (
          <div key={root} className={`occurrenceCard ${frame.changedRoot === root && !showFinal ? "occurrenceCardAdvanced" : ""}`}>
            <div className="occurrenceRootLabel">Singlet <strong>{root}</strong></div>
            <div className="occurrenceCount">Counter <strong>{count}</strong></div>
            <div className="occurrencePosition">Index <strong>{position}</strong></div>
            <div className="occurrenceFormula">{root} + {count} × {trace.parentSize}</div>
            <div className="occurrenceTraversalIndex">Traversal t{t}</div>
          </div>
        ))}
      </div>
      {frame.targetOrder && !showFinal && <p className="occurrenceSmallNote">
        Target ordering for this event: [{frame.targetOrder.join(", ")}]. While counters are advancing, the current order can differ; it must agree when the event completes.
      </p>}
      <div className="occurrenceResult">
        <div><span>Inherited order</span><code>[{trace.inheritedBaseline.join(", ")}]</code></div>
        <div><span>{showFinal ? "Final" : "Current"} selected order</span><code>[{selected.map(x => x.root).join(", ")}]</code></div>
        <div><span>Verification against canonical factor</span><strong>{showFinal ? trace.benchmarkMatch ? "Matches" : "Mismatch" : "After final event"}</strong></div>
      </div>
      <p className="occurrenceCaveat">The factor-block rotation and nondecreasing occurrence-history convention remain theoretical assumptions. Matching the canonical factor is a test, not a proof of their uniqueness.</p>
    </section>
  );
}