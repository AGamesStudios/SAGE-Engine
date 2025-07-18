# SAGE UI

UI objects are represented with the `UI` role. The UI subsystem can render
labels, buttons and panels defined in `.sage_object` files. Styles are chosen via
the `style` parameter which references entries in a theme file.

Use `render_ui(objects)` to collect draw calls for testing or headless
operation.
