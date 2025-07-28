"""Python FlowScript stub."""

import inspect


async def run(script: str, context: dict) -> None:
    if not inspect.iscoroutinefunction(run):
        raise RuntimeError("run_flow must be awaited or run via asyncio")
    exec(script, context)
