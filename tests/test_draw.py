import os
os.environ["SDL_VIDEODRIVER"] = "dummy"

import pygame
from sage_engine.draw import boot, reset, draw_line, draw_rect, draw_circle, get_calls


def test_draw_calls():
    pygame.display.init()
    pygame.display.set_mode((4, 4))
    boot()
    draw_line((0, 0), (1, 1))
    draw_rect((0, 0, 2, 2))
    draw_circle((1, 1), 1)
    calls = get_calls()
    assert ("line", (0, 0), (1, 1), (255, 255, 255), 1) in calls
    assert ("rect", (0, 0, 2, 2), (255, 255, 255), 1) in calls
    assert ("circle", (1, 1), 1, (255, 255, 255), 1) in calls
    pygame.display.quit()
    reset()
