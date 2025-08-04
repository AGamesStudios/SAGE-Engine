from __future__ import annotations

_queue: list[str] = []


def setup_terminal() -> None:
    pass


def restore_terminal() -> None:
    pass


def read_key() -> str | None:
    if _queue:
        return _queue.pop(0)
    return None


def inject(key: str) -> None:  # pragma: no cover - helper
    _queue.append(key)
