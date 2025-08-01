from __future__ import annotations

"""Runtime introspection API for editor integration."""

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List
import json

from .. import core, events, world
from ..objects import runtime as obj_runtime
from .. import roles


@dataclass
class EditorState:
    flags: Dict[str, Any] = field(default_factory=dict)


class IntrospectionAPI:
    def __init__(self) -> None:
        self.state = EditorState()
        self._overlay_registered = False

    # --- Object access -------------------------------------------------
    def list_objects(self) -> List[int]:
        return [i for i, r in enumerate(world.scene.roles) if r is not None]

    def get_parameters(self, obj_id: int) -> Dict[str, Any]:
        return world.scene.serialize_object(obj_id)

    def set_parameter(self, obj_id: int, category: str, param: str, value: Any) -> None:
        edit = world.scene.begin_edit()
        edit.set(category, obj_id, param, value)
        world.scene.apply(edit)
        world.scene.commit()
        events.emit("property_changed", obj_id, category, param, value)

    # --- Roles and blueprints -----------------------------------------
    def get_roles(self) -> List[str]:
        return list(roles.registered_roles().keys())

    def describe_role(self, role_name: str) -> Dict[str, Any]:
        path = Path(__file__).resolve().parents[2] / "roles" / f"{role_name.capitalize()}.role.json"
        data: Dict[str, Any] = {}
        if path.exists():
            with open(path, "r", encoding="utf8") as fh:
                data = json.load(fh)
        return data

    def get_blueprints(self) -> List[str]:
        return list(obj_runtime.runtime.blueprints._defs.keys())

    # --- Editor flags --------------------------------------------------
    def set_editor_flag(self, key: str, value: Any) -> None:
        self.state.flags[key] = value
        events.emit("editor_flag_changed", key, value)
        if key == "editor_mode" and value and not self._overlay_registered:
            core.register("draw", self.editor_overlay_draw)
            self._overlay_registered = True

    def editor_overlay_draw(self) -> None:  # pragma: no cover - placeholder
        pass
