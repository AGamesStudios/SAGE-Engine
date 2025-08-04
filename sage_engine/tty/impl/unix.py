from __future__ import annotations

import sys
import select
import termios
import tty

_fd: int | None = None
_old_settings: list[int] | None = None


def setup_terminal() -> None:
    global _fd, _old_settings
    _fd = sys.stdin.fileno()
    _old_settings = termios.tcgetattr(_fd)
    tty.setcbreak(_fd)


def restore_terminal() -> None:
    if _fd is not None and _old_settings is not None:
        termios.tcsetattr(_fd, termios.TCSADRAIN, _old_settings)


def read_key() -> str | None:
    if _fd is None:
        return None
    dr, _, _ = select.select([sys.stdin], [], [], 0)
    if dr:
        return sys.stdin.read(1)
    return None
