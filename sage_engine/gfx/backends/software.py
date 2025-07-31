"""Software rendering backend using a pixel buffer."""
from __future__ import annotations

from ..runtime import GraphicRuntime
from ...render import rustbridge
from ...logger import logger


class MockGraphicRuntime:
    def draw_rect(self, *args, **kwargs) -> None:
        logger.debug("Mock draw_rect: native renderer not available.")

    def draw_text(self, *args, **kwargs) -> None:
        logger.debug("Mock draw_text: native renderer not available.")

    def __getattr__(self, _name):  # pragma: no cover - simple stub
        return lambda *a, **kw: None


def get_backend() -> GraphicRuntime | MockGraphicRuntime:
    if getattr(rustbridge.lib, "handle", None):
        return GraphicRuntime()
    logger.warning("Using fallback mock renderer.")
    return MockGraphicRuntime()
