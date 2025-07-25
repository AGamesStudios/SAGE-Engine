from sage_engine.logic_api import on_ready, on_update
from sage_fx import load_fx, apply_fx
from sage_engine.render import render_scene
from sage_engine.object import get_objects
from sage_engine.window import poll as poll_window, present as present_window
from sage_engine import framesync, time
import pygame
import os

_fx = None


def ready() -> None:
    global _fx
    path = os.path.join(os.path.dirname(__file__), "..", "glow_outline.sage_fx")
    _fx = load_fx(path)


def update(dt: float) -> None:
    poll_window()
    render_scene(get_objects())
    surf = pygame.display.get_surface()
    apply_fx(surf, _fx)
    present_window()
    framesync.regulate()
    time.mark()

on_ready(ready)
on_update(update)
