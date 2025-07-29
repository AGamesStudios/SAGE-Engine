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
