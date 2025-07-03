
import logging
import os
from importlib import metadata

from .sageaudio import load_sageaudio, save_sageaudio
from .sagemesh import load_sagemesh, save_sagemesh
from .sageanimation import load_sageanimation, save_sageanimation
from .sagemap import load_sagemap, save_sagemap
from .sagelogic import load_sagelogic, save_sagelogic

logger = logging.getLogger(__name__)

FORMAT_LOADERS: dict[str, callable] = {}
FORMAT_SAVERS: dict[str, callable] = {}
_PLUGINS_LOADED = False


def register_format(ext: str, *, loader=None, saver=None) -> None:
    """Register a loader or saver for ``ext`` (without the dot)."""
    if loader:
        FORMAT_LOADERS[ext] = loader
    if saver:
        FORMAT_SAVERS[ext] = saver


def _load_plugins() -> None:
    global _PLUGINS_LOADED
    if _PLUGINS_LOADED:
        return
    failed: list[str] = []
    try:
        eps = metadata.entry_points()
        entries = (
            eps.select(group="sage_engine.formats")
            if hasattr(eps, "select")
            else eps.get("sage_engine.formats", [])
        )
        for ep in entries:
            try:
                mod = ep.load()
                if hasattr(mod, "register"):
                    mod.register(register_format)
            except Exception as exc:  # pragma: no cover - plugin may raise anything
                logger.exception("Failed to load format plugin %s: %s", ep.name, exc)
                failed.append(ep.name)
    except Exception as exc:  # pragma: no cover - metadata issues
        logger.exception("Error loading format plugins: %s", exc)
    if failed:
        logger.warning("Failed format plugins: %s", ", ".join(failed))
    _PLUGINS_LOADED = True


def load_resource(path: str):
    """Load a resource by file extension using registered plugins."""
    _load_plugins()
    ext = os.path.splitext(path)[1].lstrip(".").lower()
    loader = FORMAT_LOADERS.get(ext)
    if loader is None:
        raise ValueError(f"No loader for extension: {ext}")
    return loader(path)


def save_resource(data, path: str) -> None:
    """Save ``data`` using a registered saver based on the file extension."""
    _load_plugins()
    ext = os.path.splitext(path)[1].lstrip(".").lower()
    saver = FORMAT_SAVERS.get(ext)
    if saver is None:
        raise ValueError(f"No saver for extension: {ext}")
    saver(data, path)


# register built-in formats
register_format('sageaudio', loader=load_sageaudio, saver=save_sageaudio)
register_format('sagemesh', loader=load_sagemesh, saver=save_sagemesh)
register_format('sageanimation', loader=load_sageanimation, saver=save_sageanimation)
register_format('sagemap', loader=load_sagemap, saver=save_sagemap)
register_format('sagelogic', loader=load_sagelogic, saver=save_sagelogic)

__all__ = [
    'load_sageaudio', 'save_sageaudio',
    'load_sagemesh', 'save_sagemesh',
    'load_sageanimation', 'save_sageanimation',
    'load_sagemap', 'save_sagemap',
    'load_sagelogic', 'save_sagelogic',
    'register_format',
    'load_resource',
    'save_resource',
]
