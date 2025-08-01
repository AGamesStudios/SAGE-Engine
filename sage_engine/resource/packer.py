import json
import os
import zlib
from pathlib import Path


def pack(source: str, output: str, *, compress: bool = False, limit_size_mb: int = 10):
    source_path = Path(source)
    index = {}
    contents = []
    seen = {}
    offset = 0
    total = 0
    for root, _, files in os.walk(source_path):
        for name in files:
            fpath = Path(root) / name
            with open(fpath, "rb") as fh:
                data = fh.read()
            h = hash(data)
            key = str(fpath.relative_to(source_path)).replace(os.sep, "/")
            if h in seen:
                existing = seen[h]
                index[key] = {"offset": existing[0], "size": existing[1]}
                continue
            size = len(data)
            index[key] = {"offset": offset, "size": size}
            contents.append(data)
            seen[h] = (offset, size)
            offset += size
            total += size
    if total > limit_size_mb * 1024 * 1024:
        raise ValueError("resource pack too big")
    out_path = Path(output)
    header = json.dumps({"index": index, "compressed": compress}).encode("utf8")
    with out_path.open("wb") as out:
        out.write(len(header).to_bytes(4, "little"))
        out.write(header)
        for data in contents:
            if compress:
                data = zlib.compress(data)
            out.write(data)
    return index
