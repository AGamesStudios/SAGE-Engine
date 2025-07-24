"""Profiling helpers."""
from dataclasses import dataclass
from contextlib import contextmanager
import time


@dataclass
class ProfileEntry:
    name: str
    ms: float


@dataclass
class ProfileFrame:
    entries: list[ProfileEntry]


@contextmanager
def profile_frame() -> ProfileFrame:
    """Measure execution time of a block of code."""
    start = time.perf_counter()
    entries: list[ProfileEntry] = []

    def mark(name: str) -> None:
        now = time.perf_counter()
        entries.append(ProfileEntry(name, (now - start) * 1000.0))

    try:
        yield mark
    finally:
        total = (time.perf_counter() - start) * 1000.0
        entries.append(ProfileEntry("frame", total))
        print(f"[perf] frame {total:.2f} ms")


__all__ = ["ProfileEntry", "ProfileFrame", "profile_frame"]
