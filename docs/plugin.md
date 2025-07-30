# 📘 Плагины

Движок поддерживает расширения через простую систему плагинов.
Функция `load_plugins()` загружает все `.py` файлы из каталога и
исполняет их как модули. Плагин может зарегистрировать себя через
`plugins.register()` и реализовать интерфейс `IExtensible`.

```python
from sage_engine.plugins import register
from sage_engine.core.extensible import IExtensible

class MyPlugin:
    def on_attach(self, engine):
        print("Attached!")
    def on_shutdown(self):
        print("Bye")

register(MyPlugin())
```

Для удаления плагина вызывается `plugins.unregister(plugin)`.
