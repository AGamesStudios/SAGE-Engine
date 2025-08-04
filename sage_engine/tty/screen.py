from __future__ import annotations

import sys

from .buffer import TTYBuffer
from . import color


def render(buffer: TTYBuffer) -> str:
    lines: list[str] = []
    for row in buffer.cells:
        line = ""
        for cell in row:
            line += f"{color.fg(cell.fg)}{color.bg(cell.bg)}{color.style(cell.bold)}{cell.char}{color.RESET}"
        lines.append(line)
    output = "\n".join(lines)
    sys.stdout.write("\033[H" + output)
    return output


def flush() -> None:
    sys.stdout.flush()


def clear_terminal() -> None:
    sys.stdout.write("\033[2J\033[H")
    sys.stdout.flush()
