# SAGE Terminal

`SAGE Terminal` is a lightweight GUI console built with CustomTkinter. It helps manage SAGE projects and resources using simple text commands.

Run the terminal:

```bash
python tools/sage_terminal/terminal.py
```

Available commands include:

- `help` — show all commands
- `newproject NAME` — create a project skeleton
- `checkproject PATH` — validate a project's layout
- `cleanproject PATH` — remove `__pycache__` and `.pyc` files
- `newscene NAME` — create an empty scene file
- `newobject NAME` — create an empty object file
- `newscript [scene|object] NAME` — create a script stub
- `showlayout` — list the required project layout
- `exit` / `quit` — close the terminal

The interface stores command history and supports basic text selection and scrolling. Themes can be customised by editing `themes.py`.
