from __future__ import annotations

import os
import time
from typing import List

from .levels import DEBUG, INFO, WARN, ERROR, _level_names, _name_to_level
from .handlers.console import ConsoleHandler

class Logger:
    def __init__(self) -> None:
        level_name = os.getenv("SAGE_LOGLEVEL", "INFO").upper()
        self.level = _name_to_level.get(level_name, INFO)
        self.handlers: List[ConsoleHandler] = [ConsoleHandler()]
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
            h.emit(text)

    def debug(self, msg: str, *args, tag: str = "core", extra: dict | None = None) -> None:
        self._log(DEBUG, msg, tag, extra, False, *args)

    def info(self, msg: str, *args, tag: str = "core", extra: dict | None = None) -> None:
        self._log(INFO, msg, tag, extra, False, *args)

    def warn(self, msg: str, *args, tag: str = "core", extra: dict | None = None) -> None:
        self._log(WARN, msg, tag, extra, False, *args)

    def error(self, msg: str, *args, tag: str = "core", extra: dict | None = None, exc_info: bool = False) -> None:
        self._log(ERROR, msg, tag, extra, exc_info, *args)

logger = Logger()


def boot(_config: dict) -> None:  # pragma: no cover - simple init
    logger.info("Logger boot", tag="logger")
