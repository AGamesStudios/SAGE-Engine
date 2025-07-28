"""Effect pipeline utilities."""

from __future__ import annotations

from typing import Iterable, Mapping

from .api import apply


def apply_pipeline(pipeline: Iterable[tuple[str, Mapping | None]], buffer: bytearray, width: int, height: int) -> None:
    """Apply a sequence of effects to ``buffer``.

    Each pipeline item is ``(name, params_dict)``. Parameters are passed as
    keyword arguments to the registered effect.
    """
    for name, params in pipeline:
        if params is None:
            params = {}
        apply(name, buffer, width, height, **params)
