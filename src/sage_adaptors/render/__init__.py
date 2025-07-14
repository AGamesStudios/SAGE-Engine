__version__ = "0.1.0"

from logging import getLogger

logger = getLogger(__name__)


def register():
    logger.info("Render adaptor registered")
