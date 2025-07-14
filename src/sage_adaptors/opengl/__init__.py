__version__ = "0.1.0"

from logging import getLogger

logger = getLogger(__name__)

_CAPS = ["render_opengl"]


def register() -> None:
    """Register the OpenGL render adaptor."""
    logger.info("OpenGL render adaptor registered")


def get_capabilities() -> list[str]:
    """Return supported capability flags."""
    return list(_CAPS)
