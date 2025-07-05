import os
from pathlib import Path

try:
    import tomllib  # Python 3.11+
except Exception:  # pragma: no cover - optional dependency
    import tomli as tomllib  # type: ignore

CONFIG_ENV = "SAGE_CONFIG"
DEFAULT_PATH = os.path.join("~", ".config", "sage", "sage.toml")

_config_cache: dict | None = None


def _load() -> dict:
    path = Path(os.path.expanduser(os.environ.get(CONFIG_ENV, DEFAULT_PATH)))
    if not path.is_file():
        return {}
    with path.open("rb") as f:
        return tomllib.load(f)


def get(key: str, default=None):
    """Return a configuration value from ``sage.toml`` using dotted keys."""
    global _config_cache
    if _config_cache is None:
        try:
            _config_cache = _load()
        except Exception:
            _config_cache = {}
    data = _config_cache
    for part in key.split('.'):
        if isinstance(data, dict) and part in data:
            data = data[part]
        else:
            return default
    return data
