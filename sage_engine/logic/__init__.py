"""SAGE Logic - modular condition/action event system."""

from .base import (
    Condition, Action, Event, EventSystem,
    register_condition, register_action,
    condition_from_dict, action_from_dict,
    CONDITION_REGISTRY, ACTION_REGISTRY,
)

# Import built-in conditions and actions so they register themselves
from .conditions import (
    KeyPressed, KeyReleased, MouseButton, InputState, Collision, AfterTime,
    OnStart, EveryFrame, VariableCompare,
)
from .actions import (
    Move, SetPosition, Destroy, Print, PlaySound, Spawn,
    SetVariable, ModifyVariable,
)

__all__ = [
    'Condition', 'Action', 'Event', 'EventSystem',
    'register_condition', 'register_action',
    'condition_from_dict', 'action_from_dict',
    'KeyPressed', 'KeyReleased', 'MouseButton', 'InputState', 'Collision', 'AfterTime',
    'OnStart', 'EveryFrame', 'VariableCompare',
    'Move', 'SetPosition', 'Destroy', 'Print', 'PlaySound', 'Spawn',
    'SetVariable', 'ModifyVariable'
]
