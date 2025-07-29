# 📘 SAGE Logger

Модуль `sage_engine.logger` обеспечивает уровневое логирование.

Поддерживаемые уровни:
- DEBUG
- INFO
- WARN
- ERROR
- FATAL
- CRITICAL

Логи выводятся цветом в терминал и сохраняются в файлы `logs/YYYY-MM-DD.log`.
Старше семи дней файлы удаляются при инициализации.
Уровень можно задать через переменную окружения `SAGE_LOGLEVEL`.

Пример:
```python
from sage_engine.logger import logger
logger.info("Window created", tag="window")
```
