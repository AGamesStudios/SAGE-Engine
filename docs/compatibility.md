# 📘 Совместимость

SAGE Engine не использует номера версий ни в каких данных. Все изменения
обрабатываются функцией `compat.migrate`, которая преобразует старые структуры
к актуальным. Миграции описываются декларативно при помощи правил
`migrate_field`, `remove_field` и `set_default` и применяются автоматически при
загрузке файлов.

## 🔹 Примеры правил

```python
from sage_engine import compat

compat.MIGRATIONS = {
    "blueprint_object": [
        compat.migrate_field("objectName", "name"),
        compat.remove_field("deprecatedField"),
        compat.set_default("enabled", True),
    ],
}
```

## 🔹 `@stable_api`

`@stable_api` помечает функции и классы, интерфейс которых нельзя ломать.

### 📦 Правила контракта API

- Нельзя удалять параметры или менять их смысл.
- Новые параметры должны иметь значения по умолчанию.
- Старые вызовы поддерживаются через адаптеры.
