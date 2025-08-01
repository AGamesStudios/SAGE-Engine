from sage_engine.gui import widgets, manager

class Config:
    volume = 0.5

cfg = Config()
slider = widgets.Slider(width=100)
slider.bind('value', cfg)
manager.root.add_child(slider)

# simulate change
slider.value = 0.75
slider.on_change.emit()
print('cfg.volume=', cfg.volume)
