# SAGE Logic - a simple condition/action event system
# Provides Clickteam-style logic for both 2D and 3D games

from ..log import logger

# registries used to map names to classes so new logic blocks can be added
# without modifying the loader code
CONDITION_REGISTRY: dict[str, type] = {}
ACTION_REGISTRY: dict[str, type] = {}
# store parameter metadata so conditions and actions can be created
# automatically from dictionaries. Each entry maps a name to a list of
# ``(param_name, kind)`` tuples where ``kind`` can be ``object`` for
# GameObject references or ``variable`` for variable names.
CONDITION_META: dict[str, list[tuple[str, str, str | None]]] = {}
ACTION_META: dict[str, list[tuple[str, str, str | None]]] = {}


def register_condition(name: str, params: list[tuple[str, str, str | None]] | None = None):
    """Decorator to register a Condition subclass.

    ``params`` defines a list of tuples describing constructor arguments. Each
    tuple is ``(arg_name, kind, key)`` where ``arg_name`` is the parameter name
    of the constructor, ``kind`` is ``'value'`` for plain values, ``'object'`` to
    look up a scene object by index and ``'variable'`` for variables. ``key`` is
    the dictionary key when loading; when omitted it defaults to ``arg_name``.
    """

    def decorator(cls):
        CONDITION_REGISTRY[name] = cls
        CONDITION_META[name] = params or []
        return cls

    return decorator


def register_action(name: str, params: list[tuple[str, str, str | None]] | None = None):
    """Decorator to register an Action subclass with optional metadata."""

    def decorator(cls):
        ACTION_REGISTRY[name] = cls
        ACTION_META[name] = params or []
        return cls

    return decorator


def get_registered_conditions() -> list[str]:
    """Return the list of available condition names."""
    return list(CONDITION_REGISTRY.keys())


def get_condition_params(name: str) -> list[tuple[str, str, str | None]]:
    """Return parameter metadata for ``name``."""
    return CONDITION_META.get(name, [])


def get_registered_actions() -> list[str]:
    """Return the list of available action names."""
    return list(ACTION_REGISTRY.keys())


def get_action_params(name: str) -> list[tuple[str, str, str | None]]:
    """Return parameter metadata for ``name``."""
    return ACTION_META.get(name, [])

class Condition:
    """Base condition interface."""
    def check(self, engine, scene, dt):
        raise NotImplementedError

class Action:
    """Base action interface."""
    def execute(self, engine, scene, dt):
        raise NotImplementedError

class Event:
    """Combination of conditions and actions."""

    def __init__(self, conditions, actions, once=False):
        self.conditions = conditions
        self.actions = actions
        self.once = once
        self.triggered = False

    def update(self, engine, scene, dt):
        if self.once and self.triggered:
            return
        try:
            if all(cond.check(engine, scene, dt) for cond in self.conditions):
                for action in self.actions:
                    try:
                        action.execute(engine, scene, dt)
                    except Exception:
                        logger.exception('Action error')
                self.triggered = True
        except Exception:
            logger.exception('Condition error')

class EventSystem:
    """Container for events."""
    def __init__(self, variables=None):
        self.events = []
        self.variables = variables if variables is not None else {}

    def add_event(self, event):
        self.events.append(event)

    def update(self, engine, scene, dt):
        for evt in list(self.events):
            evt.update(engine, scene, dt)


def condition_from_dict(data, objects, variables):
    """Instantiate a condition from its dictionary description."""
    typ = data.get('type')
    cls = CONDITION_REGISTRY.get(typ)
    if cls is None:
        logger.warning('Unknown condition type %s', typ)
        return None
    params = {}
    for arg, kind, key in CONDITION_META.get(typ, []):
        dict_key = key or arg
        val = data.get(dict_key)
        if kind == 'object':
            if val is None or val < 0 or val >= len(objects):
                return None
            params[arg] = objects[val]
        elif kind == 'variable':
            params[arg] = variables.get(val)
        else:
            params[arg] = val
    try:
        return cls(**params)
    except Exception:
        logger.exception('Failed to construct condition %s', typ)
        return None


def action_from_dict(data, objects):
    """Instantiate an action from its dictionary description."""
    typ = data.get('type')
    cls = ACTION_REGISTRY.get(typ)
    if cls is None:
        logger.warning('Unknown action type %s', typ)
        return None
    params = {}
    for arg, kind, key in ACTION_META.get(typ, []):
        dict_key = key or arg
        val = data.get(dict_key)
        if kind == 'object':
            if val is None or val < 0 or val >= len(objects):
                return None
            params[arg] = objects[val]
        else:
            params[arg] = val
    try:
        return cls(**params)
    except Exception:
        logger.exception('Failed to construct action %s', typ)
        return None

