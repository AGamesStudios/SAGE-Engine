# SAGE Logic - a simple condition/action event system
# Provides Clickteam-style logic for both 2D and 3D games

from ..utils.log import logger
from concurrent.futures import ThreadPoolExecutor
from importlib import metadata
from .. import lang as engine_lang
from collections import defaultdict
from threading import Lock

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

_PLUGINS_LOADED = False

# fallback dictionary mapping translated names back to the English
# identifiers used internally.  This lets old projects saved with
# localized names continue to load even when the editor is not
# installed.  ``engine.lang`` ships with basic translations and
# plugins may provide additional language packs.
TRANSLATION_LOOKUP: dict[str, str] = {}

LANGUAGES = {}
LANGUAGES.update(engine_lang.LANGUAGES)


for entries in LANGUAGES.values():
    for eng, local in entries.items():
        if eng != local:
            TRANSLATION_LOOKUP[local] = eng

def load_logic_plugins(*modules) -> None:
    """Import modules or entry points that register additional logic blocks."""
    global _PLUGINS_LOADED
    for name in modules:
        try:
            __import__(name)
        except Exception:
            logger.exception('Failed to load logic plugin %s', name)
    if not _PLUGINS_LOADED:
        try:
            eps = metadata.entry_points()
            entries = (
                eps.select(group="sage_engine.logic")
                if hasattr(eps, "select")
                else eps.get("sage_engine.logic", [])
            )
            for ep in entries:
                try:
                    ep.load()
                except Exception:
                    logger.exception('Failed to load logic plugin %s', ep.name)
        except Exception:
            logger.exception('Error loading logic entry points')
        _PLUGINS_LOADED = True

import re  # noqa: E402

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


class VariableStore(defaultdict):
    """Thread-safe mapping for event variables."""

    def __init__(self):
        super().__init__(int)
        self.lock = Lock()


# support engine attribute references like ``engine.attr`` or ``engine.method(args)``
# and shorthand variable syntax ``$name`` or ``{name}``
_ENGINE_REF_RE = re.compile(r"engine((?:\.\w+)+)(?:\((.*)\))?")
_VAR_SHORT_RE = re.compile(r"\$(\w+)|\{(\w+)\}")

def parse_value(val):
    """Return ``val`` or an :class:`EngineRef` if it matches the reference syntax.

    Lists and dictionaries are processed recursively so nested references work.
    """
    if isinstance(val, list):
        return [parse_value(v) for v in val]
    if isinstance(val, dict):
        return {k: parse_value(v) for k, v in val.items()}
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
    """Return the runtime value for ``val``.

    Lists and dictionaries are expanded recursively so complex parameters can
    contain references.
    """
    if isinstance(val, EngineRef):
        return val.get(engine)
    if isinstance(val, list):
        return [resolve_value(v, engine) for v in val]
    if isinstance(val, dict):
        return {k: resolve_value(v, engine) for k, v in val.items()}
    return val


def _auto_params(cls):
    """Return parameter metadata from ``cls.__init__`` if available."""
    from inspect import signature, Parameter
    meta = []
    try:
        sig = signature(cls.__init__)
    except (TypeError, ValueError):
        return meta
    for p in list(sig.parameters.values())[1:]:  # skip ``self``
        if p.kind in (Parameter.VAR_POSITIONAL, Parameter.VAR_KEYWORD):
            continue
        meta.append((p.name, 'value'))
    return meta


def register_condition(name: str, params: list[tuple] | None = None):
    """Decorator to register a Condition subclass.

    ``params`` defines a list of tuples describing constructor arguments.
    Entries are ``(arg_name, kind[, key[, types]])`` where ``kind`` is
    ``'value'`` for plain values, ``'object'`` to look up a scene object by
    index and ``'variable'`` for variables. ``key`` is the dictionary key when
    loading, defaulting to ``arg_name``. ``types`` optionally lists allowed
    object types when ``kind`` is ``'object'``. When ``params`` is omitted the
    ``__init__`` signature is inspected and each argument defaults to a
    ``'value'`` entry, allowing quick creation of simple logic blocks.
    """

    def decorator(cls):
        CONDITION_REGISTRY[name] = cls
        CONDITION_META[name] = params or _auto_params(cls)
        return cls

    return decorator


def register_action(name: str, params: list[tuple] | None = None):
    """Decorator to register an Action subclass with optional metadata.

    ``params`` follows the same format as :func:`register_condition`. If omitted
    the ``__init__`` signature is inspected to build simple ``'value'``
    parameters automatically.
    """

    def decorator(cls):
        ACTION_REGISTRY[name] = cls
        ACTION_META[name] = params or _auto_params(cls)
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
    """Base condition interface.

    Subclasses may override :meth:`reset` to clear any internal state when an
    event is reset.  This keeps conditions reusable and ensures predictable
    behaviour when events are toggled on or off at runtime.
    """

    def check(self, engine, scene, dt):
        raise NotImplementedError

    def reset(self):  # pragma: no cover - optional
        """Reset internal state."""
        pass

class Action:
    """Base action interface.

    Actions may also override :meth:`reset` in case they maintain temporary
    state that should be cleared when the owning event is reset.
    """

    def execute(self, engine, scene, dt):
        raise NotImplementedError

    def reset(self):  # pragma: no cover - optional
        pass

class Event:
    """Combination of conditions and actions."""

    def __init__(
        self,
        conditions,
        actions,
        name: str | None = None,
        enabled: bool = True,
        priority: int = 0,
        groups: list[str] | None = None,
    ):
        """Create a new event instance.

        When ``conditions`` is empty the ``actions`` will execute every frame
        until the event is disabled.
        """
        self.conditions = conditions
        self.actions = actions
        self.name = name
        self.enabled = enabled
        self.priority = priority
        self.groups = set(groups or [])

    def enable(self):
        self.enabled = True
        if self.name:
            logger.debug('Enabled event %s', self.name)

    def disable(self):
        self.enabled = False
        if self.name:
            logger.debug('Disabled event %s', self.name)

    def reset(self):
        """Enable the event and reset all conditions and actions."""
        self.enabled = True
        for cond in self.conditions:
            if hasattr(cond, 'reset'):
                try:
                    cond.reset()
                except Exception:
                    logger.exception('Condition reset error')
        for act in self.actions:
            if hasattr(act, 'reset'):
                try:
                    act.reset()
                except Exception:
                    logger.exception('Action reset error')
        if self.name:
            logger.debug('Reset event %s', self.name)

    def update(self, engine, scene, dt):
        if not self.enabled:
            return
        if self.conditions:
            try:
                if all(cond.check(engine, scene, dt) for cond in self.conditions):
                    for action in self.actions:
                        try:
                            action.execute(engine, scene, dt)
                        except Exception:
                            logger.exception('Action error')
                    if self.name:
                        logger.debug('Event %s triggered', self.name)
            except Exception:
                logger.exception('Condition error')
        else:
            for action in self.actions:
                try:
                    action.execute(engine, scene, dt)
                except Exception:
                    logger.exception('Action error')
            if self.name:
                logger.debug('Event %s triggered', self.name)

class EventSystem:
    """Container for events."""

    def __init__(self, variables=None, engine_version: str | None = None):
        from .. import ENGINE_VERSION
        self.events: list[Event] = []
        if variables is None:
            variables = VariableStore()
        self.variables = variables
        self.groups: dict[str, list[Event]] = {}
        self.engine_version = engine_version or ENGINE_VERSION
        self.paused = False
        self._executor: ThreadPoolExecutor | None = None
        self._worker_count = 0

    def get_event(self, name):
        for evt in self.events:
            if evt.name == name:
                return evt
        return None

    def get_event_names(self):
        """Return the list of names for all registered events."""
        return [e.name for e in self.events if e.name]

    def remove_event(self, name):
        evt = self.get_event(name)
        if evt is None:
            return
        self.events.remove(evt)
        for g in list(evt.groups):
            if g in self.groups:
                try:
                    self.groups[g].remove(evt)
                    if not self.groups[g]:
                        del self.groups[g]
                except ValueError:
                    pass
        logger.debug('Removed event %s', name)

    def add_event(self, event: Event):
        self.events.append(event)
        for g in event.groups:
            self.groups.setdefault(g, []).append(event)
        self.events.sort(key=lambda e: e.priority)
        logger.debug('Added event %s', event.name)

    def enable_event(self, name):
        evt = self.get_event(name)
        if evt is not None:
            evt.enable()
            logger.debug('Enabled event %s', name)

    def disable_event(self, name):
        evt = self.get_event(name)
        if evt is not None:
            evt.disable()
            logger.debug('Disabled event %s', name)

    def reset_event(self, name):
        evt = self.get_event(name)
        if evt is not None:
            evt.reset()
            logger.debug('Reset event %s', name)

    def reset_all(self):
        for evt in self.events:
            evt.reset()
        logger.debug('Reset all events')

    def pause(self) -> None:
        """Pause all events without clearing state."""
        self.paused = True

    def resume(self) -> None:
        """Resume event processing."""
        self.paused = False

    # group helpers -----------------------------------------------------

    def get_group_names(self) -> list[str]:
        """Return the names of all registered groups."""
        return list(self.groups.keys())

    def get_group_events(self, name: str) -> list[Event]:
        """Return the list of events in the given group."""
        return list(self.groups.get(name, []))

    def enable_group(self, name: str):
        for evt in self.groups.get(name, []):
            evt.enable()
        if name in self.groups:
            logger.debug('Enabled group %s', name)

    def disable_group(self, name: str):
        for evt in self.groups.get(name, []):
            evt.disable()
        if name in self.groups:
            logger.debug('Disabled group %s', name)

    def reset_group(self, name: str):
        for evt in self.groups.get(name, []):
            evt.reset()
        if name in self.groups:
            logger.debug('Reset group %s', name)

    def update_group(self, name: str, engine, scene, dt: float) -> None:
        """Update only events in ``name``."""
        for evt in list(self.groups.get(name, [])):
            evt.update(engine, scene, dt)

    def update_async(self, engine, scene, dt: float, *, workers: int = 4) -> None:
        """Update events concurrently using a thread pool."""
        if self.paused or not self.events:
            return
        if self._executor is None or self._worker_count != workers:
            if self._executor is not None:
                self._executor.shutdown(wait=False)
            self._executor = ThreadPoolExecutor(max_workers=workers)
            self._worker_count = workers
        futures = [
            self._executor.submit(evt.update, engine, scene, dt)
            for evt in list(self.events)
        ]
        for fut in futures:
            fut.result()

    def shutdown(self) -> None:
        if self._executor is not None:
            self._executor.shutdown(wait=False)
            self._executor = None
            self._worker_count = 0

    def update(self, engine, scene, dt):
        if self.paused or not self.events:
            return
        for evt in list(self.events):
            evt.update(engine, scene, dt)


def condition_from_dict(data, objects, variables):
    """Instantiate a condition from its dictionary description."""
    typ = data.get('type')
    if typ in TRANSLATION_LOOKUP:
        typ = TRANSLATION_LOOKUP[typ]
    cls = CONDITION_REGISTRY.get(typ)
    if cls is None:
        raise ValueError(f'Unknown condition type {typ}')
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
                    raise ValueError(f'Condition {typ} has invalid object index for {arg}')
            else:
                obj = objects[val]
            if allowed:
                from ..core.objects import get_object_type
                typ_name = get_object_type(obj)
                if typ_name not in allowed:
                    raise ValueError(f'Condition {typ} requires {allowed} for {arg}')
            params[arg] = obj
        elif kind == 'variable':
            params[arg] = VarRef(val) if isinstance(val, str) else val
        else:
            params[arg] = parse_value(val)
    try:
        return cls(**params)
    except Exception as exc:
        logger.exception('Failed to construct condition %s', typ)
        raise ValueError(f'Failed to construct condition {typ}') from exc


def action_from_dict(data, objects):
    """Instantiate an action from its dictionary description."""
    typ = data.get('type')
    if typ in TRANSLATION_LOOKUP:
        typ = TRANSLATION_LOOKUP[typ]
    cls = ACTION_REGISTRY.get(typ)
    if cls is None:
        raise ValueError(f'Unknown action type {typ}')
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
                    raise ValueError(f'Action {typ} has invalid object index for {arg}')
            else:
                obj = objects[val]
            if allowed:
                from ..core.objects import get_object_type
                typ_name = get_object_type(obj)
                if typ_name not in allowed:
                    raise ValueError(f'Action {typ} requires {allowed} for {arg}')
            params[arg] = obj
        elif kind == 'variable':
            params[arg] = VarRef(val) if isinstance(val, str) else val
        else:
            params[arg] = parse_value(val)
    try:
        return cls(**params)
    except Exception as exc:
        logger.exception('Failed to construct action %s', typ)
        raise ValueError(f'Failed to construct action {typ}') from exc


def event_from_dict(data, objects, variables):
    """Instantiate an :class:`Event` from a dictionary."""
    conditions = []
    for cond in data.get('conditions', []):
        if not isinstance(cond, dict):
            continue
        conditions.append(condition_from_dict(cond, objects, variables))
    actions = []
    for act in data.get('actions', []):
        if not isinstance(act, dict):
            continue
        actions.append(action_from_dict(act, objects))
    return Event(
        conditions,
        actions,
        data.get('name'),
        data.get('enabled', True),
        data.get('priority', 0),
        data.get('groups', []),
    )

