# 📘 Быстрый старт

1. Установите зависимости:
   ```bash
   pip install -e .
   ```
2. Запустите тесты и убедитесь, что всё работает:
   ```bash
   pytest -q
   ```
3. Создайте простую сцену:
   ```python
   from sage_engine import world

   edit = world.scene.begin_edit()
   edit.create(role="sprite", name="Hero", x=0, y=0)
   world.scene.apply(edit)
   world.scene.commit()
   ```
4. Изучите примеры в каталоге `docs/examples/`.

Теперь вы готовы экспериментировать с ролями и фазами.
