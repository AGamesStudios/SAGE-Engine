from __future__ import annotations

import json
import time
from contextlib import contextmanager
from typing import Dict, List

__all__ = ["TraceProfiler"]


class TraceProfiler:
    """Collect simple Chrome Trace JSON events."""

    def __init__(self, path: str = "trace.json") -> None:
        self.path = path
        self.events: List[Dict[str, float]] = []

    @contextmanager
    def phase(self, name: str):
        start = time.perf_counter()
        yield
        end = time.perf_counter()
        self.events.append({
            "name": name,
            "ph": "X",
            "ts": start * 1_000_000,
            "dur": (end - start) * 1_000_000,
            "pid": 0,
            "tid": 0,
        })

    def write(self) -> None:
        with open(self.path, "w", encoding="utf-8") as f:
            json.dump({"traceEvents": self.events}, f)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc, tb):
        self.write()
        return False
