from .log import logger

__all__ = ['warn', 'error', 'exception']


def warn(msg: str, *args, **kwargs) -> None:
    """Log a warning message."""
    logger.warning(msg, *args, **kwargs)


def error(msg: str, *args, **kwargs) -> None:
    """Log an error message."""
    logger.error(msg, *args, **kwargs)


def exception(msg: str, *args, exc_info=True, **kwargs) -> None:
    """Log an exception with traceback."""
    logger.exception(msg, *args, exc_info=exc_info, **kwargs)
