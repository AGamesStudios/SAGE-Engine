from __future__ import annotations

from typing import Callable, List, Any

try:  # pragma: no cover - optional dependency
    import numpy as np  # type: ignore
    from numpy.typing import NDArray
except Exception:  # pragma: no cover - numpy optional
    np = None  # type: ignore
    NDArray = Any  # type: ignore

from . import theme
from .. import text as _text
from ..render.font import Font


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
        if hasattr(self, "text_obj") and self.text_obj is not None:
            self.text_obj.color = theme.color_rgba(self.fg_color)


class Button(Widget):
    def __init__(self, text: str = "", font: Font | None = None) -> None:
        super().__init__()
        self.on_hover = Signal()
        self.on_click = Signal()
        self.text_obj = None
        if font is not None:
            fg = theme.current.colors.get("fg", "#ffffff")
            self.text_obj = _text.TextObject(text, font=font, color=theme.color_rgba(fg))
            _text.add(self.text_obj)

    def hover(self, inside: bool) -> None:
        self.on_hover.emit(inside)

    def click(self) -> None:
        self.on_click.emit()


class Label(Widget):
    def __init__(self, text: str = "", font: Font | None = None) -> None:
        super().__init__()
        self.text = text
        self.text_obj = None
        if font is not None:
            fg = theme.current.colors.get("fg", "#ffffff")
            self.text_obj = _text.TextObject(text, font=font, color=theme.color_rgba(fg))
            _text.add(self.text_obj)


class Panel(Widget):
    pass


def collect_instances() -> NDArray | list[list[float]]:
    if np is None:
        result = []
        for w in _widgets:
            if hasattr(w, "text_obj") and w.text_obj is not None:
                w.text_obj.x = w.x
                w.text_obj.y = w.y
            result.append([w.x, w.y, 0.0, 0.0, *theme.color_rgba(w.bg_color)])
        return result
    arr = np.zeros((len(_widgets), 8), dtype=np.float32)
    for i, w in enumerate(_widgets):
        if hasattr(w, "text_obj") and w.text_obj is not None:
            w.text_obj.x = w.x
            w.text_obj.y = w.y
        arr[i] = (w.x, w.y, 0.0, 0.0, *theme.color_rgba(w.bg_color))
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
