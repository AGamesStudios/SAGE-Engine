# 📘 Модуль `effects`

Модуль `effects` содержит регистр процедур обработки кадров. Каждый эффект
получает буфер `bytearray` и размеры кадра. Эффекты независимы от `graphic`
и могут использоваться в любых проектах.

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

Эффекты по умолчанию: blur, glow, ripple, color_matrix.
