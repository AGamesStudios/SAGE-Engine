import json
from typing import List

from .game_object import GameObject
from .camera import Camera
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
        self.objects: List[GameObject] = []
        self.variables: dict = {}
        self.camera: Camera | None = None

    def add_object(self, obj: GameObject):
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
            scene.camera = Camera(
                cam.get("x", 0),
                cam.get("y", 0),
                cam.get("width", 640),
                cam.get("height", 480),
            )
        for entry in data.get("objects", []):
            scale_x = entry.get("scale_x")
            scale_y = entry.get("scale_y")
            scale = entry.get("scale", 1.0)
            if scale_x is None:
                scale_x = scale
            if scale_y is None:
                scale_y = scale
            obj = GameObject(
                entry.get("image", ""),
                entry.get("x", 0),
                entry.get("y", 0),
                entry.get("z", 0),
                entry.get("name"),
                scale_x,
                scale_y,
                entry.get("angle", 0.0),
                tuple(entry.get("color", [255, 255, 255, 255])) if entry.get("color") is not None else None,
            )
            if "quaternion" in entry:
                q = entry["quaternion"]
                if isinstance(q, list) and len(q) == 4:
                    obj.rotation = tuple(float(v) for v in q)
            obj.events = entry.get("events", [])
            obj.settings = entry.get("settings", {})
            scene.add_object(obj)
        return scene

    @classmethod
    def load(cls, path: str) -> "Scene":
        with open(path, "r") as f:
            data = json.load(f)
        return cls.from_dict(data)

    def to_dict(self) -> dict:
        """Return a dictionary representation of the scene."""
        data = {
            "variables": self.variables,
            "objects": [
                {
                    "image": o.image_path,
                    "x": o.x,
                    "y": o.y,
                    "z": getattr(o, "z", 0),
                    "name": o.name,
                    "scale_x": o.scale_x,
                    "scale_y": o.scale_y,
                    "scale": o.scale,
                    "angle": o.angle,
                    "quaternion": list(o.rotation),
                    "color": list(o.color) if o.color is not None else None,
                    "events": getattr(o, "events", []),
                    "settings": getattr(o, "settings", {}),
                }
                for o in self.objects
            ],
        }
        if self.camera:
            data["camera"] = {
                "x": self.camera.x,
                "y": self.camera.y,
                "width": self.camera.width,
                "height": self.camera.height,
            }
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
