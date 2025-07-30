# 📘 Конфигурация `engine.sagecfg`

Файл `engine.sagecfg` описывает параметры запуска движка. Поддерживаемые ключи:

| Ключ        | Значение                        |
|-------------|---------------------------------|
| `name`      | Название окна                   |
| `script`    | Путь к основному FlowScript     |
| `width`     | Ширина окна в пикселях          |
| `height`    | Высота окна в пикселях          |
| `fullscreen`| `true` или `false`              |
| `language`  | Язык FlowScript (`ru`, `en`)    |

Неизвестные поля игнорируются с предупреждением в логе. Пример:

```cfg
name = "Demo Game"
script = "logic.flow"
width = 640
height = 360
fullscreen = false
language = "ru"
```
