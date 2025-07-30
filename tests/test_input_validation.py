import asyncio

from sage_engine.input import Input
from sage_engine.flow.runtime import FlowRuntime


def test_unbound_action_returns_false():
    Input.unmap_action("move")
    Input.poll()
    assert not Input.is_pressed("move")
    assert not Input.is_down("move")
    assert not Input.is_up("move")


def test_flowscript_invalid_key_does_not_move():
    rt = FlowRuntime()
    script = """
переменная pos = 0
при обновление сделай
    global pos
    если нажата_клавиша("LFT") тогда
        прибавить pos на 1
конец
"""
    ctx: dict[str, object] = {}
    asyncio.run(rt.run(script, ctx, dialect="ru"))
    Input._handle_key("LEFT", True)
    asyncio.run(ctx["on_update"]())
    assert ctx["on_update"].__globals__["pos"] == 0


def test_flowscript_valid_key_moves():
    rt = FlowRuntime()
    script = """
переменная pos = 0
при обновление сделай
    global pos
    если нажата_клавиша("LEFT") тогда
        прибавить pos на 1
конец
"""
    ctx: dict[str, object] = {}
    asyncio.run(rt.run(script, ctx, dialect="ru"))
    Input._handle_key("LEFT", True)
    asyncio.run(ctx["on_update"]())
    assert ctx["on_update"].__globals__["pos"] == 1
