def test_custom_widget_registration():
    from sage_engine.gui import base, registry

    class MyBar(base.Widget):
        value: float = 0.0
        def on_draw(self, gfx):
            pass

    registry.register_widget('MyBar', MyBar)
    inst = registry.create('MyBar')
    assert isinstance(inst, MyBar)
