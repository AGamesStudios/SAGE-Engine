from sage_engine import events, profiling


def test_event_emit():
    got = []

    def handler(x):
        got.append(x)

    events.dispatcher.on(1, handler)
    events.dispatcher.emit(1, 42)
    events.dispatcher.flush()
    assert got == [42]


def test_event_overflow():
    events.reset()
    count = 0

    def handler(x):
        nonlocal count
        count += 1

    events.dispatcher.on(1, handler)
    for i in range(events.dispatcher.MAX_PER_FRAME + 10):
        events.dispatcher.emit(1, i)
    events.dispatcher.flush()
    assert count == events.dispatcher.MAX_PER_FRAME
    assert profiling.profile.events_dropped > 0
