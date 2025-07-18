from sage_object import object_from_dict
from sage_engine.object import (
    add_object,
    reset,
    get_children,
    get_parent,
    remove_object,
    cleanup,
    get_objects,
)


def test_parent_relationships():
    reset()
    parent = object_from_dict({"id": "p", "role": "Sprite"})
    child = object_from_dict({"id": "c", "role": "UI", "parent_id": "p"})
    add_object(parent)
    add_object(child)
    assert get_children("p") == [child]
    assert get_parent("c") is parent


def test_remove_parent_removes_children():
    reset()
    parent = object_from_dict({"id": "p2", "role": "Sprite"})
    child = object_from_dict({"id": "c2", "role": "Sprite", "parent_id": "p2"})
    add_object(parent)
    add_object(child)
    remove_object("p2")
    assert get_objects() == []


def test_cleanup():
    reset()
    obj = object_from_dict({"id": "dead", "role": "Sprite"})
    add_object(obj)
    obj.mark_for_removal()
    cleanup()
    assert get_objects() == []
