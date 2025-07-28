import asyncio
import gc
import pytest
from sage_engine.flow.python import run as run_flow


def test_run_flow_asyncio():
    ctx = {"ctx": {}}
    asyncio.run(run_flow("ctx['done'] = True", ctx))
    assert ctx["ctx"]["done"]


def test_run_flow_warning():
    ctx = {"ctx": {}}
    with pytest.warns(RuntimeWarning):
        coro = run_flow("ctx['done'] = True", ctx)
        del coro
        gc.collect()

