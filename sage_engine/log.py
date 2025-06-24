import logging
import os

LOG_FILE = os.path.join(os.path.expanduser('~'), '.sage_engine.log')

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

__all__ = ['logger', 'LOG_FILE']
