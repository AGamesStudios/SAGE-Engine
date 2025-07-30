from __future__ import annotations

from typing import Any, Callable

__all__ = ["get_builtins"]


def move(obj: Any, dx: int, dy: int) -> None:
    if hasattr(obj, "position"):
        obj.position.x += dx
        obj.position.y += dy


def log(msg: str) -> None:
    from ..logger import logger
    logger.info("[flow] %s", msg)


_BUILTINS = {
    "move": move,
    "log": log,
}


def get_builtins() -> dict[str, Callable[..., Any]]:
    return dict(_BUILTINS)
