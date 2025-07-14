import logging
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


def test_load_lua_with_runtime(tmp_path, monkeypatch):
    class FakeRuntime:
        def execute(self, code):
            def func(t):
                t.append('ok')
            return func
    monkeypatch.setattr(lua, 'AVAILABLE', True)
    monkeypatch.setattr(lua, 'get_runtime', lambda: FakeRuntime())
    file = tmp_path / "sample.lua"
    file.write_text("return function(t) table.insert(t, 'ok') end")
    p = patchers.load_patcher(file)
    lst = []
    p.apply(lst)
    assert lst == ['ok']


def test_load_lua_no_runtime(tmp_path, monkeypatch, caplog):
    monkeypatch.setattr(lua, 'AVAILABLE', False)
    monkeypatch.setattr(lua, 'get_runtime', lambda: None)
    file = tmp_path / 'sample.lua'
    file.write_text('return function(t) table.insert(t, "x") end')
    caplog.set_level(logging.WARNING)
    p = patchers.load_patcher(file)
    lst = []
    p.apply(lst)
    assert lst == []
    assert any('Lua runtime not available' in r.message for r in caplog.records)
    pytest.xfail('lua runtime unavailable')
