# 📘 SAGE Format System

`format` implements compact TLV based files used by the engine. Supported types
are integers (8/16/32 bit), floats, strings, arrays, maps and flags. Files start
with the header `SAGE` and a version byte.

Example usage:

```python
from sage_engine.format import SAGECompiler, SAGEDecompiler

compiler = SAGECompiler()
compiler.compile(Path('object.yaml'), Path('object.sageobj'))

obj = SAGEDecompiler().decompile(Path('object.sageobj'))
```

Schemas can be registered using `SAGESchemaSystem` to validate data when
compiling.
