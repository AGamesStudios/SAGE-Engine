import os
os.environ["SDL_VIDEODRIVER"] = "dummy"

import pygame
from sage_engine.render import boot, set_clear_color, render_scene


def test_set_clear_color():
    pygame.display.init()
    pygame.display.set_mode((4, 4))
    boot()
    set_clear_color(10, 20, 30, 255)
    render_scene([])
    surf = pygame.display.get_surface()
    assert surf.get_at((0, 0)) == pygame.Color(10, 20, 30, 255)
    pygame.display.quit()
