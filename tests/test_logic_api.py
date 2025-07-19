from sage_engine.logic_api import (
    create_object,
    set_param,
    get_param,
    destroy_object,
)
from sage_engine.object import reset, get_object


def test_create_set_destroy():
    reset()
    create_object("t1", "Sprite", {"image": "hero.png"})
    assert get_object("t1") is not None
    set_param("t1", "x", 5)
    assert get_param("t1", "x") == 5
    destroy_object("t1")
    assert get_object("t1") is None
