# 📘 Blueprint Module

Модуль `blueprint` предоставляет функции для загрузки и проверки чертежей объектов.

## API
- `load(path)` — загрузить и мигрировать файл blueprint.
- `Blueprint` и `BlueprintMeta` описывают данные и метаданные.

API публикуется через `core.expose("blueprint", {...})` и может быть получен через `core.get("blueprint")`.
