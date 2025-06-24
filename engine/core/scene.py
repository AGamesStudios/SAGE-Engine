import json
from typing import List

from .game_object import GameObject
from .camera import Camera
from .objects import object_from_dict, object_to_dict
from ..logic import (
    EventSystem, Event,
    condition_from_dict, action_from_dict,
    KeyPressed, KeyReleased, MouseButton, Collision, AfterTime,
    OnStart, EveryFrame, VariableCompare,
    Move, SetPosition, Destroy, Print, PlaySound, Spawn,
    SetVariable, ModifyVariable,
)


class Scene:
    """Collection of objects and events."""
    def __init__(self):
        self.objects: List[GameObject | Camera] = []
        self.variables: dict = {}
        self.camera: Camera | None = None

    def add_object(self, obj: GameObject | Camera):
        existing = {o.name for o in self.objects}
        base = obj.name
        if base in existing:
            i = 1
            new_name = f"{base} ({i})"
            while new_name in existing:
                i += 1
                new_name = f"{base} ({i})"
            obj.name = new_name
        self.objects.append(obj)
        if isinstance(obj, Camera):
            self.camera = obj

    def remove_object(self, obj: GameObject):
        if obj in self.objects:
            self.objects.remove(obj)

    def update(self, dt: float):
        for obj in self.objects:
            obj.update(dt)

    def draw(self, surface):
        for obj in sorted(self.objects, key=lambda o: getattr(o, 'z', 0)):
            obj.draw(surface)

    @classmethod
    def from_dict(cls, data: dict) -> "Scene":
        """Construct a Scene from a plain dictionary."""
        scene = cls()
        scene.variables = data.get("variables", {})
        cam = data.get("camera")
        if isinstance(cam, dict):
            # legacy single camera field
            obj = object_from_dict({"type": "camera", **cam})
            if obj is not None:
                scene.add_object(obj)
                scene.camera = obj
        for entry in data.get("objects", []):
            obj = object_from_dict(entry)
            if obj is None:
                continue
            obj.events = entry.get("events", [])
            obj.settings = entry.get("settings", {})
            scene.add_object(obj)
            if isinstance(obj, Camera):
                scene.camera = obj
        return scene

    @classmethod
    def load(cls, path: str) -> "Scene":
        with open(path, "r") as f:
            data = json.load(f)
        return cls.from_dict(data)

    def to_dict(self) -> dict:
        """Return a dictionary representation of the scene."""
        obj_list = []
        for o in self.objects:
            data = object_to_dict(o)
            if data is None:
                continue
            if hasattr(o, "events"):
                data["events"] = getattr(o, "events", [])
            if hasattr(o, "settings"):
                data["settings"] = getattr(o, "settings", {})
            if hasattr(o, "rotation"):
                data["quaternion"] = list(o.rotation)
            obj_list.append(data)
        data = {
            "variables": self.variables,
            "objects": obj_list,
        }
        if self.camera:
            data["camera"] = object_to_dict(self.camera)
        return data

    def save(self, path: str):
        with open(path, "w") as f:
            json.dump(self.to_dict(), f, indent=2)

    def build_event_system(self) -> EventSystem:
        es = EventSystem(variables=self.variables)
        for obj in self.objects:
            events = getattr(obj, "events", [])
            if not isinstance(events, list):
                continue
            for evt in events:
                if not isinstance(evt, dict):
                    continue
                conditions = []
                for cond in evt.get("conditions", []):
                    if not isinstance(cond, dict):
                        continue
                    cobj = condition_from_dict(cond, self.objects, self.variables)
                    if cobj is not None:
                        conditions.append(cobj)

                actions = []
                for act in evt.get("actions", []):
                    if not isinstance(act, dict):
                        continue
                    aobj = action_from_dict(act, self.objects)
                    if aobj is not None:
                        actions.append(aobj)
                es.add_event(Event(conditions, actions, evt.get("once", False)))
        return es
