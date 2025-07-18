from sage import on, once, off, emit, cleanup_events
from sage_object import object_from_dict
from sage_engine.object import add_object, remove_object, reset


def test_on_emit_and_off_once():
    out = []
    def handler(v):
        out.append(v)
    on("evt", handler)
    emit("evt", 1)
    off("evt", handler)
    emit("evt", 2)
    once("evt", lambda v: out.append(v))
    emit("evt", 3)
    emit("evt", 4)
    assert out == [1, 3]


def test_object_event_registration_and_cleanup():
    reset()
    out = []
    obj = object_from_dict({"id": "o", "role": "UI"})
    obj.params["on_test"] = lambda v: out.append(obj.id)
    add_object(obj)
    emit("test", None)
    assert out == ["o"]
    remove_object("o")
    emit("test", None)
    assert out == ["o"]
    cleanup_events()
    emit("test", None)
    assert out == ["o"]
