# 📘 Модуль `texture`

`texture` содержит класс `Texture` и хранилище `TextureCache`. Поддерживаются только файлы `.sageimg`.

```python
from sage_engine.texture import TextureCache

tex = TextureCache.load("sprites/button.sageimg")
w, h = tex.get_size()
```

`TextureCache.load()` возвращает уже загруженный экземпляр, предотвращая повторное чтение с диска. Методы `unload()` и `clear()` освобождают память.

Для группирования изображений используйте `TextureAtlas`:

```python
from sage_engine.texture import TextureCache

atlas = TextureCache.load_atlas("sprites/ui.sageimg")
button_rect = atlas.get_region("button")
```

Все шрифты также загружаются через `TextureCache`. При первом вызове
`load_font()` внутри `sprite.text` формируется текстура глифов и
увеличивается счётчик `textures_loaded`.
