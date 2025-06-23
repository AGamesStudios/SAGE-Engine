# SAGE Logic - a simple condition/action event system
# Provides Clickteam-style logic for both 2D and 3D games

import pygame

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
        if all(cond.check(engine, scene, dt) for cond in self.conditions):
            for action in self.actions:
                action.execute(engine, scene, dt)
            self.triggered = True

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
        if typ == 'Timer':
            return cls(data['duration'])
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

# Built-in conditions
@register_condition('KeyPressed')
class KeyPressed(Condition):
    def __init__(self, key):
        self.key = key

    def check(self, engine, scene, dt):
        keys = pygame.key.get_pressed()
        return keys[self.key]

@register_condition('Collision')
class Collision(Condition):
    def __init__(self, obj_a, obj_b):
        self.obj_a = obj_a
        self.obj_b = obj_b

    def check(self, engine, scene, dt):
        return self.obj_a.rect().colliderect(self.obj_b.rect())

@register_condition('Timer')
class Timer(Condition):
    """True every `duration` seconds."""
    def __init__(self, duration):
        self.duration = duration
        self.elapsed = 0.0

    def check(self, engine, scene, dt):
        self.elapsed += dt
        if self.elapsed >= self.duration:
            self.elapsed -= self.duration
            return True
        return False

@register_condition('KeyReleased')
class KeyReleased(Condition):
    """True once when the key transitions from pressed to released."""

    def __init__(self, key):
        self.key = key
        self.prev = False

    def check(self, engine, scene, dt):
        keys = pygame.key.get_pressed()
        pressed = keys[self.key]
        result = self.prev and not pressed
        self.prev = pressed
        return result

@register_condition('MouseButton')
class MouseButton(Condition):
    """Check mouse button state ('down' or 'up')."""

    def __init__(self, button, state='down'):
        self.button = button
        self.state = state
        self.prev = False

    def check(self, engine, scene, dt):
        buttons = pygame.mouse.get_pressed()
        pressed = buttons[self.button - 1]
        if self.state == 'down':
            return pressed
        else:
            result = self.prev and not pressed
            self.prev = pressed
            return result

@register_condition('Always')
class Always(Condition):
    """Condition that is always true."""

    def check(self, engine, scene, dt):
        return True

@register_condition('OnStart')
class OnStart(Condition):
    """True only on the first frame."""
    def __init__(self):
        self.triggered = False

    def check(self, engine, scene, dt):
        if not self.triggered:
            self.triggered = True
            return True
        return False

@register_condition('EveryFrame')
class EveryFrame(Condition):
    """Alias for Always to clarify intent."""
    def check(self, engine, scene, dt):
        return True

@register_condition('VariableCompare')
class VariableCompare(Condition):
    """Compare a variable to a value using an operator."""
    OPS = {
        '==': lambda a, b: a == b,
        '!=': lambda a, b: a != b,
        '<': lambda a, b: a < b,
        '<=': lambda a, b: a <= b,
        '>': lambda a, b: a > b,
        '>=': lambda a, b: a >= b,
    }

    def __init__(self, name, op, value):
        self.name = name
        self.op = op
        self.value = value

    def check(self, engine, scene, dt):
        val = engine.events.variables.get(self.name)
        cmp = self.OPS.get(self.op, lambda a, b: False)
        try:
            if isinstance(val, (int, float)):
                return cmp(float(val), float(self.value))
            if isinstance(val, bool):
                if self.op not in ('==', '!='):
                    return False
                ref = self.value
                if isinstance(ref, str):
                    ref = ref.lower() in ('true', '1', 'yes')
                return cmp(val, bool(ref))
            # strings or other types use string comparison for == and !=
            if self.op in ('==', '!='):
                return cmp(str(val), str(self.value))
        except Exception:
            pass
        return False

# Built-in actions
@register_action('Move')
class Move(Action):
    def __init__(self, obj, dx, dy):
        self.obj = obj
        self.dx = dx
        self.dy = dy

    def execute(self, engine, scene, dt):
        self.obj.x += self.dx
        self.obj.y += self.dy

@register_action('SetPosition')
class SetPosition(Action):
    def __init__(self, obj, x, y):
        self.obj = obj
        self.x = x
        self.y = y

    def execute(self, engine, scene, dt):
        self.obj.x = self.x
        self.obj.y = self.y

@register_action('Destroy')
class Destroy(Action):
    def __init__(self, obj):
        self.obj = obj

    def execute(self, engine, scene, dt):
        if hasattr(scene, 'remove_object'):
            scene.remove_object(self.obj)

@register_action('Print')
class Print(Action):
    def __init__(self, text):
        self.text = text

    def execute(self, engine, scene, dt):
        try:
            msg = self.text.format(**engine.events.variables)
        except Exception:
            msg = self.text
        print(msg)

_SOUND_CACHE = {}

@register_action('PlaySound')
class PlaySound(Action):
    """Play a sound file using pygame.mixer."""

    def __init__(self, path):
        self.path = path

    def execute(self, engine, scene, dt):
        sound = _SOUND_CACHE.get(self.path)
        if sound is None:
            try:
                if pygame.mixer.get_init() is None:
                    pygame.mixer.init()
                sound = pygame.mixer.Sound(self.path)
                _SOUND_CACHE[self.path] = sound
            except pygame.error as exc:
                print(f'Failed to load sound {self.path}: {exc}')
                return
        try:
            sound.play()
        except pygame.error as exc:
            print(f'Failed to play sound {self.path}: {exc}')

@register_action('Spawn')
class Spawn(Action):
    """Spawn a new GameObject into the scene."""

    def __init__(self, image, x=0, y=0):
        self.image = image
        self.x = x
        self.y = y

    def execute(self, engine, scene, dt):
        try:
            from sage2d import GameObject
        except Exception:
            return
        obj = GameObject(self.image, self.x, self.y)
        if hasattr(scene, 'add_object'):
            scene.add_object(obj)

@register_action('SetVariable')
class SetVariable(Action):
    """Set a variable in the event system."""
    def __init__(self, name, value):
        self.name = name
        self.value = value

    def execute(self, engine, scene, dt):
        engine.events.variables[self.name] = self.value

@register_action('ModifyVariable')
class ModifyVariable(Action):
    """Modify a numeric variable with an operation."""

    OPS = {
        '+': lambda a, b: a + b,
        '-': lambda a, b: a - b,
        '*': lambda a, b: a * b,
        '/': lambda a, b: a / b if b != 0 else a,
    }

    def __init__(self, name, op, value):
        self.name = name
        self.op = op
        self.value = value

    def execute(self, engine, scene, dt):
        cur = engine.events.variables.get(self.name)
        if not isinstance(cur, (int, float)):
            print(f'Variable {self.name} is not numeric; ModifyVariable skipped')
            return
        try:
            cur = float(cur)
            val = float(self.value)
            func = self.OPS.get(self.op, lambda a, b: a)
            engine.events.variables[self.name] = func(cur, val)
        except Exception:
            print(f'Failed to modify variable {self.name}')

__all__ = [
    'Condition', 'Action', 'Event', 'EventSystem',
    'register_condition', 'register_action',
    'condition_from_dict', 'action_from_dict',
    'KeyPressed', 'KeyReleased', 'MouseButton', 'Collision', 'Timer', 'Always',
    'OnStart', 'EveryFrame', 'VariableCompare',
    'Move', 'SetPosition', 'Destroy', 'Print', 'PlaySound', 'Spawn',
    'SetVariable', 'ModifyVariable'
]
