"""Small Minecraft-like example using SAGE Engine."""

from ursina import *
from random import uniform

from ursina.prefabs.first_person_controller import FirstPersonController
from sage_engine import SageEngine, load_config

TEXTURES = [
    'grass',
    'white_cube',
    'brick'
]
current_block = 0
held_cube = None


def update_held_cube() -> None:
    """Refresh the cube model shown in the player's hand."""
    global held_cube
    if held_cube:
        held_cube.texture = TEXTURES[current_block]

def build_ground(size: int = 32) -> None:
    for x in range(size):
        for z in range(size):
            Voxel(position=(x, 0, z))


class Voxel(Button):
    def __init__(self, position=(0, 0, 0), texture='white_cube'):
        super().__init__(
            parent=scene,
            position=position,
            model='cube',
            origin_y=0.5,
            texture=texture,
            color=color.hsv(0, 0, uniform(0.9, 1)),
            scale=1,
        )

    def input(self, key):
        global current_block
        if self.hovered:
            if key == 'left mouse down':
                Voxel(position=self.position + mouse.normal, texture=TEXTURES[current_block])
            if key == 'right mouse down':
                fade = Entity(model='cube', position=self.position, color=self.color, texture=self.texture, scale=self.scale)
                fade.animate_scale(0.1, duration=0.15)
                destroy(fade, delay=0.15)
                destroy(self)


def input(key):
    global current_block
    if key == 'scroll up' or key == 'scroll down':
        if key == 'scroll up':
            current_block = (current_block + 1) % len(TEXTURES)
        else:
            current_block = (current_block - 1) % len(TEXTURES)
        update_held_cube()


def setup():
    Sky()
    build_ground()
    FirstPersonController()
    global held_cube
    held_cube = Entity(
        parent=camera.ui,
        model='cube',
        texture=TEXTURES[current_block],
        scale=0.2,
        position=(0.7, -0.6),
        rotation=Vec3(-20, 40, 20)
    )


if __name__ == '__main__':
    engine = SageEngine(load_config())
    engine.start(setup)
