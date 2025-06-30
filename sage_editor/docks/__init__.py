"""Dock widgets for SAGE Editor."""

from .console import ConsoleDock
from .properties import PropertiesDock
from .resources import ResourceDock
from .scenes import ScenesDock
from .logic import LogicTab
from .profiler import ProfilerDock

__all__ = [
    'ConsoleDock', 'PropertiesDock', 'ResourceDock', 'ScenesDock',
    'LogicTab', 'ProfilerDock'
]
