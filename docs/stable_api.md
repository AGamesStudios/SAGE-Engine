# 📘 Стабильные интерфейсы

SAGE Engine не использует номера версий в данных и полагается на миграции
через модуль ``compat``. Для описания миграций доступны правила
``migrate_field``, ``remove_field`` и ``set_default``. Эти функции, а также
``compat.migrate`` отмечены как стабильные API и не должны ломаться.

В SAGE Engine помеченные `@stable_api` функции и классы не должны изменяться без
обратной совместимости.

- `core.core_boot`
- `core.core_tick`
- `core.core_shutdown`
- `world.Scene`
- `world.SceneEdit`
- `roles.register_role`
- `compat.register`
- `compat.migrate`
- `compat.migrate_field`
- `compat.remove_field`
- `compat.set_default`
- `api.Blueprint`
- `api.Scene`
- `api.RoleSchema`
- `api.EventSystem`
- `window.init`
- `window.poll_events`
- `window.get_window_handle`
- `window.shutdown`
