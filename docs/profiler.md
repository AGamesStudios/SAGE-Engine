# 📘 Профайлер

`ProfileFrame` измеряет FPS и время выполнения фаз.
Данные можно вывести через `SAGE Debug`.
Пример сбора метрик:
```python
from sage_engine.profiling import profiler

profiler.start("frame")
# game logic
profiler.stop("frame")
```
