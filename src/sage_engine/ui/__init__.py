from __future__ import annotations

from typing import Callable, List


class Signal:
    def __init__(self) -> None:
        self._subscribers: List[Callable[..., None]] = []

    def connect(self, fn: Callable[..., None]) -> None:
        self._subscribers.append(fn)

    def emit(self, *args, **kwargs) -> None:
        for fn in list(self._subscribers):
            fn(*args, **kwargs)


class Button:
    def __init__(self) -> None:
        self.on_hover = Signal()
        self.on_click = Signal()

    def hover(self, inside: bool) -> None:
        self.on_hover.emit(inside)

    def click(self) -> None:
        self.on_click.emit()


__all__ = ["Signal", "Button"]
