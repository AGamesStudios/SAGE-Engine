"""Minimal alternative core without plugins or ECS."""
from .engine import Engine
from .scene import Scene
from .game_object import GameObject, Transform

__all__ = ["Engine", "Scene", "GameObject", "Transform"]
