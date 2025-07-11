import cProfile
import io
import pstats
from .log import logger

__all__ = ["Profiler"]

class Profiler:
    """Simple cProfile wrapper that logs and saves statistics."""

    def __init__(self, path: str = "profile.prof") -> None:
        self.path = path
        self._prof = cProfile.Profile()

    def start(self) -> None:
        self._prof.enable()

    def stop(self) -> None:
        self._prof.disable()
        self._prof.dump_stats(self.path)
        s = io.StringIO()
        pstats.Stats(self._prof, stream=s).sort_stats("cumulative").print_stats(20)
        logger.info("Profile saved to %s", self.path)
        logger.info("\n%s", s.getvalue())

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, exc_type, exc, tb):
        self.stop()
        return False

