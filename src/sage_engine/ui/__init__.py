from __future__ import annotations

from typing import Callable, List, Any

try:  # pragma: no cover - optional dependency
    import numpy as np  # type: ignore
    from numpy.typing import NDArray
except Exception:  # pragma: no cover - numpy optional
    np = None  # type: ignore
    NDArray = Any  # type: ignore

from . import theme


class Signal:
    def __init__(self) -> None:
        self._subscribers: List[Callable[..., None]] = []

    def connect(self, fn: Callable[..., None]) -> None:
        self._subscribers.append(fn)

    def emit(self, *args, **kwargs) -> None:
        for fn in list(self._subscribers):
            fn(*args, **kwargs)


_widgets: List["Widget"] = []


class Widget:
    """Base widget applying the global UI theme."""

    def __init__(self) -> None:
        self.x = 0.0
        self.y = 0.0
        self.width = 0.0
        self.height = 0.0
        self.apply_theme()
        theme.register(self)
        _widgets.append(self)

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


def collect_instances() -> NDArray | list[list[float]]:
    if np is None:
        return [[w.x, w.y, 0.0, 0.0] for w in _widgets]
    arr = np.zeros((len(_widgets), 4), dtype=np.float32)
    for i, w in enumerate(_widgets):
        arr[i] = (w.x, w.y, 0.0, 0.0)
    return arr


__all__ = [
    "Signal",
    "Widget",
    "Button",
    "Label",
    "Panel",
    "theme",
    "collect_instances",
]
