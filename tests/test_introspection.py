from sage_engine import core, world
import sage_engine.introspection as intros


def setup_module():
    world.reset()
    edit = world.scene.begin_edit()
    global OBJ_ID
    OBJ_ID = edit.create(role="sprite", name="hero")
    world.scene.apply(edit)
    world.scene.commit()


def test_list_and_get():
    api = core.get("introspection")
    objs = api.list_objects()
    assert OBJ_ID in objs
    params = api.get_parameters(OBJ_ID)
    assert params["role"] == "sprite"


def test_set_parameter():
    api = core.get("introspection")
    api.set_parameter(OBJ_ID, "sprite", "layer", 3)
    assert world.scene.storage["sprite"]["sprite"]["layer"][0] == 3


def test_describe_role():
    api = core.get("introspection")
    info = api.describe_role("sprite")
    assert info.get("name") == "Sprite"


def test_editor_flag():
    api = core.get("introspection")
    api.set_editor_flag("editor_mode", True)
    assert api.state.flags.get("editor_mode")
