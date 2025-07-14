import mmap
import os
import struct
from dataclasses import dataclass

from .smart_slice import SmartSlice, SLICE_SIZE

__all__ = ["Node", "NanoTree"]
HEADER_STRUCT = "<II"
NODE_STRUCT = "<III"
HEADER_SIZE = struct.calcsize(HEADER_STRUCT)
NODE_SIZE = struct.calcsize(NODE_STRUCT)


@dataclass
class Node:
    """Metadata for a tree node."""

    crc: int
    parent: int
    slice_index: int




class NanoTree:
    """Minimal tree stored in a memory mapped file."""

    def __init__(self, path: str, max_nodes: int = 64, max_slices: int = 64) -> None:
        self._max_nodes = max_nodes
        self._max_slices = max_slices
        size = (
            HEADER_SIZE
            + NODE_SIZE * max_nodes
            + SLICE_SIZE * max_slices
        )
        exists = os.path.exists(path)
        self._file = open(path, "r+b" if exists else "w+b")
        if not exists:
            self._file.truncate(size)
            self._file.write(struct.pack(HEADER_STRUCT, 0, 0))
            self._file.flush()
        self._mmap = mmap.mmap(self._file.fileno(), size)
        self._load_header()

    def _load_header(self) -> None:
        self._mmap.seek(0)
        self._node_count, self._slice_count = struct.unpack(
            HEADER_STRUCT, self._mmap.read(HEADER_SIZE)
        )

    def _save_header(self) -> None:
        self._mmap.seek(0)
        self._mmap.write(struct.pack(HEADER_STRUCT, self._node_count, self._slice_count))

    def add_node(self, crc: int, parent: int = 0) -> int:
        if self._node_count >= self._max_nodes:
            raise ValueError("Node limit reached")
        index = self._node_count
        offset = HEADER_SIZE + index * NODE_SIZE
        self._mmap[offset : offset + NODE_SIZE] = struct.pack(
            NODE_STRUCT, crc, parent, 0
        )
        self._node_count += 1
        self._save_header()
        return index

    def get_node(self, index: int) -> Node:
        if index >= self._node_count:
            raise IndexError("Invalid node index")
        offset = HEADER_SIZE + index * NODE_SIZE
        crc, parent, slice_idx = struct.unpack(
            NODE_STRUCT, self._mmap[offset : offset + NODE_SIZE]
        )
        return Node(crc, parent, slice_idx)

    def set_node_slice(self, index: int, slice_index: int) -> None:
        if index >= self._node_count:
            raise IndexError("Invalid node index")
        if slice_index >= self._max_slices:
            raise IndexError("Invalid slice index")
        offset = HEADER_SIZE + index * NODE_SIZE + struct.calcsize("<II")
        self._mmap[offset : offset + 4] = struct.pack("<I", slice_index)

    def alloc_slice(self) -> int:
        if self._slice_count >= self._max_slices:
            raise ValueError("Slice limit reached")
        index = self._slice_count
        self._slice_count += 1
        self._save_header()
        return index

    def slice(self, index: int) -> SmartSlice:
        if index >= self._max_slices:
            raise IndexError("Invalid slice index")
        start = HEADER_SIZE + NODE_SIZE * self._max_nodes + index * SLICE_SIZE
        end = start + SLICE_SIZE
        return SmartSlice(memoryview(self._mmap)[start:end])

    def close(self) -> None:
        self._save_header()
        self._mmap.flush()
        self._mmap.close()
        self._file.close()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc, tb):
        self.close()
