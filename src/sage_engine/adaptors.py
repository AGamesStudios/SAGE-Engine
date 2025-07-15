"""Load registered adaptors via entry points."""

from importlib import metadata
import logging

logger = logging.getLogger(__name__)

_LOADED: set[str] = set()


def load_adaptors(selected: list[str] | None = None) -> None:
    """Load and register available adaptors.

    If ``selected`` is provided, only adaptors with matching entry point names
    are loaded.
    """
    global _LOADED
    try:
        eps = metadata.entry_points()
        entries = (
            eps.select(group="sage_adaptor")
            if hasattr(eps, "select")
            else eps.get("sage_adaptor", [])
        )
        for ep in entries:
            if selected and ep.name not in selected:
                continue
            if ep.name in _LOADED:
                continue
            try:
                func = ep.load()
                if hasattr(func, "register"):
                    func.register()
                else:
                    func()
                _LOADED.add(ep.name)
            except Exception as exc:  # pragma: no cover - plugin may fail
                logger.warning("Failed to load adaptor %s: %s", ep.name, exc)
    except Exception as exc:  # pragma: no cover - metadata issues
        logger.error("Error loading adaptors: %s", exc)
