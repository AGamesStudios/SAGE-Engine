from __future__ import annotations

from dataclasses import dataclass
from typing import Tuple, Any

from ..inputs import InputBackend
from ..renderers import Renderer


@dataclass(slots=True)
class EngineSettings:
    """Configuration container used when creating :class:`Engine`."""

    width: int = 640
    height: int = 480
    title: str = "SAGE 2D"
    fps: int = 30
    renderer: Renderer | str | None = None
    camera: "Camera | None" = None
    scene: "Scene | None" = None
    events: Any = None
    keep_aspect: bool = True
    background: Tuple[int, int, int] = (0, 0, 0)
    input_backend: str | type | InputBackend = "qt"
    max_delta: float = 0.1
