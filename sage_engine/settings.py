"""Engine settings used across modules."""

from dataclasses import dataclass, field
import os

@dataclass
class Settings:
    cpu_threads: int = os.cpu_count() or 1
    enable_multithread: bool = True
    features: dict = field(default_factory=dict)
    window_backend: str | None = None
    render_backend: str | None = None
    target_fps: int = 60
    frame_sync: str = "adaptive"

settings = Settings()
