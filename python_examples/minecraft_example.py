from ursina import *
from random import uniform
from panda3d.core import loadPrcFileData
from pathlib import Path

# Ensure window size uses integer values to avoid warnings
loadPrcFileData('', 'win-size 1536 864')

# Use a local icon if available to avoid missing icon warnings
ICON_PATH = Path(__file__).resolve().parent.parent / 'SAGE Engine' / 'Assets' / 'TutorialInfo' / 'Icons' / 'URP.png'
if ICON_PATH.exists():
    window.icon = ICON_PATH


def build_ground(size=16):
    for x in range(size):
        for z in range(size):
            Voxel(position=(x,0,z))


class Voxel(Button):
    def __init__(self, position=(0,0,0), texture='white_cube'):
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


if __name__ == '__main__':
    app = Ursina()
    Sky()
    build_ground()
    player = FirstPersonController()
    app.run()
