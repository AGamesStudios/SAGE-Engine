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
CONDITION_META: dict[str, list[tuple]] = {}
ACTION_META: dict[str, list[tuple]] = {}

# fallback dictionary mapping translated names back to the English
# identifiers used internally.  This lets old projects saved with
# localized names continue to load even when the editor is not
# installed.  ``engine.lang`` ships with a minimal dictionary while
# ``sage_editor.lang`` provides the full set used by the editor.
TRANSLATION_LOOKUP: dict[str, str] = {}
LANGUAGES = {}

try:
    from .. import lang as engine_lang
    LANGUAGES.update(engine_lang.LANGUAGES)
except Exception:
    pass

try:
    from sage_editor.lang import LANGUAGES as editor_lang
    for k, v in editor_lang.items():
        if k not in LANGUAGES:
            LANGUAGES[k] = v
        else:
            LANGUAGES[k].update(v)
except Exception:
    # editor may not be installed
    pass

for entries in LANGUAGES.values():
    for eng, local in entries.items():
        if eng != local:
            TRANSLATION_LOOKUP[local] = eng

import re

class EngineRef:
    """Reference to an engine attribute or helper function."""

    def __init__(self, path: list[str], args: list | None = None, call: bool = False):
        self.path = path
        self.args = args or []
        self.call = call

    def get(self, engine):
        target = engine
        for attr in self.path:
            target = getattr(target, attr, None)
            if target is None:
                return None
        if self.call:
            if callable(target):
                return target(*[resolve_value(a, engine) for a in self.args])
            return None
        return target


class VarRef(EngineRef):
    """Reference to a variable that resolves at runtime."""

    def __init__(self, name: str):
        super().__init__(['variable'], [name], call=True)


# support engine attribute references like ``engine.attr`` or ``engine.method(args)``
# and shorthand variable syntax ``$name`` or ``{name}``
_ENGINE_REF_RE = re.compile(r"engine((?:\.\w+)+)(?:\((.*)\))?")
_VAR_SHORT_RE = re.compile(r"\$(\w+)|\{(\w+)\}")

def parse_value(val):
    """Return ``val`` or an :class:`EngineRef` if it matches the reference syntax."""
    if isinstance(val, str):
        text = val.strip()
        m = _ENGINE_REF_RE.fullmatch(text)
        if m:
            path_text = m.group(1).lstrip('.')
            path = path_text.split('.') if path_text else []
            call = m.group(2) is not None
            args_text = (m.group(2) or '').strip()
            args = []
            if args_text:
                import ast
                try:
                    parsed = ast.literal_eval(f'({args_text},)')
                    args = list(parsed)
                except Exception:
                    args = [args_text.strip("'\"")]
            if path == ['variable'] and args:
                return VarRef(str(args[0]))
            return EngineRef(path, args, call)
        m = _VAR_SHORT_RE.fullmatch(text)
        if m:
            name = m.group(1) or m.group(2)
            return VarRef(name)
    return val


def resolve_value(val, engine):
    """Return the runtime value for ``val``."""
    if isinstance(val, EngineRef):
        return val.get(engine)
    return val


def register_condition(name: str, params: list[tuple] | None = None):
    """Decorator to register a Condition subclass.

    ``params`` defines a list of tuples describing constructor arguments.
    Entries are ``(arg_name, kind[, key[, types]])`` where ``kind`` is
    ``'value'`` for plain values, ``'object'`` to look up a scene object by
    index and ``'variable'`` for variables. ``key`` is the dictionary key when
    loading, defaulting to ``arg_name``. ``types`` optionally lists allowed
    object types when ``kind`` is ``'object'``.
    """

    def decorator(cls):
        CONDITION_REGISTRY[name] = cls
        CONDITION_META[name] = params or []
        return cls

    return decorator


def register_action(name: str, params: list[tuple] | None = None):
    """Decorator to register an Action subclass with optional metadata.

    ``params`` follows the same format as :func:`register_condition` and may
    include a list of allowed object types for ``'object'`` parameters.
    """

    def decorator(cls):
        ACTION_REGISTRY[name] = cls
        ACTION_META[name] = params or []
        return cls

    return decorator


def get_registered_conditions() -> list[str]:
    """Return the list of available condition names."""
    return list(CONDITION_REGISTRY.keys())


def get_condition_params(name: str) -> list[tuple]:
    """Return parameter metadata for ``name``."""
    return CONDITION_META.get(name, [])


def get_registered_actions() -> list[str]:
    """Return the list of available action names."""
    return list(ACTION_REGISTRY.keys())


def get_action_params(name: str) -> list[tuple]:
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

    def __init__(self, conditions, actions, once=False, name=None, enabled=True):
        self.conditions = conditions
        self.actions = actions
        self.once = once
        self.name = name
        self.enabled = enabled
        self.triggered = False

    def enable(self):
        self.enabled = True

    def disable(self):
        self.enabled = False

    def update(self, engine, scene, dt):
        if not self.enabled or (self.once and self.triggered):
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

    def get_event(self, name):
        for evt in self.events:
            if evt.name == name:
                return evt
        return None

    def remove_event(self, name):
        self.events = [e for e in self.events if e.name != name]

    def add_event(self, event):
        self.events.append(event)

    def update(self, engine, scene, dt):
        for evt in list(self.events):
            evt.update(engine, scene, dt)


def condition_from_dict(data, objects, variables):
    """Instantiate a condition from its dictionary description."""
    typ = data.get('type')
    if typ in TRANSLATION_LOOKUP:
        typ = TRANSLATION_LOOKUP[typ]
    cls = CONDITION_REGISTRY.get(typ)
    if cls is None:
        logger.warning('Unknown condition type %s', typ)
        return None
    params = {}
    for entry in CONDITION_META.get(typ, []):
        arg = entry[0]
        kind = entry[1] if len(entry) > 1 else 'value'
        key = entry[2] if len(entry) > 2 else arg
        allowed = entry[3] if len(entry) > 3 else None
        val = data.get(key)
        if kind == 'object':
            if val is None or val < 0 or val >= len(objects):
                # try to find a default object matching the allowed type
                obj = None
                if allowed:
                    from ..core.objects import get_object_type
                    for candidate in objects:
                        typ_name = get_object_type(candidate)
                        if typ_name in allowed:
                            obj = candidate
                            break
                if obj is None:
                    logger.warning('Condition %s has invalid object index for %s', typ, arg)
                    return None
            else:
                obj = objects[val]
            if allowed:
                from ..core.objects import get_object_type
                typ_name = get_object_type(obj)
                if typ_name not in allowed:
                    logger.warning('Condition %s requires %s for %s', typ, allowed, arg)
                    return None
            params[arg] = obj
        elif kind == 'variable':
            params[arg] = VarRef(val) if isinstance(val, str) else val
        else:
            params[arg] = parse_value(val)
    try:
        return cls(**params)
    except Exception:
        logger.exception('Failed to construct condition %s', typ)
        return None


def action_from_dict(data, objects):
    """Instantiate an action from its dictionary description."""
    typ = data.get('type')
    if typ in TRANSLATION_LOOKUP:
        typ = TRANSLATION_LOOKUP[typ]
    cls = ACTION_REGISTRY.get(typ)
    if cls is None:
        logger.warning('Unknown action type %s', typ)
        return None
    params = {}
    for entry in ACTION_META.get(typ, []):
        arg = entry[0]
        kind = entry[1] if len(entry) > 1 else 'value'
        key = entry[2] if len(entry) > 2 else arg
        allowed = entry[3] if len(entry) > 3 else None
        val = data.get(key)
        if kind == 'object':
            if val is None or val < 0 or val >= len(objects):
                obj = None
                if allowed:
                    from ..core.objects import get_object_type
                    for candidate in objects:
                        typ_name = get_object_type(candidate)
                        if typ_name in allowed:
                            obj = candidate
                            break
                if obj is None:
                    logger.warning('Action %s has invalid object index for %s', typ, arg)
                    return None
            else:
                obj = objects[val]
            if allowed:
                from ..core.objects import get_object_type
                typ_name = get_object_type(obj)
                if typ_name not in allowed:
                    logger.warning('Action %s requires %s for %s', typ, allowed, arg)
                    return None
            params[arg] = obj
        elif kind == 'variable':
            params[arg] = VarRef(val) if isinstance(val, str) else val
        else:
            params[arg] = parse_value(val)
    try:
        return cls(**params)
    except Exception:
        logger.exception('Failed to construct action %s', typ)
        return None

