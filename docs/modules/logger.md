# 📘 Logger

Модуль `logger` предоставляет единый интерфейс логирования для всех подсистем.

```python
from sage_engine.logger import logger
logger.info("Window created", tag="window")
```

Уровень логирования задаётся переменной окружения `SAGE_LOGLEVEL`.
