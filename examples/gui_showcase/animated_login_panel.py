from sage_engine.gui import widgets, animation, layout, manager

panel = widgets.Panel()
panel.opacity = 0.0
title = widgets.Label(text="Login")
user = widgets.TextInput(width=80)
passwd = widgets.TextInput(width=80)
submit = widgets.Button(text="Send")


def on_click():
    submit.text = "Done"


submit.on_click = on_click
panel.add_child(title)
panel.add_child(user)
panel.add_child(passwd)
panel.add_child(submit)
layout.VBoxLayout(spacing=2, auto_size=True).apply(panel)
animation.animate(panel, "opacity", 0.0, 1.0, 60)
manager.root.add_child(panel)
