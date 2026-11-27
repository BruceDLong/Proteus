import assert from "node:assert/strict";
import { existsSync, readFileSync } from "node:fs";
import { createRequire } from "node:module";
import path from "node:path";
import test from "node:test";
import { fileURLToPath } from "node:url";
import vm from "node:vm";
import React from "react";
import ts from "typescript";

const require = createRequire(import.meta.url);
const sourceDirectory = fileURLToPath(new URL("../src/", import.meta.url));
const compiledSources = new Map();

function compile(filename) {
  if (!compiledSources.has(filename)) {
    const result = ts.transpileModule(readFileSync(filename, "utf8"), {
      compilerOptions: {
        target: ts.ScriptTarget.ES2020,
        module: ts.ModuleKind.CommonJS,
        jsx: ts.JsxEmit.ReactJSX,
      },
      reportDiagnostics: true,
    });
    assert.deepEqual(result.diagnostics?.filter((item) => item.category === ts.DiagnosticCategory.Error), []);
    compiledSources.set(filename, result.outputText);
  }
  return compiledSources.get(filename);
}

const sameDependencies = (before, after) => before !== undefined && after !== undefined
  && before.length === after.length && before.every((item, index) => Object.is(item, after[index]));

/**
 * Run the actual AdditiveDecomposer and its event callbacks with deterministic
 * state, memo and effect semantics. Only React scheduling and browser timers
 * are replaced; the components and both arithmetic models are real sources.
 * This checks event wiring, not browser layout, focus handling or paint.
 */
function createHarness() {
  const slots = [];
  const intervals = new Map();
  const modules = new Map();
  let cursor = 0;
  let pendingEffects = [];
  let dirty = false;
  let nextTimer = 0;
  let tree;
  const browser = {
    setInterval(callback) {
      const id = ++nextTimer;
      intervals.set(id, callback);
      return id;
    },
    clearInterval(id) {
      intervals.delete(id);
    },
  };
  const hooks = {
    ...React,
    useState(initial) {
      const index = cursor++;
      if (!slots[index]) {
        slots[index] = {
          kind: "state",
          value: typeof initial === "function" ? initial() : initial,
        };
      }
      const record = slots[index];
      assert.equal(record.kind, "state");
      record.set ??= (update) => {
        const value = typeof update === "function" ? update(record.value) : update;
        if (!Object.is(value, record.value)) {
          record.value = value;
          dirty = true;
        }
      };
      return [record.value, record.set];
    },
    useMemo(calculate, dependencies) {
      const index = cursor++;
      if (!slots[index] || !sameDependencies(slots[index].dependencies, dependencies)) {
        slots[index] = { kind: "memo", value: calculate(), dependencies };
      }
      assert.equal(slots[index].kind, "memo");
      return slots[index].value;
    },
    useEffect(effect, dependencies) {
      const index = cursor++;
      slots[index] ??= { kind: "effect", dependencies: undefined, cleanup: undefined };
      const record = slots[index];
      assert.equal(record.kind, "effect");
      if (!sameDependencies(record.dependencies, dependencies)) {
        pendingEffects.push(() => {
          record.cleanup?.();
          record.dependencies = dependencies;
          record.cleanup = effect();
        });
      }
    },
  };

  function load(filename) {
    if (modules.has(filename)) return modules.get(filename).exports;
    const module = { exports: {} };
    modules.set(filename, module);
    const localRequire = (name) => {
      if (name === "react") return hooks;
      if (name.endsWith(".css")) return {};
      if (!name.startsWith(".")) return require(name);
      const base = path.resolve(path.dirname(filename), name);
      const target = [base, `${base}.ts`, `${base}.tsx`].find((candidate) => existsSync(candidate));
      assert.ok(target, `local import ${name} must resolve`);
      return load(target);
    };
    vm.runInNewContext(compile(filename), {
      module,
      exports: module.exports,
      require: localRequire,
      window: browser,
    }, { filename });
    return module.exports;
  }

  const { AdditiveDecomposer } = load(path.join(sourceDirectory, "App.tsx"));
  function render() {
    for (let attempt = 0; attempt < 25; attempt += 1) {
      dirty = false;
      cursor = 0;
      pendingEffects = [];
      tree = AdditiveDecomposer({ onModeChange: () => {} });
      for (const effect of pendingEffects) effect();
      if (!dirty) return tree;
    }
    throw new Error("component effects did not settle");
  }

  return {
    render,
    get tree() { return tree; },
    get intervalCount() { return intervals.size; },
    event(callback, argument) {
      callback(argument);
      return render();
    },
    tick() {
      for (const callback of [...intervals.values()]) callback();
      return render();
    },
    dispose() {
      for (const record of slots) if (record.kind === "effect") record.cleanup?.();
      assert.equal(intervals.size, 0);
    },
  };
}

function* elements(node) {
  if (Array.isArray(node)) {
    for (const child of node) yield* elements(child);
  } else if (React.isValidElement(node)) {
    yield node;
    yield* elements(node.props.children);
  }
}

function text(node) {
  if (node === undefined || node === null || typeof node === "boolean") return "";
  if (Array.isArray(node)) return node.map(text).join("");
  if (React.isValidElement(node)) return text(node.props.children);
  return String(node);
}

function find(tree, predicate, description) {
  const found = [...elements(tree)].find(predicate);
  assert.ok(found, `rendered element not found: ${description}`);
  return found;
}

function methodButton(tree, label) {
  return find(tree, (node) => node.type === "button" && text(node) === label, label);
}

function component(tree, name) {
  return find(tree, (node) => typeof node.type === "function" && node.type.name === name, name);
}

function control(tree, label, tag = "input", inputType = "number") {
  const wrapper = find(tree, (node) => node.type === "label"
    && [...elements(node)].some((child) => child.type === "span" && text(child) === label), label);
  return find(wrapper, (node) => node.type === tag && (tag !== "input" || node.props.type === inputType), `${label} ${tag}`);
}

function transportButton(tree, title) {
  const transport = component(tree, "TransportControls");
  const buttons = transport.type(transport.props); // This real component is stateless.
  return find(buttons, (node) => node.type === "button" && node.props.title === title, `${title} transport`);
}

function signature(tree) {
  return text(find(tree, (node) => node.props.className === "signature", "header signature"))
    .replace(/\s+/g, " ").trim();
}

function change(harness, label, value, tag = "input") {
  return harness.event(control(harness.tree, label, tag).props.onChange, { target: { value: String(value) } });
}

test("additive method events preserve inputs and old overlap, reset transport, and clamp singleton values", () => {
  const app = createHarness();
  try {
    app.render();
    assert.equal(methodButton(app.tree, "Current method").props["aria-pressed"], true);
    assert.equal(methodButton(app.tree, "Signed construction").props["aria-pressed"], false);
    assert.equal(control(app.tree, "B overlap", "select").props.value, "end");

    change(app, "Parent size", 7);
    change(app, "Parent value", 5);
    change(app, "A size", 3);
    change(app, "B overlap", "zero", "select");
    const previousSignature = signature(app.tree);
    assert.match(previousSignature, /\*7\+5/);
    assert.match(previousSignature, /shared 0/);

    app.event(transportButton(app.tree, "Step").props.onClick);
    assert.equal(component(app.tree, "DecompositionStageRail").props.stage, 1);
    app.event(transportButton(app.tree, "Play").props.onClick);
    assert.equal(component(app.tree, "TransportControls").props.playing, true);
    assert.equal(app.intervalCount, 1);

    app.event(methodButton(app.tree, "Signed construction").props.onClick);
    assert.equal(methodButton(app.tree, "Signed construction").props["aria-pressed"], true);
    assert.equal(component(app.tree, "TransportControls").props.playing, false);
    assert.equal(app.intervalCount, 0, "switching method must clean up the playing timer");
    let signed = component(app.tree, "SignedAdditiveView");
    assert.equal(signed.props.stage, 0);
    assert.equal(signed.props.model.size, 7);
    assert.equal(signed.props.model.value, 5);
    assert.equal(signed.props.model.a.size, 3);
    assert.deepEqual([...control(app.tree, "A value", "select").props.children].map((option) => option.props.value), [1, 2]);

    change(app, "A value", 1, "select");
    signed = component(app.tree, "SignedAdditiveView");
    assert.equal(signed.props.model.a.value, 1);
    assert.equal(signed.props.model.b.size, 5);
    assert.equal(signed.props.model.b.value, 4);
    assert.equal(signed.props.model.passed, true);
    assert.match(signature(app.tree), /\*7\+5 = \*3\+1 \+ \*5\+4/);

    app.event(transportButton(app.tree, "Step").props.onClick);
    app.event(transportButton(app.tree, "Play").props.onClick);
    app.tick();
    assert.equal(component(app.tree, "SignedAdditiveView").props.stage, 2);
    assert.equal(app.intervalCount, 1);

    app.event(methodButton(app.tree, "Current method").props.onClick);
    assert.equal(component(app.tree, "DecompositionStageRail").props.stage, 0);
    assert.equal(component(app.tree, "TransportControls").props.playing, false);
    assert.equal(app.intervalCount, 0);
    assert.equal(control(app.tree, "B overlap", "select").props.value, "zero");
    assert.equal(signature(app.tree), previousSignature, "returning to Current must retain its own labels and overlap choice");
    assert.equal(control(app.tree, "Parent size").props.value, 7);
    assert.equal(control(app.tree, "Parent value").props.value, 5);
    assert.equal(control(app.tree, "A size").props.value, 3);

    app.event(methodButton(app.tree, "Signed construction").props.onClick);
    assert.equal(control(app.tree, "A value", "select").props.value, 1);
    change(app, "Parent size", 1);
    signed = component(app.tree, "SignedAdditiveView");
    assert.equal(control(app.tree, "Parent size").props.value, 1);
    assert.equal(control(app.tree, "Parent value").props.value, 0);
    assert.equal(control(app.tree, "A size").props.value, 1);
    assert.equal(control(app.tree, "A value", "select").props.value, 0);
    assert.equal(signed.props.model.size, 1);
    assert.equal(signed.props.model.value, 0);
    assert.equal(signed.props.model.a.size, 1);
    assert.equal(signed.props.model.a.value, 0);
    assert.equal(signed.props.model.b.size, 1);
    assert.equal(signed.props.model.b.value, 0);
    assert.equal(signed.props.model.passed, true);
  } finally {
    app.dispose();
  }
});
