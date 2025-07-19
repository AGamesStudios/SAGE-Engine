from sage_engine.logic_api import (
    create_object,
    set_param,
    get_param,
    on_ready,
    on_update,
)
from sage_engine.lua_runner import run_lua_script, set_lua_globals
from sage import emit
from sage_engine.object import reset


def test_lua_on_ready(tmp_path):
    reset()
    script = tmp_path / "script.lua"
    script.write_text(
        """
        on_ready(function()
          create_object("enemy", "Sprite", {x = 0})
        end)
        """,
        encoding="utf-8",
    )
    set_lua_globals(
        create_object=create_object,
        set_param=set_param,
        get_param=get_param,
        on_ready=on_ready,
        on_update=on_update,
    )
    run_lua_script(str(script))
    emit("ready")
    assert get_param("enemy", "x") == 0
