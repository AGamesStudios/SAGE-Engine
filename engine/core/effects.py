# SPDX-License-Identifier: MIT
"""Registry and helpers for sprite effects."""
from __future__ import annotations

from typing import Dict, List, Tuple
from dataclasses import dataclass
import logging

logger = logging.getLogger(__name__)


@dataclass
class Effect:
    """Base effect applying optional transformations."""

    def apply_position(self, obj, camera, params: dict, pos: Tuple[float, float]) -> Tuple[float, float]:
        return pos

    def apply_scale(self, obj, camera, params: dict, scale: float) -> float:
        return scale

    def apply_uvs(self, obj, camera, params: dict, uvs: List[float]) -> List[float]:
        return uvs


EFFECT_REGISTRY: Dict[str, Effect] = {}


def register_effect(name: str, effect: Effect) -> None:
    """Register ``effect`` under ``name``."""
    EFFECT_REGISTRY[name] = effect


def get_effect(name: str) -> Effect | None:
    """Return the effect registered as ``name``."""
    return EFFECT_REGISTRY.get(name)


# built-in effects -----------------------------------------------------------

class PerspectiveEffect(Effect):
    def apply_position(self, obj, camera, params: dict, pos: Tuple[float, float]) -> Tuple[float, float]:
        if camera is None:
            return pos
        fx = params.get("factor_x", params.get("factor", 0.0))
        fy = params.get("factor_y", params.get("factor", 0.0))
        x, y = pos
        return x + camera.x * fx, y + camera.y * fy

    def apply_scale(self, obj, camera, params: dict, scale: float) -> float:
        if camera is None:
            return scale
        depth = params.get("depth", params.get("factor_z", params.get("factor", 0.0)))
        return scale * (1.0 + (camera.zoom - 1.0) * depth)


class PanoramaEffect(PerspectiveEffect):
    """Panorama effect extends perspective with optional equirectangular UVs."""

    def apply_uvs(self, obj, camera, params: dict, uvs: List[float]) -> List[float]:
        if camera is None:
            return uvs
        if params.get("projection") != "equirect":
            return uvs
        fx = params.get("factor_x", params.get("factor", 1.0))
        fy = params.get("factor_y", params.get("factor", 1.0))
        half_w = obj.width / 2
        half_h = obj.height / 2
        verts = [(-half_w, -half_h), (half_w, -half_h), (half_w, half_h), (-half_w, half_h)]
        result: List[float] = []
        import math
        for vx, vy in verts:
            lon = (obj.x + vx - camera.x) * fx
            lat = (obj.y + vy - camera.y) * fy
            u = (lon / (2 * math.pi)) % 1.0
            v = 0.5 - (lat / math.pi)
            result.extend([u, v])
        return result


register_effect("perspective", PerspectiveEffect())
register_effect("panorama", PanoramaEffect())

__all__ = ["Effect", "register_effect", "get_effect", "EFFECT_REGISTRY"]
