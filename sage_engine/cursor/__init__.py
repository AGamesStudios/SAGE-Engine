from .core import CursorState
from .style import CursorStyle, load_style
from .runtime import CursorRenderer

_renderer = CursorRenderer()


def set_style(name: str) -> None:
    """Change cursor style at runtime."""
    _renderer.state.set_style(name)


__all__ = [
    "CursorState",
    "CursorStyle",
    "load_style",
    "CursorRenderer",
    "set_style",
]
