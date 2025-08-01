from dataclasses import dataclass, field
from typing import List, Optional, Tuple


@dataclass(slots=True)
class RaycastHit:
    object_id: str
    point: Tuple[float, float]
    distance: float
    normal: Tuple[float, float] | None = None
    tags: List[str] = field(default_factory=list)
    role: Optional[str] = None
