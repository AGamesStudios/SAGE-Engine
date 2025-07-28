# 📘 Logger

Модуль `logger` предоставляет единый интерфейс логирования для всех подсистем.

```python
from sage_engine.logger import logger
logger.info("Window created", tag="window")
```

Уровень логирования задаётся переменной окружения `SAGE_LOGLEVEL`.

### 🔹 Crash Intelligence
Модуль автоматически перехватывает необработанные исключения и сигналы через `sys.excepthook`, `threading.excepthook`, обработчик событий `asyncio` и `signal`. Информация о сбое сохраняется в файл `logs/crash_*.json` и выводится в консоль.

```python
from sage_engine.logger import log_crash

try:
    1 / 0
except ZeroDivisionError as e:
    log_crash(type(e), e, e.__traceback__, code="SAGE_ERR_SCRIPT_DIV0")
```
