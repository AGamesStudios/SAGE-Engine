"""Effect pipeline utilities."""

from __future__ import annotations

from typing import Iterable, Mapping

from .api import apply_pipeline as _api_pipeline


def apply_pipeline(pipeline: Iterable[tuple[str, Mapping | None]], buffer: bytearray, width: int, height: int) -> None:
    _api_pipeline([(n, p or {}) for n, p in pipeline], buffer, width, height)
