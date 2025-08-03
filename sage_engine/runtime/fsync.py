"""Software frame synchronization independent of VSync."""

from __future__ import annotations

import time
from time import perf_counter_ns

from ..logger import logger


class FrameSync:
    """Software frame pacing with optional limits."""

    def __init__(self, target_fps: int = 60, *, mode: str = "capped", tolerance: float = 0.2, enabled: bool = True) -> None:
        self.enabled = enabled
        self.mode = mode
        self.tolerance = tolerance
        self.target_fps = target_fps
        self.target_dt = 1.0 / target_fps if target_fps > 0 else 0.0
        self._start_ns = 0
        self._next_frame_ns: int | None = None
        self.last_sleep_time = 0.0

    def start_frame(self) -> None:
        now = perf_counter_ns()
        self._start_ns = now
        if self._next_frame_ns is not None:
            drift = now - self._next_frame_ns
            if abs(drift) > self.tolerance * 1_000_000:
                # Correct accumulated phase drift
                self._next_frame_ns = now + int(self.target_dt * 1e9)
        else:
            self._next_frame_ns = now + int(self.target_dt * 1e9)

    def end_frame(self) -> float:
        """Finish the frame and return the time slept in seconds."""
        self.last_sleep_time = 0.0
        if not self.enabled or self.mode == "unlimited":
            return 0.0
        now = perf_counter_ns()
        elapsed_ns = now - self._start_ns
        target_dt_ns = int(self.target_dt * 1e9)
        if self.mode == "adaptive" and elapsed_ns > target_dt_ns and target_dt_ns > 0:
            mult = int(elapsed_ns / target_dt_ns) + 1
            self.target_fps = max(1, int(self.target_fps / mult))
            self.target_dt = 1.0 / self.target_fps
            target_dt_ns = int(self.target_dt * 1e9)

        target_end = self._start_ns + target_dt_ns
        if now < target_end:
            sleep_time = (target_end - now) / 1e9
            if sleep_time <= 0:
                logger.warn("[fsync] Negative sleep time prevented %.6f", sleep_time)
            else:
                self.last_sleep_time = sleep_time
                if sleep_time > 0.0003:
                    time.sleep(sleep_time - 0.0003)
                while perf_counter_ns() < target_end:
                    pass
        self._next_frame_ns = target_end
        return self.last_sleep_time
    def sleep_until_next_frame(self) -> None:
        """Sleep the remaining time of the frame without updating timers."""
        if not self.enabled or self._next_frame_ns is None:
            return
        now = perf_counter_ns()
        if now < self._next_frame_ns:
            sleep_time = (self._next_frame_ns - now) / 1e9
            if sleep_time <= 0:
                logger.warn("[fsync] negative sleep time %.6f", sleep_time)
            else:
                if sleep_time > 0.0003:
                    time.sleep(sleep_time - 0.0003)
                while perf_counter_ns() < self._next_frame_ns:
                    pass
        self._next_frame_ns += int(self.target_dt * 1e9)

