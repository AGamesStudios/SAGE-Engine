from __future__ import annotations

from typing import Any, Dict

from sage_object import object_from_dict
from .object import add_object, get_object, remove_object
from sage import emit
from . import input as input_mod, time as time_mod


def create_object(obj_id: str, role: str, params: Dict[str, Any] | None = None):
    data = {"id": obj_id, "role": role}
    if params:
        data.update(params)
    obj = object_from_dict(data)
    add_object(obj)
    return obj


def set_param(obj_id: str, key: str, value: Any) -> None:
    obj = get_object(obj_id)
    if obj is None:
        return
    if key == "x":
        pos = obj.transform.setdefault("pos", [0, 0])
        pos[0] = float(value)
    elif key == "y":
        pos = obj.transform.setdefault("pos", [0, 0])
        pos[1] = float(value)
    elif key in obj.transform:
        obj.transform[key] = value
    elif hasattr(obj, key):
        setattr(obj, key, value)
    else:
        obj.params[key] = value


def get_param(obj_id: str, key: str) -> Any:
    obj = get_object(obj_id)
    if obj is None:
        return None
    if key == "x":
        return obj.transform.get("pos", [0, 0])[0]
    if key == "y":
        return obj.transform.get("pos", [0, 0])[1]
    if key in obj.transform:
        return obj.transform[key]
    if hasattr(obj, key):
        return getattr(obj, key)
    return obj.params.get(key)


def destroy_object(obj_id: str) -> None:
    remove_object(obj_id)


def emit_event(name: str, data: Any = None) -> int:
    return emit(name, data)


def log(message: str) -> None:
    print(message)


def on_ready(handler) -> None:
    from sage import on
    on("ready", handler)


def on_update(handler) -> None:
    from sage import on
    on("update", handler)


def is_key_down(key: str) -> bool:
    return input_mod.is_key_down(key)


def is_mouse_pressed(button: str) -> bool:
    return input_mod.is_mouse_pressed(button)


def get_mouse_pos() -> tuple[int, int]:
    return input_mod.get_mouse_pos()


def get_time() -> float:
    return time_mod.get_time()


def get_delta() -> float:
    return time_mod.get_delta()


def wait(ms: float) -> None:
    time_mod.wait(ms)

__all__ = [
    "create_object",
    "set_param",
    "get_param",
    "destroy_object",
    "emit_event",
    "log",
    "on_ready",
    "on_update",
    "is_key_down",
    "is_mouse_pressed",
    "get_mouse_pos",
    "get_time",
    "get_delta",
    "wait",
]
