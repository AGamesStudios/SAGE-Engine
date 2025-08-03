"""Lightweight GUI system built on top of SAGE Graphic."""
from __future__ import annotations

from sage_engine.gui.manager import GUIManager
from sage_engine.gui.base import Widget
from sage_engine.gui import widgets, registry, tool, i18n, drag, animation, layout
from sage_engine.gui.widgets.button import Button
from sage_engine.gui.widgets.label import Label
from sage_engine.gui.widgets.textinput import TextInput
from sage_engine.gui.widgets.checkbox import Checkbox
from sage_engine.gui.widgets.slider import Slider
from sage_engine.gui.widgets.dropdown import Dropdown
from sage_engine.gui.widgets.window import Window
from sage_engine.gui.widgets.panel import Panel
from sage_engine.gui.widgets.popup import Popup
from sage_engine.gui.widgets.dockpanel import DockPanel
from sage_engine.gui.widgets.scrollview import ScrollView
from sage_engine.gui.widgets.inspector import InspectorPanel
from sage_engine.gui.style import load_theme, apply_theme
from sage_engine import core

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
    "Window",
    "Panel",
    "Popup",
    "DockPanel",
    "ScrollView",
    "InspectorPanel",
    "drag",
    "tool",
    "animation",
    "layout",
    "i18n",
    "load_theme",
    "apply_theme",
    "registry",
]
