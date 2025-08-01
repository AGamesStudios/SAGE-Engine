def test_flowscript_event_emit():
    from sage_engine.gui import widgets
    from sage_engine import events

    btn = widgets.Button()
    captured = []
    events.on('gui:action', lambda data: captured.append(data))
    btn.on_click.connect("run_script('test')")
    btn.on_click.emit()
    events.update()
    assert captured and captured[0]['script'] == "run_script('test')"
