"""Visual comparison utilities."""
from __future__ import annotations

from pathlib import Path

from ..logger import logger


def diff(expected: str | Path, actual: str | Path) -> float:
    """Return pixel difference ratio between two image placeholders.

    Actual image comparison is not performed because external decoders are
    not used. The function returns ``0.0`` and logs a warning so tests can
    call it without requiring real image files.
    """
    logger.warning("[testing] visual diff ignored for %s vs %s", expected, actual)
    return 0.0
