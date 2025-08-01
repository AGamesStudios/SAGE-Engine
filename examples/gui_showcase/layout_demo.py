from sage_engine.gui import widgets, layout, manager

c = widgets.Button()
c.add_child(widgets.Button(width=50, height=20, text="A"))
c.add_child(widgets.Button(width=50, height=20, text="B"))
layout.HBoxLayout(spacing=4, auto_size=True).apply(c)
manager.root.add_child(c)
