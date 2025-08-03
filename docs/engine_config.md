# Engine Configuration

`engine.sagecfg` controls core startup options. The first non-empty line must be `[SAGECFG]`.

## Supported keys

| Key | Description |
|-----|-------------|
| `window_title` | Title used for the main window |
| `screen_width` | Initial width of the window in pixels |
| `screen_height` | Initial height of the window in pixels |
| `render_backend` | Rendering backend (`software` or `vulkan`) |
| `boot_modules` | List of module names to import during boot |

Additional nested mappings `features` and `settings` are preserved when present.

## Deprecated keys

| Key | Replacement |
|-----|-------------|
| `modules` | `boot_modules` |
| `width` | `screen_width` |
| `height` | `screen_height` |

Unknown keys are ignored with a warning:

```
[WARN] [config] Unknown key in engine.sagecfg: foo | Tip: See docs/engine_config.md or run 'sage check-config'
```

To validate a configuration file use the CLI:

```
$ sage check-config --file engine.sagecfg
```
