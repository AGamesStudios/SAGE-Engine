# SPDX-License-Identifier: MIT
"""Registry and helpers for sprite effects."""

from typing import Dict, List, Tuple
from dataclasses import dataclass
import logging

logger = logging.getLogger(__name__)


@dataclass
class Effect:
    """Base effect applying optional transformations."""

    def apply_position(
        self, obj, camera, params: dict, pos: Tuple[float, float]
    ) -> Tuple[float, float]:
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

class OffsetEffect(Effect):
    """Translate an object by fixed x/y offsets."""

    def apply_position(
        self, obj, camera, params: dict, pos: Tuple[float, float]
    ) -> Tuple[float, float]:
        dx = params.get("dx", 0.0)
        dy = params.get("dy", 0.0)
        x, y = pos
        return x + dx, y + dy


register_effect("offset", OffsetEffect())


class OutlineEffect(Effect):
    """Placeholder for an outline post effect.

    This effect is registered so scenes can reference ``"outline"`` but it does
    not yet implement any drawing logic.
    """

    # actual outline rendering would depend on the active renderer
    # and is therefore left for future work


register_effect("outline", OutlineEffect())

__all__ = ["Effect", "register_effect", "get_effect", "EFFECT_REGISTRY"]
