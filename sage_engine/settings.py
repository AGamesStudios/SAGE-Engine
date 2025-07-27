"""Engine settings used across modules."""

from dataclasses import dataclass
import os

@dataclass
class Settings:
    cpu_threads: int = os.cpu_count() or 1
    enable_multithread: bool = True

settings = Settings()
