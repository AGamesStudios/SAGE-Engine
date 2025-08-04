from __future__ import annotations

import logging

from sage_engine.core import register, expose

from .buffer import TTYBuffer
from . import draw, screen, input
from .impl import setup_terminal, restore_terminal

log = logging.getLogger("tty")


class TTYSystem:
    def __init__(self) -> None:
        self.buffer = TTYBuffer()

    def init(self, _cfg: dict | None = None) -> None:
        setup_terminal()
        screen.clear_terminal()
        expose("tty", self)
        log.info("TTY initialized")

    def update(self) -> None:
        input.update()

    def draw(self) -> None:
        screen.render(self.buffer)

    def flush(self) -> None:
        screen.flush()

    def shutdown(self) -> None:
        screen.clear_terminal()
        restore_terminal()
        log.info("TTY shutdown")

    # convenience API -------------------------------------------------
    def draw_text(self, x: int, y: int, text: str, fg: str = "white", bg: str = "black", bold: bool = False) -> None:
        draw.draw_text(self.buffer, x, y, text, fg, bg, bold)

    def draw_rect(self, x: int, y: int, w: int, h: int, char: str = "#", fg: str = "white", bg: str = "black") -> None:
        draw.draw_rect(self.buffer, x, y, w, h, char, fg, bg)

    def clear(self) -> None:
        draw.clear(self.buffer)


tty_system = TTYSystem()


register("boot", tty_system.init)
register("update", tty_system.update)
register("draw", tty_system.draw)
register("flush", tty_system.flush)
register("shutdown", tty_system.shutdown)

# re-export drawing helpers
__all__ = [
    "tty_system",
    "draw_text",
    "draw_rect",
    "clear",
    "input",
]


def draw_text(*args, **kwargs):
    tty_system.draw_text(*args, **kwargs)


def draw_rect(*args, **kwargs):
    tty_system.draw_rect(*args, **kwargs)


def clear() -> None:
    tty_system.clear()
