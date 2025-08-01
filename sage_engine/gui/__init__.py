"""Lightweight GUI system built on top of SAGE Graphic."""
from __future__ import annotations

from .manager import GUIManager
from .base import Widget
from . import widgets
from .widgets.button import Button
from .widgets.label import Label
from .widgets.textinput import TextInput
from .widgets.checkbox import Checkbox
from .widgets.slider import Slider
from .widgets.dropdown import Dropdown
from .style import load_theme, apply_theme
from . import i18n
from . import drag, animation, layout
from .. import core

manager = GUIManager()
core.expose("gui", manager)

__all__ = [
    "GUIManager",
    "manager",
    "Widget",
    "Button",
    "Label",
    "TextInput",
    "Checkbox",
    "Slider",
    "Dropdown",
    "drag",
    "animation",
    "layout",
    "i18n",
    "load_theme",
    "apply_theme",
]
