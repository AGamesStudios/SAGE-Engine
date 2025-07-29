from __future__ import annotations

"""Blueprint loader and object builder."""

from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, Mapping, Optional
import json

from ..format import SAGEDecompiler

from .object import Object, Vector2
from .roles import get as get_role
from .store import ObjectStore


@dataclass
class Blueprint:
    name: str
    roles: list[str] = field(default_factory=list)
    parameters: Dict[str, Dict[str, object]] = field(default_factory=dict)
    position: Vector2 = field(default_factory=Vector2)
    rotation: float = 0.0
    scale: float = 1.0
    extends: str | None = None
    override: bool = False


class BlueprintSystem:
    """Registry of blueprints that supports inheritance."""

    def __init__(self) -> None:
        self._defs: Dict[str, Mapping[str, object]] = {}

    def load(self, path: Path) -> None:
        if path.suffix == ".sagebp":
            data = SAGEDecompiler().decompile(path)
        else:
            data = json.loads(path.read_text(encoding="utf8"))
        self._defs[data["name"]] = data

    def register(self, data: Mapping[str, object]) -> None:
        self._defs[data["name"]] = dict(data)

    def get(self, name: str) -> Mapping[str, object]:
        return self._defs[name]

    def resolve(self, name: str) -> Blueprint:
        data = self._resolve_dict(name, set())
        return _dict_to_blueprint(data)

    def _resolve_dict(self, name: str, seen: set[str]) -> dict:
        if name in seen:
            raise ValueError("cyclic blueprint inheritance")
        seen.add(name)
        data = dict(self._defs[name])
        parent = data.get("extends")
        if parent:
            base = self._resolve_dict(parent, seen)
            if data.get("override"):
                base.update(data)
                return base
            merged = dict(base)
            merged["roles"] = list(dict.fromkeys(base.get("roles", []) + data.get("roles", [])))
            params = dict(base.get("parameters", {}))
            for r, p in data.get("parameters", {}).items():
                params.setdefault(r, {}).update(p)
            merged["parameters"] = params
            for k in ("position", "rotation", "scale"):
                if k in data:
                    merged[k] = data[k]
            merged.update({k: v for k, v in data.items() if k not in {"roles", "parameters", "position", "rotation", "scale", "extends", "override"}})
            return merged
        return data


def _dict_to_blueprint(data: Mapping[str, object]) -> Blueprint:
    pos = data.get("position", [0, 0])
    bp = Blueprint(
        name=data.get("name", ""),
        roles=list(data.get("roles", [])),
        parameters={k: dict(v) for k, v in data.get("parameters", {}).items()},
        position=Vector2(*pos) if isinstance(pos, (list, tuple)) else Vector2(),
        rotation=float(data.get("rotation", 0.0)),
        scale=float(data.get("scale", 1.0)),
        extends=data.get("extends"),
        override=bool(data.get("override", False)),
    )
    return bp


@dataclass
class ObjectBuilder:
    store: ObjectStore
    blueprints: BlueprintSystem

    def build(self, blueprint_name: str, obj_id: Optional[str] = None) -> Object:
        bp = self.blueprints.resolve(blueprint_name)
        obj = Object(
            id=obj_id or f"obj_{len(self.store.objects)}",
            name=bp.name,
            position=bp.position,
            rotation=bp.rotation,
            scale=bp.scale,
        )
        for role_name in bp.roles:
            role_cls = get_role(role_name)
            params = bp.parameters.get(role_name, {})
            role = role_cls(**params)
            obj.add_role(role_name, role)
        self.store.add_object(obj)
        return obj
