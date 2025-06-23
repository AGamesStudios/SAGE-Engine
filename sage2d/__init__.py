import sys
import json
import pygame

from sage_logic import (
    EventSystem,
    Event,
    KeyPressed,
    Collision,
    Timer,
    Move,
    SetPosition,
    Destroy,
    Print,
)

__all__ = [
    'GameObject', 'Scene', 'Engine', 'EventSystem',
    'Event', 'KeyPressed', 'Collision', 'Timer',
    'Move', 'SetPosition', 'Destroy', 'Print', 'main'
]


class GameObject:
    """Simple sprite-based object."""

    def __init__(self, image, x=0, y=0):
        self.image_path = image
        self.x = x
        self.y = y
        self.sprite = None  # lazily loaded when running the game

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
        for entry in data.get('objects', []):
            obj = GameObject(entry['image'], entry.get('x', 0), entry.get('y', 0))
            scene.add_object(obj)
        return scene

    def save(self, path):
        data = {
            'objects': [
                {'image': o.image_path, 'x': o.x, 'y': o.y} for o in self.objects
            ]
        }
        with open(path, 'w') as f:
            json.dump(data, f, indent=2)


class Engine:
    def __init__(self, width=640, height=480, scene=None, events=None):
        pygame.init()
        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption('SAGE 2D')
        self.clock = pygame.time.Clock()
        self.scene = scene or Scene()
        self.events = events or EventSystem()

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
    Engine(scene=scene).run()


if __name__ == '__main__':
    main()
