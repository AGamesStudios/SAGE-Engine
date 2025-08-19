# Плагин API

Каждый плагин — папка в `plugins/<id>/` с файлами:
- `plugin.json` — манифест
- `plugin.py` — код регистрации

## `plugin.json`

Минимальный пример:
```json
{
  "name": "SAGE2D Plugin",
  "id": "sage2d",
  "version": "1.0.0",
  "module": "plugin",
  "profiles": ["2d"],
  "tags": ["2d", "render:2d", "render:opengl"],
  "requires": {
    "core": ">=1.2.0",
    "features": { "render": ">=1.0" }
  }
}
```

### Поля
- **id** — уникальный идентификатор (используется в `--enable/--disable`).
- **profiles** / **tags** — для фильтрации набора плагинов.
- **module** — модуль-код (обычно `plugin` → файл `plugin.py`).
- **requires** — необязательные требования.

## `plugin.py`

Плагин должен экспортировать функцию:

```python
def register(api):
    # регистрируем сервисы / системы / команды / события / хуки
```

Доступные методы `api`:

### Сервисы
```python
api.provide_service('renderer_factory', lambda: MyRenderer(api))
api.provide_service('world_factory', lambda: MyWorld())
api.provide_service('camera_factory', lambda: MyCamera())
# ...любой ваш сервис по имени
```

### Системы (per-frame)
```python
def tick(engine=None, dt: float = 0.0):
    # ваш апдейт (input, скрипты, UI и т.д.)
    pass
api.register_system('my.tick', tick, order=50)  # чем меньше order, тем раньше
```

### Команды
```python
def cmd_toggle_wireframe(engine=None, **_):
    engine.renderer.set_wireframe(not getattr(engine.renderer, 'wire', False))
api.register_command('toggle_wireframe', cmd_toggle_wireframe)
# вызов: api.invoke_command('toggle_wireframe', engine=engine)
```

### События
```python
def on_start(engine=None, **_):
    print('Engine started')
api.events.on('engine.start', on_start)
# emit со стороны ядра или систем: api.events.emit('engine.start', engine=eng)
```

### Рендер‑хуки
```python
def pre_hook(renderer=None, **kw):
    # Нарисовать оси/грид перед основным рендером
    pass
api.register_renderer_hook('pre', pre_hook)
```

## Практика: загрузка локальных модулей плагина
Чтобы плагин работал без пакетных импортов, используйте загрузчик по пути:

```python
def _load_local(module_filename):
    import importlib.util, os
    here = os.path.dirname(__file__)
    path = os.path.join(here, module_filename)
    name = f"sage_plugin_{os.path.basename(here)}_{module_filename.replace('.py','')}"
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod
```

## Соглашения по именам сервисов
- `renderer_factory` — фабрика рендера (2D/3D/OpenGL/Vulkan/Null).
- `world_factory` — фабрика мира/сцены.
- `camera_factory` — фабрика камеры.

Можно добавлять свои сервисы: `ui_factory`, `audio_mixer`, `physics_system` и т.д.
