from __future__ import annotations

import os
from .base import LogHandler


class FileHandler(LogHandler):
    def __init__(self, path: str) -> None:
        os.makedirs(os.path.dirname(path), exist_ok=True)
        self._fh = open(path, "a", encoding="utf-8")

    def emit(self, record: str, level: int) -> None:
        self._fh.write(record + "\n")
        self._fh.flush()

    def close(self) -> None:
        try:
            self._fh.close()
        except Exception:
            pass
