# 📘 Graphics

По умолчанию движок загружает шрифт `sage_engine/resources/fonts/default.ttf`. Этот файл основан на **IBM Plex Sans** (лицензия Apache 2.0) и поставляется в репозитории. При необходимости можно заменить `default.ttf` на любой другой TTF либо явно вызвать `gfx.load_font()` с нужным путём.

Спрайты и текстуры хранятся только в формате `.sageimg`. Для их загрузки используйте классы `Texture` и `Sprite`:

```python
from sage_engine.texture import TextureCache
from sage_engine.sprite import sprite

tex = TextureCache.load("resources/textures/ui.sageimg")
s = sprite.Sprite(tex, (0, 0, 32, 32))
s.draw(10, 10)
```
