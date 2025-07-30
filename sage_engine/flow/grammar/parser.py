from __future__ import annotations

import re
from .tokenizer import tokenize

__all__ = ["parse"]


def _translate_ru(line: str) -> str:
    patterns = [
        (r"^переменная\s+(\w+)\s*=\s*(.+)$", r"\1 = \2"),
        (r"^если\s+(.+)\s+тогда$", r"if \1:"),
        (r"^при\s+обновление\s+сделай$", "async def on_update():"),
        (r"^при\s+событие\s+\"([^\"]+)\"\s+сделай$", r"async def on_\1():"),
        (r"^вызвать\s+(\w+)\(\)$", r"\1()"),
        (r"найти\s+\"([^\"]+)\"", r'find("\1")'),
        (r"прибавить\s+(\w+)\s+на\s+([\w\d]+)", r"\1 += \2"),
        (r"уменьшить\s+(\w+)\s+на\s+([\w\d]+)", r"\1 -= \2"),
        (r"завершить игру\(\)", "end_game()"),
    ]
    for pat, repl in patterns:
        if re.search(pat, line):
            return re.sub(pat, repl, line)
    return line


def _translate_en(line: str) -> str:
    patterns = [
        (r"^variable\s+(\w+)\s*=\s*(.+)$", r"\1 = \2"),
        (r"^if\s+(.+)\s+then$", r"if \1:"),
        (r"^on\s+update\s+do$", "async def on_update():"),
        (r"^on\s+event\s+\"([^\"]+)\"\s+do$", r"async def on_\1():"),
        (r"^call\s+(\w+)\(\)$", r"\1()"),
        (r"find\s+\"([^\"]+)\"", r'find("\1")'),
        (r"add\s+(\w+)\s+by\s+([\w\d]+)", r"\1 += \2"),
        (r"sub\s+(\w+)\s+by\s+([\w\d]+)", r"\1 -= \2"),
    ]
    for pat, repl in patterns:
        if re.search(pat, line):
            return re.sub(pat, repl, line)
    return line


_LANG_MAP = {
    "ru": _translate_ru,
    "en": _translate_en,
}


def parse(script: str, *, lang: str = "ru") -> str:
    lines = tokenize(script)
    translate = _LANG_MAP.get(lang, _translate_ru)
    out_lines = []
    for line in lines:
        indent = len(line) - len(line.lstrip())
        stripped = line.lstrip()
        translated = translate(stripped)
        out_lines.append(" " * indent + translated)
    return "\n".join(out_lines) + "\n"
