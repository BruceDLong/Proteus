import { Fragment, useId, useState, type CSSProperties, type ReactNode } from "react";

import type { SignedAdditiveChild, SignedAdditiveModel, SignedRoute } from "./signedAdditiveModel";

import "./signedAdditive.css";

const STAGES = ["Real states", "Return paths", "Composition", "Checks"] as const;
const COLORS = ["#dc5f45", "#2f9d7e", "#5d7ee8", "#c35fb7", "#d49b2f", "#2c9ab7", "#8c6ccf", "#76a83b"];

export type SignedFocus = "parent" | "a" | "b";

function classes(...names: Array<string | false | undefined>): string {
  return names.filter(Boolean).join(" ");
}

function residue(position: number, size: number): number {
  return ((position % size) + size) % size;
}

function signed(value: number): string {
  return value < 0 ? `−${Math.abs(value)}` : `+${value}`;
}

function downloadCertificate(model: SignedAdditiveModel): void {
  const url = URL.createObjectURL(new Blob([JSON.stringify(model, null, 2)], { type: "application/json" }));
  const link = document.createElement("a");
  link.href = url;
  link.download = `signed-additive_N${model.size}_v${model.value}_A${model.a.size}_x${model.a.value}.json`;
  link.hidden = true;
  document.body.append(link);
  link.click();
  link.remove();
  window.setTimeout(() => URL.revokeObjectURL(url), 1000);
}

function RootChip({ root, color, small = false, outside = false }: {
  root: number;
  color: string;
  small?: boolean;
  outside?: boolean;
}) {
  return (
    <span
      className={classes("signedRootChip", small && "signedRootChipSmall", outside && "signedRootChipOutside")}
      style={{ "--signed-chip-color": color } as CSSProperties}
      title={`Root identity ${root}${outside ? "; outside this child's real states" : ""}`}
    >
      {root}
    </span>
  );
}

function Status({ passed, children }: { passed: boolean; children: ReactNode }) {
  return <span className={classes("signedStatus", !passed && "signedStatusFailed")}>
    <span aria-hidden="true">{passed ? "✓" : "×"}</span>{children}
  </span>;
}

function CosetCard({ title, size, value, cosets, active, colorForRoot }: {
  title: string;
  size: number;
  value: number;
  cosets: number[][];
  active: boolean;
  colorForRoot: (root: number) => string;
}) {
  return (
    <section className={classes("signedPanel", active && "signedFocusPanel")} aria-label={`${title} cosets`}>
      <span className="signedEyebrow">{title} · {cosets.length} {cosets.length === 1 ? "coset" : "cosets"}</span>
      <h2 className="signedMono">*{size}+{value}</h2>
      <div className="signedCosetList">
        {cosets.map((items, index) => (
          <div className="signedCoset" key={index} aria-label={`Coset ${index + 1}: ${items.join(", ")}, then back to ${items[0]}`}>
            <span className="signedCosetLabel">C{index + 1}</span>
            {items.map((root, itemIndex) => (
              <Fragment key={root}>
                {itemIndex > 0 && <span className="signedCycleArrow" aria-hidden="true">→</span>}
                <RootChip root={root} color={colorForRoot(root)} small />
              </Fragment>
            ))}
            <span className="signedCycleArrow" title={`Returns to ${items[0]}`} aria-hidden="true">↻</span>
          </div>
        ))}
      </div>
    </section>
  );
}

function OccurrenceAxis({ positions, increments, size, real, colorForRoot }: {
  positions: number[];
  increments: number[];
  size: number;
  real: number[];
  colorForRoot: (root: number) => string;
}) {
  const id = useId().replace(/:/g, "");
  const low = Math.min(0, ...positions);
  const high = Math.max(size - 1, ...positions);
  const spacing = 34;
  const padding = 24;
  const width = Math.max(280, (high - low) * spacing + padding * 2);
  const x = (position: number) => high === low ? width / 2 : padding + (position - low) * ((width - 2 * padding) / (high - low));
  const visited = new Set(positions);
  const ticks = Array.from({ length: high - low + 1 }, (_, index) => low + index);
  const firstLap = Math.floor(low / size);
  const lastLap = Math.floor(high / size);
  const laps = Array.from({ length: lastLap - firstLap + 1 }, (_, index) => firstLap + index);

  return (
    <>
      <div className="signedAxisViewport" tabIndex={0} role="region" aria-label="Signed route on the repeated root baseline; scroll horizontally when needed">
        <svg className="signedAxis" width={width} viewBox={`0 0 ${width} 140`} role="img" aria-label={`Occurrence path ${positions.join(" to ")}; signed increments ${increments.map(signed).join(", ")}`}>
          <defs>
            <marker id={`${id}-forward`} markerWidth="5" markerHeight="5" refX="8" refY="5" orient="auto" viewBox="0 0 10 10">
              <path d="M 0 0 L 10 5 L 0 10 Z" fill="#478672" />
            </marker>
            <marker id={`${id}-backward`} markerWidth="5" markerHeight="5" refX="8" refY="5" orient="auto" viewBox="0 0 10 10">
              <path d="M 0 0 L 10 5 L 0 10 Z" fill="#b73e32" />
            </marker>
          </defs>
          {laps.map((lap) => {
            const left = x(Math.max(low, lap * size)) - 15;
            const right = x(Math.min(high, (lap + 1) * size - 1)) + 15;
            return <g key={lap}>
              <rect x={left} y="3" width={right - left} height="130" rx="5" fill={lap === 0 ? "#eff4f6" : "#f4f0f8"} />
              <text x={(left + right) / 2} y="16" textAnchor="middle" fill="#8a91a0" fontSize="9">lap {lap}</text>
            </g>;
          })}
          <line x1={padding - 12} y1="76" x2={width - padding + 12} y2="76" stroke="#b5c1ca" strokeWidth="1.3" />
          {increments.map((increment, index) => {
            const from = x(positions[index]);
            const to = x(positions[index + 1]);
            const backward = increment < 0;
            const controlY = backward ? 20 : 29 + (index % 3) * 7;
            const path = increment === 0
              ? `M ${from - 7} 65 C ${from - 28} 28, ${from + 28} 28, ${from + 7} 65`
              : `M ${from} 64 Q ${(from + to) / 2} ${controlY}, ${to} 64`;
            return <g key={index}>
              <path d={path} fill="none" stroke={backward ? "#b73e32" : "#478672"} strokeWidth="1.8" markerEnd={`url(#${id}-${backward ? "backward" : "forward"})`} />
              <text x={(from + to) / 2} y={increment === 0 ? 35 : (64 + controlY) / 2 - 4} textAnchor="middle" fill={backward ? "#b73e32" : "#478672"} fontFamily="monospace" fontSize="9" fontWeight="700">{signed(increment)}</text>
            </g>;
          })}
          {ticks.map((position) => {
            const root = residue(position, size);
            const active = visited.has(position);
            const outside = !real.includes(root);
            return <g key={position}>
              <line x1={x(position)} y1="72" x2={x(position)} y2="82" stroke="#bdc8d0" />
              {active && <circle cx={x(position)} cy="76" r="11" fill="#ffffff" stroke={colorForRoot(root)} strokeWidth="2" strokeDasharray={outside ? "3 2" : undefined} />}
              <text x={x(position)} y={active ? 80 : 101} textAnchor="middle" fill={active ? "#263843" : "#8b97a2"} fontFamily="monospace" fontSize={active ? "11" : "10"} fontWeight={active ? "700" : "400"}>{root}</text>
              {active && <text x={x(position)} y="102" textAnchor="middle" fill="#83919c" fontSize="8">root</text>}
              <text x={x(position)} y="122" textAnchor="middle" fill="#5e6f7d" fontFamily="monospace" fontSize="9">{position}</text>
            </g>;
          })}
        </svg>
      </div>
      <p className="signedAxisCaption">Root identities repeat on each lap. Bottom numbers are occurrence positions. Red arrows move backward.</p>
    </>
  );
}

function VisitPath({ positions, increments, size, real, colorForRoot }: {
  positions: number[];
  increments: number[];
  size: number;
  real: number[];
  colorForRoot: (root: number) => string;
}) {
  return <div className="signedRoutePath" aria-label="Route in visit order">
    {positions.map((position, index) => {
      const root = residue(position, size);
      const increment = increments[index - 1];
      return <Fragment key={index}>
        {index > 0 && <span className={classes("signedDelta", increment < 0 && "signedNegative")} title={`${signed(increment)} ${increment < 0 ? "backward" : "forward"}`}>
          <span aria-hidden="true">{increment < 0 ? "←" : "→"}</span>
          <span>{signed(increment)}</span>
        </span>}
        <span className="signedVisit">
          <RootChip root={root} color={colorForRoot(root)} outside={!real.includes(root)} />
          <small className="signedVisitPosition">at {position}</small>
          <small>lap {Math.floor(position / size)}</small>
        </span>
      </Fragment>;
    })}
  </div>;
}

function ReturnCard({ label, child, size, selectedRoot, onSelectRoot, colorForRoot }: {
  label: "A" | "B";
  child: SignedAdditiveChild;
  size: number;
  selectedRoot: number;
  onSelectRoot: (root: number) => void;
  colorForRoot: (root: number) => string;
}) {
  const route = child.routes.find((item) => item.start === selectedRoot) ?? child.routes[0];
  const backwardCount = child.increments.filter((increment) => increment < 0).length;
  const routePassed = child.routes.every((item) => item.passed);

  return (
    <section className="signedPanel" aria-label={`${label} signed return paths`}>
      <div className="signedPanelHeader">
        <div>
          <span className="signedEyebrow">{label} · *{child.size}+{child.value}</span>
          <h2>Return to a real state</h2>
        </div>
        <Status passed={routePassed}>{child.routes.filter((item) => item.passed).length}/{child.routes.length} returns hold</Status>
      </div>
      <p className="signedDescription">Choose a real starting root. Follow {label} until it reaches a real root again.</p>
      <div className="signedPicker" role="group" aria-label={`${label} starting root`}>
        {child.routes.map((item) => (
          <button
            aria-pressed={item.start === route?.start}
            aria-label={`${label} return from root ${item.start}${item.increments.some((increment) => increment < 0) ? "; includes backward movement" : ""}`}
            className={classes(item.increments.some((increment) => increment < 0) && "signedHasBackward")}
            key={item.start}
            onClick={() => onSelectRoot(item.start)}
            type="button"
          >{item.start}</button>
        ))}
      </div>
      {route ? <>
        <div className="signedPathHeading">
          <strong>root {route.start} → root {route.end}</strong>
          <Status passed={route.passed}>net {route.net} · expected {route.expectedNet}</Status>
        </div>
        <OccurrenceAxis positions={route.positions} increments={route.increments} size={size} real={child.real} colorForRoot={colorForRoot} />
        <span className="signedEyebrow">Visit order · {route.increments.length} {route.increments.length === 1 ? "step" : "steps"}</span>
        <VisitPath positions={route.positions} increments={route.increments} size={size} real={child.real} colorForRoot={colorForRoot} />
      </> : <div className="signedEmpty">No return path is available for this child.</div>}
      <div className="signedReturnSummary">
        <span>{child.real.length} real roots</span>
        <span>{child.ghosts.length} roots outside {label}</span>
        <span className={classes(backwardCount > 0 && "signedNegative")}>{backwardCount} backward {backwardCount === 1 ? "edge" : "edges"}</span>
      </div>
      <p className="signedDescription">Dashed chips are existing roots outside {label}'s real set. Their positions can change lap; their identities stay the same.</p>
      <details className="signedGhostDetails">
        <summary>{child.ghostCycles.length} cycles entirely outside {label}'s real states · inspect net displacement</summary>
        {child.ghostCycles.length === 0 ? <p className="signedDescription">Every cycle meets a real {label} state.</p> : (
          <div className="signedGhostCycleList">
            {child.ghostCycles.map((cycle: SignedRoute, index: number) => (
              <div className="signedGhostCycle" key={`${cycle.start}-${index}`}>
                <div>
                  <strong>roots {cycle.roots.join(" → ")}</strong>
                  <span>positions {cycle.positions.join(" → ")}</span>
                </div>
                <Status passed={cycle.passed}>net {cycle.net} · expected 0</Status>
              </div>
            ))}
          </div>
        )}
      </details>
    </section>
  );
}

function PositionCard({ label, position, size, active }: { label: string; position: number; size: number; active: boolean }) {
  return <div className={classes("signedPositionCard", active && "signedPositionCardActive")}>
    <span>{label}</span>
    <strong>{position}</strong>
    <small>root {residue(position, size)}<br />lap {Math.floor(position / size)}</small>
  </div>;
}

export function SignedAdditiveView({ model, focus, onFocusChange, stage, onStageChange }: {
  model: SignedAdditiveModel;
  focus: SignedFocus;
  onFocusChange: (focus: SignedFocus) => void;
  stage: number;
  onStageChange: (stage: number) => void;
}) {
  const [selectedA, setSelectedA] = useState(0);
  const [selectedB, setSelectedB] = useState(0);
  const [selectedRoot, setSelectedRoot] = useState(0);
  const currentStage = Math.max(0, Math.min(3, stage));
  const selectedComposition = model.composition.find((row) => row.start === selectedRoot) ?? model.composition[0];
  const rootColors = new Map<number, string>();
  model.parentCosets.forEach((coset, index) => coset.forEach((root) => rootColors.set(root, COLORS[index % COLORS.length])));
  const colorForRoot = (root: number) => rootColors.get(root) ?? "#8795a2";
  const focusedReal = focus === "a" ? model.a.real : focus === "b" ? model.b.real : model.baseline;
  const checksPassed = model.checks.filter((check) => check.passed).length;
  const allPassed = model.passed && model.checks.length > 0 && checksPassed === model.checks.length;
  const failedCount = model.checks.length - checksPassed;
  const certificateLabel = allPassed ? `PASS · ${checksPassed}/${model.checks.length} checks`
    : failedCount > 0 ? `FAIL · ${failedCount} ${failedCount === 1 ? "check" : "checks"}` : "Certificate not verified";

  return (
    <div className="animationShell signedView">
      <nav className="stepRail" aria-label="Signed evaluation stage">
        {STAGES.map((label, index) => (
          <button
            className={classes("stepButton", currentStage === index && "selected", index < currentStage && "complete")}
            aria-label={`Step ${index + 1}: ${label}`}
            aria-current={currentStage === index ? "step" : undefined}
            key={label}
            onClick={() => onStageChange(index)}
            type="button"
          >
            <span className="stepDot">{index + 1}</span>
            <span className="stepLabel">{label}</span>
          </button>
        ))}
      </nav>

      <section className="workspace" aria-label="Signed additive construction">
        <section className="signedPanel" aria-label="Fixed root baseline and real state membership">
          <div className="signedHeader">
            <div>
              <span className="signedEyebrow">Signed return model</span>
              <h2>One fixed baseline, shared root 0</h2>
            </div>
            <div className="focusButtons signedFocus" role="group" aria-label="Displayed signed object">
              {(["parent", "a", "b"] as const).map((item) => (
                <button aria-pressed={focus === item} className={classes(focus === item && "selected")} key={item} onClick={() => onFocusChange(item)} type="button">
                  {item === "parent" ? "Parent" : item.toUpperCase()}
                </button>
              ))}
            </div>
          </div>
          <div className="signedEquation" aria-label={`Parent star ${model.size} plus ${model.value}; A star ${model.a.size} plus ${model.a.value}; B star ${model.b.size} plus ${model.b.value}`}>
            <button aria-pressed={focus === "parent"} onClick={() => onFocusChange("parent")} type="button">*{model.size}+{model.value}</button>
            <span>{allPassed ? "=" : "≟"}</span>
            <button aria-pressed={focus === "a"} onClick={() => onFocusChange("a")} type="button">A *{model.a.size}+{model.a.value}</button>
            <span>+</span>
            <button aria-pressed={focus === "b"} onClick={() => onFocusChange("b")} type="button">B *{model.b.size}+{model.b.value}</button>
          </div>
          <div className="signedBaselineViewport" tabIndex={0} role="region" aria-label="Root identities in fixed baseline order; scroll horizontally when needed">
            <div className="signedBaseline">
              {model.baseline.map((root) => (
                <div className="signedBaselineCell" key={root}>
                  <RootChip root={root} color={colorForRoot(root)} outside={!focusedReal.includes(root)} />
                  <span className="signedMembership" aria-label={`Root ${root} is real in ${model.a.real.includes(root) ? "A" : ""}${model.a.real.includes(root) && model.b.real.includes(root) ? " and " : ""}${model.b.real.includes(root) ? "B" : ""}`}>
                    {model.a.real.includes(root) && <span className="signedMember">A</span>}
                    {model.b.real.includes(root) && <span className="signedMember signedMemberB">B</span>}
                  </span>
                </div>
              ))}
            </div>
          </div>
          <div className="signedLegend">
            <span className="signedLegendItem"><span className="signedDot" />A real states</span>
            <span className="signedLegendItem"><span className="signedDot signedDotB" />B real states</span>
            <span className="signedLegendItem"><strong className="signedMono">0</strong> is shared by A + B</span>
            <span>{model.a.size} + {model.b.size} − 1 = {model.size} root identities</span>
          </div>
        </section>

        <div className="signedStageBody" key={currentStage}>
          {currentStage === 0 && <>
            <section className="signedPanel">
              <div className="signedPanelHeader">
                <div><span className="signedEyebrow">1 · Real states</span><h2>Keep each coset intact</h2></div>
                <Status passed={allPassed}>{certificateLabel}</Status>
              </div>
              <p className="signedDescription">Each row below is a separate closed cycle. The chips keep their root identities; a cycle's last arrow returns to its first chip.</p>
              <div className="signedMetrics">
                <div className="signedMetric"><span>Root identities</span><strong>{model.size}</strong></div>
                <div className="signedMetric"><span>Shared state</span><strong>root 0</strong></div>
                <div className="signedMetric"><span>Values</span><strong>{model.a.value} + {model.b.value} = {model.value}</strong></div>
              </div>
            </section>
            <div className="signedCosetGrid">
              <CosetCard title="Parent" size={model.size} value={model.value} cosets={model.parentCosets} active={focus === "parent"} colorForRoot={colorForRoot} />
              <CosetCard title="A" size={model.a.size} value={model.a.value} cosets={model.a.cosets} active={focus === "a"} colorForRoot={colorForRoot} />
              <CosetCard title="B" size={model.b.size} value={model.b.value} cosets={model.b.cosets} active={focus === "b"} colorForRoot={colorForRoot} />
            </div>
          </>}

          {currentStage === 1 && <>
            <section className="signedPanel">
              <span className="signedEyebrow">2 · Return paths</span>
              <h2>Follow identities through signed steps</h2>
              <p className="signedDescription">A return can visit roots outside the child and move backward along the repeated baseline. Its exact net displacement must match the child's next real state. A red dot on a starting-root button marks a route with backward movement.</p>
            </section>
            <div className={focus === "parent" ? "signedRouteGrid" : "workspace"}>
              {focus !== "b" && <ReturnCard label="A" child={model.a} size={model.size} selectedRoot={selectedA} onSelectRoot={setSelectedA} colorForRoot={colorForRoot} />}
              {focus !== "a" && <ReturnCard label="B" child={model.b} size={model.size} selectedRoot={selectedB} onSelectRoot={setSelectedB} colorForRoot={colorForRoot} />}
            </div>
          </>}

          {currentStage === 2 && <section className="signedPanel">
            <div className="signedPanelHeader">
              <div><span className="signedEyebrow">3 · Composition</span><h2>A, then B: keep the full displacement</h2></div>
              <Status passed={model.composition.every((row) => row.passed)}>{model.composition.filter((row) => row.passed).length}/{model.composition.length} roots agree</Status>
            </div>
            <p className="signedDescription">Choose a starting root. The large numbers are actual occurrence positions; the smaller labels retain root identity and lap.</p>
            <div className="signedPicker" role="group" aria-label="Composition starting root">
              {model.composition.map((row) => <button key={row.start} aria-pressed={selectedComposition?.start === row.start} aria-label={`Compose from root ${row.start}`} onClick={() => setSelectedRoot(row.start)} type="button">{row.start}</button>)}
            </div>
            {selectedComposition && <>
              <div className="signedComposition">
                <PositionCard label="Start" position={selectedComposition.start} size={model.size} active={focus === "parent"} />
                <span className={classes("signedDelta", selectedComposition.deltaA < 0 && "signedNegative")}><span aria-hidden="true">{selectedComposition.deltaA < 0 ? "←" : "→"}</span><span>A {signed(selectedComposition.deltaA)}</span></span>
                <PositionCard label="After A" position={selectedComposition.afterA} size={model.size} active={focus === "a"} />
                <span className={classes("signedDelta", selectedComposition.deltaB < 0 && "signedNegative")}><span aria-hidden="true">{selectedComposition.deltaB < 0 ? "←" : "→"}</span><span>B {signed(selectedComposition.deltaB)}</span></span>
                <PositionCard label="After B" position={selectedComposition.end} size={model.size} active={focus === "b"} />
              </div>
              <div className={classes("signedFormula", !selectedComposition.passed && "signedFormulaFailed")}>
                {selectedComposition.deltaA < 0 ? `(${selectedComposition.deltaA})` : selectedComposition.deltaA} + {selectedComposition.deltaB < 0 ? `(${selectedComposition.deltaB})` : selectedComposition.deltaB} = {selectedComposition.total}
                <br />
                expected {selectedComposition.expected} · final position {selectedComposition.end} {selectedComposition.passed ? "=" : "≟"} {selectedComposition.start} + {selectedComposition.expected}
              </div>
            </>}
            <p className="signedDescription">The table checks composition from every root identity.</p>
            <div className="signedTableViewport" tabIndex={0} role="region" aria-label="Composition certificate for every root">
              <table className="signedTable">
                <thead><tr><th scope="col">Start</th><th scope="col">After A</th><th scope="col">After B</th><th scope="col">Net</th><th scope="col">Check</th></tr></thead>
                <tbody>{model.composition.map((row) => <tr key={row.start} className={classes(row.start === selectedComposition?.start && "signedSelectedRow")}>
                  <td><button type="button" aria-label={`Inspect root ${row.start}`} onClick={() => setSelectedRoot(row.start)}>{row.start}</button></td>
                  <td>{row.afterA}</td><td>{row.end}</td><td>{row.total}</td>
                  <td><span className={classes(!row.passed && "signedNegative")} aria-label={row.passed ? `Matches expected displacement ${row.expected}` : `Does not match expected displacement ${row.expected}`}>{row.passed ? "✓" : "×"}</span></td>
                </tr>)}</tbody>
              </table>
            </div>
          </section>}

          {currentStage === 3 && <section className="signedPanel">
            <div className="signedPanelHeader">
              <div><span className="signedEyebrow">4 · Checks</span><h2>Check the signed construction's rules</h2></div>
              <Status passed={allPassed}>{certificateLabel}</Status>
            </div>
            <p className="signedDescription">These results are calculated for the current inputs and the displayed signed completion.</p>
            <div className="signedChecksGrid">
              {model.checks.map((check) => <article className={classes("signedCheck", !check.passed && "signedCheckFailed")} key={check.id}>
                <span className="signedCheckIcon" aria-label={check.passed ? "Passed" : "Failed"}>{check.passed ? "✓" : "×"}</span>
                <div><strong>{check.label}</strong><p>{check.detail}</p></div>
              </article>)}
            </div>
            <button className="signedDownload" type="button" onClick={() => downloadCertificate(model)}>Download this certificate</button>
          </section>}
        </div>
        <p className="signedFooterNote">This construction selects one completion on fixed real supports. It does not enumerate all possible interleaves; other valid completions may have different internal routes.</p>
      </section>
    </div>
  );
}

export default SignedAdditiveView;
