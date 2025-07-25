from pathlib import Path
from sage_fx import load_fx, apply_fx


def test_fx_demo_screenshot(tmp_path):
    fx = load_fx(Path("examples/fx_lab/glow_outline.sage_fx"))
    log = apply_fx(None, fx)
    assert "blend_add" in log
