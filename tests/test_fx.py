from sage_fx import load_fx, apply_fx, FXParseError
import pygame


def test_parse_and_apply(tmp_path):
    fx_file = tmp_path / "glow.sage_fx"
    fx_file.write_text("""\
PASS
blit
blend_add factor:1.0
""", encoding="utf-8")
    fx = load_fx(str(fx_file))
    pygame.display.init()
    pygame.display.set_mode((4, 4))
    surf = pygame.display.get_surface()
    result = apply_fx(surf, fx)
    assert result[0].startswith("backend=")
    assert result[1:] == ["blit", "blend_add"]
    pygame.display.quit()


def test_invalid_op(tmp_path):
    fx_file = tmp_path / "bad.sage_fx"
    fx_file.write_text("PASS\nunknown\n", encoding="utf-8")
    try:
        load_fx(str(fx_file))
    except FXParseError:
        pass
    else:
        assert False, "Should raise FXParseError"
