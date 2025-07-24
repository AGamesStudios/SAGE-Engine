import asyncio
import pytest
from sage import (
    on,
    once,
    off,
    emit,
    emit_async,
    add_filter,
    cleanup_events,
    get_event_handlers,
)
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


def test_emit_various_handlers():
    events = []

    def no_arg():
        events.append("no")

    def with_data(data):
        events.append(data)

    on("mix", no_arg)
    on("mix", with_data)
    emit("mix", "ok")
    assert events == ["no", "ok"]


def test_emit_none_and_error_handler():
    events = []

    def capture(data):
        events.append(data)

    def bad_handler(_):
        raise RuntimeError("boom")

    on("none", capture)
    on("none", bad_handler)
    emit("none")  # should not raise
    assert events == [None]


def test_invalid_handler_registration():
    with pytest.raises(ValueError):
        on("bad", lambda a, b: None)


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
    table = get_event_handlers()
    assert "test" not in table


def test_async_emit_and_filters():
    events = []

    def add_prefix(data):
        return f"x{data}"

    async def handler(value):
        events.append(value)

    async def main():
        add_filter("aevt", add_prefix)
        on("aevt", handler)
        await emit_async("aevt", "1")

    asyncio.run(main())
    assert events == ["x1"]
