import json
from pathlib import Path


def build(index: dict[str, bytes]) -> bytes:
    offset = 0
    info = {}
    content = []
    for name, data in index.items():
        size = len(data)
        info[name] = {"offset": offset, "size": size}
        content.append(data)
        offset += size
    header = json.dumps(info).encode('utf8')
    out = len(header).to_bytes(4, 'little') + header + b''.join(content)
    return out


def parse(path: Path):
    buf = path.read_bytes()
    head_len = int.from_bytes(buf[:4], 'little')
    info = json.loads(buf[4:4+head_len].decode('utf8'))
    data = buf[4+head_len:]
    return info, data
