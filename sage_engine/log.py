import logging
import os
import atexit

LOG_FILE = os.path.join(os.path.expanduser('~'), '.sage_engine.log')

def _setup_logger() -> logging.Logger:
    """Configure and return the engine logger."""
    os.makedirs(os.path.dirname(LOG_FILE), exist_ok=True)
    logger = logging.getLogger('sage_engine')
    if not logger.handlers:
        logger.setLevel(logging.INFO)
        fmt = logging.Formatter('%(asctime)s %(levelname)s: %(message)s')
        fh = logging.FileHandler(LOG_FILE, encoding='utf-8')
        fh.setFormatter(fmt)
        ch = logging.StreamHandler()
        ch.setFormatter(fmt)
        logger.addHandler(fh)
        logger.addHandler(ch)
        logger.info('Logger initialised')
    return logger

logger = _setup_logger()
atexit.register(logging.shutdown)

__all__ = ['logger', 'LOG_FILE']
