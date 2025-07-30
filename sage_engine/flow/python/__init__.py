"""Python FlowScript dialect."""

from ..runtime import FlowRuntime

_runtime = FlowRuntime()

async def run(script: str, context: dict) -> None:
    """Execute a snippet of Python code as FlowScript."""
    await _runtime.run(script, context, dialect="python")
