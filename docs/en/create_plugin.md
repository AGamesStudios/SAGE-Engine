# Creating a Plugin

This guide explains the extension lifecycle for SAGE Engine.

## init-plugin

Run `sage init-plugin MyPlugin` to generate a plugin skeleton. The command creates a package with `__init__.py`, `pyproject.toml` and a `tests/` folder so your plugin can be developed separately.

## Structure

A plugin exposes an `init_engine()` function or a `plugin` object derived from `PluginBase`. Adaptors may provide a `register()` function so the engine can call it when loaded. Keep the version in `__version__`.

```
my_plugin/
  pyproject.toml
  my_plugin/__init__.py
  tests/test_basic.py
```

## Version

Plugins use Semantic Versioning. The engine may warn if the major number is incompatible with the running version.

## Tests

Use `pytest` from the plugin directory to verify that `init_engine()` succeeds and that your adaptors register correctly.

