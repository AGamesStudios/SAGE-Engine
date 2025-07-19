from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, List, Tuple, Dict
import shlex
import time
import yaml

from sage_engine.logic_api import create_object, set_param, destroy_object, emit_event


def load_grammar() -> Tuple[Dict[str, Dict[str, Any]], Dict[str, str]]:
    """Load grammar and alias table from YAML files."""
    base = Path(__file__).with_name("grammar.yaml")
    data: Dict[str, Dict[str, Any]] = {}
    if base.exists():
        data.update(yaml.safe_load(base.read_text(encoding="utf-8")) or {})
    modules = base.with_name("flow_modules")
    if modules.exists():
        for mod in modules.glob("*.yaml"):
            data.update(yaml.safe_load(mod.read_text(encoding="utf-8")) or {})

    keywords: Dict[str, str] = {}
    for name, entry in data.items():
        keywords[name] = name
        for key in entry.get("aliases", []):
            keywords[key] = name
        for key in entry.get("alias_ru", []):
            keywords[key] = name
        for key in entry.get("alias_en", []):
            keywords[key] = name
    return data, keywords


def _get_grammar() -> Tuple[Dict[str, Dict[str, Any]], Dict[str, str]]:
    return load_grammar()


@dataclass
class Node:
    cmd: str
    args: List[str]
    children: List["Node"] = field(default_factory=list)
    else_children: List["Node"] | None = None


def parse_script(text: str) -> List[Node]:
    """Parse FlowScript into a hierarchy of commands."""
    grammar, keywords = _get_grammar()
    commands: List[Node] = []
    stack: List[tuple[int, List[Node], Node | None]] = [(-1, commands, None)]
    for raw in text.splitlines():
        if not raw.strip() or raw.lstrip().startswith("#"):
            continue
        indent = len(raw) - len(raw.lstrip())
        line = raw.strip()
        if line.endswith(":"):
            line = line[:-1]
        parts = shlex.split(line)
        cmd = keywords.get(parts[0], parts[0])
        args = parts[1:]
        node = Node(cmd, args)
        while indent <= stack[-1][0]:
            stack.pop()
        parent_list = stack[-1][1]
        prev_node = stack[-1][2]
        if cmd == "else" and prev_node is not None and prev_node.cmd == "if":
            prev_node.else_children = node.children
            stack.append((indent, node.children, node))
            continue
        parent_list.append(node)
        stack[-1] = (stack[-1][0], parent_list, node)
        if cmd in {"if", "while", "loop"}:
            stack.append((indent, node.children, node))
    return commands


class FlowContext:
    """Shared variables between FlowScript runs."""

    def __init__(self) -> None:
        self.variables: dict[str, Any] = {}


def _eval(expr: List[str], local: dict[str, Any], global_vars: dict[str, Any]) -> Any:
    if not expr:
        return None
    text = " ".join(expr)
    return eval(text, {}, {**global_vars, **local})


class FlowRunner:
    """Execute parsed FlowScript commands."""

    def __init__(self, context: FlowContext | None = None) -> None:
        self.context = context or FlowContext()
        self.locals: dict[str, Any] = {}
        self.global_mode = False
        self.grammar, _ = _get_grammar()

    def reload_grammar(self) -> None:
        self.grammar, _ = _get_grammar()

    def run_file(self, path: str) -> None:
        text = Path(path).read_text(encoding="utf-8")
        cmds = parse_script(text)
        self.execute(cmds)

    def execute(self, cmds: List[Node]) -> None:
        self.locals = {}
        self.global_mode = False
        for node in cmds:
            self._exec(node)

    # object helpers
    def _create_obj(self, role: str, obj_id: str) -> None:
        create_object(obj_id, role)

    def _set_prop(self, obj_id: str, prop: str, values: List[str]) -> None:
        if prop == "pos" and len(values) >= 2:
            set_param(obj_id, "x", float(values[0]))
            set_param(obj_id, "y", float(values[1]))
        elif prop == "color" and len(values) == 3:
            set_param(obj_id, "color", [float(v) for v in values])
        elif prop == "visible" and values:
            val = bool(_eval([values[0]], self.locals, self.context.variables))
            set_param(obj_id, "visible", val)

    def _get_target(self) -> dict[str, Any]:
        return self.context.variables if self.global_mode else self.locals

    def _get_var(self, name: str) -> Any:
        if name in self.locals:
            return self.locals[name]
        return self.context.variables.get(name)

    def _set_var(self, name: str, value: Any) -> None:
        target = self._get_target()
        target[name] = value

    def _exec_block(self, nodes: List[Node]) -> None:
        for n in nodes:
            self._exec(n)

    def _exec(self, node: Node) -> None:
        cmd = node.cmd
        args = node.args
        if cmd == "@global":
            self.global_mode = True
            return
        if cmd == "@local":
            self.global_mode = False
            return

        action = self.grammar.get(cmd, {}).get("action")
        if not action:
            return
        method = getattr(self, f"_act_{action}", None)
        if method:
            method(args, node)

    # action implementations
    def _act_var(self, args: List[str], node: Node) -> None:
        name = args[0]
        expr = args[1:]
        if expr and expr[0] == "=":
            expr = expr[1:]
        value = _eval(expr, self.locals, self.context.variables)
        self._set_var(name, value)

    def _act_set(self, args: List[str], node: Node) -> None:
        if args and args[0] in {"pos", "color", "visible"}:
            prop = args[0]
            obj_id = args[1].strip('"')
            self._set_prop(obj_id, prop, args[2:])
            return
        name = args[0]
        expr = args[1:]
        if expr and expr[0] == "=":
            expr = expr[1:]
        val = _eval(expr, self.locals, self.context.variables)
        self._set_var(name, val)

    def _act_get(self, args: List[str], node: Node) -> None:
        print(self._get_var(args[0]))

    def _act_add(self, args: List[str], node: Node) -> None:
        self._math_op(args[0], args[1:], lambda a, b: a + b)

    def _act_sub(self, args: List[str], node: Node) -> None:
        self._math_op(args[0], args[1:], lambda a, b: a - b)

    def _act_mul(self, args: List[str], node: Node) -> None:
        self._math_op(args[0], args[1:], lambda a, b: a * b)

    def _act_div(self, args: List[str], node: Node) -> None:
        self._math_op(args[0], args[1:], lambda a, b: a / b)

    def _act_print(self, args: List[str], node: Node) -> None:
        msg = " ".join(args)
        if msg.startswith('"') and msg.endswith('"'):
            msg = msg[1:-1]
        print(msg)

    def _act_emit(self, args: List[str], node: Node) -> None:
        if args:
            emit_event(args[0], None)

    def _act_wait(self, args: List[str], node: Node) -> None:
        if args:
            time.sleep(float(args[0]))

    def _act_create(self, args: List[str], node: Node) -> None:
        if len(args) >= 2:
            self._create_obj(args[0], args[1])

    def _act_remove(self, args: List[str], node: Node) -> None:
        if args:
            destroy_object(args[0])

    def _act_if(self, args: List[str], node: Node) -> None:
        cond = _eval(args, self.locals, self.context.variables)
        if cond:
            self._exec_block(node.children)
        elif node.else_children is not None:
            self._exec_block(node.else_children)

    def _act_while(self, args: List[str], node: Node) -> None:
        while _eval(args, self.locals, self.context.variables):
            self._exec_block(node.children)

    def _act_loop(self, args: List[str], node: Node) -> None:
        count = int(_eval(args, self.locals, self.context.variables)) if args else 0
        for _ in range(count):
            self._exec_block(node.children)

    def _act_sqrt(self, args: List[str], node: Node) -> None:
        import math
        val = _eval(args, self.locals, self.context.variables)
        print(math.sqrt(float(val)))

    def _math_op(self, name: str, expr: List[str], func) -> None:
        value = _eval(expr, self.locals, self.context.variables)
        current = self._get_var(name) or 0
        self._set_var(name, func(current, value))


__all__ = ["parse_script", "FlowRunner", "FlowContext", "Node", "load_grammar"]

