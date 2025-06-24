"""SAGE Editor package.

This module exposes the public API while the implementation
lives in :mod:`sage_editor.editor` with supporting widgets
under :mod:`sage_editor.docks` and :mod:`sage_editor.widgets`.
"""

from .editor import Editor, logger, load_recent, save_recent
from .app import main, ProjectManager
from .plugins import register_plugin, load_plugins

__all__ = [
    'Editor', 'ProjectManager', 'main',
    'register_plugin', 'load_plugins',
    'logger', 'load_recent', 'save_recent',
]
