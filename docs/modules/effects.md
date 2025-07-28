# 📘 Модуль `effects`

Модуль `effects` работает с кадрами формата BGRA8 premultiplied. Все функции
получают объект `Frame`, обёртку над `bytearray`. Эффекты независимы от
`graphic` и могут использоваться в любых проектах.

```python
from sage_engine import effects

# перечисление доступных эффектов
print(effects.list_effects())

# применение glow к буферу
effects.apply("glow", buffer, width, height)
```

Вы можете регистрировать собственные эффекты:

```python
from sage_engine.effects import register

def invert(buf, w, h):
    for i in range(0, len(buf), 4):
        buf[i] = 255 - buf[i]       # B
        buf[i+1] = 255 - buf[i+1]   # G
        buf[i+2] = 255 - buf[i+2]   # R

register("invert", invert)
```

Дополнительно можно собрать цепочку эффектов. Буфер переключается между двумя
поверхностями, поэтому исходный кадр сохраняется:

```python
effects.apply_pipeline([
    ("blur", {"radius": 2}),
    ("glow", {"color": (255,255,0)}),
], buffer, width, height)

effects.save_preset("soft.json", [
    ("blur", {"radius": 2})
])
spec = effects.load_preset("soft.json")
effects.apply_pipeline(spec, buffer, width, height)
```

Можно ограничить область действия `set_scissor` и задать альфа‑маску через
`set_mask(frame)`. Бэкенд пока только `cpu`, но интерфейс позволяет
подключить GPU.

Эффекты по умолчанию: blur, glow, ripple, wave, pixelate, glitch,
fade, noise, outline и color_matrix.
