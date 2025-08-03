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

`TextureCache.load(path, generate_mipmap=True)` создаёт дополнительный уровень
мелких копий для уменьшения артефактов при масштабировании.

Все шрифты также загружаются через `TextureCache`. При первом вызове
`load_font()` внутри `sprite.text` формируется текстура глифов и
увеличивается счётчик `textures_loaded`.

Если файл текстуры или атласа не найден (например, используется
плейсхолдер `sprite.png`), функция выдаёт предупреждение и возвращает
пустой объект вместо исключения. Это позволяет запускать тесты без
реальных ресурсов.
