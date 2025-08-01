"""Software rendering backend using a pixel buffer."""
from __future__ import annotations

from ..runtime import GraphicRuntime
from ...logger import logger


class MockGraphicRuntime:
    def draw_rect(self, *args, **kwargs) -> None:
        logger.debug("Mock draw_rect: native renderer not available.")

    def draw_text(self, *args, **kwargs) -> None:
        logger.debug("Mock draw_text: native renderer not available.")

    def __getattr__(self, _name):  # pragma: no cover - simple stub
        return lambda *a, **kw: None


def get_backend() -> GraphicRuntime:
    """Return the default software renderer."""
    return GraphicRuntime()
