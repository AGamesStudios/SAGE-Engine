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
