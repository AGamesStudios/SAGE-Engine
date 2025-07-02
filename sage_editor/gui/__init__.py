"""UI components for SAGE Editor."""

from .main_window import EditorWindow
from .viewport import Viewport
from .console import ConsoleWidget
from .object_list import ObjectListWidget
from .property_editor import PropertyEditor

__all__ = [
    'EditorWindow',
    'Viewport',
    'ConsoleWidget',
    'ObjectListWidget',
    'PropertyEditor',
]
