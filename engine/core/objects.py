"""Object registry for SAGE Engine."""

from __future__ import annotations
from typing import Any, Callable, Type

OBJECT_REGISTRY: dict[str, Type[Any]] = {}
OBJECT_META: dict[str, list[tuple[str, str | None]]] = {}


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
        return None
    params = {}
    for attr, key in OBJECT_META.get(typ, []):
        k = key or attr
        if k in data:
            params[attr] = data[k]
    try:
        return cls(**params)
    except Exception:
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
    return None
