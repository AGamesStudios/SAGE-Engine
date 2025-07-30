# 📘 FlowScript

💡 Модуль `flow` позволяет выполнять простые скрипты для управления сценой.

## 🔹 `run`

```python
async def run(script: str, context: dict) -> None
```

Функция исполняет код в переданном контексте. Она асинхронная,
поэтому её нужно вызывать через `await` или `asyncio.run`:

```python
import asyncio
from sage_engine.flow.python import run as run_flow

asyncio.run(run_flow("ctx['done'] = True", {'ctx': {}}))
```

⚠️ Если вызвать `run` без `await`, интерпретатор выдаст предупреждение
`RuntimeWarning: coroutine was never awaited`.


## Architecture

FlowScript consists of a tiny interpreter located in `sage_engine.flow`. The runtime can execute Python style code or parse the experimental FlowScript syntax via the parsers and compilers found under `flow/`.
The entry point is `FlowRuntime.run(script, context, dialect='python')`.
