from sage_object import object_from_dict
from sage_engine.scene import (
    add_object,
    reset,
    get_children,
    get_parent,
    remove_object,
    cleanup,
    get_objects,
    iter_dag,
    serialize,
    load_scene,
    get_object,
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


def test_serialize_and_load(tmp_path):
    reset()
    obj = object_from_dict({"id": "o1", "role": "Sprite"})
    add_object(obj)
    data = serialize(get_objects())
    file = tmp_path / "scene.sage_scene"
    file.write_text(data)
    reset()
    load_scene(str(file))
    assert get_object("o1") is not None


def test_iter_dag_order():
    reset()
    parent = object_from_dict({"id": "p", "role": "Sprite"})
    child = object_from_dict({"id": "c", "role": "Sprite", "parent_id": "p"})
    add_object(parent)
    add_object(child)
    ids = [obj.id for obj in iter_dag()]
    assert ids == ["p", "c"]
