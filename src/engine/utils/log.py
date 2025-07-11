import logging
import os
import atexit
import faulthandler
from engine.utils.config import get as cfg_get

LOG_DIR = os.path.expanduser(
    str(cfg_get('logs.dir', os.path.join('~', '.cache', 'sage', 'logs')))
)
LOG_FILE = os.path.join(LOG_DIR, 'engine.log')

logger = logging.getLogger('engine')


_fault_file = None


def init_logger(enable_crash_dumps: bool = True) -> logging.Logger:
    """Configure and return the engine logger."""
    if logger.handlers:
        return logger
    os.makedirs(LOG_DIR, exist_ok=True)
    level = str(cfg_get('logs.level', os.environ.get('SAGE_LOG_LEVEL', 'INFO'))).upper()
    logger.setLevel(getattr(logging, level, logging.INFO))
    fmt = logging.Formatter('%(asctime)s %(levelname)s: %(message)s')
    fh = logging.FileHandler(LOG_FILE, encoding='utf-8')
    fh.setFormatter(fmt)
    ch = logging.StreamHandler()
    ch.setFormatter(fmt)
    logger.addHandler(fh)
    logger.addHandler(ch)
    logger.info('Logger initialised')
    art = (
        "\033[31m  ____   ___   ____  _____ \033[0m\n"
        "\033[32m / ___| / _ \\ / ___|| ____|\033[0m\n"
        "\033[33m| |    | | | | |    |  _|  \033[0m\n"
        "\033[34m| |___ | |_| | |___ | |___ \033[0m\n"
        "\033[35m \\____| \\___/ \\____||_____|\033[0m"
    )
    logger.info("\n%s", art)
    atexit.register(logging.shutdown)
    if enable_crash_dumps and not faulthandler.is_enabled():
        global _fault_file
        try:
            _fault_file = open(LOG_FILE, 'a', encoding='utf-8')
        except OSError as exc:
            logger.error("Failed to open log file for crash dumps: %s", exc)
        else:
            faulthandler.enable(file=_fault_file, all_threads=True)
            atexit.register(_fault_file.close)
    return logger


def set_level(level: str | int) -> None:
    """Set the logger verbosity at runtime."""
    if isinstance(level, str):
        level = getattr(logging, level.upper(), logging.INFO)
    logger.setLevel(level)

def set_stream(stream) -> None:
    """Redirect console output to the given stream without affecting the log file."""
    for handler in logger.handlers:
        # Skip FileHandler so logs continue to write to disk
        if isinstance(handler, logging.StreamHandler) and not isinstance(
            handler, logging.FileHandler
        ):
            handler.setStream(stream)

__all__ = ['logger', 'LOG_FILE', 'set_stream', 'set_level', 'init_logger']
