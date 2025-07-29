# 📘 Blueprint

💡 **Blueprint** описывает сцену или набор объектов в бинарном формате `.sagebp`.
Схемы могут храниться в YAML для удобства, но в проект попадает уже
скомпилированный файл. В секции `meta` указывается версия, источник и теги.

```yaml
meta:
  version: "1.0"
  origin: demo
  tags: [level1]
objects:
  - role: sprite
    transform:
      x: 0
      y: 0
```

Blueprint поддерживает переменные, которые можно использовать в FlowScript:

```yaml
variables:
  speed: 10
```

Редактор позволяет изменять объекты в реальном времени и сохранять изменения обратно в файл. Переменные могут быть связаны со свойствами ролей.

```python
from sage_engine.blueprint import load
bp = load(Path('scenes/level1.sagebp'))
```
\nObjects module uses small blueprints describing roles and parameters.
