"""Object registry for SAGE Engine."""

from __future__ import annotations
from typing import Any, Callable, Type
from importlib import metadata
from ..utils.log import logger

OBJECT_REGISTRY: dict[str, Type[Any]] = {}
OBJECT_META: dict[str, list[tuple]] = {}
_PLUGINS_LOADED = False


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


def _load_entry_points() -> None:
    """Register object classes exposed via entry points."""
    global _PLUGINS_LOADED
    if _PLUGINS_LOADED:
        return
    failed: list[str] = []
    try:
        eps = metadata.entry_points()
        entries = (
            eps.select(group="sage_engine.objects")
            if hasattr(eps, "select")
            else eps.get("sage_engine.objects", [])
        )
        for ep in entries:
            try:
                cls = ep.load()
                register_object(ep.name)(cls)
            except Exception as exc:  # pragma: no cover - plugin may raise anything
                logger.exception("Failed to load object plugin %s: %s", ep.name, exc)
                failed.append(ep.name)
    except Exception as exc:  # pragma: no cover - metadata issues
        logger.exception("Error loading object entry points: %s", exc)
    if failed:
        logger.warning("Failed object plugins: %s", ", ".join(failed))
    _PLUGINS_LOADED = True


def load_object_plugins() -> None:
    """Public helper to load object plugins on demand."""
    _load_entry_points()


def object_from_dict(data: dict) -> Any | None:
    """Instantiate a registered object from ``data``."""
    _load_entry_points()
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
        raise


def object_to_dict(obj: Any) -> dict | None:
    """Return a dictionary representation based on registry metadata."""
    _load_entry_points()
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
    _load_entry_points()
    for name, cls in OBJECT_REGISTRY.items():
        if isinstance(obj, cls):
            return name
    return None


__all__ = ['register_object', 'object_from_dict', 'object_to_dict', 'get_object_type',
           'OBJECT_REGISTRY', 'OBJECT_META', 'load_object_plugins']
