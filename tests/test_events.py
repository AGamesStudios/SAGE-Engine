from sage_engine import events


def test_event_emit():
    got = []

    def handler(x):
        got.append(x)

    events.dispatcher.on(1, handler)
    events.dispatcher.emit(1, 42)
    events.dispatcher.flush()
    assert got == [42]
