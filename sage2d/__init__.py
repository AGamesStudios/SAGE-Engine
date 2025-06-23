import sys
import json
import pygame

from sage_logic import (
    EventSystem,
    Event,
    KeyPressed,
    KeyReleased,
    MouseButton,
    Collision,
    Timer,
    Always,
    OnStart,
    EveryFrame,
    VariableEquals,
    Move,
    SetPosition,
    Destroy,
    Print,
    PlaySound,
    Spawn,
    SetVariable,
)

__all__ = [
    'GameObject', 'Scene', 'Engine', 'EventSystem',
    'Event', 'KeyPressed', 'KeyReleased', 'MouseButton', 'Collision', 'Timer',
    'Always',
    'Move', 'SetPosition', 'Destroy', 'Print', 'PlaySound', 'Spawn',
    'OnStart', 'EveryFrame', 'VariableEquals', 'SetVariable', 'main'
]


class GameObject:
    """Simple sprite-based object."""

    def __init__(self, image, x=0, y=0):
        self.image_path = image
        self.x = x
        self.y = y
        self.sprite = None  # lazily loaded when running the game
        self.events = []  # object specific events

    def update(self, dt):
        pass

    def draw(self, surface):
        self._ensure_sprite()
        surface.blit(self.sprite, (self.x, self.y))

    def rect(self):
        self._ensure_sprite()
        return self.sprite.get_rect(topleft=(self.x, self.y))

    def _ensure_sprite(self):
        if self.sprite is None:
            self.sprite = pygame.image.load(self.image_path).convert_alpha()


class Scene:
    def __init__(self):
        self.objects = []
        self.variables = {}

    def add_object(self, obj):
        self.objects.append(obj)

    def remove_object(self, obj):
        if obj in self.objects:
            self.objects.remove(obj)

    def update(self, dt):
        for obj in self.objects:
            obj.update(dt)

    def draw(self, surface):
        for obj in self.objects:
            obj.draw(surface)

    @classmethod
    def load(cls, path):
        with open(path, 'r') as f:
            data = json.load(f)
        scene = cls()
        scene.variables = data.get('variables', {})
        for entry in data.get('objects', []):
            obj = GameObject(entry['image'], entry.get('x', 0), entry.get('y', 0))
            obj.events = entry.get('events', [])
            scene.add_object(obj)
        return scene

    def save(self, path):
        data = {
            'variables': self.variables,
            'objects': [
                {
                    'image': o.image_path,
                    'x': o.x,
                    'y': o.y,
                    'events': getattr(o, 'events', []),
                }
                for o in self.objects
            ],
        }
        with open(path, 'w') as f:
            json.dump(data, f, indent=2)

    def build_event_system(self):
        es = EventSystem(variables=self.variables)
        for obj in self.objects:
            events = getattr(obj, 'events', [])
            if not isinstance(events, list):
                continue
            for evt in events:
                if not isinstance(evt, dict):
                    continue
                conditions = []
                for cond in evt.get('conditions', []):
                    if not isinstance(cond, dict):
                        continue
                    typ = cond.get('type')
                    if typ == 'KeyPressed':
                        conditions.append(KeyPressed(cond['key']))
                    elif typ == 'KeyReleased':
                        conditions.append(KeyReleased(cond['key']))
                    elif typ == 'MouseButton':
                        conditions.append(MouseButton(cond['button'], cond.get('state', 'down')))
                    elif typ == 'Timer':
                        conditions.append(Timer(cond['duration']))
                    elif typ == 'OnStart':
                        conditions.append(OnStart())
                    elif typ == 'EveryFrame':
                        conditions.append(EveryFrame())
                    elif typ == 'Collision':
                        a_idx = cond.get('a', -1)
                        b_idx = cond.get('b', -1)
                        if 0 <= a_idx < len(self.objects) and 0 <= b_idx < len(self.objects):
                            a = self.objects[a_idx]
                            b = self.objects[b_idx]
                            conditions.append(Collision(a, b))
                    elif typ == 'Always':
                        conditions.append(Always())
                    elif typ == 'VariableEquals':
                        conditions.append(VariableEquals(cond['name'], cond['value']))
                actions = []
                for act in evt.get('actions', []):
                    if not isinstance(act, dict):
                        continue
                    typ = act.get('type')
                    if typ == 'Move':
                        t = act.get('target')
                        if t is not None and 0 <= t < len(self.objects):
                            target = self.objects[t]
                            actions.append(Move(target, act['dx'], act['dy']))
                    elif typ == 'SetPosition':
                        t = act.get('target')
                        if t is not None and 0 <= t < len(self.objects):
                            target = self.objects[t]
                            actions.append(SetPosition(target, act['x'], act['y']))
                    elif typ == 'Destroy':
                        t = act.get('target')
                        if t is not None and 0 <= t < len(self.objects):
                            target = self.objects[t]
                            actions.append(Destroy(target))
                    elif typ == 'Print':
                        actions.append(Print(act['text']))
                    elif typ == 'PlaySound':
                        actions.append(PlaySound(act['path']))
                    elif typ == 'Spawn':
                        actions.append(Spawn(act['image'], act.get('x',0), act.get('y',0)))
                    elif typ == 'SetVariable':
                        actions.append(SetVariable(act['name'], act['value']))
                es.add_event(Event(conditions, actions, evt.get('once', False)))
        return es


class Engine:
    def __init__(self, width=640, height=480, scene=None, events=None):
        pygame.init()
        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption('SAGE 2D')
        self.clock = pygame.time.Clock()
        self.scene = scene or Scene()
        if events is not None:
            self.events = events
        else:
            self.events = self.scene.build_event_system()

    def run(self):
        running = True
        while running:
            dt = self.clock.tick(60) / 1000.0
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
            self.events.update(self, self.scene, dt)
            self.scene.update(dt)
            self.screen.fill((0, 0, 0))
            self.scene.draw(self.screen)
            pygame.display.flip()
        pygame.quit()


def main(argv=None):
    if argv is None:
        argv = sys.argv
    scene = Scene.load(argv[1]) if len(argv) > 1 else Scene()
    Engine(scene=scene, events=scene.build_event_system()).run()


if __name__ == '__main__':
    main()
