# 📘 Модуль `resource`

`resource` управляет загрузкой и упаковкой всех игровых данных. Система
поддерживает кеширование, упаковку в `.sagepack` и ленивую загрузку.

```python
from sage_engine import resource

resource.preload(["sprites/"])
img = resource.load("sprites/player.sageimg")
```

`packer.pack()` собирает каталог в единый файл без дубликатов и проверяет
лимит размера.

CLI утилита `sage-pack` автоматизирует сборку:

```bash
sage-pack build assets/ build/resources.sagepack --report pack.json
```

## Ошибки и диагностика

При загрузке конфигураций `load_engine_cfg` отфильтровывает неизвестные
поля и выводит предупреждение в лог. Попытка загрузить несуществующий
ресурс из пакета вызывает `KeyError`. Функция `packer.pack` генерирует
`ValueError`, если размер архива превышает `limit_size_mb`.

## Рекомендуемые приёмы

- Храните ассеты в каталоге, собирайте их в `.sagepack` и подключайте его
  через `resource.manager.configure(path)`.
- Используйте `resource.preload([...])` для предварительной загрузки
  необходимых файлов.
- Вызывайте `cache.clear()` при смене уровня или для освобождения памяти.
