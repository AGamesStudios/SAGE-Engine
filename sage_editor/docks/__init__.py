"""Dock widgets for SAGE Editor."""

from .console import ConsoleDock
from .properties import PropertiesDock
from .resources import ResourceDock
from .logic import LogicTab

__all__ = [
    'ConsoleDock', 'PropertiesDock', 'ResourceDock', 'LogicTab'
]
