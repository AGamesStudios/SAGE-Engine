# 📘 Конфигурация `engine.sagecfg`

Файл `engine.sagecfg` описывает параметры запуска движка. Первая строка
должна содержать заголовок `[SAGECFG]`. Поддерживаемые ключи:

| Ключ        | Значение                        |
|-------------|---------------------------------|
| `window_title` | Название окна                   |
| `screen_width` | Ширина окна в пикселях          |
| `screen_height`| Высота окна в пикселях          |
| `render_backend`| Рендер-бэкенд (`software`, `vulkan`) |
| `boot_modules` | Список подключаемых модулей     |

Неизвестные поля игнорируются с предупреждением в логе. См. также `docs/engine_config.md`. Пример:

```cfg
[SAGECFG]
window_title = "Demo Game"
screen_width = 320
screen_height = 240
boot_modules = ["input", "render"]
width = 640
height = 360
fullscreen = false
language = "ru"
```

Создать шаблон конфигурации и необходимые каталоги можно командой:

```bash
sage init-project
```
