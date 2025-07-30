from __future__ import annotations

from types import CodeType
from typing import Any

__all__ = ["run"]


def run(code: CodeType, ctx: dict[str, Any]) -> None:
    exec(code, ctx)
