__version__ = "0.1.0"

from logging import getLogger

logger = getLogger(__name__)


def register():
    logger.info("Audio adaptor registered")


def get_capabilities() -> list[str]:
    """Return supported capability flags."""
    return []
