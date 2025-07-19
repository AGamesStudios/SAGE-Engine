from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, List
import shlex
import time

from sage_engine import object as object_mod
from sage_engine.object import object_from_dict
from sage import emit

KEYWORDS = {
    "let": "var",
    "var": "var",
    "set": "set",
    "get": "get",
    "add": "add",
    "sub": "sub",
    "mul": "mul",
    "div": "div",
    "if": "if",
    "else": "else",
    "while": "while",
    "loop": "loop",
    "wait": "wait",
    "emit": "emit",
    "print": "print",
    "create": "create",
    "remove": "remove",
    # Russian synonyms
    "задать": "var",
    "перем": "var",
    "установить": "set",
    "получить": "get",
    "добавить": "add",
    "вычесть": "sub",
    "умножить": "mul",
    "разделить": "div",
    "если": "if",
    "иначе": "else",
    "пока": "while",
    "цикл": "loop",
    "ждать": "wait",
    "подождать": "wait",
    "отправить": "emit",
    "послать": "emit",
    "сказать": "print",
    "создать": "create",
    "удалить": "remove",
}


@dataclass
class Node:
    cmd: str
    args: List[str]
    children: List["Node"] = field(default_factory=list)
    else_children: List["Node"] | None = None


def parse_script(text: str) -> List[Node]:
    """Parse FlowScript into a hierarchy of commands."""
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
        cmd = KEYWORDS.get(parts[0], parts[0])
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
        obj = object_from_dict({"role": role, "id": obj_id})
        object_mod.add_object(obj)

    def _set_prop(self, obj_id: str, prop: str, values: List[str]) -> None:
        obj = object_mod.get_object(obj_id)
        if obj is None:
            return
        if prop == "pos" and len(values) >= 2:
            obj.transform["pos"] = [float(values[0]), float(values[1])]
        elif prop == "color" and len(values) == 3:
            obj.params["color"] = [float(v) for v in values]
        elif prop == "visible" and values:
            obj.visible = bool(_eval([values[0]], self.locals, self.context.variables))

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
        if cmd == "var":
            name = args[0]
            expr = args[1:]
            if expr and expr[0] == "=":
                expr = expr[1:]
            value = _eval(expr, self.locals, self.context.variables)
            self._set_var(name, value)
        elif cmd == "set" and args and args[0] in {"pos", "color", "visible"}:
            prop = args[0]
            obj_id = args[1].strip('"')
            self._set_prop(obj_id, prop, args[2:])
        elif cmd == "set":
            name = args[0]
            expr = args[1:]
            if expr and expr[0] == "=":
                expr = expr[1:]
            val = _eval(expr, self.locals, self.context.variables)
            self._set_var(name, val)
        elif cmd == "get":
            name = args[0]
            print(self._get_var(name))
        elif cmd in {"add", "sub", "mul", "div"}:
            name = args[0]
            value = _eval(args[1:], self.locals, self.context.variables)
            current = self._get_var(name) or 0
            if cmd == "add":
                current += value
            elif cmd == "sub":
                current -= value
            elif cmd == "mul":
                current *= value
            elif cmd == "div":
                current /= value
            self._set_var(name, current)
        elif cmd == "print":
            msg = " ".join(args)
            if msg.startswith('"') and msg.endswith('"'):
                msg = msg[1:-1]
            print(msg)
        elif cmd == "emit":
            if args:
                emit(args[0], None)
        elif cmd == "wait":
            if args:
                time.sleep(float(args[0]))
        elif cmd == "create":
            if len(args) >= 2:
                self._create_obj(args[0], args[1])
        elif cmd == "remove":
            if args:
                object_mod.remove_object(args[0])
        elif cmd == "if":
            cond = _eval(args, self.locals, self.context.variables)
            if cond:
                self._exec_block(node.children)
            elif node.else_children is not None:
                self._exec_block(node.else_children)
        elif cmd == "while":
            while _eval(args, self.locals, self.context.variables):
                self._exec_block(node.children)
        elif cmd == "loop":
            count = int(_eval(args, self.locals, self.context.variables)) if args else 0
            for _ in range(count):
                self._exec_block(node.children)
        else:
            pass


__all__ = ["parse_script", "FlowRunner", "FlowContext", "Node"]

