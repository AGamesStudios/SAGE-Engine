# SAGE Engine Agents

Агент — изолированный модуль движка, который подключается к циклу
через ``core.register()`` и выполняет код в фазах ``boot → update → draw →
flush → shutdown``. Агенты используются для инкапсуляции логики без
жёстких связей между подсистемами.

## Встроенные агенты

| Агент            | Назначение                              |
|-----------------|------------------------------------------|
| InputAgent      | Обработка пользовательского ввода        |
| RenderAgent     | Формирование финального кадра            |
| ResourceAgent   | Загрузка и кеширование ресурсов          |
| AIProcessingAgent | Логика NPC и AI                         |
| PhysicsAgent    | Обработка столкновений и движения        |

## Шаблон агента

```python
class MyAgent:
    def __init__(self):
        self.name = "MyAgent"

    def boot(self, cfg):
        pass

    def update(self, dt):
        pass

    def draw(self):
        pass

    def flush(self):
        pass

    def shutdown(self):
        pass
```

Агент регистрируется в ``boot`` через ``core.register(MyAgent())``. Все
зависимости обращаются через ``core.get()`` или события ``events``; прямые
импорты запрещены. Агенты не создают бинарных файлов и должны иметь
модульные тесты.
