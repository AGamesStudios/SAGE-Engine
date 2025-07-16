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
from .. import sprites, resources


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
        self.width = 64.0
        self.height = 24.0
        self.layer = 0
        self.z = 0.0
        self.hovered = False
        self.active = False
        self.icon: resources.Texture | None = None
        self.apply_theme()
        theme.register(self)
        _widgets.append(self)

    def apply_theme(self) -> None:  # pragma: no cover - trivial
        self.bg_color = theme.current.colors.get("bg", "#000000")
        self.fg_color = theme.current.colors.get("fg", "#ffffff")
        self.hover_color = theme.current.colors.get("hover", self.bg_color)
        self.active_color = theme.current.colors.get("active", self.hover_color)
        self.radius = theme.current.radius
        if hasattr(self, "text_obj") and self.text_obj is not None:
            self.text_obj.color = theme.color_rgba(self.fg_color)

    def set_icon(self, path: str) -> None:
        """Load *path* as widget icon."""
        self.icon = resources.manager.get_texture(path)


class Button(Widget):
    def __init__(self, text: str = "", font: Font | None = None) -> None:
        super().__init__()
        self.on_hover = Signal()
        self.on_click = Signal()
        if font is None:
            font = theme.get_font()
        self.text_obj = None
        if font is not None:
            fg = theme.current.colors.get("fg", "#ffffff")
            self.text_obj = _text.TextObject(text, font=font, color=theme.color_rgba(fg))
            _text.add(self.text_obj)

    def hover(self, inside: bool) -> None:
        self.hovered = inside
        self.on_hover.emit(inside)

    def click(self) -> None:
        self.active = True
        self.on_click.emit()


class Label(Widget):
    def __init__(self, text: str = "", font: Font | None = None) -> None:
        super().__init__()
        self.text = text
        self.text_obj = None
        if font is None:
            font = theme.get_font()
        if font is not None:
            fg = theme.current.colors.get("fg", "#ffffff")
            self.text_obj = _text.TextObject(text, font=font, color=theme.color_rgba(fg))
            _text.add(self.text_obj)


class Panel(Widget):
    pass


def collect_instances() -> NDArray | list[list[float]]:
    ordered = sorted(_widgets, key=lambda w: (w.layer, w.z))
    if np is None:
        result = []
        for w in ordered:
            if hasattr(w, "text_obj") and w.text_obj is not None:
                w.text_obj.x = w.x
                w.text_obj.y = w.y
            depth = w.layer * sprites._LAYER_SCALE + w.z
            atlas = float(w.icon.atlas) if w.icon else 0.0
            if w.icon:
                u0, v0, u1, v1 = w.icon.uv
            else:
                u0, v0, u1, v1 = 0.0, 0.0, 1.0, 1.0
            color_hex = w.bg_color
            if w.active:
                color_hex = w.active_color
            elif w.hovered:
                color_hex = w.hover_color
            result.append([
                w.x,
                w.y,
                w.width,
                w.height,
                0.0,
                atlas,
                u0,
                v0,
                u1,
                v1,
                0.0,
                *theme.color_rgba(color_hex),
                depth,
            ])
        return result
    arr = np.zeros((len(ordered), 16), dtype=np.float32)
    for i, w in enumerate(ordered):
        if hasattr(w, "text_obj") and w.text_obj is not None:
            w.text_obj.x = w.x
            w.text_obj.y = w.y
        depth = w.layer * sprites._LAYER_SCALE + w.z
        atlas = float(w.icon.atlas) if w.icon else 0.0
        if w.icon:
            u0, v0, u1, v1 = w.icon.uv
        else:
            u0, v0, u1, v1 = 0.0, 0.0, 1.0, 1.0
        color_hex = w.bg_color
        if w.active:
            color_hex = w.active_color
        elif w.hovered:
            color_hex = w.hover_color
        arr[i] = (
            w.x,
            w.y,
            w.width,
            w.height,
            0.0,
            atlas,
            u0,
            v0,
            u1,
            v1,
            0.0,
            *theme.color_rgba(color_hex),
            depth,
        )
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
