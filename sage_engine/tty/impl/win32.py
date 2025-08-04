from __future__ import annotations

try:
    import msvcrt
except ImportError:  # pragma: no cover - non-windows
    msvcrt = None  # type: ignore


def setup_terminal() -> None:
    pass


def restore_terminal() -> None:
    pass


def read_key() -> str | None:
    if msvcrt and msvcrt.kbhit():
        ch = msvcrt.getwch()
        return ch
    return None
