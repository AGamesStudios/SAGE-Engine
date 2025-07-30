# 📘 Быстрый старт

1. Установите зависимости:
   ```bash
   pip install -e .
   ```
2. Запустите тесты и убедитесь, что всё работает:
   ```bash
   pytest -q
   ```
3. Подготовьте конфигурацию `game.sagecfg`:
   ```cfg
   название = "Demo"
   ширина = 640
   высота = 360
   скрипт = "logic.flow"
   ```

   Минимальный скрипт `logic.flow` может выглядеть так:

   ```flow
   при обновление сделай
       если нажата_клавиша "ESCAPE" тогда
           завершить игру()
   конец
   ```

4. Создайте сцену и загрузите скрипт:
   ```python
   from sage_engine import window, gfx, render
   from sage_engine.runtime.fsync import FrameSync
   from sage_engine.flow.runtime import run_flow_script
   from sage_engine.resource.loader import load_cfg

   cfg = load_cfg("game.sagecfg")
   window.init(cfg.get("название"), cfg.get("ширина"), cfg.get("высота"))
   render.init(window.get_window_handle())
   gfx.init(cfg.get("ширина"), cfg.get("высота"))
   fs = FrameSync(target_fps=60)
   while not window.should_close():
       window.poll_events()
       fs.start_frame()
       gfx.begin_frame()
       run_flow_script(cfg.get("скрипт"))
       gfx.flush_frame(window.get_window_handle())
   render.shutdown()
   window.shutdown()
   ```

5. Изучите примеры в каталоге `docs/examples/`.

Теперь вы готовы экспериментировать с ролями и фазами.
