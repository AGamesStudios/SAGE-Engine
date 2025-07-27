from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict

@dataclass
class ProfileFrame:
    role_calls: Dict[str, int] = field(default_factory=dict)
    events_dropped: int = 0
    timers_dropped: int = 0

profile = ProfileFrame()
