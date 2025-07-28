from .base import LogHandler
import sys

class ConsoleHandler(LogHandler):
    def emit(self, record: str) -> None:
        sys.stdout.write(record + "\n")
