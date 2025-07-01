import json
from ..utils import load_json
from typing import List

from .game_object import GameObject
from .camera import Camera
from .objects import object_from_dict, object_to_dict
from ..log import logger
from ..logic import (
    EventSystem, Event,
    condition_from_dict, action_from_dict, event_from_dict,
)


class Scene:
    """Collection of objects and events."""

    def __init__(self, with_defaults: bool = True):
        """Create a new scene.

        Parameters
        ----------
        with_defaults:
            When ``True`` (the default) a camera and a blank sprite are added
            automatically so new scenes start with something visible.  Loading
            a scene from disk sets this to ``False`` to avoid duplicating these
            objects every time the scene is opened.
        """
        # collections of scene objects and metadata
        self.objects = []
        self.object_lookup = {}
        self.variables = {}
        if with_defaults:
            self.events = [
                {
                    "conditions": [{"type": "OnStart"}],
                    "actions": [{"type": "Print", "text": "Hello, SAGE!"}],
                }
            ]
        else:
            self.events = []
        self.event_system = None
        self.camera = None
        self.active_camera = None
        self.metadata = {}

        if with_defaults:
            # default camera and sprite for new scenes
            try:
                cam = Camera(0.0, 0.0, 640, 480, active=True)
                self.add_object(cam)
                obj = GameObject(
                    image_path='',
                    shape='square',
                    x=0,
                    y=0,
                    z=0,
                    name='Sprite',
                    scale_x=1.0,
                    scale_y=1.0,
                    angle=0.0,
                    pivot_x=0.5,
                    pivot_y=0.5,
                    color=(255, 255, 255, 255),
                )
                self.add_object(obj)
            except Exception:
                logger.exception('Failed to create default objects')

    def sort_objects(self) -> None:
        """Sort objects by their z order."""
        self.objects.sort(key=lambda o: getattr(o, 'z', 0))

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
        key = getattr(obj, 'id', id(obj))
        self.object_lookup[key] = obj
        logger.debug('Added object %s', obj.name)
        if isinstance(obj, Camera):
            if obj.active:
                self.set_active_camera(obj)
            elif self.camera is None:
                self.set_active_camera(obj)
        self.sort_objects()

    def remove_object(self, obj: GameObject):
        if obj in self.objects:
            self.objects.remove(obj)
            self.object_lookup.pop(getattr(obj, 'id', id(obj)), None)
            logger.debug('Removed object %s', obj.name)
            if obj is self.camera:
                self.camera = None
                self.active_camera = None
            if isinstance(obj, Camera):
                obj.active = False
            self.sort_objects()

    def find_object(self, key) -> GameObject | Camera | None:
        """Return an object by name or id."""
        if isinstance(key, int):
            return self.object_lookup.get(key)
        for obj in self.objects:
            if getattr(obj, 'name', None) == key:
                return obj
        return None

    def remove_object_by_name(self, name: str) -> None:
        obj = self.find_object(name)
        if obj is not None:
            self.remove_object(obj)

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
            logger.debug('Active camera cleared')
        else:
            self.camera = camera
            self.active_camera = camera.name
            for obj in self.objects:
                if isinstance(obj, Camera):
                    obj.active = obj is camera
            logger.debug('Active camera set to %s', camera.name)

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
        self.sort_objects()
        for obj in self.objects:
            obj.update(dt)

    def draw(self, surface):
        self.sort_objects()
        for obj in self.objects:
            obj.draw(surface)

    @classmethod
    def from_dict(cls, data: dict) -> "Scene":
        """Construct a Scene from a plain dictionary."""
        scene = cls(with_defaults=False)
        scene.variables = data.get("variables", {})
        scene.metadata = data.get("metadata", {})
        scene.events = data.get("events", [])
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
            if hasattr(obj, 'variables'):
                obj.variables = entry.get('variables', {})
                obj.public_vars = set(entry.get('public_vars', []))
            elif 'variables' in entry or 'public_vars' in entry:
                logger.debug('Object %s does not support variables', type(obj).__name__)
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
        data = load_json(path)
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
            if hasattr(o, "variables") and getattr(o, "variables", {}):
                data["variables"] = getattr(o, "variables")
            if hasattr(o, "public_vars"):
                if getattr(o, "public_vars", None):
                    data["public_vars"] = list(o.public_vars)
                else:
                    data.pop("public_vars", None)
            if hasattr(o, "rotation"):
                data["quaternion"] = list(o.rotation)
            obj_list.append(data)
        data = {
            "variables": self.variables,
            "objects": obj_list,
        }
        if getattr(self, "events", None):
            data["events"] = self.events
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
        """Create event systems for the scene and its objects."""
        vars_dict = dict(self.variables)
        for obj in self.objects:
            if hasattr(obj, 'variables'):
                for k, v in getattr(obj, 'variables', {}).items():
                    if k not in vars_dict:
                        vars_dict[k] = v
        es = EventSystem(variables=vars_dict)
        for evt in getattr(self, "events", []):
            if isinstance(evt, dict):
                obj = event_from_dict(evt, self.objects, vars_dict)
                if obj is not None:
                    es.add_event(obj)
        self.event_system = es
        for obj in self.objects:
            if hasattr(obj, "build_event_system"):
                obj_es = obj.build_event_system(self.objects, obj.variables)
                if aggregate:
                    es.events.extend(obj_es.events)
        return es

    def update_events(self, engine, dt: float) -> None:
        """Update all object event systems."""
        for obj in self.objects:
            es = getattr(obj, "event_system", None)
            if es is not None:
                es.update(engine, self, dt)
