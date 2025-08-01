# 📘 Модуль `texture`

`texture` содержит класс `Texture` и хранилище `TextureCache`. Поддерживаются только файлы `.sageimg`.

```python
from sage_engine.texture import TextureCache

tex = TextureCache.load("sprites/button.sageimg")
w, h = tex.get_size()
```

`TextureCache.load()` возвращает уже загруженный экземпляр, предотвращая повторное чтение с диска. Методы `unload()` и `clear()` освобождают память.
