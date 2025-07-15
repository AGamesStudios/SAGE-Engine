import logging
import pytest
from sage_engine import patchers
from sage_engine.extras import lua


def test_load_python_patcher(tmp_path):
    file = tmp_path / "sample.py"
    file.write_text("def apply(tree):\n    tree.append('ok')\n")
    p = patchers.load_patcher(file)
    lst = []
    p.apply(lst)
    assert lst == ['ok']


@pytest.mark.skipif(not lua.AVAILABLE, reason="lupa not installed")
def test_load_lua_patcher(tmp_path):
    file = tmp_path / "sample.lua"
    file.write_text("return function(t) t.count = (t.count or 0) + 1 end")
    p = patchers.load_patcher(file)
    obj = {'count': 0}
    p.apply(obj)
    assert obj['count'] == 1


@pytest.mark.xfail(not lua.AVAILABLE, reason="lupa missing")
def test_lua_patcher_no_runtime(tmp_path, caplog):
    if lua.AVAILABLE:
        pytest.skip("requires missing lupa")
    file = tmp_path / "sample.lua"
    file.write_text("return function(t) t.count = 1 end")
    caplog.set_level(logging.WARNING)
    p = patchers.load_patcher(file)
    obj = {}
    p.apply(obj)
    assert obj == {}
    assert 'Lua runtime not available' in caplog.text

