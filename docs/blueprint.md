# 📘 Blueprint

💡 **Blueprint** описывает сцену или набор объектов в бинарном формате `.sagebp`.
В проект включаются только бинарные файлы, компилятор превращает YAML во время
подготовки ресурсов. В секции `meta` хранятся источник и теги.

```yaml
meta:
  # migrate_blueprint() добавляет недостающие поля
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
from sage_engine.format.loader import load_sage_file
bp = load_sage_file('scenes/level1.sagebp')
```
\nObjects module uses small blueprints describing roles and parameters.
