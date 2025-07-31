# 📘 Быстрый старт

1. Подготовьте виртуальное окружение и установите зависимости:
   ```bash
   python -m venv .venv && source .venv/bin/activate
   pip install -e . -r requirements-dev.txt
   ```
2. Запустите тесты и убедитесь, что всё работает:
   ```bash
   python tools/run_tests.py
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

5. Ознакомьтесь с примером `render_test` в `docs/examples/render_test.md`.

### Использование Rust-бэкенда

При необходимости можно собрать экспериментальную библиотеку рендеринга на Rust:

```bash
make build-rust
export SAGE_RENDER_BACKEND=rust
```

Если библиотека недоступна, будет использован стандартный Python-бэкенд.

### ⚙️ Автозагрузка и сборка рендер-бэкенда

При запуске движка выполняется поиск нативной библиотеки `libsagegfx.dll` / `.so` / `.dylib` в каталоге `sage_engine/native/`.

- Не требует ручной установки переменных
- При отсутствии библиотека собирается через `make build-rust` автоматически
- После успешной сборки файл копируется в `sage_engine/native/`
- Если сборка не удалась, будет использован Software Renderer

Вы можете вручную собрать библиотеку командой `make build-rust`, она сама положит файл в нужное место.

Теперь вы готовы экспериментировать с ролями и фазами.

### FPS режимы

Модуль ``sage_engine.runtime.fps`` предоставляет глобальный переключатель
``fps_mode``. Значение ``FPSMode.CLASSIC`` показывает обычный FPS ``1/dt``.
Режим ``FPSMode.GAMEFRAME`` возвращает частоту на основе индекса GameFrame,
используя метрики SDK. Переключайте режим для сравнения отображаемых значений.
