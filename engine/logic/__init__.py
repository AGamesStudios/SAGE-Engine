"""SAGE Logic - modular condition/action event system."""

from .base import (
    Condition, Action, Event, EventSystem,
    register_condition, register_action,
    get_condition_params, get_action_params,
    get_registered_conditions, get_registered_actions,
    condition_from_dict, action_from_dict, event_from_dict,
    load_logic_plugins,
    CONDITION_REGISTRY, ACTION_REGISTRY,
)

# Import built-in conditions and actions so static analyzers
# can resolve them. They also register themselves here.
from . import conditions  # noqa: F401
from . import actions     # noqa: F401

# load plugins registered via entry points
load_logic_plugins()

# Automatically import all submodules so built-ins register themselves
import importlib
import pkgutil

for mod in pkgutil.iter_modules(__path__):
    if mod.name not in {"base", "actions", "conditions", "__pycache__"}:
        importlib.import_module(f"{__name__}.{mod.name}")

# expose registered classes at the package level so users can simply import
# them from ``engine.logic``
globals().update(CONDITION_REGISTRY)
globals().update(ACTION_REGISTRY)

__all__ = [
    'Condition', 'Action', 'Event', 'EventSystem',
    'register_condition', 'register_action',
    'get_condition_params', 'get_action_params',
    'get_registered_conditions', 'get_registered_actions',
    'condition_from_dict', 'action_from_dict', 'event_from_dict',
    'load_logic_plugins',
    'CONDITION_REGISTRY', 'ACTION_REGISTRY',
] + list(CONDITION_REGISTRY.keys()) + list(ACTION_REGISTRY.keys())

# warn about any missing exports at import time
_missing = [name for name in __all__ if name not in globals()]
from ..utils.diagnostics import warn
for _name in _missing:
    warn("Missing reference %s in logic.__init__", _name)
del _missing
