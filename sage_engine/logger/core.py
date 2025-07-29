from __future__ import annotations

import os
import time
from typing import List

from .levels import (
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    CRITICAL,
    _level_names,
    _name_to_level,
)
from .handlers.base import LogHandler
from .handlers.console import ConsoleHandler
from .handlers.file import FileHandler

class Logger:
    def __init__(self) -> None:
        level_name = os.getenv("SAGE_LOGLEVEL", "INFO").upper()
        self.level = _name_to_level.get(level_name, INFO)
        self.handlers: List[LogHandler] = [ConsoleHandler()]
        self.phase_func = lambda: ""

    def set_level(self, level: str) -> None:
        self.level = _name_to_level.get(level.upper(), self.level)

    def add_handler(self, handler) -> None:
        self.handlers.append(handler)

    def _log(self, lvl: int, msg: str, tag: str, extra: dict | None, exc_info: bool, *args) -> None:
        if lvl < self.level:
            return
        if args:
            msg = msg % args
        ts = time.strftime("%H:%M:%S")
        phase = self.phase_func() or ""
        prefix = f"[{ts}] [{_level_names[lvl]}] [{tag}]"
        if phase:
            prefix += f"[{phase}]"
        text = f"{prefix} {msg}"
        if exc_info:
            import traceback, sys
            text += "\n" + "".join(traceback.format_exception(*sys.exc_info()))
        for h in list(self.handlers):
            h.emit(text, lvl)

    def debug(self, msg: str, *args, tag: str = "core", extra: dict | None = None) -> None:
        self._log(DEBUG, msg, tag, extra, False, *args)

    def info(self, msg: str, *args, tag: str = "core", extra: dict | None = None) -> None:
        self._log(INFO, msg, tag, extra, False, *args)

    def warn(self, msg: str, *args, tag: str = "core", extra: dict | None = None) -> None:
        self._log(WARN, msg, tag, extra, False, *args)

    def error(self, msg: str, *args, tag: str = "core", extra: dict | None = None, exc_info: bool = False) -> None:
        self._log(ERROR, msg, tag, extra, exc_info, *args)

    def fatal(self, msg: str, *args, tag: str = "core", extra: dict | None = None) -> None:
        self._log(FATAL, msg, tag, extra, False, *args)

    def critical(self, msg: str, *args, tag: str = "core", extra: dict | None = None) -> None:
        self._log(CRITICAL, msg, tag, extra, False, *args)

    def exception(self, msg: str, *args, tag: str = "core", extra: dict | None = None) -> None:
        """Log an error message with the current exception info."""
        self.error(msg, *args, tag=tag, extra=extra, exc_info=True)

logger = Logger()


def boot(_config: dict) -> None:  # pragma: no cover - simple init
    logger.info("Logger boot", tag="logger")
    log_dir = "logs"
    os.makedirs(log_dir, exist_ok=True)
    today = time.strftime("%Y-%m-%d")
    handler = FileHandler(os.path.join(log_dir, f"{today}.log"))
    logger.add_handler(handler)

    # cleanup old logs older than 7 days
    now = time.time()
    for name in os.listdir(log_dir):
        if name.endswith(".log") and len(name) >= 14:
            path = os.path.join(log_dir, name)
            try:
                if now - os.path.getmtime(path) > 7 * 24 * 3600:
                    os.remove(path)
            except OSError:
                pass
