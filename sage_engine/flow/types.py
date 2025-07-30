from __future__ import annotations

from dataclasses import dataclass
from typing import Any

__all__ = ["ObjectRef"]


dataclass = dataclass  # export

@dataclass
class ObjectRef:
    """Safe reference to an object by id."""

    id: str

    def resolve(self, lookup) -> Any | None:
        return lookup(self.id)
