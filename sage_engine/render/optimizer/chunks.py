from __future__ import annotations

"""Simple chunk-based visibility management."""

from typing import Dict, List, Tuple, Any


class ChunkGrid:
    """Divide space into square chunks and store objects by chunk."""

    def __init__(self, chunk_size: int = 128) -> None:
        self.chunk_size = chunk_size
        self.chunks: Dict[Tuple[int, int], List[Tuple[Any, Tuple[int,int,int,int]]]] = {}

    def _chunk_coord(self, x: int, y: int) -> Tuple[int, int]:
        return x // self.chunk_size, y // self.chunk_size

    def add(self, obj: Any, bbox: Tuple[int, int, int, int]) -> None:
        cx, cy = self._chunk_coord(bbox[0], bbox[1])
        self.chunks.setdefault((cx, cy), []).append((obj, bbox))

    def active(self, viewport: Tuple[int, int, int, int]) -> List[Any]:
        """Return objects in chunks intersecting the viewport."""
        x0, y0, x1, y1 = viewport
        result: List[Any] = []
        for (cx, cy), objs in self.chunks.items():
            c_left = cx * self.chunk_size
            c_top = cy * self.chunk_size
            c_right = c_left + self.chunk_size
            c_bottom = c_top + self.chunk_size
            if not (c_right < x0 or c_left > x1 or c_bottom < y0 or c_top > y1):
                result.extend(obj for obj, _ in objs)
        return result
