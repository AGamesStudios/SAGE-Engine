# SAGE Logic - a simple condition/action event system
# Provides Clickteam-style logic for both 2D and 3D games

import traceback

# registries used to map names to classes so new logic blocks can be added
# without modifying the loader code
CONDITION_REGISTRY: dict[str, type] = {}
ACTION_REGISTRY: dict[str, type] = {}


def register_condition(name: str):
    """Decorator to register a Condition subclass."""

    def decorator(cls):
        CONDITION_REGISTRY[name] = cls
        return cls

    return decorator


def register_action(name: str):
    """Decorator to register an Action subclass."""

    def decorator(cls):
        ACTION_REGISTRY[name] = cls
        return cls

    return decorator


def get_registered_conditions() -> list[str]:
    """Return the list of available condition names."""
    return list(CONDITION_REGISTRY.keys())


def get_registered_actions() -> list[str]:
    """Return the list of available action names."""
    return list(ACTION_REGISTRY.keys())

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
                    except Exception as exc:
                        print(f'Action error: {exc}')
                        traceback.print_exc()
                self.triggered = True
        except Exception as exc:
            print(f'Condition error: {exc}')
            traceback.print_exc()

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
        return None
    try:
        if typ == 'Collision':
            a = data.get('a'); b = data.get('b')
            if 0 <= a < len(objects) and 0 <= b < len(objects):
                return cls(objects[a], objects[b])
            return None
        if typ in ('KeyPressed', 'KeyReleased'):
            return cls(data['key'])
        if typ == 'MouseButton':
            return cls(data['button'], data.get('state', 'down'))
        if typ == 'InputState':
            return cls(data.get('device', 'keyboard'), data.get('code'), data.get('state', 'down'))
        if typ == 'AfterTime':
            return cls(
                data.get('seconds', 0.0),
                data.get('minutes', 0.0),
                data.get('hours', 0.0),
            )
        if typ == 'VariableCompare':
            return cls(data['name'], data.get('op', '=='), data.get('value'))
        return cls()
    except Exception:
        return None


def action_from_dict(data, objects):
    """Instantiate an action from its dictionary description."""
    typ = data.get('type')
    cls = ACTION_REGISTRY.get(typ)
    if cls is None:
        return None
    try:
        if typ in ('Move', 'SetPosition', 'Destroy'):
            t = data.get('target')
            if t is None or t < 0 or t >= len(objects):
                return None
            target = objects[t]
            if typ == 'Move':
                return cls(target, data.get('dx', 0), data.get('dy', 0))
            if typ == 'SetPosition':
                return cls(target, data.get('x', 0), data.get('y', 0))
            return cls(target)
        if typ == 'Print':
            return cls(data.get('text', ''))
        if typ == 'PlaySound':
            return cls(data.get('path', ''))
        if typ == 'Spawn':
            return cls(data.get('image', ''), data.get('x', 0), data.get('y', 0))
        if typ == 'SetVariable':
            return cls(data.get('name'), data.get('value'))
        if typ == 'ModifyVariable':
            return cls(data.get('name'), data.get('op', '+'), data.get('value', 0))
        return cls()
    except Exception:
        return None

