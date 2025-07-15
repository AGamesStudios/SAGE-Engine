# Using SAGE UI with Qt

The GUI layer loads backends via the `sage_gui` entry point. Install the Qt
plugin and run the example:

```bash
pip install .[qt6]
python examples/hello_qt/main.py
```

If no backend is found the engine works in headless mode.
