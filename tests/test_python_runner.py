from sage_engine.python_runner import run_python_script, set_python_globals
from sage_engine.object import get_objects, add_object, reset
from sage_object import object_from_dict
import pytest


def test_run_python_script(tmp_path, capsys):
    script = tmp_path / "hello.py"
    script.write_text("print('py!')", encoding="utf-8")
    run_python_script(str(script))


def test_python_can_create_object(tmp_path):
    reset()

    def create_obj(role, name):
        add_object(object_from_dict({"role": role, "id": name}))

    set_python_globals(create_object=create_obj)
    script = tmp_path / "obj.py"
    script.write_text("create_object('Sprite', 'hero')", encoding='utf-8')
    run_python_script(str(script))
    assert any(o.id == 'hero' for o in get_objects())


def test_import_blocked(tmp_path):
    script = tmp_path / "bad.py"
    script.write_text("import os", encoding="utf-8")
    with pytest.raises(ImportError):
        run_python_script(str(script))


def test_from_import_allowed(tmp_path):
    script = tmp_path / "good.py"
    script.write_text("from math import sin\nassert sin(0) == 0", encoding="utf-8")
    run_python_script(str(script))


def test_from_import_blocked(tmp_path):
    script = tmp_path / "bad.py"
    script.write_text("from os import system", encoding="utf-8")
    with pytest.raises(ImportError):
        run_python_script(str(script))


def test_custom_allowed_module(tmp_path, monkeypatch):
    script = tmp_path / "custom.py"
    script.write_text("import textwrap", encoding="utf-8")

    def fake_config():
        return {"enable_python": True, "allowed_modules": ["textwrap"]}

    monkeypatch.setattr("sage.config.load_config", fake_config)
    monkeypatch.setattr("sage_engine.python_runner.load_config", fake_config)
    run_python_script(str(script))

def test_builtins_blocked(tmp_path):
    script = tmp_path / "bad.py"
    script.write_text("open('x')", encoding="utf-8")
    with pytest.raises(Exception):
        run_python_script(str(script))


def test_env_reset(tmp_path):
    script = tmp_path / "s1.py"
    script.write_text("a = 1", encoding="utf-8")
    run_python_script(str(script))
    script2 = tmp_path / "s2.py"
    script2.write_text("a", encoding="utf-8")
    with pytest.raises(NameError):
        run_python_script(str(script2))


def test_import_from_logic_api(tmp_path):
    script = tmp_path / "api.py"
    script.write_text(
        "from sage_engine.logic_api import on_ready\n"
        "def cb():\n    pass\n"
        "on_ready(cb)\n",
        encoding="utf-8",
    )
    run_python_script(str(script))


def test_import_input_from_logic_api(tmp_path):
    script = tmp_path / "input.py"
    script.write_text(
        "from sage_engine.logic_api import input\ninput.is_key_down('A')",
        encoding="utf-8",
    )
    run_python_script(str(script))


def test_lambda_allowed(tmp_path):
    script = tmp_path / "lam.py"
    script.write_text(
        "f = lambda x: x * 2\nassert f(3) == 6",
        encoding="utf-8",
    )
    run_python_script(str(script))

