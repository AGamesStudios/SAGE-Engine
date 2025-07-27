# 📘 Сцена

🧠 **Scene** хранит все объекты и их роли в виде Struct‑of‑Arrays. Изменения допускаются только через `SceneEdit` — атомарный набор операций `create`, `destroy`, `mutate`.

## 🔹 SceneEdit

`SceneEdit` аккумулирует операции и применяется сценой между кадрами. Это гарантирует отсутствие гонок и корректное обновление индексов. Использовать его можно так:
```python
from sage_engine import world

edit = world.scene.begin_edit()
obj = edit.create(role="sprite", x=1)
edit.destroy(other_obj)
world.scene.apply(edit)
world.scene.commit()
```

🔧 Объекты выбираются по роли через `world.scene.each_role("sprite")` или через представление `world.scene.view.with_transform()`.
