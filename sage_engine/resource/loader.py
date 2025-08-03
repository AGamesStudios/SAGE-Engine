import json
import zlib
from pathlib import Path
import yaml
from ..logger import logger
from .. import compat


_PACK_INDEX = None
_PACK_FILE = None


def _load_pack(path: Path):
    global _PACK_INDEX, _PACK_FILE
    with path.open('rb') as fh:
        header_len = int.from_bytes(fh.read(4), 'little')
        header = fh.read(header_len)
        meta = json.loads(header.decode('utf8'))
        if isinstance(meta, dict) and "index" in meta:
            _PACK_INDEX = meta["index"]
            compressed = meta.get("compressed", False)
        else:
            _PACK_INDEX = meta
            compressed = False
        data = fh.read()
        if compressed:
            data = zlib.decompress(data)
        _PACK_FILE = data


def load_from_pack(key: str, pack_path: Path) -> bytes:
    if _PACK_INDEX is None:
        _load_pack(pack_path)
    info = _PACK_INDEX.get(key)
    if not info:
        raise KeyError(key)
    offset = info['offset']
    size = info['size']
    return _PACK_FILE[offset:offset + size]


def load_file(path: Path) -> bytes:
    with path.open('rb') as fh:
        return fh.read()


def _parse_cfg_text(text: str) -> dict:
    """Parse configuration *text* into a dictionary."""
    try:
        loaded = yaml.safe_load(text)
        if isinstance(loaded, dict):
            return loaded
    except Exception:
        pass
    data: dict[str, object] = {}
    for line in text.splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if "=" not in line:
            continue
        key, val = line.split("=", 1)
        key = key.strip()
        val = val.strip()
        try:
            data[key] = yaml.safe_load(val)
        except Exception:
            data[key] = val.strip('"\'')
    return compat.migrate(data)


def load_cfg(path: str | Path) -> dict:
    """Load a `.sagecfg` configuration file.

    Supports both YAML syntax and simple ``key = value`` lines.
    """
    p = Path(path)
    text = p.read_text(encoding="utf8")
    return _parse_cfg_text(text)


_ENGINE_ALLOWED = {
    "name",
    "script",
    "window_title",
    "screen_width",
    "screen_height",
    "fullscreen",
    "render_backend",
    "language",
    "boot_modules",
    "features",
    "settings",
}


def load_engine_cfg(path: str | Path) -> dict:
    """Load and validate ``engine.sagecfg``.

    The first non-empty line must be ``[SAGECFG]``. Unknown keys are
    discarded with a warning.
    """
    p = Path(path)
    text = p.read_text(encoding="utf8")
    lines = [ln.strip() for ln in text.splitlines() if ln.strip()]
    if not lines or lines[0] != "[SAGECFG]":
        raise ValueError("invalid header")
    cfg = _parse_cfg_text("\n".join(lines[1:]))
    for key in list(cfg):
        if key not in _ENGINE_ALLOWED:
            logger.warn(
                "[config] Unknown key in %s: %s | Tip: See docs/engine_config.md or run 'sage check-config' or 'sage init-project'",
                path,
                key,
            )
            cfg.pop(key)
    return cfg
