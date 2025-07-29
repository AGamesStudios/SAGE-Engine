# 📘 SAGE Crash

Функция `log_crash` перехватывает исключения и записывает стек вызовов
в файл `logs/crash_log_*.json`.
При инициализации хук устанавливается автоматически и также
обрабатывает сигналы `SIGINT` и `SIGTERM`.

Пример:
```python
from sage_engine.logger import log_crash
try:
    1/0
except ZeroDivisionError as e:
    log_crash(type(e), e, e.__traceback__, code="SAGE_ERR_SCRIPT_DIV0")
```
