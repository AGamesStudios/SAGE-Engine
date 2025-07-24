import os
os.environ["SDL_VIDEODRIVER"] = "dummy"

import pygame
from sage_engine.gizmo import draw_transform_gizmo, set_enabled
from sage_engine.draw import boot, get_calls, reset
from sage_object import object_from_dict


def test_gizmo_respects_enabled():
    pygame.display.init()
    pygame.display.set_mode((4, 4))
    boot()
    obj = object_from_dict({"role": "Sprite", "x": 1, "y": 1})
    set_enabled(True)
    draw_transform_gizmo(obj)
    assert get_calls()  # something was drawn
    reset()
    boot()
    set_enabled(False)
    draw_transform_gizmo(obj)
    assert get_calls() == []
    pygame.display.quit()
    reset()
