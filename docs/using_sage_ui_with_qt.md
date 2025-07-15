# Using SAGE UI with Qt

The GUI layer can run with either PyQt6 or PyQt5. Install the desired extra
requirements and run the example:

```bash
pip install .[qt6]  # or .[qt5]
python examples/hello_qt/main.py
```

If neither PyQt6 nor PyQt5 is installed, the engine falls back to headless mode
and the GUI package provides only stub classes.
