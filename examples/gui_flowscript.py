from sage_engine.gui import widgets, manager
manager.debug = True

btn = widgets.Button(text='Save')
btn.on_click.connect("run_script('save_game')")
manager.root.add_child(btn)
btn.on_click.emit()
