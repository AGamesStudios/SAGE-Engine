# 📘 SAGE Format System

`format` implements compact TLV based files used by the engine. Supported types
are integers (8/16/32 bit), floats, strings, arrays, maps and flags. Files start
with the header `SAGE` and a version byte.

🚫 Использование JSON, YAML, TOML, XML строго запрещено.
✅ Допускается только использование `.sage*` форматов, генерируемых через `SAGECompiler`.

Example usage:

```python
from sage_engine.format import SAGECompiler, SAGEDecompiler

compiler = SAGECompiler()
compiler.compile(Path('object.yaml'), Path('object.sageobj'))

obj = SAGEDecompiler().decompile(Path('object.sageobj'))
```

Низкоуровневые модули доступны для работы с конкретными файлами:
`sageimg`, `sagesfx`, `sagebp`, `sageflow` и `sagepack`.

Schemas can be registered using `SAGESchemaSystem` to validate data when
compiling.

CLI helpers are available:

```bash
$ sage format compile object.yaml object.sageobj
$ sage format decompile object.sageobj
$ sage convert old.json new.sageobj
$ sage validate new.sageobj --schema schema.yaml
$ sage-pack build assets/ game.sagepack
```
