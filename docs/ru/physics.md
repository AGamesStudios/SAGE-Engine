# Физика

Модуль `physics` использует библиотеку [pymunk](https://www.pymunk.org/).
Установите дополнительную зависимость и создайте мир. Без `pymunk` движок выводит предупреждение и физика отключена:

```bash
pip install .[physics]
```

```python
from engine.physics import PhysicsWorld, PhysicsExtension
world = PhysicsWorld()
obj = scene.objects[0]
world.add_box(obj, size=(32, 32))
engine.add_extension(PhysicsExtension(world))
```

`PhysicsExtension` обновляет мир каждый кадр и синхронизирует позиции объектов.
Вызывайте `world.remove(pb)`, когда объект удаляется, чтобы удалить тело из
физического пространства.

Если в сцене есть объекты с ``physics_enabled=True``, движок автоматически
создаёт ``PhysicsWorld`` и подключает ``PhysicsExtension`` – физика работает
без дополнительного кода.
