"""Minimal usage example for SAGE Engine.

This script demonstrates loading configuration from ``sage_config.json`` and starting
Ursina with a single rotating cube. Edit the JSON file to tweak resolution,
lighting and other engine options.
"""

from ursina import Entity, color

from sage_engine import SageEngine, load_config


def setup() -> None:
    cube = Entity(model='cube', color=color.azure, rotation=(45, 45, 45))
    cube.animate('rotation_y', 360, duration=4, loop=True)


if __name__ == '__main__':
    SageEngine(load_config()).start(setup)
