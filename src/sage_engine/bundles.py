"""Load build configuration bundles."""

from __future__ import annotations

import logging
from pathlib import Path

try:
    import tomllib as tomli  # Python 3.11+
except ModuleNotFoundError:  # pragma: no cover
    import tomli  # type: ignore

logger = logging.getLogger(__name__)

_BUNDLE_DIR = Path(__file__).resolve().parent.parent.parent / "tools" / "bundles"


def load_bundle(name: str) -> dict:
    """Return bundle configuration from ``tools/bundles``."""
    path = _BUNDLE_DIR / f"{name}.toml"
    if not path.exists():
        raise FileNotFoundError(f"unknown bundle: {name}")
    with path.open("rb") as f:
        return tomli.load(f)
