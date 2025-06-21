from ursina import *
from random import uniform
from pathlib import Path

from sage_engine import SageEngine, EngineConfig, load_config


def build_ground(size=16):
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
        if self.hovered:
            if key == 'left mouse down':
                Voxel(position=self.position + mouse.normal)
            if key == 'right mouse down':
                destroy(self)


def setup():
    Sky()
    build_ground()
    FirstPersonController()


if __name__ == '__main__':
    config_path = Path(__file__).resolve().parent.parent / 'sage_config.json'
    config = load_config(config_path)
    SageEngine(config).start(setup)
