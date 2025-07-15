from __future__ import annotations

from typing import Callable, List

from . import theme


class Signal:
    def __init__(self) -> None:
        self._subscribers: List[Callable[..., None]] = []

    def connect(self, fn: Callable[..., None]) -> None:
        self._subscribers.append(fn)

    def emit(self, *args, **kwargs) -> None:
        for fn in list(self._subscribers):
            fn(*args, **kwargs)


class Widget:
    """Base widget applying the global UI theme."""

    def __init__(self) -> None:
        self.apply_theme()
        theme.register(self)

    def apply_theme(self) -> None:  # pragma: no cover - trivial
        self.bg_color = theme.current.colors.get("bg", "#000000")
        self.fg_color = theme.current.colors.get("fg", "#ffffff")
        self.radius = theme.current.radius


class Button(Widget):
    def __init__(self) -> None:
        super().__init__()
        self.on_hover = Signal()
        self.on_click = Signal()

    def hover(self, inside: bool) -> None:
        self.on_hover.emit(inside)

    def click(self) -> None:
        self.on_click.emit()


class Label(Widget):
    def __init__(self, text: str = "") -> None:
        super().__init__()
        self.text = text


class Panel(Widget):
    pass


__all__ = ["Signal", "Widget", "Button", "Label", "Panel", "theme"]
