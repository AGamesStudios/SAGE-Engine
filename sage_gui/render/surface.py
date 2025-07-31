"""Surface abstraction for SAGE GUI."""

from dataclasses import dataclass


@dataclass
class Surface:
    width: int
    height: int
    buffer: bytearray | None = None
