# 📘 Graphics

По умолчанию движок загружает шрифт `sage_engine/resources/fonts/default.ttf`. Это переименованный **Public Sans** под лицензией Apache 2.0. При необходимости можно заменить файл либо явно вызвать `gfx.load_font()` с нужным путём.

Спрайты и текстуры хранятся только в формате `.sageimg`. Для их загрузки используйте классы `Texture` и `Sprite`:

```python
from sage_engine.texture import TextureCache
from sage_engine.sprite import sprite

tex = TextureCache.load("resources/textures/ui.sageimg")
s = sprite.Sprite(tex, (0, 0, 32, 32))
s.draw(10, 10)
```

Текстуры могут быть объединены в атласы. Для доступа к подспрайтам используйте
`TextureAtlas` и функцию `draw.sprite_from_atlas`.

Для профилирования рендера доступны счётчики `render.stats`. Они
обнуляются при каждом вызове `gfx.begin_frame()` через функцию
`render.stats.reset_frame()`.
Текущие значения можно вывести командой `sage debug stats`. Помимо
`sprites_drawn` и `textures_loaded`, коллекция содержит фазовые тайминги
`ms_update`, `ms_draw`, `ms_flush` и усреднённый FPS.  Система
`FrameSync` позволяет ограничить частоту кадров.  Режимы
"capped", "adaptive" и "unlimited" выбираются через `settings` или
`engine.sagecfg` (`render.frame_sync`).
Конверсию координат между мировым и экранным пространством выполняет
модуль `transform`.
