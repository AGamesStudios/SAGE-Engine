import json
from pathlib import Path
import yaml

_PACK_INDEX = None
_PACK_FILE = None


def _load_pack(path: Path):
    global _PACK_INDEX, _PACK_FILE
    with path.open('rb') as fh:
        header_len = int.from_bytes(fh.read(4), 'little')
        header = fh.read(header_len)
        _PACK_INDEX = json.loads(header.decode('utf8'))
        _PACK_FILE = fh.read()


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


def load_cfg(path: str | Path) -> dict:
    """Load a `.sagecfg` configuration file.

    Supports both YAML syntax and simple ``key = value`` lines.
    """
    p = Path(path)
    text = p.read_text(encoding="utf8")
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
    return data
