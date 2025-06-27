"""Default action implementations have been removed."""

from .base import Action, register_action, resolve_value  # unused but kept for compatibility
from ..log import logger  # unused but kept for compatibility

__all__: list[str] = []
