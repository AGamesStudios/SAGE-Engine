from types import SimpleNamespace

from .query import cast_line, cast_circle
from .results import RaycastHit
from .. import core

core.expose(
    "raycast",
    SimpleNamespace(cast_line=cast_line, cast_circle=cast_circle, RaycastHit=RaycastHit),
)

__all__ = ["cast_line", "cast_circle", "RaycastHit"]
