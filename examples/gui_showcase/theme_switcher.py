from sage_engine.gui import widgets, style, manager

btn = widgets.Button(text="Switch")
manager.root.add_child(btn)
style.load_theme("retro", "sage_engine/gui/theme/retro.sagegui")
style.apply_theme(btn.style, "retro")
