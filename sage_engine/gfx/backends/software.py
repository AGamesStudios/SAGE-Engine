"""Software rendering backend using a pixel buffer."""
from __future__ import annotations

from ..runtime import GraphicRuntime


def get_backend() -> GraphicRuntime:
    return GraphicRuntime()
