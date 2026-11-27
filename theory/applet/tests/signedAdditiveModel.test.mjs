import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import ts from "typescript";

// Compile the standalone model in memory. No extra test dependency or generated
// source is needed, and this works on the app's supported Node 20.19+ runtime.
const source = await readFile(new URL("../src/signedAdditiveModel.ts", import.meta.url), "utf8");
const compiled = ts.transpileModule(source, {
  compilerOptions: { target: ts.ScriptTarget.ES2020, module: ts.ModuleKind.ESNext },
  reportDiagnostics: true,
});
assert.deepEqual(compiled.diagnostics?.filter((item) => item.category === ts.DiagnosticCategory.Error), []);
const { allowedAValues, buildSignedAdditiveModel } = await import(
  `data:text/javascript;base64,${Buffer.from(compiled.outputText).toString("base64")}`
);

const modulo = (value, size) => ((value % size) + size) % size;
const indices = (size) => Array.from({ length: size }, (_, index) => index);

function expectedCosets(real, value) {
  const left = new Set(real);
  const cycles = [];
  for (let rank = 0; rank < real.length; rank += 1) {
    if (!left.has(real[rank])) continue;
    const cycle = [];
    let current = rank;
    do {
      cycle.push(real[current]);
      left.delete(real[current]);
      current = (current + value) % real.length;
    } while (current !== rank);
    cycles.push(cycle);
  }
  assert.equal(left.size, 0);
  return cycles;
}

/**
 * Independent certificate audit. It uses literal additions of the published
 * edge increments, rather than the constructor's periodic-window composition,
 * correction, inverse, or first-return routines.
 */
function auditChild(child, size) {
  assert.equal(child.window.length, size);
  assert.equal(child.successors.length, size);
  assert.equal(child.increments.length, size);
  const images = new Set();
  for (let root = 0; root < size; root += 1) {
    assert.ok(Number.isSafeInteger(child.increments[root]));
    const destination = modulo(root + child.increments[root], size);
    images.add(destination);
    assert.equal(child.successors[root], destination);
    assert.equal(child.window[root], root + child.increments[root]);
  }
  assert.equal(images.size, size);
  assert.equal(child.real.length, child.size);
  const real = new Set(child.real);
  assert.equal(real.size, child.size);
  assert.deepEqual(child.ghosts, indices(size).filter((root) => !real.has(root)));
  assert.deepEqual(child.cosets, expectedCosets(child.real, child.value));

  const usedEdges = new Set();
  assert.equal(child.routes.length, child.size);
  for (let rank = 0; rank < child.size; rank += 1) {
    const start = child.real[rank];
    const positions = [start];
    const increments = [];
    let cursor = start;
    for (let steps = 0; ; steps += 1) {
      assert.ok(steps < size, "real first return must occur within N steps");
      const root = modulo(cursor, size);
      assert.ok(!usedEdges.has(root), "real-return interiors must be disjoint");
      usedEdges.add(root);
      increments.push(child.increments[root]);
      cursor += child.increments[root];
      positions.push(cursor);
      if (real.has(modulo(cursor, size))) break;
    }
    const expectedEnd = child.real[(rank + child.value) % child.size];
    const expectedNet = modulo(expectedEnd - start, size);
    assert.equal(modulo(cursor, size), expectedEnd);
    assert.equal(cursor - start, expectedNet);
    const route = child.routes[rank];
    assert.equal(route.start, start);
    assert.equal(route.end, expectedEnd);
    assert.equal(route.expectedNet, expectedNet);
    assert.equal(route.net, expectedNet);
    assert.deepEqual(route.positions, positions);
    assert.deepEqual(route.roots, positions.map((position) => modulo(position, size)));
    assert.deepEqual(route.increments, increments);
    assert.equal(route.passed, true);
  }

  let cycleIndex = 0;
  for (let start = 0; start < size; start += 1) {
    if (usedEdges.has(start)) continue;
    let cursor = start;
    const positions = [start];
    const increments = [];
    for (let steps = 0; ; steps += 1) {
      assert.ok(steps < size, "ghost cycle must close within N steps");
      const root = modulo(cursor, size);
      assert.ok(!real.has(root));
      assert.ok(!usedEdges.has(root));
      usedEdges.add(root);
      increments.push(child.increments[root]);
      cursor += child.increments[root];
      positions.push(cursor);
      if (modulo(cursor, size) === start) break;
    }
    assert.equal(cursor, start, "ghost-only cycles must have zero winding");
    const cycle = child.ghostCycles[cycleIndex++];
    assert.equal(cycle.start, start);
    assert.equal(cycle.end, start);
    assert.equal(cycle.net, 0);
    assert.equal(cycle.expectedNet, 0);
    assert.deepEqual(cycle.positions, positions);
    assert.deepEqual(cycle.increments, increments);
    assert.deepEqual(cycle.roots, positions.map((position) => modulo(position, size)));
    assert.equal(cycle.passed, true);
  }
  assert.equal(cycleIndex, child.ghostCycles.length);
  assert.equal(usedEdges.size, size, "every root edge belongs to a return route or ghost-only cycle");
  assert.equal(child.increments.reduce((sum, step) => sum + step, 0), size * child.value);
}

function auditModel(model) {
  const { size, value, a, b } = model;
  assert.deepEqual(model.baseline, indices(size));
  assert.deepEqual(a.real, indices(a.size));
  assert.deepEqual(b.real, [0, ...indices(size).slice(a.size)]);
  assert.equal(a.size + b.size - 1, size);
  assert.equal(a.value + b.value, value);
  assert.deepEqual(a.real.filter((root) => b.real.includes(root)), [0]);
  assert.deepEqual(model.parentCosets, expectedCosets(indices(size), value));
  auditChild(a, size);
  auditChild(b, size);
  assert.equal(model.composition.length, size);
  for (let root = 0; root < size; root += 1) {
    const afterA = root + a.increments[root];
    const deltaB = b.increments[modulo(afterA, size)];
    const end = afterA + deltaB;
    assert.equal(end, root + value);
    assert.deepEqual(model.composition[root], {
      start: root,
      afterA,
      end,
      deltaA: a.increments[root],
      deltaB,
      total: value,
      expected: value,
      passed: true,
    });
    for (const lap of [-2, 3]) {
      const start = root + lap * size;
      const first = start + a.increments[modulo(start, size)];
      const final = first + b.increments[modulo(first, size)];
      assert.equal(final, start + value, "composition must retain absolute occurrence coordinates");
    }
  }
  assert.ok(model.checks.length > 0);
  assert.ok(model.checks.every((check) => check.passed));
  assert.equal(model.passed, true);
}

test("signed construction realizes the previously omitted *3+1 + *5+4 split", () => {
  const model = buildSignedAdditiveModel(7, 5, 3, 1);
  auditModel(model);
  assert.deepEqual(model.a.real, [0, 1, 2]);
  assert.deepEqual(model.b.real, [0, 3, 4, 5, 6]);
  assert.deepEqual(model.a.cosets, [[0, 1, 2]]);
  assert.deepEqual(model.b.cosets, [[0, 6, 5, 4, 3]]);
  assert.ok([...model.a.increments, ...model.b.increments].some((step) => step < 0));
});

test("zero values retain their singleton cosets and exact zero-return paths", () => {
  const zero = buildSignedAdditiveModel(3, 0, 2, 0);
  auditModel(zero);
  assert.deepEqual(zero.parentCosets, [[0], [1], [2]]);
  assert.deepEqual(zero.a.cosets, [[0], [1]]);
  assert.deepEqual(zero.b.cosets, [[0], [2]]);
  assert.ok([...zero.a.routes, ...zero.b.routes].every((route) => route.net === 0));

  const movingZero = buildSignedAdditiveModel(7, 5, 2, 0);
  auditModel(movingZero);
  assert.ok(movingZero.a.routes.some((route) => route.positions.length > 2 && route.net === 0));
  assert.ok(movingZero.a.increments.some((step) => step < 0));
});

test("singleton parents and either singleton child preserve the same carrier", () => {
  for (const parameters of [[1, 0, 1, 0], [7, 5, 1, 0], [7, 5, 7, 5]]) {
    auditModel(buildSignedAdditiveModel(...parameters));
  }
  const one = buildSignedAdditiveModel(1, 0, 1, 0);
  assert.deepEqual(one.a.routes[0].positions, [0, 0]);
  assert.deepEqual(one.a.ghostCycles, []);
  assert.deepEqual(one.b.ghostCycles, []);
});

test("every numeric split through N=30 passes an independent literal certificate audit", () => {
  let total = 0;
  for (let size = 1; size <= 30; size += 1) {
    for (let value = 0; value < size; value += 1) {
      let parentCount = 0;
      for (let aSize = 1; aSize <= size; aSize += 1) {
        const bSize = size + 1 - aSize;
        const expected = indices(aSize).filter((candidate) => value - candidate >= 0 && value - candidate < bSize);
        const allowed = allowedAValues(size, value, aSize);
        assert.deepEqual(allowed, expected);
        for (const aValue of allowed) {
          const model = buildSignedAdditiveModel(size, value, aSize, aValue);
          assert.equal(model.size, size);
          assert.equal(model.value, value);
          assert.equal(model.a.size, aSize);
          assert.equal(model.a.value, aValue);
          assert.equal(model.b.size, bSize);
          assert.equal(model.b.value, value - aValue);
          auditModel(model);
          total += 1;
          parentCount += 1;
        }
      }
      assert.equal(parentCount, (value + 1) * (size - value));
    }
  }
  assert.equal(total, 40920);
});

test("invalid finite parameters fail before constructing a certificate", () => {
  for (const parameters of [
    [0, 0, 1], [7, -1, 3], [7, 7, 3], [7, 5, 0], [7, 5, 8],
    [7.5, 5, 3], [7, 5.5, 3], [7, 5, 3.5], [NaN, 0, 1],
    [Infinity, 0, 1], [Number.MAX_SAFE_INTEGER + 1, 0, 1],
  ]) {
    assert.throws(() => allowedAValues(...parameters));
    assert.throws(() => buildSignedAdditiveModel(...parameters, 1));
  }
  for (const aValue of [-1, 0, 3, 0.5, NaN, Infinity]) {
    assert.throws(() => buildSignedAdditiveModel(7, 5, 3, aValue));
  }
});
