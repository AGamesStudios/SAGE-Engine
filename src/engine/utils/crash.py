import os
import datetime
import traceback
from .log import LOG_DIR

__all__ = ["write_crash_report"]


def write_crash_report(exc_type, exc, tb, *, directory: str | None = None) -> str:
    """Write a detailed crash report and return its file path."""
    if directory is None:
        directory = os.path.join(LOG_DIR, "crashes")
    os.makedirs(directory, exist_ok=True)
    stamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    path = os.path.join(directory, f"crash_{stamp}.log")
    with open(path, "w", encoding="utf-8") as fh:
        fh.write(f"{exc_type.__name__}: {exc}\n")
        traceback.print_exception(exc_type, exc, tb, file=fh)
    return path
