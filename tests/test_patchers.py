import pytest
from engine import patchers
from engine.extras import lua


def test_load_python_patcher(tmp_path):
    file = tmp_path / "sample.py"
    file.write_text("def apply(tree):\n    tree.append('ok')\n")
    p = patchers.load_patcher(file)
    lst = []
    p.apply(lst)
    assert lst == ['ok']


def test_load_lua_patcher(tmp_path):
    file = tmp_path / "sample.lua"
    file.write_text("return function(t) table.insert(t, 'ok') end")
    if not lua.AVAILABLE:
        pytest.xfail('lua runtime unavailable')
    p = patchers.load_patcher(file)
    lst = []
    p.apply(lst)
    assert lst == ['ok']
