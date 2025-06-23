# SAGE Logic - a simple condition/action event system
# Provides Clickteam-style logic for both 2D and 3D games

import pygame

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
    def __init__(self):
        self.events = []

    def add_event(self, event):
        self.events.append(event)

    def update(self, engine, scene, dt):
        for evt in list(self.events):
            evt.update(engine, scene, dt)

# Built-in conditions
class KeyPressed(Condition):
    def __init__(self, key):
        self.key = key

    def check(self, engine, scene, dt):
        keys = pygame.key.get_pressed()
        return keys[self.key]

class Collision(Condition):
    def __init__(self, obj_a, obj_b):
        self.obj_a = obj_a
        self.obj_b = obj_b

    def check(self, engine, scene, dt):
        return self.obj_a.rect().colliderect(self.obj_b.rect())

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

# Built-in actions
class Move(Action):
    def __init__(self, obj, dx, dy):
        self.obj = obj
        self.dx = dx
        self.dy = dy

    def execute(self, engine, scene, dt):
        self.obj.x += self.dx
        self.obj.y += self.dy

class SetPosition(Action):
    def __init__(self, obj, x, y):
        self.obj = obj
        self.x = x
        self.y = y

    def execute(self, engine, scene, dt):
        self.obj.x = self.x
        self.obj.y = self.y

class Destroy(Action):
    def __init__(self, obj):
        self.obj = obj

    def execute(self, engine, scene, dt):
        if hasattr(scene, 'remove_object'):
            scene.remove_object(self.obj)

class Print(Action):
    def __init__(self, text):
        self.text = text

    def execute(self, engine, scene, dt):
        print(self.text)

__all__ = [
    'Condition', 'Action', 'Event', 'EventSystem',
    'KeyPressed', 'Collision', 'Timer',
    'Move', 'SetPosition', 'Destroy', 'Print'
]
