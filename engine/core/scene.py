import json
from typing import List

from .game_object import GameObject
from .camera import Camera
from .objects import object_from_dict, object_to_dict
from ..log import logger
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
        # collections of scene objects and metadata
        self.objects = []
        self.variables = {}
        self.camera = None
        self.active_camera = None
        self.metadata = {}
        self._sorted = False

    def _sort_objects(self):
        if not self._sorted:
            self.objects.sort(key=lambda o: getattr(o, 'z', 0))
            self._sorted = True

    def sort_objects(self) -> None:
        """Public wrapper to sort objects by their z order."""
        self._sort_objects()

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
            if obj.active:
                self.set_active_camera(obj)
            elif self.camera is None:
                self.set_active_camera(obj)
        self._sorted = False

    def remove_object(self, obj: GameObject):
        if obj in self.objects:
            self.objects.remove(obj)
            if obj is self.camera:
                self.camera = None
                self.active_camera = None
            if isinstance(obj, Camera):
                obj.active = False
            self._sorted = False

    def set_active_camera(self, camera: Camera | str | None):
        """Select which camera is used for rendering."""
        if isinstance(camera, str):
            camera = next((o for o in self.objects
                           if isinstance(o, Camera) and o.name == camera), None)
        if camera is None or camera not in self.objects:
            self.camera = None
            self.active_camera = None
            for obj in self.objects:
                if isinstance(obj, Camera):
                    obj.active = False
        else:
            self.camera = camera
            self.active_camera = camera.name
            for obj in self.objects:
                if isinstance(obj, Camera):
                    obj.active = obj is camera

    def get_active_camera(self) -> Camera | None:
        """Return the currently active camera."""
        if self.camera is None and self.active_camera:
            self.set_active_camera(self.active_camera)
        return self.camera

    def ensure_active_camera(self, width: int = 640, height: int = 480) -> Camera:
        """Return an active camera, creating one if necessary."""
        cam = self.get_active_camera()
        if cam is not None:
            return cam
        cam = next((o for o in self.objects if isinstance(o, Camera)), None)
        if cam is None:
            cam = Camera(width / 2, height / 2, width, height, active=True)
            self.add_object(cam)
        else:
            self.set_active_camera(cam)
        return cam

    def update(self, dt: float):
        self._sort_objects()
        for obj in self.objects:
            obj.update(dt)

    def draw(self, surface):
        self._sort_objects()
        for obj in self.objects:
            obj.draw(surface)

    @classmethod
    def from_dict(cls, data: dict) -> "Scene":
        """Construct a Scene from a plain dictionary."""
        scene = cls()
        scene.variables = data.get("variables", {})
        scene.metadata = data.get("metadata", {})
        scene.active_camera = data.get("active_camera")
        for entry in data.get("objects", []):
            obj = object_from_dict(entry)
            if obj is None:
                continue
            if hasattr(obj, 'events'):
                obj.events = entry.get('events', [])
            elif 'events' in entry:
                logger.debug('Object %s does not support events', type(obj).__name__)
            if hasattr(obj, 'settings'):
                obj.settings = entry.get('settings', {})
            elif 'settings' in entry:
                logger.debug('Object %s does not support settings', type(obj).__name__)
            scene.add_object(obj)
            if isinstance(obj, Camera):
                if obj.active:
                    scene.set_active_camera(obj)
                elif scene.camera is None:
                    scene.set_active_camera(obj)
                elif scene.active_camera and obj.name == scene.active_camera:
                    scene.set_active_camera(obj)
        has_cam = scene.camera is not None
        cam = data.get("camera")
        if not has_cam and isinstance(cam, dict):
            # legacy single camera field
            obj = object_from_dict({"type": "camera", **cam})
            if obj is not None:
                scene.add_object(obj)
                scene.camera = obj
                scene.active_camera = obj.name
        if scene.camera is None and scene.active_camera:
            scene.set_active_camera(scene.active_camera)
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
        if self.metadata:
            data["metadata"] = self.metadata
        if self.camera:
            data["active_camera"] = self.camera.name
        elif self.active_camera:
            data["active_camera"] = self.active_camera
        if self.camera and self.camera not in self.objects:
            data["camera"] = object_to_dict(self.camera)
        return data

    def save(self, path: str):
        with open(path, "w") as f:
            json.dump(self.to_dict(), f, indent=2)

    def build_event_system(self, aggregate: bool = True) -> EventSystem:
        """Create event systems for objects and optionally aggregate them."""
        es = EventSystem(variables=self.variables)
        for obj in self.objects:
            if hasattr(obj, "build_event_system"):
                obj_es = obj.build_event_system(self.objects, self.variables)
                if aggregate:
                    es.events.extend(obj_es.events)
        return es

    def update_events(self, engine, dt: float) -> None:
        """Update all object event systems."""
        for obj in self.objects:
            es = getattr(obj, "event_system", None)
            if es is not None:
                es.update(engine, self, dt)
