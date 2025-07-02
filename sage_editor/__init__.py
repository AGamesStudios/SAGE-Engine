"""SAGE Editor package built on PyQt6.

The editor uses the engine but the engine is completely independent.
Modules can register plugins using :func:`register_plugin` to extend the
interface without modifying the core.
"""

from __future__ import annotations

import logging

from .gui import EditorWindow, Viewport
from .app import main
from .style import apply_dark_fusion

logger = logging.getLogger("sage_editor")

__all__ = [
    'EditorWindow',
    'Viewport',
    'main',
    'logger',
    'apply_dark_fusion',
]
