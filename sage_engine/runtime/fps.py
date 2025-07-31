from __future__ import annotations

from enum import Enum, auto

class FPSMode(Enum):
    CLASSIC = auto()
    GAMEFRAME = auto()

# Global mode switch
fps_mode: FPSMode = FPSMode.CLASSIC


def get_current_fps(delta_time: float, *, gf_index: float | None = None, target_fps: int = 60) -> float:
    """Return FPS according to current ``fps_mode``.

    Parameters
    ----------
    delta_time: float
        Frame time in seconds.
    gf_index: float | None
        GameFrame index value (0-100) if available.
    target_fps: int
        Target FPS used with GameFrame mode.
    """
    if fps_mode is FPSMode.CLASSIC:
        return 1.0 / delta_time if delta_time > 0 else 0.0
    if gf_index is None:
        return 0.0
    return (gf_index / 100.0) * float(target_fps)
