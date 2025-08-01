from sage_engine.gui import widgets, animation, layout, manager

panel = widgets.Button()
user = widgets.TextInput(width=80)
passwd = widgets.TextInput(width=80)
panel.add_child(user)
panel.add_child(passwd)
layout.VBoxLayout(spacing=2, auto_size=True).apply(panel)
animation.animate(panel, "opacity", 0.0, 1.0, 60)
manager.root.add_child(panel)
