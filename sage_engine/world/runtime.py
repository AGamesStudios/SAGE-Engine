from __future__ import annotations

"""Minimal runtime for loading a world and spawning objects."""

from dataclasses import dataclass, field
from pathlib import Path
from typing import List

from ..objects import runtime as obj_runtime
from ..objects.object import Object
from ..flow.runtime import run_flow_script

from .loader import load_world
from .context import WorldConfig


@dataclass
class WorldRuntime:
    scene_id: str = "default"
    config: WorldConfig | None = None
    objects: List[Object] = field(default_factory=list)

    def load(self, path: str | Path) -> None:
        self.reset()
        self.config = load_world(path)
        for obj_cfg in self.config.objects:
            obj = Object(
                id=f"{self.scene_id}_{len(obj_runtime.store.objects)}",
                name=obj_cfg.role,
                world_id=self.scene_id,
            )
            obj.position.x = obj_cfg.x
            obj.position.y = obj_cfg.y
            for k, v in obj_cfg.fields.items():
                obj.data[k] = v
            obj_runtime.store.add_object(obj)
            self.objects.append(obj)
        for script in self.config.scripts:
            run_flow_script(script, {})

    def reset(self) -> None:
        for obj in self.objects:
            obj_runtime.store.remove_object(obj.id)
        self.objects.clear()
        self.config = None
