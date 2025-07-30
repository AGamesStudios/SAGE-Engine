from __future__ import annotations

"""Parser for `.sagecfg` world files."""

from pathlib import Path
from typing import Iterable
import yaml

from .context import WorldConfig, WorldObject


def parse_world_file(path: str | Path) -> WorldConfig:
    data = yaml.safe_load(Path(path).read_text(encoding="utf8"))
    world_data = data.get("world", data)
    cfg = WorldConfig(name=str(world_data.get("name", "world")))

    for obj in world_data.get("objects", []):
        role = obj.get("type") or obj.get("role")
        if not role:
            continue
        x, y = 0.0, 0.0
        if "position" in obj:
            x, y = obj["position"]
        elif "позиция" in obj:
            x, y = obj["позиция"]
        fields = {k: v for k, v in obj.items() if k not in {"type", "role", "position", "позиция"}}
        cfg.objects.append(WorldObject(role=role, x=float(x), y=float(y), fields=fields))

    for sc in world_data.get("on_start", []) or world_data.get("при_запуске", []):
        cfg.scripts.append(str(sc))

    return cfg
