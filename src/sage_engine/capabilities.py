"""Capability flags for scenes and patch data."""

from importlib import resources
import logging
from .extras import lua
try:
    import tomllib as tomli  # Python 3.11+
except ModuleNotFoundError:  # pragma: no cover
    import tomli  # type: ignore

logger = logging.getLogger(__name__)

try:
    with resources.files("engine").joinpath("caps.toml").open("rb") as f:
        data = tomli.load(f)
    FEATURES = list(data.get("features", {}).keys())
    FEATURE_MAP = {name: i for i, name in enumerate(FEATURES)}
    SUPPORTED_CAPS = set(FEATURES)
except Exception as exc:  # pragma: no cover - file missing
    logger.error("Failed to load caps.toml: %s", exc)
    FEATURES = []
    FEATURE_MAP = {}
    SUPPORTED_CAPS = set()


def caps_from_flags(flags: int) -> list[str]:
    """Return capability names encoded in ``flags``."""
    return [name for name, idx in FEATURE_MAP.items() if flags & (1 << idx)]


def missing_caps(caps: list[str]) -> list[str]:
    missing = []
    for c in caps:
        if c == "vm_lua" and not lua.AVAILABLE:
            missing.append(c)
        elif c not in SUPPORTED_CAPS:
            missing.append(c)
    return missing


def missing_caps_from_flags(flags: int) -> list[str]:
    return missing_caps(caps_from_flags(flags))


def check_scene_caps(scene) -> list[str]:
    """Return unsupported capabilities required by ``scene``."""
    req = scene.metadata.get("caps", []) if hasattr(scene, "metadata") else []
    return missing_caps(list(req))

