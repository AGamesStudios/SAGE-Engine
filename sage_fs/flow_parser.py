from __future__ import annotations

from pathlib import Path
from typing import Any, List, Tuple

KEYWORDS = {
    "let": "let",
    "set": "set",
    "if": "if",
    "loop": "loop",
    "wait": "wait",
    "emit": "emit",
    "print": "print",
    "задать": "let",
    "установить": "set",
    "если": "if",
    "цикл": "loop",
    "подождать": "wait",
    "отправить": "emit",
    "послать": "emit",
    "сказать": "print",
}


def parse_script(text: str) -> List[Tuple[str, str]]:
    """Parse FlowScript text into a list of (command, argument)."""
    commands: List[Tuple[str, str]] = []
    for raw in text.splitlines():
        line = raw.strip()
        if not line or line.startswith('#'):
            continue
        parts = line.split(maxsplit=1)
        kw = KEYWORDS.get(parts[0], parts[0])
        arg = parts[1] if len(parts) > 1 else ""
        commands.append((kw, arg))
    return commands


class FlowRunner:
    """Execute parsed FlowScript commands."""

    def __init__(self) -> None:
        self.vars: dict[str, Any] = {}

    def run_file(self, path: str) -> None:
        text = Path(path).read_text(encoding="utf-8")
        cmds = parse_script(text)
        self.execute(cmds)

    def execute(self, cmds: List[Tuple[str, str]]) -> None:
        for cmd, arg in cmds:
            if cmd == "print":
                if arg.startswith('"') and arg.endswith('"'):
                    arg = arg[1:-1]
                print(arg)
            elif cmd == "emit":
                from sage import emit
                emit(arg, None)
            elif cmd in {"let", "set"}:
                if "=" in arg:
                    name, value = arg.split("=", 1)
                    self.vars[name.strip()] = eval(value.strip(), {}, self.vars)

__all__ = ["parse_script", "FlowRunner"]
