import json
from pathlib import Path

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
