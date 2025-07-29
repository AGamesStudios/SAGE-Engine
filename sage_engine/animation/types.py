from dataclasses import dataclass
from typing import List, Optional


@dataclass
class FrameData:
    index: int
    duration: int
    event: Optional[str] = None


@dataclass
class AnimationData:
    frames: List[FrameData]
    loop: int | bool
