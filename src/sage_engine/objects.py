from __future__ import annotations

from dataclasses import dataclass, field
from typing import List, Set, Dict, Any, Sequence, Tuple

try:  # pragma: no cover - optional
    from numpy.typing import NDArray
    import numpy as np  # type: ignore
except Exception:  # pragma: no cover - numpy optional
    NDArray = Any  # type: ignore
    np = None  # type: ignore

from .sprites import Sprite, DEFAULT_MATERIAL, _LAYER_SCALE


@dataclass
class Object:
    sprite: Sprite
    role: str = "generic"
    category: str = "default"
    tags: Set[str] = field(default_factory=set)
    visible: bool = True
    layer: int = 0

    def to_dict(self) -> dict:
        return {
            "role": self.role,
            "category": self.category,
            "tags": list(self.tags),
            "visible": self.visible,
            "layer": self.layer,
            "sprite": {
                "x": self.sprite.x,
                "y": self.sprite.y,
                "sx": self.sprite.sx,
                "sy": self.sprite.sy,
                "rot": self.sprite.rot,
                "atlas": self.sprite.atlas,
                "uv": self.sprite.uv,
                "blend": self.sprite.blend,
                "color": self.sprite.color,
                "z": self.sprite.z,
            },
        }


_ROLES: Dict[str, Set[str]] = {}
_objects: List[Object] = []


def register_role(role: str, categories: Sequence[str]) -> None:
    """Register *role* with allowed *categories*."""
    _ROLES[role] = set(categories)


def list_roles() -> List[str]:
    return sorted(_ROLES)


def categories_for(role: str) -> List[str]:
    return sorted(_ROLES.get(role, []))


def add(obj: Object) -> None:
    _objects.append(obj)


def clear() -> None:
    _objects.clear()


def collect_groups() -> List[Tuple[str, str, Any, NDArray | List[List[float]]]]:
    """Return visible objects grouped by role and category."""
    ordered = sorted([o for o in _objects if o.visible], key=lambda o: (o.layer, o.sprite.z))
    grouped: Dict[Tuple[str, str, Any], List[List[float]]] = {}
    for obj in ordered:
        s = obj.sprite
        depth = obj.layer * _LAYER_SCALE + s.z
        blend = 0.0 if s.blend == "alpha" else 1.0
        row = [
            s.x,
            s.y,
            s.sx,
            s.sy,
            s.rot,
            s.atlas,
            *s.uv,
            blend,
            *s.color,
            depth,
        ]
        mat = s.material or DEFAULT_MATERIAL
        key = (obj.role, obj.category, mat)
        grouped.setdefault(key, []).append(row)
    result: List[Tuple[str, str, Any, NDArray | List[List[float]]]] = []
    for (role, category, mat), rows in grouped.items():
        if np is None:
            arr = rows
        else:
            arr = np.asarray(rows, dtype=np.float32)
        result.append((role, category, mat, arr))
    return result


__all__ = [
    "Object",
    "register_role",
    "list_roles",
    "categories_for",
    "add",
    "clear",
    "collect_groups",
]
