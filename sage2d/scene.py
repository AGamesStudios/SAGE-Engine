import json
from typing import List

from .game_object import GameObject
from sage_logic import (
    EventSystem, Event,
    KeyPressed, KeyReleased, MouseButton, Collision, Timer,
    Always, OnStart, EveryFrame, VariableEquals, VariableCompare,
    Move, SetPosition, Destroy, Print, PlaySound, Spawn,
    SetVariable, ModifyVariable,
)


class Scene:
    """Collection of objects and events."""
    def __init__(self):
        self.objects: List[GameObject] = []
        self.variables: dict = {}

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
        for obj in self.objects:
            obj.draw(surface)

    @classmethod
    def load(cls, path: str) -> "Scene":
        with open(path, "r") as f:
            data = json.load(f)
        scene = cls()
        scene.variables = data.get("variables", {})
        for entry in data.get("objects", []):
            obj = GameObject(
                entry["image"],
                entry.get("x", 0),
                entry.get("y", 0),
                entry.get("name"),
            )
            obj.events = entry.get("events", [])
            scene.add_object(obj)
        return scene

    def save(self, path: str):
        data = {
            "variables": self.variables,
            "objects": [
                {
                    "image": o.image_path,
                    "x": o.x,
                    "y": o.y,
                    "name": o.name,
                    "events": getattr(o, "events", []),
                }
                for o in self.objects
            ],
        }
        with open(path, "w") as f:
            json.dump(data, f, indent=2)

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
                    typ = cond.get("type")
                    if typ == "KeyPressed":
                        conditions.append(KeyPressed(cond["key"]))
                    elif typ == "KeyReleased":
                        conditions.append(KeyReleased(cond["key"]))
                    elif typ == "MouseButton":
                        conditions.append(
                            MouseButton(cond["button"], cond.get("state", "down"))
                        )
                    elif typ == "Timer":
                        conditions.append(Timer(cond["duration"]))
                    elif typ == "OnStart":
                        conditions.append(OnStart())
                    elif typ == "EveryFrame":
                        conditions.append(EveryFrame())
                    elif typ == "Collision":
                        a_idx = cond.get("a", -1)
                        b_idx = cond.get("b", -1)
                        if 0 <= a_idx < len(self.objects) and 0 <= b_idx < len(self.objects):
                            a = self.objects[a_idx]
                            b = self.objects[b_idx]
                            conditions.append(Collision(a, b))
                    elif typ == "Always":
                        conditions.append(Always())
                    elif typ == "VariableEquals":
                        conditions.append(VariableEquals(cond["name"], cond["value"]))
                    elif typ == "VariableCompare":
                        conditions.append(
                            VariableCompare(
                                cond["name"], cond.get("op", "=="), cond.get("value")
                            )
                        )
                actions = []
                for act in evt.get("actions", []):
                    if not isinstance(act, dict):
                        continue
                    typ = act.get("type")
                    if typ == "Move":
                        t = act.get("target")
                        if t is not None and 0 <= t < len(self.objects):
                            target = self.objects[t]
                            actions.append(Move(target, act["dx"], act["dy"]))
                    elif typ == "SetPosition":
                        t = act.get("target")
                        if t is not None and 0 <= t < len(self.objects):
                            target = self.objects[t]
                            actions.append(SetPosition(target, act["x"], act["y"]))
                    elif typ == "Destroy":
                        t = act.get("target")
                        if t is not None and 0 <= t < len(self.objects):
                            target = self.objects[t]
                            actions.append(Destroy(target))
                    elif typ == "Print":
                        actions.append(Print(act["text"]))
                    elif typ == "PlaySound":
                        path = act.get("path")
                        if path:
                            actions.append(PlaySound(path))
                    elif typ == "Spawn":
                        actions.append(Spawn(act["image"], act.get("x", 0), act.get("y", 0)))
                    elif typ == "SetVariable":
                        actions.append(SetVariable(act["name"], act["value"]))
                    elif typ == "ModifyVariable":
                        actions.append(
                            ModifyVariable(act["name"], act.get("op", "+"), act.get("value", 0))
                        )
                es.add_event(Event(conditions, actions, evt.get("once", False)))
        return es
