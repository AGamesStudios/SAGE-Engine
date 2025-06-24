"""SAGE Logic - modular condition/action event system."""

from .base import (
    Condition, Action, Event, EventSystem,
    register_condition, register_action,
    condition_from_dict, action_from_dict,
    CONDITION_REGISTRY, ACTION_REGISTRY,
)

# Automatically import all submodules so built-ins register themselves
import importlib
import pkgutil

for mod in pkgutil.iter_modules(__path__):
    if mod.name not in {'base', '__pycache__'}:
        importlib.import_module(f'{__name__}.{mod.name}')

__all__ = [
    'Condition', 'Action', 'Event', 'EventSystem',
    'register_condition', 'register_action',
    'condition_from_dict', 'action_from_dict',
    'CONDITION_REGISTRY', 'ACTION_REGISTRY',
] + list(CONDITION_REGISTRY.keys()) + list(ACTION_REGISTRY.keys())
