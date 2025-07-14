import mmap
from dataclasses import dataclass
from typing import Dict

__all__ = ["SLICE_SIZE", "SmartSlice", "SmartSliceAllocator"]

SLICE_SIZE = 1024


@dataclass
class SmartSlice:
    """A 1 KiB view into mapped memory."""

    view: memoryview

    def write(self, data: bytes) -> None:
        self.view[: len(data)] = data

    def read(self, size: int) -> bytes:
        return bytes(self.view[:size])


class SmartSliceAllocator:
    """Circular allocator for fixed-size :class:`SmartSlice` blocks."""

    def __init__(self, mmap_obj: mmap.mmap, offset: int, total_slices: int) -> None:
        self._mmap = mmap_obj
        self._offset = offset
        self._capacity = total_slices
        self._alloc_pos = 0
        self._free_pos = 0
        self._marks: Dict[int, int] = {}
        self._size = 0

    def slice(self, index: int) -> SmartSlice:
        if index >= self._capacity:
            raise IndexError("invalid slice index")
        start = self._offset + index * SLICE_SIZE
        end = start + SLICE_SIZE
        return SmartSlice(memoryview(self._mmap)[start:end])

    def available(self) -> int:
        return self._capacity - self._size

    def alloc_slice(self, type_id: int, count: int = 1) -> int:
        del type_id  # placeholder for future type metadata
        if count <= 0 or count > self._capacity:
            raise ValueError("invalid count")
        if count > self.available():
            raise MemoryError("slice pool exhausted")
        start = self._alloc_pos
        self._alloc_pos = (self._alloc_pos + count) % self._capacity
        self._size += count
        return start

    def mark(self, snapshot_id: int) -> None:
        self._marks[snapshot_id] = self._alloc_pos

    def free_mark(self, snapshot_id: int) -> None:
        pos = self._marks.pop(snapshot_id, None)
        if pos is None:
            return
        for sid in list(self._marks):
            if sid <= snapshot_id:
                self._marks.pop(sid, None)
        freed = (pos - self._free_pos) % self._capacity
        self._free_pos = pos
        self._size -= freed
