import logging
import os
import atexit

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
LOG_DIR = os.path.join(BASE_DIR, 'logs')
LOG_FILE = os.path.join(LOG_DIR, 'engine.log')

def _setup_logger() -> logging.Logger:
    """Configure and return the engine logger."""
    os.makedirs(LOG_DIR, exist_ok=True)
    logger = logging.getLogger('engine')
    if not logger.handlers:
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
    return logger

logger = _setup_logger()
atexit.register(logging.shutdown)

__all__ = ['logger', 'LOG_FILE']
