from __future__ import annotations

"""Minimal runtime for loading a world and spawning objects."""

from dataclasses import dataclass, field
from pathlib import Path
from typing import List

from ..objects import runtime as obj_runtime, spawn, delete
from ..objects.object import Object
from ..flow.runtime import run_flow_script

from .loader import load_world
from .context import WorldConfig


@dataclass
class WorldRuntime:
    scenes: dict[str, WorldConfig] = field(default_factory=dict)
    active_scene: str | None = None
    active_layer: str = "default"
    objects: List[Object] = field(default_factory=list)

    def load_scene(self, name: str, path: str | Path) -> None:
        """Load a scene configuration under *name*."""
        self.scenes[name] = load_world(path)

    def set_scene(self, name: str) -> None:
        """Activate a loaded scene."""
        self.reset()
        cfg = self.scenes[name]
        self.active_scene = name
        for obj_cfg in cfg.objects:
            try:
                obj = spawn(obj_cfg.role, world_id=name, **obj_cfg.fields)
            except KeyError:
                obj = Object(id=f"{name}_{len(obj_runtime.store.objects)}", name=obj_cfg.role, world_id=name)
                obj_runtime.store.add_object(obj)
                for k, v in obj_cfg.fields.items():
                    obj.data[k] = v
            obj.position.x = obj_cfg.x
            obj.position.y = obj_cfg.y
            self.objects.append(obj)
        for script in cfg.scripts:
            run_flow_script(script, {})

    def switch_layer(self, layer: str) -> None:
        """Set the active layer used for rendering or queries."""
        self.active_layer = layer

    def reset(self) -> None:
        for obj in self.objects:
            delete(obj)
        self.objects.clear()
        self.active_scene = None
