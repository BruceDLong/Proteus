import { Pause, Play, RotateCcw, StepForward } from "lucide-react";
import { useEffect, useMemo, useState } from "react";

type Mode = "divisor" | "quotient";

type Coset = {
  id: number;
  items: number[];
};

type Placement = {
  item: number;
  coord: number;
  slot: number;
};

type TransitionMeasure = {
  from: number;
  to: number;
  distance: number;
  spans: number;
  residual: number;
};

type GhostMove = {
  item: number;
  sourceSlot: number;
  ghostSlot: number;
  targetSlot: number;
};

type CaseAnalysis = {
  size: number;
  value: number;
  quotientSize: number;
  divisorSize: number;
  phase: number;
  mode: Mode;
  activeStep: number;
  selectedItems: number[];
  generatedItems: number[];
  mismatchIndex: number;
};

type Focus = "parent" | "divisor" | "quotient";

const MAX_SIZE = 24;

const PALETTE = [
  "#dc5f45",
  "#2f9d7e",
  "#5d7ee8",
  "#c35fb7",
  "#d49b2f",
  "#2c9ab7",
  "#8c6ccf",
  "#76a83b",
  "#c45872",
  "#4f8fce",
  "#a86b3c",
  "#4d9a55",
];

const STAGES = [
  "Select",
  "Ignore",
  "Ghosts",
  "Rebase",
] as const;

const GHOST_STAGE = 2;
const REBASE_STAGE = 3;

function range(size: number): number[] {
  return Array.from({ length: size }, (_, index) => index);
}

function mod(value: number, size: number): number {
  return ((value % size) + size) % size;
}

function divisorsOf(size: number): number[] {
  return range(size)
    .map((index) => index + 1)
    .filter((candidate) => size % candidate === 0);
}

function canonicalTraversal(baseline: number[], value: number): number[] {
  const size = baseline.length;
  if (size === 0) return [];

  const indexByItem = new Map(baseline.map((item, index) => [item, index]));
  const items: number[] = [];
  let startItem = baseline[0];
  let currentItem = startItem;

  for (let index = 0; index < size; index += 1) {
    items.push(currentItem);
    const currentIndex = indexByItem.get(currentItem) ?? 0;
    currentItem = baseline[mod(currentIndex + value, size)];

    if (currentItem === startItem) {
      const startIndex = indexByItem.get(startItem) ?? 0;
      currentItem = baseline[mod(startIndex + 1, size)];
      startItem = currentItem;
    }
  }

  return items;
}

function decomposeCosets(baseline: number[], value: number): Coset[] {
  const size = baseline.length;
  const indexByItem = new Map(baseline.map((item, index) => [item, index]));
  const visited = new Set<number>();
  const cosets: Coset[] = [];

  for (const seed of baseline) {
    if (visited.has(seed)) continue;

    const items: number[] = [];
    let current = seed;

    while (!visited.has(current)) {
      items.push(current);
      visited.add(current);
      const currentIndex = indexByItem.get(current) ?? 0;
      current = baseline[mod(currentIndex + value, size)];
    }

    cosets.push({ id: cosets.length, items });
  }

  return cosets;
}

function makeCosetLookup(cosets: Coset[]): Map<number, number> {
  const lookup = new Map<number, number>();
  for (const coset of cosets) {
    for (const item of coset.items) {
      lookup.set(item, coset.id);
    }
  }
  return lookup;
}

function baselineDistance(from: number, to: number, size: number): number {
  return mod(to - from, size);
}

function modeOf(values: number[]): number {
  const counts = new Map<number, number>();
  let bestValue = values[0] ?? 0;
  let bestCount = 0;

  for (const value of values) {
    const count = (counts.get(value) ?? 0) + 1;
    counts.set(value, count);
    if (count > bestCount) {
      bestValue = value;
      bestCount = count;
    }
  }

  return bestValue;
}

function parentTransitionMeasures(
  traversal: number[],
  quotientSize: number,
  parentSize: number,
): TransitionMeasure[] {
  const measures: TransitionMeasure[] = [];

  for (let index = 0; index < traversal.length - 1; index += 1) {
    const from = traversal[index];
    const to = traversal[index + 1];
    const distance = baselineDistance(from, to, parentSize);
    measures.push({
      from,
      to,
      distance,
      spans: Math.floor(distance / quotientSize),
      residual: distance % quotientSize,
    });
  }

  return measures;
}

function selectDivisorItems(
  parentTraversal: number[],
  divisorSize: number,
  phase: number,
): number[] {
  const start = phase * divisorSize;
  return parentTraversal.slice(start, start + divisorSize);
}

function selectQuotientItems(
  parentTraversal: number[],
  divisorSize: number,
  quotientSize: number,
  phase: number,
): number[] {
  const parentSize = parentTraversal.length;
  const items: number[] = [];
  let startIndex = mod(phase, divisorSize);
  let currentIndex = startIndex;

  for (let index = 0; index < quotientSize; index += 1) {
    items.push(parentTraversal[currentIndex]);
    currentIndex = mod(currentIndex + divisorSize, parentSize);

    if (currentIndex === startIndex) {
      currentIndex = mod(currentIndex + 1, parentSize);
      startIndex = currentIndex;
    }
  }

  return items;
}

function placeOnUnwrappedLine(
  selectedItems: number[],
  step: number,
  period: number,
): Placement[] {
  if (period === 0) return [];

  const placements: Placement[] = [];
  let coord = 0;
  let startSlot = 0;

  for (const item of selectedItems) {
    placements.push({ item, coord, slot: mod(coord, period) });

    let nextCoord = coord + step;
    if (mod(nextCoord, period) === startSlot) {
      nextCoord += 1;
      startSlot = mod(nextCoord, period);
    }
    coord = nextCoord;
  }

  return placements;
}

function foldedBaseline(placements: Placement[], period: number): Array<number | null> {
  const baseline = Array<number | null>(period).fill(null);
  for (const placement of placements) {
    baseline[placement.slot] = placement.item;
  }
  return baseline;
}

function restrictedBaseline(parentBaseline: number[], selectedItems: number[]): number[] {
  const selected = new Set(selectedItems);
  return parentBaseline.filter((item) => selected.has(item));
}

function baselinesMatch(sourceBase: number[], targetBase: Array<number | null>): boolean {
  return sourceBase.length === targetBase.length && sourceBase.every((item, index) => targetBase[index] === item);
}

function ghostBaselineMoves(parentBaseline: number[], targetBase: Array<number | null>): GhostMove[] {
  const sourceSlotByItem = new Map(parentBaseline.map((item, index) => [item, index]));
  const moves: GhostMove[] = [];
  let previousGhostSlot = -1;

  targetBase.forEach((item, targetSlot) => {
    if (item === null) return;

    const sourceSlot = sourceSlotByItem.get(item) ?? 0;
    let ghostSlot = sourceSlot;
    while (ghostSlot <= previousGhostSlot) {
      ghostSlot += parentBaseline.length;
    }

    moves.push({ item, sourceSlot, ghostSlot, targetSlot });
    previousGhostSlot = ghostSlot;
  });

  return moves;
}

function selectionOrder(items: number[]): Map<number, number> {
  return new Map(items.map((item, index) => [item, index]));
}

function isStageDisabled(index: number, focus: Focus, rebaseNeeded: boolean): boolean {
  if (focus === "parent") return index >= GHOST_STAGE;
  if (index === GHOST_STAGE) return !rebaseNeeded;
  return false;
}

function nextStage(current: number, focus: Focus, rebaseNeeded: boolean): number {
  for (let offset = 1; offset <= STAGES.length; offset += 1) {
    const candidate = (current + offset) % STAGES.length;
    if (!isStageDisabled(candidate, focus, rebaseNeeded)) return candidate;
  }

  return 0;
}

function stepLabel(
  focus: Focus,
  stage: number,
  divisorSize: number,
  quotientSize: number,
  phase: number,
  rebaseNeeded: boolean,
): string {
  if (focus === "parent") {
    return "Parent source: all singlet identities are visible";
  }

  if (stage === 0) {
    if (focus === "quotient") {
      const startSlot = phase * divisorSize;
      if (phase === 0) {
        return `Step 1: Select the first ${divisorSize} singlets`;
      }
      return `Step 1: Select ${divisorSize} singlets starting at traversal slot ${startSlot}`;
    }
    if (phase === 0) {
      return `Step 1: Select every ${divisorSize}th singlet`;
    }
    return `Step 1: Select every ${divisorSize}th singlet starting at traversal slot ${phase}`;
  }

  if (stage === 1) {
    return "Step 2: Ignore unused singlets";
  }

  if (stage === 2) {
    if (!rebaseNeeded) {
      return "Step 3: Ghost slots not needed; the inherited baseline already matches";
    }
    return "Step 3: Move selected identities into parent ghost slots";
  }

  return "Step 4: Shrink ignored and ghost singlets, then relabel the result";
}

function analyzeCase(
  caseSize: number,
  caseValue: number,
  caseQuotientSize: number,
  casePhase: number,
  caseMode: Mode,
): CaseAnalysis {
  const parentBaseline = range(caseSize);
  const parentTraversal = canonicalTraversal(parentBaseline, caseValue);
  const divisorSize = caseSize / caseQuotientSize;
  const transitionMeasures = parentTransitionMeasures(parentTraversal, caseQuotientSize, caseSize);
  const divisorStep = modeOf(transitionMeasures.map((measure) => measure.spans));
  const quotientStep = modeOf(transitionMeasures.map((measure) => measure.residual));
  const divisorItems = selectDivisorItems(parentTraversal, divisorSize, casePhase);
  const quotientItems = selectQuotientItems(parentTraversal, divisorSize, caseQuotientSize, casePhase);
  const activeItems = caseMode === "divisor" ? divisorItems : quotientItems;
  const activePeriod = caseMode === "divisor" ? divisorSize : caseQuotientSize;
  const activeStep = caseMode === "divisor" ? divisorStep : quotientStep;
  const activeBase =
    caseMode === "divisor"
      ? foldedBaseline(placeOnUnwrappedLine(divisorItems, divisorStep, divisorSize), divisorSize)
      : foldedBaseline(
          placeOnUnwrappedLine(quotientItems, quotientStep, caseQuotientSize),
          caseQuotientSize,
        );
  const childBaseline = compactItems(activeBase);
  const generatedItems =
    childBaseline.length === activePeriod ? canonicalTraversal(childBaseline, activeStep) : [];

  return {
    size: caseSize,
    value: caseValue,
    quotientSize: caseQuotientSize,
    divisorSize,
    phase: casePhase,
    mode: caseMode,
    activeStep,
    selectedItems: activeItems,
    generatedItems,
    mismatchIndex: firstMismatch(activeItems, generatedItems),
  };
}

function scanCases(caseSize: number, caseMode: Mode): { total: number; failures: CaseAnalysis[] } {
  const failures: CaseAnalysis[] = [];
  let total = 0;

  for (const caseValue of range(caseSize)) {
    for (const caseQuotientSize of divisorsOf(caseSize)) {
      const divisorSize = caseSize / caseQuotientSize;
      const phaseCount = caseMode === "divisor" ? caseQuotientSize : divisorSize;
      for (let casePhase = 0; casePhase < phaseCount; casePhase += 1) {
        const analysis = analyzeCase(
          caseSize,
          caseValue,
          caseQuotientSize,
          casePhase,
          caseMode,
        );
        total += 1;
        if (analysis.mismatchIndex !== -1) {
          failures.push(analysis);
        }
      }
    }
  }

  return { total, failures };
}

function compactItems(items: Array<number | null>): number[] {
  return items.filter((item): item is number => item !== null);
}

function firstMismatch(left: number[], right: number[]): number {
  const maxLength = Math.max(left.length, right.length);
  for (let index = 0; index < maxLength; index += 1) {
    if (left[index] !== right[index]) return index;
  }
  return -1;
}

function classNames(...names: Array<string | false | undefined>): string {
  return names.filter(Boolean).join(" ");
}

function colorForCoset(cosetId: number): string {
  return PALETTE[cosetId % PALETTE.length];
}

function SingletChip({
  item,
  cosetId,
  muted = false,
  active = false,
  selectionIndex,
  label,
}: {
  item: number | null;
  cosetId?: number;
  muted?: boolean;
  active?: boolean;
  selectionIndex?: number;
  label?: string;
}) {
  const color = item === null ? "#d7dce2" : colorForCoset(cosetId ?? 0);

  return (
    <span
      className={classNames("chip", muted && "chipMuted", active && "chipActive")}
      style={
        {
          "--chip-color": color,
          "--selection-delay": `${selectionIndex ?? 0}`,
        } as React.CSSProperties
      }
      title={label}
    >
      {item === null ? "" : item}
    </span>
  );
}

function Row({
  title,
  items,
  cosetLookup,
  selectedSet,
  fadeUnselected,
  active = false,
}: {
  title: string;
  items: Array<number | null>;
  cosetLookup: Map<number, number>;
  selectedSet?: Set<number>;
  fadeUnselected?: boolean;
  active?: boolean;
}) {
  return (
    <section className={classNames("rowPanel", active && "rowPanelActive")}>
      <div className="rowTitle">{title}</div>
      <div className="chipRow">
        {items.map((item, index) => {
          const cosetId = item === null ? undefined : cosetLookup.get(item) ?? 0;

          return (
            <span className="chipWrap" key={`${title}-${index}-${item ?? "empty"}`}>
              <SingletChip
                item={item}
                cosetId={cosetId}
                muted={fadeUnselected && item !== null && !selectedSet?.has(item)}
              />
            </span>
          );
        })}
      </div>
    </section>
  );
}

function OperationExpression({
  size,
  value,
  quotientSize,
  divisorSize,
  divisorStep,
  quotientStep,
  focus,
}: {
  size: number;
  value: number;
  quotientSize: number;
  divisorSize: number;
  divisorStep: number;
  quotientStep: number;
  focus: Focus;
}) {
  return (
    <div className="operationExpression">
      <span className={classNames("exprToken", focus === "parent" && "exprTokenActive")}>
        *{size}+{value}
      </span>
      <span className="exprOperator">/ {quotientSize}</span>
      <span className="exprOperator">=</span>
      <span className="exprBrace">{"{"}</span>
      <span className={classNames("exprToken", focus === "quotient" && "exprTokenActive")}>
        *{divisorSize}+{divisorStep}
      </span>
      <span className={classNames("exprToken", focus === "divisor" && "exprTokenActive")}>
        *{quotientSize}+{quotientStep}
      </span>
      <span className="exprBrace">{"}"}</span>
    </div>
  );
}

function ParentIdentityMap({
  parentBaseline,
  parentTraversal,
  cosetLookup,
  selectedSet,
  fadeUnselected,
  selectedOrder,
  selectionActive,
  ghostActive,
  rebaseActive,
  ghostMoves,
  stepText,
  active,
  focus,
  size,
  value,
  quotientSize,
  divisorSize,
  divisorStep,
  quotientStep,
  onFocusChange,
}: {
  parentBaseline: number[];
  parentTraversal: number[];
  cosetLookup: Map<number, number>;
  selectedSet: Set<number>;
  fadeUnselected: boolean;
  selectedOrder: Map<number, number>;
  selectionActive: boolean;
  ghostActive: boolean;
  rebaseActive: boolean;
  ghostMoves: GhostMove[];
  stepText: string;
  active: boolean;
  focus: Focus;
  size: number;
  value: number;
  quotientSize: number;
  divisorSize: number;
  divisorStep: number;
  quotientStep: number;
  onFocusChange: (focus: Focus) => void;
}) {
  const cellWidth = 42;
  const chipCenter = 17;
  const ghostMoveByItem = new Map(ghostMoves.map((move) => [move.item, move]));
  const ghostMoveBySlot = new Map(ghostMoves.map((move) => [move.ghostSlot, move]));
  const showGhostSlots = ghostActive || rebaseActive;
  const maxGhostSlot = showGhostSlots
    ? Math.max(parentBaseline.length - 1, ...ghostMoves.map((move) => move.ghostSlot))
    : parentBaseline.length - 1;
  const trackCount = maxGhostSlot + 1;
  const trackWidth = trackCount * cellWidth;
  const baselineIndexByItem = new Map(parentBaseline.map((item, index) => [item, index]));
  const ghostMoveText = ghostMoves.map((move) => `${move.item}->${move.ghostSlot}`).join("  ");
  let previousCoset: number | undefined;

  return (
    <section className={classNames("parentMapPanel", active && "rowPanelActive")}>
      <div className="parentMapHeader">
        <div className="rowTitle">Parent identities</div>
        <div className="focusButtons" role="group" aria-label="Displayed object">
          <button
            className={classNames(focus === "parent" && "selected")}
            onClick={() => onFocusChange("parent")}
            type="button"
          >
            Parent
          </button>
          <button
            className={classNames(focus === "quotient" && "selected")}
            onClick={() => onFocusChange("quotient")}
            type="button"
          >
            Quotient
          </button>
          <button
            className={classNames(focus === "divisor" && "selected")}
            onClick={() => onFocusChange("divisor")}
            type="button"
          >
            Divisor
          </button>
        </div>
        <OperationExpression
          divisorSize={divisorSize}
          divisorStep={divisorStep}
          focus={focus}
          quotientSize={quotientSize}
          quotientStep={quotientStep}
          size={size}
          value={value}
        />
      </div>
      <div className={classNames("stepCallout", focus !== "parent" && "stepCalloutActive")}>
        {stepText}
      </div>
      {showGhostSlots && ghostMoves.length > 0 && (
        <div className="ghostMoveSummary">{ghostMoveText}</div>
      )}
      <div className="identityViewport">
        <div className="identityLabels">
          <span>Baseline</span>
          <span>Traversal</span>
        </div>
        <div
          className={classNames("identityTrack", rebaseActive && "identityTrackCompress")}
          style={
            {
              "--identity-count": trackCount,
              width: trackWidth,
            } as React.CSSProperties
          }
        >
          <div className={classNames("identityRow", rebaseActive && "identityRowCompress")}>
            {range(trackCount).map((slot) => {
              const item = parentBaseline[slot] ?? null;
              const targetMove = ghostMoveBySlot.get(slot);
              const keepInRebase = rebaseActive && targetMove !== undefined;
              if (item === null) {
                const ghostItem = parentBaseline[mod(slot, parentBaseline.length)];
                return (
                  <div
                    className={classNames(
                      "identityCell",
                      "identityCellGhost",
                      targetMove && "identityCellGhostTarget",
                      rebaseActive && "identityCellCompress",
                      rebaseActive && !keepInRebase && "identityCellCollapse",
                      keepInRebase && "identityCellKeep",
                    )}
                    key={`baseline-ghost-${slot}`}
                    style={
                      {
                        "--ghost-color": colorForCoset(cosetLookup.get(ghostItem) ?? 0),
                        "--selection-delay": `${targetMove?.targetSlot ?? 0}`,
                      } as React.CSSProperties
                    }
                  >
                    {keepInRebase ? (
                      <span className="identityRelabelWrap">
                        <span className="newLabelBadge">{targetMove.targetSlot}</span>
                        <SingletChip item={targetMove.item} cosetId={cosetLookup.get(targetMove.item)} />
                      </span>
                    ) : (
                      <span className="ghostSlotMarker">
                        <span>{slot}</span>
                        <strong>{ghostItem}</strong>
                      </span>
                    )}
                  </div>
                );
              }

              const ghostMove = ghostMoveByItem.get(item);
              const moving = ghostActive && ghostMove !== undefined && ghostMove.ghostSlot !== slot;
              const keepOriginalInRebase = rebaseActive && targetMove !== undefined;
              const collapseOriginalInRebase = rebaseActive && !keepOriginalInRebase;
              return (
                <div
                  className={classNames(
                    "identityCell",
                    rebaseActive && "identityCellCompress",
                    collapseOriginalInRebase && "identityCellCollapse",
                    keepOriginalInRebase && "identityCellKeep",
                  )}
                  key={`baseline-${item}`}
                  style={
                    {
                      "--selection-delay": `${targetMove?.targetSlot ?? selectedOrder.get(item) ?? 0}`,
                    } as React.CSSProperties
                  }
                >
                  <span
                    className={classNames("identityChipMover", moving && "identityChipMoverActive")}
                    style={
                      {
                        "--ghost-shift": `${((ghostMove?.ghostSlot ?? slot) - slot) * cellWidth}px`,
                        "--selection-delay": `${selectedOrder.get(item) ?? ghostMove?.targetSlot ?? 0}`,
                      } as React.CSSProperties
                    }
                  >
                    {keepOriginalInRebase && <span className="newLabelBadge">{targetMove.targetSlot}</span>}
                    {selectionActive && selectedSet.has(item) && (
                      <span className="selectionBadge">{(selectedOrder.get(item) ?? 0) + 1}</span>
                    )}
                    <SingletChip
                      item={item}
                      active={selectionActive && selectedSet.has(item)}
                      cosetId={cosetLookup.get(item)}
                      muted={fadeUnselected && !selectedSet.has(item)}
                      selectionIndex={selectedOrder.get(item)}
                    />
                  </span>
                </div>
              );
            })}
          </div>
          <svg
            aria-hidden="true"
            className="identityLines"
            preserveAspectRatio="none"
            viewBox={`0 0 ${trackWidth} 70`}
          >
            {parentTraversal.map((item, traversalIndex) => {
              const baselineIndex = baselineIndexByItem.get(item) ?? 0;
              const ghostMove = ghostMoveByItem.get(item);
              const rebaseLine = ghostActive && selectedSet.has(item) && ghostMove !== undefined;
              const lineTargetIndex = rebaseLine ? ghostMove.ghostSlot : baselineIndex;
              const cosetId = cosetLookup.get(item) ?? 0;
              const muted = fadeUnselected && !selectedSet.has(item);
              const selected = selectionActive && selectedSet.has(item);
              const lineDelay = `${(selectedOrder.get(item) ?? ghostMove?.targetSlot ?? 0) * 0.08}s`;
              const sourceX = traversalIndex * cellWidth + chipCenter;
              const baselineX = baselineIndex * cellWidth + chipCenter;
              const targetX = lineTargetIndex * cellWidth + chipCenter;
              return (
                <line
                  className={classNames(
                    "identityLine",
                    muted && "identityLineMuted",
                    selected && "identityLineSelected",
                    rebaseLine && "identityLineRebase",
                  )}
                  key={`line-${traversalIndex}-${item}-${rebaseLine ? targetX : baselineX}`}
                  stroke={colorForCoset(cosetId)}
                  x1={sourceX}
                  x2={baselineX}
                  y1={64}
                  y2={6}
                  style={
                    {
                      "--selection-delay": `${selectedOrder.get(item) ?? 0}`,
                    } as React.CSSProperties
                  }
                >
                  {rebaseLine && (
                    <animate
                      attributeName="x2"
                      begin={lineDelay}
                      dur="1.05s"
                      fill="freeze"
                      from={baselineX}
                      to={targetX}
                    />
                  )}
                </line>
              );
            })}
          </svg>
          {rebaseActive && (
            <svg
              aria-hidden="true"
              className="finalIdentityLines"
              preserveAspectRatio="none"
              viewBox={`0 0 ${trackWidth} 70`}
            >
              {parentTraversal.map((item, traversalIndex) => {
                const ghostMove = ghostMoveByItem.get(item);
                if (!selectedSet.has(item) || ghostMove === undefined) return null;

                const cosetId = cosetLookup.get(item) ?? 0;
                const lineDelay = `${ghostMove.targetSlot * 0.028}s`;
                const sourceX = traversalIndex * cellWidth + chipCenter;
                const compactSourceX = (selectedOrder.get(item) ?? ghostMove.targetSlot) * cellWidth + chipCenter;
                const startTargetX = ghostMove.ghostSlot * cellWidth + chipCenter;
                const targetX = ghostMove.targetSlot * cellWidth + chipCenter;
                return (
                  <line
                    className="finalIdentityLine"
                    key={`final-line-${traversalIndex}-${item}-${ghostMove.targetSlot}`}
                    stroke={colorForCoset(cosetId)}
                    x1={sourceX}
                    x2={startTargetX}
                    y1={64}
                    y2={6}
                  >
                    <animate
                      attributeName="x1"
                      begin={lineDelay}
                      dur="1.65s"
                      fill="freeze"
                      from={sourceX}
                      to={compactSourceX}
                    />
                    <animate
                      attributeName="x2"
                      begin={lineDelay}
                      dur="1.65s"
                      fill="freeze"
                      from={startTargetX}
                      to={targetX}
                    />
                    <animate attributeName="opacity" begin={lineDelay} dur="1.65s" fill="freeze" from="0.7" to="0.96" />
                  </line>
                );
              })}
            </svg>
          )}
          <div className={classNames("identityRow", "traversalIdentityRow", rebaseActive && "identityRowCompress")}>
            {parentTraversal.map((item, index) => {
              const cosetId = cosetLookup.get(item) ?? 0;
              const selected = selectedSet.has(item);
              return (
                <div
                  className={classNames(
                    "identityCell",
                    rebaseActive && "identityCellCompress",
                    rebaseActive && !selected && "identityCellCollapse",
                    rebaseActive && selected && "identityCellKeep",
                  )}
                  key={`traversal-${index}-${item}`}
                  style={
                    {
                      "--selection-delay": `${selectedOrder.get(item) ?? index}`,
                    } as React.CSSProperties
                  }
                >
                  {selectionActive && selected && (
                    <span className="selectionBadge">{(selectedOrder.get(item) ?? 0) + 1}</span>
                  )}
                  <SingletChip
                    item={item}
                    active={selectionActive && selected}
                    cosetId={cosetId}
                    muted={fadeUnselected && !selected}
                    selectionIndex={selectedOrder.get(item)}
                  />
                </div>
              );
            })}
          </div>
        </div>
      </div>
    </section>
  );
}

function ParentCosetStack({
  parentTraversal,
  cosetLookup,
  horizontalPeriod,
  verticalPeriod,
  selectedSet,
  selectedOrder,
  selectionActive,
  fadeUnselected,
  active,
}: {
  parentTraversal: number[];
  cosetLookup: Map<number, number>;
  horizontalPeriod: number;
  verticalPeriod: number;
  selectedSet: Set<number>;
  selectedOrder: Map<number, number>;
  selectionActive: boolean;
  fadeUnselected: boolean;
  active: boolean;
}) {
  const columnCount = Math.max(1, Math.min(parentTraversal.length, Math.round(horizontalPeriod)));
  const rowCount = Math.max(1, Math.ceil(parentTraversal.length / columnCount), Math.round(verticalPeriod));
  const matrixRows = range(rowCount).map((row) =>
    parentTraversal.slice(row * columnCount, row * columnCount + columnCount),
  );

  return (
    <section className={classNames("cosetStrip", active && "rowPanelActive")}>
      <div className="cosetStackHeader">
        <div className="rowTitle">Parent cosets</div>
        <div className="cosetAxisLegend">
          <span>Horizontal period: quotient</span>
          <span>Vertical period: divisor</span>
        </div>
      </div>
      <div className="cosetStackViewport">
        <div
          className="cosetStack"
          style={
            {
              "--coset-cols": columnCount,
            } as React.CSSProperties
          }
        >
          {matrixRows.map((rowItems, row) => (
            <div className="cosetStackRow" key={`stack-row-${row}`}>
              {range(columnCount).map((column) => {
                const item = rowItems[column] ?? null;
                const cosetId = item === null ? undefined : cosetLookup.get(item) ?? 0;
                const selected = item !== null && selectedSet.has(item);
                const muted = item === null || (fadeUnselected && item !== null && !selected);
                return (
                  <div
                    className={classNames(
                      "cosetStackCell",
                      row === 0 && "cosetStackQuotientAxis",
                      column === 0 && "cosetStackDivisorAxis",
                      muted && "cosetStackCellMuted",
                    )}
                    key={`stack-cell-${row}-${column}-${item ?? "empty"}`}
                    style={
                      {
                        "--coset-color": colorForCoset(cosetId ?? 0),
                      } as React.CSSProperties
                    }
                  >
                    {item !== null && selectionActive && selected && (
                      <span className="selectionBadge">{(selectedOrder.get(item) ?? 0) + 1}</span>
                    )}
                      <SingletChip
                        item={item}
                        active={selectionActive && selected}
                        cosetId={cosetId}
                        muted={muted}
                        selectionIndex={item === null ? undefined : selectedOrder.get(item)}
                      />
                  </div>
                );
              })}
            </div>
          ))}
        </div>
      </div>
    </section>
  );
}

function ScanPanel({
  scan,
  active,
}: {
  scan: { total: number; failures: CaseAnalysis[] };
  active: boolean;
}) {
  const sampleFailures = scan.failures.slice(0, 5);

  return (
    <section className={classNames("scanPanel", active && "rowPanelActive")}>
      <div>
        <div className="rowTitle">Current-size sweep</div>
        <div className="scanSummary">
          {scan.failures.length} failures / {scan.total} cases
        </div>
      </div>
      {sampleFailures.length > 0 && (
        <div className="failureList">
          {sampleFailures.map((failure) => (
            <div
              className="failureItem"
              key={`${failure.mode}-${failure.size}-${failure.value}-${failure.quotientSize}-${failure.phase}`}
            >
              <strong>
                *{failure.size}+{failure.value}, divisor {failure.quotientSize}, phase {failure.phase}
              </strong>
              <span>
                selected [{failure.selectedItems.join(" ")}], got [
                {failure.generatedItems.join(" ")}]
              </span>
            </div>
          ))}
        </div>
      )}
    </section>
  );
}

function App() {
  const [size, setSize] = useState(10);
  const [value, setValue] = useState(5);
  const [quotientSize, setQuotientSize] = useState(2);
  const [phase, setPhase] = useState(0);
  const [focus, setFocus] = useState<Focus>("parent");
  const [stage, setStage] = useState(0);
  const [playing, setPlaying] = useState(false);
  const [speed, setSpeed] = useState(1100);
  const mode: Mode = focus === "divisor" ? "quotient" : "divisor";
  const quotientOptions = useMemo(() => divisorsOf(size), [size]);
  const effectiveQuotientSize = quotientOptions.includes(quotientSize)
    ? quotientSize
    : quotientOptions[0] ?? 1;
  const activeValue = Math.min(value, size - 1);
  const divisorSize = size / effectiveQuotientSize;
  const phaseMax = focus === "divisor" ? divisorSize - 1 : effectiveQuotientSize - 1;
  const activePhase = Math.min(phase, phaseMax);

  useEffect(() => {
    if (!quotientOptions.includes(quotientSize)) {
      setQuotientSize(quotientOptions[0] ?? 1);
    }
  }, [quotientOptions, quotientSize]);

  useEffect(() => {
    setValue((current) => Math.min(current, size - 1));
  }, [size]);

  useEffect(() => {
    setPhase((current) => Math.min(current, phaseMax));
  }, [phaseMax]);

  function handleFocusChange(nextFocus: Focus) {
    setFocus(nextFocus);
    setStage(0);
  }

  const model = useMemo(() => {
    const parentBaseline = range(size);
    const parentTraversal = canonicalTraversal(parentBaseline, activeValue);
    const parentCosets = decomposeCosets(parentBaseline, activeValue);
    const cosetLookup = makeCosetLookup(parentCosets);
    const transitionMeasures = parentTransitionMeasures(parentTraversal, effectiveQuotientSize, size);
    const divisorStep = modeOf(transitionMeasures.map((measure) => measure.spans));
    const quotientStep = modeOf(transitionMeasures.map((measure) => measure.residual));
    const divisorItems = selectDivisorItems(parentTraversal, divisorSize, activePhase);
    const quotientItems = selectQuotientItems(parentTraversal, divisorSize, effectiveQuotientSize, activePhase);
    const activeItems = mode === "divisor" ? divisorItems : quotientItems;
    const activePeriod = mode === "divisor" ? divisorSize : effectiveQuotientSize;
    const activeStep = mode === "divisor" ? divisorStep : quotientStep;
    const inheritedBase = restrictedBaseline(parentBaseline, activeItems);
    const divisorPlacements = placeOnUnwrappedLine(divisorItems, divisorStep, divisorSize);
    const quotientPlacements = placeOnUnwrappedLine(quotientItems, quotientStep, effectiveQuotientSize);
    const divisorBase = foldedBaseline(divisorPlacements, divisorSize);
    const quotientBase = foldedBaseline(quotientPlacements, effectiveQuotientSize);
    const activeBase = mode === "divisor" ? divisorBase : quotientBase;
    const ghostMoves = ghostBaselineMoves(parentBaseline, activeBase);
    const rebaseNeeded = !baselinesMatch(inheritedBase, activeBase);
    const childBaseline = compactItems(activeBase);
    const generatedItems =
      childBaseline.length === activePeriod ? canonicalTraversal(childBaseline, activeStep) : [];
    const mismatchIndex = firstMismatch(activeItems, generatedItems);
    const scan = scanCases(size, mode);

    return {
      parentBaseline,
      parentTraversal,
      parentCosets,
      cosetLookup,
      divisorSize,
      divisorItems,
      quotientItems,
      activeItems,
      activePeriod,
      activeStep,
      divisorStep,
      quotientStep,
      inheritedBase,
      activeBase,
      ghostMoves,
      rebaseNeeded,
      generatedItems,
      mismatchIndex,
      scan,
    };
  }, [activePhase, activeValue, divisorSize, effectiveQuotientSize, mode, size]);

  useEffect(() => {
    if (isStageDisabled(stage, focus, model.rebaseNeeded)) {
      setStage(1);
    }
  }, [focus, model.rebaseNeeded, stage]);

  useEffect(() => {
    if (!playing) return;
    const id = window.setInterval(() => {
      setStage((current) => nextStage(current, focus, model.rebaseNeeded));
    }, speed);
    return () => window.clearInterval(id);
  }, [focus, model.rebaseNeeded, playing, speed]);

  const activeItems = mode === "divisor" ? model.divisorItems : model.quotientItems;
  const displayedItems = focus === "parent" ? model.parentTraversal : activeItems;
  const displayedSet = new Set(displayedItems);
  const selectedOrder = selectionOrder(displayedItems);
  const fadeUnselected = focus !== "parent" && stage >= 1;
  const selectionActive = focus !== "parent" && stage === 0;
  const stepText = stepLabel(focus, stage, model.divisorSize, effectiveQuotientSize, activePhase, model.rebaseNeeded);
  const passed = focus === "parent" || model.mismatchIndex === -1;
  const activeResult = focus === "parent" ? `*${size}+${activeValue}` : `*${model.activePeriod}+${model.activeStep}`;
  const statusText =
    focus === "parent" ? "SOURCE" : passed ? "PASS" : `FAIL @ ${model.mismatchIndex}`;

  return (
    <main className="appShell">
      <header className="topBar">
        <div>
          <h1>Proteus Infon Decomposer</h1>
          <div className="signature">
            *{size}+{activeValue} = *{model.divisorSize}+{model.divisorStep} x *{effectiveQuotientSize}+
            {model.quotientStep}
          </div>
        </div>
        <div className="transport">
          <button
            className="iconButton"
            onClick={() => setPlaying((current) => !current)}
            title={playing ? "Pause" : "Play"}
            type="button"
          >
            {playing ? <Pause size={18} /> : <Play size={18} />}
          </button>
          <button
            className="iconButton"
            onClick={() => setStage((current) => nextStage(current, focus, model.rebaseNeeded))}
            title="Step"
            type="button"
          >
            <StepForward size={18} />
          </button>
          <button
            className="iconButton"
            onClick={() => setStage(0)}
            title="Reset"
            type="button"
          >
            <RotateCcw size={18} />
          </button>
        </div>
      </header>

      <section className="controlBand">
        <label className="control">
          <span>Size</span>
          <input
            min={1}
            max={MAX_SIZE}
            type="range"
            value={size}
            onChange={(event) => setSize(Number(event.target.value))}
          />
          <input
            min={1}
            max={MAX_SIZE}
            type="number"
            value={size}
            onChange={(event) => setSize(Math.max(1, Math.min(MAX_SIZE, Number(event.target.value))))}
          />
        </label>
        <label className="control">
          <span>Value</span>
          <input
            min={0}
            max={size - 1}
            type="range"
            value={activeValue}
            onChange={(event) => setValue(Number(event.target.value))}
          />
          <input
            min={0}
            max={size - 1}
            type="number"
            value={activeValue}
            onChange={(event) => setValue(Math.max(0, Math.min(size - 1, Number(event.target.value))))}
          />
        </label>
        <label className="control compact">
          <span>Divisor</span>
          <select value={effectiveQuotientSize} onChange={(event) => setQuotientSize(Number(event.target.value))}>
            {quotientOptions.map((option) => (
              <option key={option} value={option}>
                {option}
              </option>
            ))}
          </select>
        </label>
        <label className="control compact">
          <span>Phase</span>
          <input
            min={0}
            max={Math.max(0, phaseMax)}
            type="number"
            value={activePhase}
            onChange={(event) =>
              setPhase(Math.max(0, Math.min(phaseMax, Number(event.target.value))))
            }
          />
        </label>
        <label className="control compact">
          <span>Speed</span>
          <input
            min={500}
            max={1800}
            step={100}
            type="range"
            value={speed}
            onChange={(event) => setSpeed(Number(event.target.value))}
          />
        </label>
      </section>

      <div className="animationShell">
        <nav className="stepRail" aria-label="Animation stage">
          {STAGES.map((label, index) => {
            const disabled = isStageDisabled(index, focus, model.rebaseNeeded);
            return (
              <button
                className={classNames(
                  "stepButton",
                  index === stage && "selected",
                  index < stage && !disabled && "complete",
                )}
                disabled={disabled}
                key={label}
                onClick={() => setStage(index)}
                aria-label={`Step ${index + 1}: ${label}`}
                type="button"
              >
                <span className="stepDot">{index + 1}</span>
                <span className="stepLabel">{label}</span>
              </button>
            );
          })}
        </nav>

        <section className="workspace">
          <ParentIdentityMap
            active={focus === "parent" || stage <= REBASE_STAGE}
            cosetLookup={model.cosetLookup}
            divisorSize={model.divisorSize}
            divisorStep={model.divisorStep}
            fadeUnselected={fadeUnselected}
            focus={focus}
            ghostActive={focus !== "parent" && stage === GHOST_STAGE && model.rebaseNeeded}
            ghostMoves={model.ghostMoves}
            onFocusChange={handleFocusChange}
            parentBaseline={model.parentBaseline}
            parentTraversal={model.parentTraversal}
            quotientSize={effectiveQuotientSize}
            quotientStep={model.quotientStep}
            rebaseActive={focus !== "parent" && stage === REBASE_STAGE}
            selectedOrder={selectedOrder}
            selectedSet={displayedSet}
            selectionActive={selectionActive}
            size={size}
            stepText={stepText}
            value={activeValue}
          />

          <ParentCosetStack
            active={focus === "parent" && stage === 0}
            cosetLookup={model.cosetLookup}
            fadeUnselected={fadeUnselected}
            horizontalPeriod={model.divisorSize}
            parentTraversal={model.parentTraversal}
            selectedOrder={selectedOrder}
            selectedSet={displayedSet}
            selectionActive={selectionActive}
            verticalPeriod={effectiveQuotientSize}
          />

          {focus !== "parent" && (
            <>
              <Row
                title={focus === "quotient" ? "Remaining quotient singlets" : "Remaining divisor singlets"}
                items={activeItems}
                cosetLookup={model.cosetLookup}
                active={stage <= 1}
              />
            </>
          )}

          <section className={classNames("resultBand", passed ? "passBand" : "failBand")}>
            <div>
              <span>Active result</span>
              <strong>{activeResult}</strong>
            </div>
            <div>
              <span>{focus === "parent" ? "Traversal" : "Selected"}</span>
              <strong>[{displayedItems.join(" ")}]</strong>
            </div>
            <div>
              <span>Status</span>
              <strong>{statusText}</strong>
            </div>
          </section>

          {focus !== "parent" && (
            <ScanPanel scan={model.scan} active={!passed || model.scan.failures.length > 0} />
          )}
        </section>
      </div>
    </main>
  );
}

export { App };
