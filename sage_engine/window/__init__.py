"""Window stub for SAGE Engine."""

from __future__ import annotations

from sage_engine.core import register


def boot(_cfg: dict | None = None) -> None:
    pass


def flush() -> None:
    pass


def shutdown() -> None:
    pass


register("boot", boot)
register("flush", flush)
register("shutdown", shutdown)
