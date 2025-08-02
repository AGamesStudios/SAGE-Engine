"""SAGE Object module."""

from .object import Object, Vector2
from .roles import register, get, registered
from .runtime import runtime, ObjectRuntime
from .store import ObjectStore
from .builder import ObjectBuilder, BlueprintSystem, Blueprint
from .roles import get as get_role
from . import groups
from dataclasses import replace
from .. import core

MAX_OBJECTS = 100_000

__all__ = [
    "Object",
    "Vector2",
    "ObjectRuntime",
    "ObjectStore",
    "ObjectBuilder",
    "BlueprintSystem",
    "Blueprint",
    "register",
    "get",
    "registered",
    "runtime",
    "spawn",
    "delete",
    "clone",
    "serialize",
    "groups",
]


def _roles_api():
    roles = core.get("roles")
    if roles is None:
        from .. import roles as roles  # fallback
    return roles


def spawn(role: str, *, name: str | None = None, world_id: str = "default", **fields: object) -> Object:
    """Create an :class:`Object` with the given *role* and add it to the store."""
    roles = _roles_api()
    role_key = role.lower()
    try:
        role_def = roles.get_role(role_key)
    except KeyError:
        if hasattr(roles, "load_json_roles"):
            roles.load_json_roles()
        role_def = roles.get_role(role_key)
    roles.validate(role_key, fields)
    if len(runtime.store.objects) >= MAX_OBJECTS:
        raise RuntimeError("object limit reached")
    obj = Object(id=f"{world_id}_{len(runtime.store.objects)}", name=name, world_id=world_id)
    role_cls = get_role(role)
    role_inst = role_cls(**fields)
    obj.add_role(role, role_inst)
    runtime.store.add_object(obj)
    try:
        groups.register_object(obj)
    except Exception:
        pass
    return obj


def delete(obj: Object | str) -> None:
    """Remove *obj* from the global store."""
    obj_id = obj if isinstance(obj, str) else obj.id
    o = runtime.store.get_object_by_id(obj_id)
    if o is not None:
        try:
            groups.unregister_object(o)
        except Exception:
            pass
    runtime.store.remove_object(obj_id)


def clone(obj: Object, *, new_id: str | None = None) -> Object:
    """Clone *obj* creating a new object with the same roles and data."""
    new_obj = replace(obj)
    new_obj.id = new_id or f"{obj.world_id}_{len(runtime.store.objects)}"
    new_obj.roles = {k: replace(v) for k, v in obj.roles.items()}
    runtime.store.add_object(new_obj)
    return new_obj


def serialize(obj: Object | str) -> dict:
    """Return a JSON-serializable representation of *obj*."""
    o = obj if isinstance(obj, Object) else runtime.store.get_object_by_id(obj)
    if o is None:
        raise ValueError("Object not found")
    return {
        "id": o.id,
        "name": o.name,
        "world_id": o.world_id,
        "position": [o.position.x, o.position.y],
        "rotation": o.rotation,
        "scale": o.scale,
        "roles": {n: vars(r) for n, r in o.roles.items()},
        "data": dict(o.data),
    }


core.expose(
    "objects",
    {
        "spawn": spawn,
        "delete": delete,
        "clone": clone,
        "serialize": serialize,
        "runtime": runtime,
        "groups": groups,
    },
)
