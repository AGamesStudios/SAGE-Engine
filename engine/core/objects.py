"""Object registry for SAGE Engine."""

from __future__ import annotations
from typing import Any, Callable, Type
from ..utils.log import logger

OBJECT_REGISTRY: dict[str, Type[Any]] = {}
OBJECT_META: dict[str, list[tuple]] = {}


def register_object(name: str, params: list[tuple[str, str | None]] | None = None) -> Callable[[Type[Any]], Type[Any]]:
    """Decorator to register an object class with optional metadata."""

    def decorator(cls: Type[Any]) -> Type[Any]:
        plist = params[:] if params else []
        # always support arbitrary metadata
        if not any(p[0] == "metadata" for p in plist):
            plist.append(("metadata", "metadata"))
        OBJECT_REGISTRY[name] = cls
        OBJECT_META[name] = plist
        return cls

    return decorator


def object_from_dict(data: dict) -> Any | None:
    """Instantiate a registered object from ``data``."""
    typ = data.get("type")
    cls = OBJECT_REGISTRY.get(typ)
    if cls is None:
        logger.warning('Unknown object type %s', typ)
        return None
    params = {}
    for attr, key in OBJECT_META.get(typ, []):
        k = key or attr
        if k in data:
            value = data[k]
            if attr == "color" and isinstance(value, list):
                value = tuple(value)
            params[attr] = value
    try:
        return cls(**params)
    except Exception:
        logger.exception('Failed to construct object %s', typ)
        return None


def object_to_dict(obj: Any) -> dict | None:
    """Return a dictionary representation based on registry metadata."""
    for name, cls in OBJECT_REGISTRY.items():
        if isinstance(obj, cls):
            params = OBJECT_META.get(name, [])
            data = {"type": name}
            for attr, key in params:
                val = getattr(obj, attr)
                if attr == "metadata" and not val:
                    continue
                data[key or attr] = val
            return data
    logger.warning('Attempted to serialize unregistered object %s', type(obj).__name__)
    return None


def get_object_type(obj: Any) -> str | None:
    """Return the registry name for ``obj`` or ``None``."""
    for name, cls in OBJECT_REGISTRY.items():
        if isinstance(obj, cls):
            return name
    return None


__all__ = ['register_object', 'object_from_dict', 'object_to_dict', 'get_object_type',
           'OBJECT_REGISTRY', 'OBJECT_META']
