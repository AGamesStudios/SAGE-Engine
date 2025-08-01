"""Frame synchronization with adaptive sleep."""
import time

_prev = 0.0

def sync(target_fps: int, *, frame_budget: float = 0.0) -> None:
    """Sleep to maintain target FPS."""
    global _prev
    now = time.perf_counter()
    if _prev == 0.0:
        _prev = now
        return
    elapsed = now - _prev
    delay = (1.0 / target_fps) - elapsed
    if delay > 0:
        time.sleep(delay)
    _prev = time.perf_counter()
