"""Python FlowScript stub."""

async def run(script: str, context: dict) -> None:
    exec(script, context)
