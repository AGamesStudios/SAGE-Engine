"""SAGE Logger providing lightweight logging utilities."""

from .core import logger, boot
from .levels import DEBUG, INFO, WARN, ERROR
from .crash import log_crash
from .hooks import install as _install_hooks
from ..core import register as _register

_register("boot", boot)
_register("boot", lambda _cfg: _install_hooks())

__all__ = ["logger", "boot", "DEBUG", "INFO", "WARN", "ERROR", "log_crash"]
