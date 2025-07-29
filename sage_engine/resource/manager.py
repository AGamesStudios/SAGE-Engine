from pathlib import Path
from . import loader, cache

_PACK_PATH = None


def configure(pack_path: str | None):
    global _PACK_PATH
    _PACK_PATH = Path(pack_path) if pack_path else None


def load(key: str) -> bytes:
    data = cache.get(key)
    if data is not None:
        return data
    if _PACK_PATH and _PACK_PATH.exists():
        data = loader.load_from_pack(key, _PACK_PATH)
    else:
        data = loader.load_file(Path(key))
    cache.set(key, data)
    return data


def get(key: str) -> bytes | None:
    return cache.get(key)


def preload(paths: list[str]):
    for p in paths:
        path = Path(p)
        if path.is_dir():
            for sub in path.rglob('*'):
                if sub.is_file():
                    load(str(sub))
        elif path.is_file():
            load(str(path))
