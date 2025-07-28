# 📘 Blueprint

💡 **Blueprint** описывает сцену или набор объектов в формате JSON. В секции `meta` указывается версия, источник и теги.

Пример файла:

```json
{
  "meta": {"version": "1.0", "origin": "demo", "tags": ["level1"]},
  "objects": [
    {"role": "sprite", "transform": {"x": 0, "y": 0}}
  ]
}
```

Загрузить blueprint можно так:

```python
from sage_engine.blueprint import load
bp = load(Path('scenes/level1.json'))
```
