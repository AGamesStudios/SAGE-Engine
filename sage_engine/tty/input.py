from __future__ import annotations

from collections import deque

from .impl import read_key

_actions: dict[str, str] = {}
_pressed: set[str] = set()
_queue: deque[str] = deque()


def map_action(name: str, key: str) -> None:
    _actions[name] = key


def update() -> None:
    _pressed.clear()
    key = read_key()
    if key:
        _queue.append(key)
    while _queue:
        k = _queue.popleft()
        for action, mapped in _actions.items():
            if k == mapped:
                _pressed.add(action)


def is_pressed(name: str) -> bool:
    return name in _pressed


# Helpers for tests -----------------------------------------------------

def _inject(key: str) -> None:  # pragma: no cover - used in tests
    _queue.append(key)
