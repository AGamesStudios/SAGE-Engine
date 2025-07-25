from sage_fx import load_fx, apply_fx, FXParseError, optimize_ops, parse_fx
import os
import pytest


def test_parse_and_apply(tmp_path):
    fx_file = tmp_path / "glow.sage_fx"
    fx_file.write_text("""\
PASS
blit
blend_add factor:1.0
""", encoding="utf-8")
    fx = load_fx(str(fx_file))
    os.environ["FEATHER_FX_BACKEND"] = "gpu"
    result = apply_fx(None, fx)
    assert result[0] == "backend=gpu"
    assert result[-2:] == ["blit", "blend_add"]
    del os.environ["FEATHER_FX_BACKEND"]


def test_invalid_op(tmp_path):
    fx_file = tmp_path / "bad.sage_fx"
    fx_file.write_text("PASS\nunknown\n", encoding="utf-8")
    with pytest.raises(FXParseError):
        load_fx(str(fx_file))


def test_missing_argument(tmp_path):
    fx_file = tmp_path / "bad2.sage_fx"
    fx_file.write_text("PASS\noutline color:#fff\n", encoding="utf-8")
    with pytest.raises(FXParseError):
        load_fx(str(fx_file))


def test_optimize(tmp_path):
    fx_file = tmp_path / "chain.sage_fx"
    fx_file.write_text("""\
PASS
blit
noop
blit
""", encoding="utf-8")
    ops = parse_fx(str(fx_file))
    out = optimize_ops(ops)
    assert [op.name for op in out] == ["blit"]
