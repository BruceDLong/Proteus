export type AdditiveGhostMove = {
  item: number;
  source: number;
  target: number;
};

export type AdditiveStepInference = {
  childSize: number;
  counts: Array<{ sample: number; count: number }>;
  distances: number[];
  samples: number[];
  step: number;
  zeroParent: boolean;
};

export type AdditiveChildAnalysis = {
  baseline: number[];
  initialBaseline: number[];
  initialLifts: number[];
  lifts: number[];
  moves: AdditiveGhostMove[];
  selected: number[];
  status: "applied" | "not needed" | "unchanged" | "value changed";
  stepInference?: AdditiveStepInference;
  stepSource: "descents" | "parent";
  value: number;
  valueBefore: number | null;
};

export type AdditiveModel = {
  a: AdditiveChildAnalysis;
  b0: AdditiveChildAnalysis;
  bend: AdditiveChildAnalysis;
  parentBaseline: number[];
  parentTraversal: number[];
  sum0: boolean;
  sumEnd: boolean;
};

function range(size: number): number[] {
  return Array.from({ length: size }, (_, index) => index);
}

function mod(value: number, size: number): number {
  return ((value % size) + size) % size;
}

function gcd(left: number, right: number): number {
  let a = Math.abs(left);
  let b = Math.abs(right);
  while (b !== 0) {
    const remainder = a % b;
    a = b;
    b = remainder;
  }
  return a;
}

export function additiveCanonicalTraversal(size: number, value: number): number[] {
  if (size < 1 || value < 0 || value >= size) {
    throw new Error("require size >= 1 and 0 <= value < size");
  }

  const cycleCount = gcd(size, value);
  const cycleSize = size / cycleCount;
  const traversal: number[] = [];
  for (let start = 0; start < cycleCount; start += 1) {
    for (let turn = 0; turn < cycleSize; turn += 1) {
      traversal.push(mod(start + turn * value, size));
    }
  }
  return traversal;
}

function arraysMatch(left: number[], right: number[]): boolean {
  return left.length === right.length && left.every((item, index) => item === right[index]);
}

function childBaseline(selected: number[]): number[] {
  if (selected.length === 0 || new Set(selected).size !== selected.length) {
    throw new Error("a child must contain distinct singlets and be nonempty");
  }
  const baseline = [...selected].sort((left, right) => left - right);
  const anchor = baseline.indexOf(selected[0]);
  return [...baseline.slice(anchor), ...baseline.slice(0, anchor)];
}

function valueOnBaseline(
  parentValue: number,
  selected: number[],
  baseline: number[],
): number | null {
  if (selected.length !== baseline.length || new Set(selected).size !== selected.length) {
    throw new Error("baseline must contain each selected singlet exactly once");
  }
  const selectedSet = new Set(selected);
  if (baseline.some((item) => !selectedSet.has(item)) || new Set(baseline).size !== selected.length) {
    throw new Error("baseline must contain each selected singlet exactly once");
  }
  if (selected.length === 1 || parentValue === 0) return 0;

  const rank = new Map(baseline.map((item, index) => [item, index]));
  const normalized = selected.map((item) => rank.get(item) ?? -1);
  for (let candidate = 1; candidate < selected.length; candidate += 1) {
    if (arraysMatch(additiveCanonicalTraversal(selected.length, candidate), normalized)) {
      return candidate;
    }
  }
  return null;
}

function forwardLifts(
  parentSize: number,
  baseline: number[],
  sourceLifts?: Map<number, number>,
): number[] {
  let previous = -1;
  return baseline.map((item) => {
    let lift = sourceLifts?.get(item) ?? item;
    if (lift <= previous) {
      lift += (Math.floor((previous - lift) / parentSize) + 1) * parentSize;
    }
    previous = lift;
    return lift;
  });
}

export function inferAdditiveParentStep(
  parentSize: number,
  parentValue: number,
  childSize: number,
): AdditiveStepInference {
  if (childSize < 1 || childSize > parentSize) {
    throw new Error("child size must be between one and parent size");
  }
  const zeroParent = parentValue === 0;
  const parentTraversal = additiveCanonicalTraversal(parentSize, parentValue);
  const distances: number[] = [];
  const samples: number[] = [];
  const counts = new Map<number, number>();
  let step = 0;
  let bestCount = 0;

  if (!zeroParent) {
    for (let index = 0; index < parentTraversal.length - 1; index += 1) {
      const distance = mod(parentTraversal[index + 1] - parentTraversal[index], parentSize);
      const sample = Math.floor((distance * childSize) / parentSize);
      distances.push(distance);
      samples.push(sample);
      const count = (counts.get(sample) ?? 0) + 1;
      counts.set(sample, count);
      if (count > bestCount) {
        step = sample;
        bestCount = count;
      }
    }
  }

  return {
    childSize,
    counts: [...counts].map(([sample, count]) => ({ sample, count })),
    distances,
    samples,
    step,
    zeroParent,
  };
}

function placeChildForStep(
  parentSize: number,
  parentValue: number,
  selected: number[],
  initialBaseline: number[],
  initialLifts: number[],
  candidate: number,
  stepSource: "descents" | "parent",
  stepInference?: AdditiveStepInference,
): AdditiveChildAnalysis {
  const baseline = Array<number>(selected.length);
  const slots = additiveCanonicalTraversal(selected.length, candidate);
  selected.forEach((item, index) => {
    baseline[slots[index]] = item;
  });

  const sources = new Map(initialBaseline.map((item, index) => [item, initialLifts[index]]));
  const lifts = forwardLifts(parentSize, baseline, sources);
  const moves = baseline.flatMap((item, index) => {
    const source = sources.get(item) ?? item;
    return source === lifts[index] ? [] : [{ item, source, target: lifts[index] }];
  });
  const valueBefore = valueOnBaseline(parentValue, selected, initialBaseline);

  return {
    baseline,
    initialBaseline,
    initialLifts,
    lifts,
    moves,
    selected,
    status:
      moves.length > 0
        ? "applied"
        : valueBefore !== candidate
          ? "value changed"
          : "unchanged",
    stepInference,
    stepSource,
    value: candidate,
    valueBefore,
  };
}

function analyzeParentInferredChild(
  parentSize: number,
  parentValue: number,
  selected: number[],
): AdditiveChildAnalysis {
  const initialBaseline = childBaseline(selected);
  const initialLifts = forwardLifts(parentSize, initialBaseline);
  const inference = inferAdditiveParentStep(parentSize, parentValue, selected.length);
  return placeChildForStep(
    parentSize,
    parentValue,
    selected,
    initialBaseline,
    initialLifts,
    inference.step,
    "parent",
    inference,
  );
}

function analyzeDescentChild(
  parentSize: number,
  parentValue: number,
  selected: number[],
): AdditiveChildAnalysis {
  const initialBaseline = childBaseline(selected);
  const initialLifts = forwardLifts(parentSize, initialBaseline);
  const valueBefore = valueOnBaseline(parentValue, selected, initialBaseline);
  if (valueBefore !== null) {
    return {
      baseline: initialBaseline,
      initialBaseline,
      initialLifts,
      lifts: initialLifts,
      moves: [],
      selected,
      status: "not needed",
      stepSource: "descents",
      value: valueBefore,
      valueBefore,
    };
  }

  const ranks = new Map(initialBaseline.map((item, index) => [item, index]));
  const candidate =
    selected.length === 1 || parentValue === 0
      ? 0
      : selected.reduce((descents, item, index) => {
          const next = selected[(index + 1) % selected.length];
          return descents + ((ranks.get(item) ?? 0) > (ranks.get(next) ?? 0) ? 1 : 0);
        }, 0);
  return placeChildForStep(
    parentSize,
    parentValue,
    selected,
    initialBaseline,
    initialLifts,
    candidate,
    "descents",
  );
}

export function buildAdditiveModel(size: number, value: number, aSize: number): AdditiveModel {
  if (size < 1 || value < 0 || value >= size || aSize < 1 || aSize > size) {
    throw new Error("invalid additive decomposition parameters");
  }

  const parentBaseline = range(size);
  const parentTraversal = additiveCanonicalTraversal(size, value);
  const aSelected = parentTraversal.slice(0, aSize);
  const b0Selected = [parentTraversal[0], ...parentTraversal.slice(aSize)];
  const bendSelected = parentTraversal.slice(aSize - 1);
  const a = analyzeParentInferredChild(size, value, aSelected);
  const b0 = analyzeDescentChild(size, value, b0Selected);
  const bend = analyzeDescentChild(size, value, bendSelected);

  return {
    a,
    b0,
    bend,
    parentBaseline,
    parentTraversal,
    sum0: a.value + b0.value === value,
    sumEnd: a.value + bend.value === value,
  };
}
