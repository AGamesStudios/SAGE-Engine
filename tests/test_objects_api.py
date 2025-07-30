import pytest
from sage_engine import objects


def test_spawn_and_delete():
    obj = objects.spawn("Sprite", name="foo")
    assert obj.name == "foo"
    assert obj.has_role("Sprite")
    objects.delete(obj)
    assert objects.runtime.store.get_object_by_id(obj.id) is None


def test_clone_object():
    obj = objects.spawn("Sprite")
    clone = objects.clone(obj)
    assert clone.id != obj.id
    assert clone.has_role("Sprite")
