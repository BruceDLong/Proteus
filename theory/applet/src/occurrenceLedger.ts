/**
 * Experimental phase-0 occurrence ledger for the contiguous (quotient) factor.
 * No predicted child baseline is supplied to this algorithm.
 *
 * Given parent *N+v and ordered factor sizes N=a*b, select the first a
 * parent-traversal singlets.  Process reduced rational thresholds from
 * alpha=v/N down to beta=floor(v/b)/a.  A threshold changes local precedence
 * relations; each root singlet then advances its own occurrence counter only
 * as far as required to satisfy those relations.  Its identity never changes.
 *
 * The b-block projection and the nondecreasing occurrence-history policy are
 * experimental theory assumptions, not established Proteus axioms.
 */

export type Fraction = Readonly<{ numerator: number; denominator: number }>;

export type OccurrenceMove = Readonly<{
  traversalIndex: number;
  rootIdentity: number;
  oldCounter: number;
  newCounter: number;
  oldPosition: number;
  newPosition: number;
  reason: "tie group" | "propagation";
}>;

export type BoundaryEvent = Readonly<{
  fraction: Fraction;
  zeroPhase: readonly number[];
  nonzeroTies: readonly (readonly number[])[];
  beforeOrder: readonly number[];
  afterOrder: readonly number[];
  moves: readonly OccurrenceMove[];
}>;

/** One frame is either an event declaration, an atomic +1 occurrence, or completion. */
export type OccurrenceFrame = Readonly<{
  title: string;
  detail: string;
  eventIndex: number;
  kind: "initial" | "boundary" | "advance" | "complete";
  counters: readonly number[]; // indexed by traversal position t
  selectedOrder: readonly number[]; // root identities, sorted by selected occurrence
  targetOrder: readonly number[] | null;
  changedRoot: number | null;
}>;

export type OccurrenceTrace = Readonly<{
  parentSize: number;
  parentValue: number;
  factorSize: number;
  opposingSize: number;
  blockStep: number;
  alpha: Fraction;
  beta: Fraction;
  selectedTraversal: readonly number[];
  inheritedBaseline: readonly number[];
  events: readonly BoundaryEvent[];
  frames: readonly OccurrenceFrame[];
  finalBaseline: readonly number[];
  finalPositions: readonly number[];
  finalCounters: readonly number[];
  benchmarkMatch: boolean; // independent assertion, not an input to the ledger
}>;

function assert(condition: unknown, message: string): asserts condition {
  if (!condition) throw new Error(`Occurrence ledger: ${message}`);
}

function gcd(a: number, b: number): number {
  while (b !== 0) [a, b] = [b, a % b];
  return a;
}

function fractionString(f: Fraction): string {
  return `${f.numerator}/${f.denominator}`;
}

export function canonicalTraversal(size: number, value: number): number[] {
  assert(Number.isInteger(size) && size > 0, "size must be positive");
  assert(Number.isInteger(value) && 0 <= value && value < size, "invalid parent value");
  const visited = new Set<number>();
  const out: number[] = [];
  for (let seed = 0; seed < size; seed += 1) {
    let index = seed;
    while (!visited.has(index)) {
      visited.add(index);
      out.push(index);
      index = (index + value) % size;
    }
  }
  return out;
}

export function thresholdEvents(a: number, beta: Fraction, alpha: Fraction): Fraction[] {
  const result: Fraction[] = [];
  // Exact integer cross products, no floating comparison near tie boundaries.
  for (let q = 2; q < a; q += 1) {
    for (let k = 1; k < q; k += 1) {
      if (gcd(k, q) !== 1) continue;
      if (k * beta.denominator <= q * beta.numerator) continue;
      if (k * alpha.denominator > q * alpha.numerator) continue;
      result.push({ numerator: k, denominator: q });
    }
  }
  return result.sort((x, y) => y.numerator * x.denominator - x.numerator * y.denominator);
}

function eventTargetOrder(current: readonly number[], q: number): {
  order: number[];
  zeroPhase: number[];
  nonzeroTies: number[][];
} {
  assert(current[0] === 0 && q >= 2, "origin and event denominator required");
  let order = [...current];
  const nonzeroTies: number[][] = [];
  for (let residue = 1; residue < q; residue += 1) {
    const positions = order.flatMap((t, i) => t % q === residue ? [i] : []);
    if (positions.length < 2) continue;
    assert(positions[positions.length - 1] - positions[0] + 1 === positions.length,
      "nonzero tie group not contiguous");
    const group = positions.map(i => order[i]);
    assert(group.every((v, i) => i === 0 || group[i - 1] < v),
      "nonzero tie group not in right-limit order");
    nonzeroTies.push(group);
    order.splice(positions[0], positions.length, ...group.slice().reverse());
  }
  const zeroPhase = order.filter(t => t !== 0 && t % q === 0);
  assert(zeroPhase.every((v, i) => order[i + 1] === v),
    "zero-phase group not adjacent to origin");
  const advancing = new Set(zeroPhase);
  order = order.filter(t => !advancing.has(t)).concat(zeroPhase.slice().reverse());
  assert(new Set(order).size === current.length && order.length === current.length,
    "event must preserve singlet identities");
  return { order, zeroPhase, nonzeroTies };
}

function fromCounters(roots: readonly number[], counters: readonly number[], N: number): number[] {
  return roots.map((_, i) => i).sort((i, j) =>
    (roots[i] + N * counters[i]) - (roots[j] + N * counters[j]));
}

export function buildOccurrenceTrace(N: number, value: number, b: number): OccurrenceTrace {
  assert(Number.isInteger(N) && N >= 4 && Number.isInteger(value) && 0 <= value && value < N,
    "expected finite nontrivial parent *N+v");
  assert(Number.isInteger(b) && b > 1 && b < N && N % b === 0,
    "opposing factor must divide N nontrivially");
  const a = N / b;
  const blockStep = Math.floor(value / b);
  const alpha = { numerator: value, denominator: N };
  const beta = { numerator: blockStep, denominator: a };
  const roots = canonicalTraversal(N, value).slice(0, a);
  const inherited = [...roots].sort((i, j) => i - j);
  const counters = roots.map(() => 0);
  const order = fromCounters(roots, counters, N);
  assert(roots[order[0]] === 0, "selected traversal must begin at origin");

  const frames: OccurrenceFrame[] = [];
  const events: BoundaryEvent[] = [];
  let currentOrder = order;
  const asRoots = (indices: readonly number[]) => indices.map(t => roots[t]);
  const snap = (title: string, detail: string, eventIndex: number,
    kind: OccurrenceFrame["kind"], changedRoot: number | null,
    targetOrder: readonly number[] | null) => {
    frames.push({title, detail, eventIndex, kind, changedRoot,
      counters: [...counters], selectedOrder: asRoots(fromCounters(roots, counters, N)),
      targetOrder: targetOrder ? asRoots(targetOrder) : null });
  };
  snap("Inherited baseline", "All singlets start at occurrence 0.", -1, "initial", null, null);

  for (const fraction of thresholdEvents(a, beta, alpha)) {
    const eventIndex = events.length;
    const before = [...currentOrder];
    const { order: target, zeroPhase, nonzeroTies } =
      eventTargetOrder(currentOrder, fraction.denominator);
    const participants = new Set<number>([...zeroPhase, ...nonzeroTies.flat()]);
    const label = fractionString(fraction);
    snap(`Boundary ${label}`, `Tie groups now require a different ordering; advance occurrences to satisfy it.`,
      eventIndex, "boundary", null, target);
    const moves: OccurrenceMove[] = [];
    let previous = -1;
    for (const t of target) {
      let pos = roots[t] + N * counters[t];
      const oldCounter = counters[t];
      const oldPosition = pos;
      while (pos <= previous) {
        counters[t] += 1;
        pos += N;
        snap(`Advance singlet ${roots[t]}`,
          `${roots[t]}: occurrence ${counters[t] - 1} → ${counters[t]}; position ${pos - N} → ${pos} (+${N}).`,
          eventIndex, "advance", roots[t], target);
      }
      if (counters[t] !== oldCounter) {
        moves.push({ traversalIndex: t, rootIdentity: roots[t],
          oldCounter, newCounter: counters[t], oldPosition, newPosition: pos,
          reason: participants.has(t) ? "tie group" : "propagation" });
      }
      previous = pos;
    }
    currentOrder = fromCounters(roots, counters, N);
    assert(target.every((t, i) => t === currentOrder[i]),
      "occurrence updates did not satisfy event order");
    events.push({fraction, zeroPhase, nonzeroTies, beforeOrder: asRoots(before),
      afterOrder: asRoots(target), moves });
    snap(`Boundary ${label} complete`, "All updated occurrence positions now satisfy this event's ordering.",
      eventIndex, "complete", null, target);
  }
  const finalOrder = fromCounters(roots, counters, N);
  const finalBaseline = asRoots(finalOrder);
  const finalPositions = finalOrder.map(t => roots[t] + N * counters[t]);
  for (let i = 0; i < finalPositions.length; i += 1) {
    assert(finalPositions[i] % N === finalBaseline[i], "root identity changed during an advance");
    if (i !== 0) assert(finalPositions[i - 1] < finalPositions[i], "occurrence order not strict");
  }

  // Independent audit only: do not use a predicted baseline to select moves.
  const predictedOrder = canonicalTraversal(a, blockStep);
  const benchmark: number[] = Array(a);
  predictedOrder.forEach((localIndex, i) => { benchmark[localIndex] = roots[i]; });
  const benchmarkMatch = benchmark.every((root, i) => root === finalBaseline[i]);

  return {parentSize: N, parentValue: value, factorSize: a, opposingSize: b,
    blockStep, alpha, beta, selectedTraversal: roots, inheritedBaseline: inherited,
    events, frames, finalBaseline, finalPositions, finalCounters: [...counters],
    benchmarkMatch};
}