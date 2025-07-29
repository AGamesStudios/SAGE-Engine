DEBUG = 10
INFO = 20
WARN = 30
ERROR = 40
FATAL = 50
CRITICAL = 60

_level_names = {
    DEBUG: 'DEBUG',
    INFO: 'INFO',
    WARN: 'WARN',
    ERROR: 'ERROR',
    FATAL: 'FATAL',
    CRITICAL: 'CRITICAL',
}
_name_to_level = {v: k for k, v in _level_names.items()}
