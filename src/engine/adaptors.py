"""Load registered adaptors via entry points."""

from importlib import metadata
import logging

logger = logging.getLogger(__name__)

_LOADED = False


def load_adaptors() -> None:
    """Load and register available adaptors."""
    global _LOADED
    if _LOADED:
        return
    try:
        eps = metadata.entry_points()
        entries = (
            eps.select(group="sage_adaptor")
            if hasattr(eps, "select")
            else eps.get("sage_adaptor", [])
        )
        for ep in entries:
            try:
                func = ep.load()
                if hasattr(func, "register"):
                    func.register()
                else:
                    func()
            except Exception as exc:  # pragma: no cover - plugin may fail
                logger.warning("Failed to load adaptor %s: %s", ep.name, exc)
    except Exception as exc:  # pragma: no cover - metadata issues
        logger.error("Error loading adaptors: %s", exc)
    _LOADED = True
