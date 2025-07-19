# SAGE Window

The window subsystem creates the main application window and emits events when
it is created, resized or closed. Configuration is read from
`sage/config/window.yaml`.

```yaml
window:
  width: 1280
  height: 720
  title: "SAGE Engine Alpha 0.3"
  vsync: true
  resizable: true
  fullscreen: false
```

Use `on('window_resize', handler)` to react to resize events.
