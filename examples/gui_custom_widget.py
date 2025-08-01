from sage_engine.gui import base, manager, registry
manager.debug = True

class ProgressBar(base.Widget):
    value: float = 0.0
    def on_draw(self, gfx):
        pct = self.value
        gfx.draw_rect(self.x, self.y, int(self.width*pct), self.height, (0,255,0,255))

registry.register_widget('ProgressBar', ProgressBar)
bar = ProgressBar(width=100, height=10)
bar.value = 0.4
manager.root.add_child(bar)
bar.draw()
