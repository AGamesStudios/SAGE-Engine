from sage_engine.lua_runner import run_lua_script, set_lua_globals
from sage_engine.object import get_objects, add_object, reset
from sage_object import object_from_dict


def test_run_lua_script(tmp_path, capsys):
    script = tmp_path / "hello.lua"
    script.write_text("print('lua!')", encoding="utf-8")
    run_lua_script(str(script))


def test_lua_can_create_object(tmp_path):
    reset()

    def create_obj(role, name):
        add_object(object_from_dict({"role": role, "id": name}))

    set_lua_globals(create_object=create_obj)
    script = tmp_path / "obj.lua"
    script.write_text("create_object('Sprite', 'hero')", encoding="utf-8")
    run_lua_script(str(script))
    assert any(o.id == "hero" for o in get_objects())
