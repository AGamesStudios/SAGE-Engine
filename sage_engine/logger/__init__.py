"""SAGE Logger providing lightweight logging utilities."""

from .core import logger, boot
from .levels import DEBUG, INFO, WARN, ERROR
from ..core import register as _register

_register("boot", boot)

__all__ = ["logger", "boot", "DEBUG", "INFO", "WARN", "ERROR"]
