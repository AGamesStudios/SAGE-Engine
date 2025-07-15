from .log import logger
import traceback

__all__ = [
    "warn",
    "error",
    "exception",
    "analyze_exception",
]


def warn(msg: str, *args, **kwargs) -> None:
    """Log a warning message."""
    logger.warning(msg, *args, **kwargs)


def error(msg: str, *args, **kwargs) -> None:
    """Log an error message."""
    logger.error(msg, *args, **kwargs)


def exception(msg: str, *args, exc_info=True, **kwargs) -> None:
    """Log an exception with traceback."""
    logger.exception(msg, *args, exc_info=exc_info, **kwargs)


def analyze_exception(exc_type, exc, tb) -> str:
    """Return a short summary of an exception."""
    stack = traceback.extract_tb(tb)
    if stack:
        origin = stack[-1]
        location = f"{origin.filename}:{origin.lineno}"
    else:
        location = "<unknown>"
    return f"{exc_type.__name__} at {location}: {exc}"

