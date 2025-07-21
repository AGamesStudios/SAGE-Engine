from pathlib import Path
import pygame
from sage_fx import load_fx, apply_fx


def test_fx_demo_screenshot(tmp_path):
    fx = load_fx(Path("examples/fx_lab/glow_outline.sage_fx"))
    pygame.display.init()
    pygame.display.set_mode((2, 2))
    surf = pygame.display.get_surface()
    surf.fill((20, 40, 60))
    apply_fx(surf, fx)
    data = pygame.image.tostring(surf, "RGBA")
    assert set(data) != {0}
    pygame.display.quit()
