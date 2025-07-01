import logging
import os
import atexit

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
LOG_DIR = os.path.join(BASE_DIR, 'logs')
LOG_FILE = os.path.join(LOG_DIR, 'engine.log')

logger = logging.getLogger('engine')


def init_logger() -> logging.Logger:
    """Configure and return the engine logger."""
    if logger.handlers:
        return logger
    os.makedirs(LOG_DIR, exist_ok=True)
    level = os.environ.get('SAGE_LOG_LEVEL', 'INFO').upper()
    logger.setLevel(getattr(logging, level, logging.INFO))
    fmt = logging.Formatter('%(asctime)s %(levelname)s: %(message)s')
    fh = logging.FileHandler(LOG_FILE, encoding='utf-8')
    fh.setFormatter(fmt)
    ch = logging.StreamHandler()
    ch.setFormatter(fmt)
    logger.addHandler(fh)
    logger.addHandler(ch)
    logger.info('Logger initialised')
    atexit.register(logging.shutdown)
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
        if isinstance(handler, logging.StreamHandler) and not isinstance(handler, logging.FileHandler):
            handler.setStream(stream)

__all__ = ['logger', 'LOG_FILE', 'set_stream', 'set_level', 'init_logger']
