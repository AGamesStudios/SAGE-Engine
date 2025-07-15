import mmap
import os
import lzma
import struct

from .smart_slice import SLICE_SIZE, SmartSlice

__all__ = ["ChronoPatchTree"]


PATCH_HEADER = "<QI"
PATCH_HEADER_SIZE = struct.calcsize(PATCH_HEADER)


class ChronoPatchTree:
    """Incremental state tree using mmap and LZMA patches."""

    def __init__(self, path: str, size: int = SLICE_SIZE * 64) -> None:
        size = (size + SLICE_SIZE - 1) // SLICE_SIZE * SLICE_SIZE
        exists = os.path.exists(path)
        self._file = open(path, "r+b" if exists else "w+b")
        if not exists:
            self._file.truncate(size)
        self._mmap = mmap.mmap(self._file.fileno(), size)
        self._log_path = f"{path}.log"
        self.version = 0

    def slice(self, index: int) -> SmartSlice:
        start = index * SLICE_SIZE
        end = start + SLICE_SIZE
        return SmartSlice(memoryview(self._mmap)[start:end])

    def snapshot(self, offset: int, data: bytes) -> None:
        self._mmap.seek(offset)
        self._mmap.write(data)
        body = struct.pack(PATCH_HEADER, offset, len(data)) + data
        compressed = lzma.compress(body)
        with open(self._log_path, "ab") as log:
            log.write(struct.pack("<I", len(compressed)))
            log.write(compressed)
        self.version += 1

    def replay(self) -> None:
        if not os.path.exists(self._log_path):
            return
        with open(self._log_path, "rb") as log:
            while True:
                header = log.read(4)
                if not header:
                    break
                length = struct.unpack("<I", header)[0]
                payload = lzma.decompress(log.read(length))
                offset, size = struct.unpack(
                    PATCH_HEADER, payload[:PATCH_HEADER_SIZE]
                )
                buf = payload[PATCH_HEADER_SIZE:PATCH_HEADER_SIZE + size]
                self._mmap.seek(offset)
                self._mmap.write(buf)
                self.version += 1

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc, tb):
        self.close()

    def iter_patches(self):
        if not os.path.exists(self._log_path):
            return
        with open(self._log_path, "rb") as log:
            while True:
                header = log.read(4)
                if not header:
                    break
                length = struct.unpack("<I", header)[0]
                payload = lzma.decompress(log.read(length))
                offset, size = struct.unpack(
                    PATCH_HEADER, payload[:PATCH_HEADER_SIZE]
                )
                data = payload[PATCH_HEADER_SIZE:PATCH_HEADER_SIZE + size]
                yield offset, data

    def close(self) -> None:
        self._mmap.flush()
        self._mmap.close()
        self._file.close()
