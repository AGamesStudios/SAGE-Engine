def test_two_way_binding():
    from sage_engine.gui import widgets

    class Obj:
        volume = 0.5

    o = Obj()
    s = widgets.Slider()
    s.bind('volume', o)
    s.value = 0.8
    s.on_change.emit()
    assert o.volume == 0.8
    o.volume = 0.3
    s.pull_bind()
    assert s.value == 0.3
