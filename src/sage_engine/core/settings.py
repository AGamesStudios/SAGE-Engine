
from __future__ import annotations

from dataclasses import dataclass, field
from typing import Tuple, Any, TYPE_CHECKING
import os

from ..inputs import InputBackend
from ..renderers import Renderer
from ..environment import Environment

if TYPE_CHECKING:  # pragma: no cover - hints for static analyzers
    from .camera import Camera
    from .scenes.scene import Scene


@dataclass(slots=True)
class EngineSettings:
    """Configuration container used when creating :class:`Engine`."""

    width: int = 640
    height: int = 480
    title: str = "SAGE 2D"
    fps: int = 30
    renderer: Renderer | str | None = None
    camera: Camera | None = None  # pyright: ignore[reportGeneralTypeIssues]
    scene: Scene | None = None
    events: Any = None
    keep_aspect: bool = True
    background: Tuple[int, int, int] = (0, 0, 0)
    input_backend: str | type | InputBackend = "sdl"
    max_delta: float = 0.1
    async_events: bool = False
    asyncio_events: bool = False
    event_workers: int = 4
    vsync: bool | None = None
    image_cache_limit: int = int(os.environ.get("SAGE_IMAGE_CACHE_LIMIT", "32"))
    max_angle: float = 360.0
    rotate_bbox: bool = False
    environment: Environment = field(default_factory=Environment)
