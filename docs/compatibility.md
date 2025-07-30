# 📘 Совместимость

## 🔹 Таблица миграций

| Схема | От | До | Изменения |
|-------|----|----|-----------|
| blueprint | 1 | 2 | поле `sprite` перенесено в `renderable.sprite` |
| scene | 1 | 2 | `engine_version` → `schema_version`, `entities` → `objects` |
| flowscript | 1 | 2 | переменная `hp` переименована в `health` |

### Использование CLI

```
$ sage blueprint migrate file.bp.json
$ sage compat check scene.json
```

Обе команды применяют цепочку миграций и сообщают о результатах через лог.

## 🔹 `@stable_api`

`@stable_api` помечает функции и классы, интерфейс которых нельзя ломать.

### 📦 Правила контракта API

- Нельзя удалять параметры или менять их смысл.
- Новые параметры должны иметь значения по умолчанию.
- Старые вызовы поддерживаются через адаптеры.

## Пример миграции

```python
from sage_engine import compat

def migrate_v1_to_v2(data: dict) -> dict:
    if "old_name" in data:
        data["new_name"] = data.pop("old_name")
    return data

compat.register("example", 1, 2, migrate_v1_to_v2)
```
