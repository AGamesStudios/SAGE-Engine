# 📘 Быстрый старт

## Запуск

После установки зависимостей движок стартует командой:

```bash
python main.py
```

В консоли появятся строки:

```text
[INFO] [core] Boot complete
[INFO] [core] Entering update loop...
```

Нажмите `Ctrl+C`, чтобы корректно завершить работу.

1. Установите библиотеку и создайте проект:
   ```bash
   pip install sage-engine
   sage new MyGame
   # или инициализация в существующей папке
   sage init-project
   ```
2. Подготовьте виртуальное окружение и установите зависимости для разработки:
   ```bash
   python -m venv .venv && source .venv/bin/activate
   pip install -e . -r requirements-dev.txt
   ```
3. Запустите тесты и убедитесь, что всё работает:
   ```bash
   python test_runner.py
   ```
4. Подготовьте конфигурацию `game.sagecfg`:
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


### 🔧 Проверка окружения

Для диагностики движка используйте утилиту:

```bash
python tools/dev_check.py
```

Она выполнит базовый тест рендера и проверит доступность модулей.

### FPS режимы

Модуль ``sage_engine.runtime.fps`` предоставляет глобальный переключатель
``fps_mode``. Значение ``FPSMode.CLASSIC`` показывает обычный FPS ``1/dt``.
Режим ``FPSMode.GAMEFRAME`` возвращает частоту на основе индекса GameFrame,
используя метрики SDK. Переключайте режим для сравнения отображаемых значений.
