/**
 * Signed additive construction on one fixed N-state carrier.
 *
 * This is the finite theory applet's signed model, not a Proteus engine change.
 * Start with B's canonical positive lift Q0, fixing A-exclusive identities.
 * Correct P0 = Q0^-1 T only at its A returns. The interval theorem guarantees
 * that the correction K stays inside each finite A block, so
 * A = K P0, B = Q0 K^-1 preserves both real returns and gives B A = T.
 * The construction and the literal certificate walks are O(N).
 */

export type SignedRoute = {
  start: number;
  end: number;
  roots: number[];
  /** Absolute occurrence coordinates, including both endpoints. */
  positions: number[];
  increments: number[];
  net: number;
  expectedNet: number;
  passed: boolean;
};

export type SignedAdditiveChild = {
  size: number;
  value: number;
  real: number[];
  ghosts: number[];
  /** Canonical local successor cycles, expressed using root identities. */
  cosets: number[][];
  /** F(i) for 0 <= i < N; F(i + kN) = F(i) + kN. */
  window: number[];
  successors: number[];
  increments: number[];
  routes: SignedRoute[];
  ghostCycles: SignedRoute[];
};

export type SignedCompositionRow = {
  start: number;
  afterA: number;
  end: number;
  deltaA: number;
  deltaB: number;
  total: number;
  /** Expected displacement, so the expected endpoint is start + expected. */
  expected: number;
  passed: boolean;
};

export type SignedCheck = {
  id: string;
  label: string;
  detail: string;
  passed: boolean;
};

export type SignedAdditiveModel = {
  size: number;
  value: number;
  baseline: number[];
  parentCosets: number[][];
  a: SignedAdditiveChild;
  b: SignedAdditiveChild;
  composition: SignedCompositionRow[];
  checks: SignedCheck[];
  passed: boolean;
};

function range(size: number): number[] {
  return Array.from({ length: size }, (_, index) => index);
}

function mod(value: number, size: number): number {
  return ((value % size) + size) % size;
}

function requireParent(size: number, value: number, aSize: number): void {
  if (![size, value, aSize].every(Number.isSafeInteger)) {
    throw new Error("sizes and values must be safe integers");
  }
  if (size < 1 || value < 0 || value >= size || aSize < 1 || aSize > size) {
    throw new Error("require N >= 1, 0 <= value < N, and 1 <= A size <= N");
  }
}

/** All numeric A values compatible with this parent and the chosen A size. */
export function allowedAValues(size: number, value: number, aSize: number): number[] {
  requireParent(size, value, aSize);
  const bSize = size + 1 - aSize;
  const minimum = Math.max(0, value - bSize + 1);
  const maximum = Math.min(aSize - 1, value);
  return Array.from({ length: maximum - minimum + 1 }, (_, index) => minimum + index);
}

function at(window: number[], position: number): number {
  const size = window.length;
  return window[mod(position, size)] + Math.floor(position / size) * size;
}

function compose(after: number[], before: number[]): number[] {
  return before.map((position) => at(after, position));
}

function inverse(window: number[]): number[] {
  const size = window.length;
  const result = Array<number>(size);
  const seen = new Set<number>();
  window.forEach((position, root) => {
    const destination = mod(position, size);
    if (!Number.isSafeInteger(position) || seen.has(destination)) {
      throw new Error("a periodic window must preserve each root identity exactly once");
    }
    seen.add(destination);
    result[destination] = root - Math.floor(position / size) * size;
  });
  return result;
}

function firstReturnPosition(window: number[], real: Set<number>, start: number): number {
  let position = start;
  for (let step = 0; step < window.length; step += 1) {
    position = at(window, position);
    if (real.has(mod(position, window.length))) return position;
  }
  throw new Error("a real state did not return within its finite permutation cycle");
}

/** Keep coset boundaries, including one singleton cycle per state at value zero. */
function canonicalCosets(real: number[], value: number): number[][] {
  const seen = new Set<number>();
  const cosets: number[][] = [];
  for (let seed = 0; seed < real.length; seed += 1) {
    if (seen.has(seed)) continue;
    const cycle: number[] = [];
    let rank = seed;
    do {
      seen.add(rank);
      cycle.push(real[rank]);
      rank = (rank + value) % real.length;
    } while (rank !== seed);
    cosets.push(cycle);
  }
  return cosets;
}

function routeFromPositions(
  positions: number[],
  size: number,
  expectedEnd: number,
  expectedNet: number,
): SignedRoute {
  const roots = positions.map((position) => mod(position, size));
  const increments = positions.slice(1).map((position, index) => position - positions[index]);
  const net = positions[positions.length - 1] - positions[0];
  const end = roots[roots.length - 1];
  return {
    start: roots[0],
    end,
    roots,
    positions,
    increments,
    net,
    expectedNet,
    passed: end === expectedEnd && net === expectedNet,
  };
}

/** Observe literal paths in the finished map; do not reuse the construction's returns. */
function describeChild(window: number[], real: number[], value: number): SignedAdditiveChild {
  const size = window.length;
  const realSet = new Set(real);
  const visited = new Set<number>();
  const routes: SignedRoute[] = [];
  for (let rank = 0; rank < real.length; rank += 1) {
    const start = real[rank];
    const positions = [start];
    let position = start;
    let returned = false;
    for (let step = 0; step < size; step += 1) {
      visited.add(mod(position, size));
      position = at(window, position);
      positions.push(position);
      if (realSet.has(mod(position, size))) {
        returned = true;
        break;
      }
    }
    if (!returned) throw new Error("invalid first-return path in signed certificate");
    const expectedEnd = real[(rank + value) % real.length];
    routes.push(routeFromPositions(positions, size, expectedEnd, mod(expectedEnd - start, size)));
  }

  const ghostCycles: SignedRoute[] = [];
  for (let start = 0; start < size; start += 1) {
    if (visited.has(start)) continue;
    const positions = [start];
    let position = start;
    let closed = false;
    for (let step = 0; step < size; step += 1) {
      const root = mod(position, size);
      if (realSet.has(root) || visited.has(root)) {
        throw new Error("ghost-only cycle overlaps another certificate route");
      }
      visited.add(root);
      position = at(window, position);
      positions.push(position);
      if (mod(position, size) === start) {
        closed = true;
        break;
      }
    }
    if (!closed) throw new Error("invalid ghost-only cycle in signed certificate");
    ghostCycles.push(routeFromPositions(positions, size, start, 0));
  }

  return {
    size: real.length,
    value,
    real,
    ghosts: range(size).filter((root) => !realSet.has(root)),
    cosets: canonicalCosets(real, value),
    window,
    successors: window.map((position) => mod(position, size)),
    increments: window.map((position, root) => position - root),
    routes,
    ghostCycles,
  };
}

/**
 * Construct one signed additive factorization of *size+value into the chosen A
 * and its forced companion B. Supports share only root 0. This normalization
 * does not claim every other interleave or nested reconstruction is equivalent.
 */
export function buildSignedAdditiveModel(
  size: number,
  value: number,
  aSize: number,
  aValue: number,
): SignedAdditiveModel {
  requireParent(size, value, aSize);
  const bSize = size + 1 - aSize;
  const bValue = value - aValue;
  if (!Number.isSafeInteger(aValue) || aValue < 0 || aValue >= aSize || bValue < 0 || bValue >= bSize) {
    throw new Error("A value is incompatible with the parent and the two child sizes");
  }

  const baseline = range(size);
  const realA = range(aSize);
  const realB = [0, ...baseline.slice(aSize)];
  const realASet = new Set(realA);
  const initialB = [...baseline];
  realB.forEach((root, rank) => {
    const target = realB[(rank + bValue) % bSize];
    initialB[root] = root + mod(target - root, size);
  });
  const initialA = compose(inverse(initialB), baseline.map((root) => root + value));
  const correction = [...baseline];
  realA.forEach((start) => {
    const initialEnd = firstReturnPosition(initialA, realASet, start);
    const desiredEnd = start + aValue + (bSize - 1) * Math.floor((start + aValue) / aSize);
    correction[mod(initialEnd, size)] = desiredEnd - Math.floor(initialEnd / size) * size;
  });

  const a = describeChild(compose(correction, initialA), realA, aValue);
  const b = describeChild(compose(initialB, inverse(correction)), realB, bValue);
  const composition = baseline.map((start): SignedCompositionRow => {
    const afterA = a.window[start];
    const end = at(b.window, afterA);
    return {
      start,
      afterA,
      end,
      deltaA: afterA - start,
      deltaB: end - afterA,
      total: end - start,
      expected: value,
      passed: end === start + value,
    };
  });
  const sumA = a.increments.reduce((sum, increment) => sum + increment, 0);
  const sumB = b.increments.reduce((sum, increment) => sum + increment, 0);
  const overlap = realB.filter((root) => realASet.has(root));
  const checks: SignedCheck[] = [
    {
      id: "support",
      label: "Shared state and coverage",
      detail: `${aSize} + ${bSize} − 1 = ${size}; only root 0 is shared.`,
      passed: aSize + bSize - 1 === size && overlap.length === 1 && overlap[0] === 0 && new Set([...realA, ...realB]).size === size,
    },
    {
      id: "identities",
      label: "State identities",
      detail: `Both successor maps permute the same ${size} roots.`,
      passed: new Set(a.successors).size === size && new Set(b.successors).size === size,
    },
    {
      id: "a-returns",
      label: "A return routes",
      detail: `All ${aSize} real returns have local step ${aValue} and the exact net displacement.`,
      passed: a.routes.every((route) => route.passed),
    },
    {
      id: "b-returns",
      label: "B return routes",
      detail: `All ${bSize} real returns have local step ${bValue} and the exact net displacement.`,
      passed: b.routes.every((route) => route.passed),
    },
    {
      id: "ghost-cycles",
      label: "Ghost-only cycles",
      detail: `${a.ghostCycles.length + b.ghostCycles.length} cycles; every net displacement is zero.`,
      passed: [...a.ghostCycles, ...b.ghostCycles].every((cycle) => cycle.passed),
    },
    {
      id: "values",
      label: "Child values",
      detail: `Signed displacement totals: A ${sumA} = ${size} × ${aValue}; B ${sumB} = ${size} × ${bValue}.`,
      passed: sumA === size * aValue && sumB === size * bValue,
    },
    {
      id: "composition",
      label: "Parent composition",
      detail: `A then B advances every root by exactly ${value}.`,
      passed: composition.every((row) => row.passed),
    },
  ];

  return {
    size,
    value,
    baseline,
    parentCosets: canonicalCosets(baseline, value),
    a,
    b,
    composition,
    checks,
    passed: checks.every((check) => check.passed),
  };
}
