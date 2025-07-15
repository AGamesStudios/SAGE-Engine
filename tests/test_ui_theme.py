from sage_engine import ui


def test_theme_switch(tmp_path):
    btn = ui.Button()
    initial = btn.bg_color
    cfg = tmp_path / "new.vel"
    cfg.write_text(
        """colors:\n  bg: '#123456'\n  fg: '#ffffff'\nfont:\n  family: Roboto\n  size: 12\nradius: 4\n"""
    )
    ui.theme.set_theme(str(cfg))
    assert btn.bg_color == "#123456" and btn.bg_color != initial
