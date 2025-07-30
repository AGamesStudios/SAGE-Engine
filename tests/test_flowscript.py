import asyncio
from sage_engine.flow.runtime import FlowRuntime


def test_compile_and_run_ru():
    rt = FlowRuntime()
    ctx = {}
    script = """
переменная здоровье = 2
прибавить здоровье на 3
если здоровье > 4 тогда
    переменная ok = 1
"""
    asyncio.run(rt.run(script, ctx, dialect="ru"))
    assert ctx["здоровье"] == 5
    assert ctx["ok"] == 1


def test_compile_and_run_en():
    rt = FlowRuntime()
    ctx = {}
    script = """
variable hp = 1
add hp by 2
if hp >= 3 then
    call done()
"""
    ctx["done"] = lambda: ctx.setdefault("flag", True)
    asyncio.run(rt.run(script, ctx, dialect="en"))
    assert ctx["hp"] == 3
    assert ctx["flag"] is True


def test_event_function_creation():
    rt = FlowRuntime()
    ctx = {}
    script = """
переменная score = 0
при событие \"hit\" сделай
    прибавить score на 1
"""
    asyncio.run(rt.run(script, ctx, dialect="ru"))
    assert "on_hit" in ctx
